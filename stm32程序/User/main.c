/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.1
 * @date        2025-01-XX
 * @brief       STM32 <-> ESP32 通信 + 空气质量系统（支持 USART1 / USART2 双控风扇）
 ****************************************************************************************************
 */

#include "./stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/LED/pm25.h"
#include "./BSP/LED/fan.h"

#include <string.h>
#include <stdio.h>


/* ================= 状态同步 ================= */
static void Nextion_SendFanStateRaw(uint8_t on)
{
    uint8_t cmd_on[]  = {0x62,0x74,0x30,0x2E,0x76,0x61,0x6C,0x3D,0x31,0xFF,0xFF,0xFF};
    uint8_t cmd_off[] = {0x62,0x74,0x30,0x2E,0x76,0x61,0x6C,0x3D,0x30,0xFF,0xFF,0xFF};

    if (on)
        HAL_UART_Transmit(&g_uart1_handle, cmd_on,  sizeof(cmd_on),  100);
    else
        HAL_UART_Transmit(&g_uart1_handle, cmd_off, sizeof(cmd_off), 100);
}

/* ================= 空气质量评分 ================= */

static uint8_t score_linear(uint16_t value, uint16_t safe, uint8_t weight)
{
    if (value <= safe) return weight;
    if (value >= safe * 2) return 0;
    return (uint8_t)((safe * 2 - value) * weight / safe);
}

static uint8_t Air_Quality_Score(const AIRMOD_T *air)
{
    uint8_t score = 0;

    score += score_linear(air->JQ,   100, 40);
    score += score_linear(air->PM25, 35,  25);
    score += score_linear(air->PM10, 50,  15);
    score += score_linear(air->VOC,  300, 20);

    if (score > 100) score = 100;
    return score;
}

/* ================= ESP32 协议定义 ================= */

#define PKT_HEAD        0xAA
#define PKT_AIR_DATA    0x02

#pragma pack(1)
typedef struct
{
    uint16_t pm25;
    uint16_t pm10;
    uint16_t voc;
    uint16_t jq;
    int16_t  temp_x10;
    uint16_t humi_x10;
    uint8_t  score;
} AIR_DATA_T;
#pragma pack()

/* ================= ESP32 通信 ================= */

/* 风扇状态（33 xx 0D 0A） */
void ESP32_SendFanState(FAN_STATE_E state)
{
    uint8_t txbuf[4] =
    {
        0x33,
        (state == FAN_ON) ? 0x01 : 0x00,
        0x0D,
        0x0A
    };

    HAL_UART_Transmit(&g_uart2_esp_handle, txbuf, 4, 1000);
}

/* 空气数据（二进制 AA 协议） */
void ESP32_SendAirData(const AIRMOD_T *air)
{
    uint8_t txbuf[32];
    uint8_t idx = 0;
    AIR_DATA_T data;

    data.pm25     = air->PM25;
    data.pm10     = air->PM10;
    data.voc      = air->VOC;
    data.jq       = air->JQ;
    data.temp_x10 = (int16_t)(air->TEMP_FLOAT * 10);
    data.humi_x10 = (uint16_t)(air->HUMI_FLOAT * 10);
    data.score    = Air_Quality_Score(air);

    txbuf[idx++] = PKT_HEAD;
    txbuf[idx++] = PKT_AIR_DATA;
    txbuf[idx++] = sizeof(AIR_DATA_T);

    memcpy(&txbuf[idx], &data, sizeof(AIR_DATA_T));
    idx += sizeof(AIR_DATA_T);

    txbuf[idx++] = 0x00;
    txbuf[idx++] = 0x0D;
    txbuf[idx++] = 0x0A;

    HAL_UART_Transmit(&g_uart2_esp_handle, txbuf, idx, 1000);
}

/* ================= 风扇统一控制入口（★新增） ================= */

static FAN_STATE_E g_fan_state_local = FAN_OFF;

static void Fan_SetState(FAN_STATE_E state)
{
    if (state == g_fan_state_local)
        return;

    g_fan_state_local = state;

    if (state == FAN_ON)
    {
        PA12_ON();
         Nextion_SendFanStateRaw(1);
    }
    else
    {
        PA12_OFF();
        Nextion_SendFanStateRaw(0);
    }

    /* 同步回 ESP32 */
    ESP32_SendFanState(state);
}

/* ================= USART2 → ESP32 控制解析（★新增） ================= */

static void USART2_ParseFanCmd(void)
{
    static uint8_t step = 0;
    static uint8_t state = 0;

    if (!usart2_msg_ready)
        return;

    usart2_msg_ready = 0;

    for (uint16_t i = 0; i < usart2_len; i++)
    {
        uint8_t c = usart2_stream[i];

        switch (step)
        {
        case 0:
            if (c == 0x33)
                step = 1;
            break;

        case 1:
            state = c;       // 0x00 / 0x01
            step = 2;
            break;

        case 2:
            if (c == 0x0D)
                step = 3;
            else
                step = 0;
            break;

        case 3:
            if (c == 0x0A)
            {
                Fan_SetState(state ? FAN_ON : FAN_OFF);
            }
            step = 0;
            break;
        }
    }

    usart2_len = 0;
}




/* ================= 主函数 ================= */

int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);
    delay_init(72);

    usart_init(115200);          // USART1 → PC / Nextion
    esp8266_usart_init(115200);  // USART2 → ESP32
    usart3_init(9600);           // USART3 → 空气模块

    PA12_GPIO_Init();
    led_init();

    printf("System Init OK\r\n");

    char nextion_buf[32];

    while (1)
    {
        /* ===== ESP32 → STM32 风扇控制（USART2）===== */
        USART2_ParseFanCmd();

        /* ===== Nextion → STM32 风扇控制（USART1）===== */
        if (usart1_msg_ready)
        {
            usart1_msg_ready = 0;

            for (uint16_t i = 0; i + 3 < usart1_len; i++)
            {
                if (usart1_stream[i]     == 0x33 &&
                    usart1_stream[i + 2] == 0x0D &&
                    usart1_stream[i + 3] == 0x0A)
                {
                    if (usart1_stream[i + 1] == 0x01)
                    {
                        Fan_SetState(FAN_ON);
                    }
                    else if (usart1_stream[i + 1] == 0x00)
                    {
                        Fan_SetState(FAN_OFF);
                    }
                    break;
                }
            }

            usart1_len = 0;
        }

        /* ===== 空气数据更新 ===== */
        if (airmod_update_flag)
        {
            airmod_update_flag = 0;

            uint8_t score = Air_Quality_Score(&airmod);

            /* → ESP32 */
            ESP32_SendAirData(&airmod);
            delay_ms(20);

            /* → Nextion */
            snprintf(nextion_buf, sizeof(nextion_buf),
                     "x0.val=%d", (uint16_t)(airmod.HUMI_FLOAT * 10));
            Nextion_Send(&g_uart1_handle, nextion_buf);
            delay_ms(60);

            snprintf(nextion_buf, sizeof(nextion_buf),
                     "x1.val=%d", (uint16_t)(airmod.TEMP_FLOAT * 10));
            Nextion_Send(&g_uart1_handle, nextion_buf);
            delay_ms(60);

            snprintf(nextion_buf, sizeof(nextion_buf),
                     "n1.val=%d", airmod.JQ);
            Nextion_Send(&g_uart1_handle, nextion_buf);
            delay_ms(60);

            snprintf(nextion_buf, sizeof(nextion_buf),
                     "n3.val=%d", airmod.VOC);
            Nextion_Send(&g_uart1_handle, nextion_buf);
            delay_ms(60);

            snprintf(nextion_buf, sizeof(nextion_buf),
                     "n4.val=%d", airmod.PM25);
            Nextion_Send(&g_uart1_handle, nextion_buf);
            delay_ms(60);

            snprintf(nextion_buf, sizeof(nextion_buf),
                     "n5.val=%d", airmod.PM10);
            Nextion_Send(&g_uart1_handle, nextion_buf);
            delay_ms(60);

            snprintf(nextion_buf, sizeof(nextion_buf),
                     "n6.val=%d", score);
            Nextion_Send(&g_uart1_handle, nextion_buf);
        }

        delay_ms(10);
    }
}
