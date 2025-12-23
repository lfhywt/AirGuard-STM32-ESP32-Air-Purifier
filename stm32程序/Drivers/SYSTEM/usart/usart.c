/**
 ****************************************************************************************************
 * @file        usart.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.1
 * @date        2023-06-05
 * @brief       串口初始化代码(一般是串口1)，支持printf
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/LED/pm25.h"

#if SYS_SUPPORT_OS
#include "os.h"
#endif

/******************************** printf 重定向 ********************************/
#if 1
#if (__ARMCC_VERSION >= 6010050)
__asm(".global __use_no_semihosting\n\t");
__asm(".global __ARM_use_no_argv \n\t");
#else
#pragma import(__use_no_semihosting)
struct __FILE { int handle; };
#endif

int _ttywrch(int ch){ return ch; }
void _sys_exit(int x){ x = x; }
char *_sys_command_string(char *cmd, int len){ return NULL; }

FILE __stdout;

int fputc(int ch, FILE *f)
{
    while ((USART_UX->SR & 0X40) == 0);
    USART_UX->DR = (uint8_t)ch;
    return ch;
}
#endif
/*******************************************************************************/

#if USART_EN_RX

/* ================= USART1 ================= */
uint8_t  g_usart_rx_buf[USART_REC_LEN];
uint16_t g_usart_rx_sta = 0;
uint8_t  g_rx_buffer[RXBUFFERSIZE];
UART_HandleTypeDef g_uart1_handle;

/* ================= USART2 (ESP8266) ================= */
UART_HandleTypeDef g_uart2_esp_handle;
uint8_t  g_esp_rx_buf[USART_REC_LEN];
uint16_t g_esp_rx_sta = 0;
uint8_t  g_esp_rx_buffer[RXBUFFERSIZE];

/* ================= USART3 ================= */
UART_HandleTypeDef g_uart3_handle;
uint8_t  g_usart3_rx_buf[USART_REC_LEN];
uint16_t g_usart3_rx_sta = 0;
uint8_t  g_usart3_rx_buffer[RXBUFFERSIZE];

/******************************** 初始化函数 ********************************/


uint8_t  usart1_rx_byte;
uint8_t  usart1_stream[128];
uint16_t usart1_len = 0;
uint8_t  usart1_msg_ready = 0;

/* USART1 */
void usart_init(uint32_t baudrate)
{
    g_uart1_handle.Instance = USART_UX;
    g_uart1_handle.Init.BaudRate = baudrate;
    g_uart1_handle.Init.WordLength = UART_WORDLENGTH_8B;
    g_uart1_handle.Init.StopBits = UART_STOPBITS_1;
    g_uart1_handle.Init.Parity = UART_PARITY_NONE;
    g_uart1_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    g_uart1_handle.Init.Mode = UART_MODE_TX_RX;

    HAL_UART_Init(&g_uart1_handle);
    //HAL_UART_Receive_IT(&g_uart1_handle, g_rx_buffer, RXBUFFERSIZE); //新增开关需要删除的
    HAL_UART_Receive_IT(&g_uart1_handle, &usart1_rx_byte, 1);

    
}


/* ================= USART2 stream（新增） ================= */
uint8_t  usart2_rx_byte;
uint8_t  usart2_stream[128];
uint16_t usart2_len = 0;
uint8_t  usart2_msg_ready = 0;



/* USART2 */
void esp8266_usart_init(uint32_t baudrate)
{
    g_uart2_esp_handle.Instance = USART_ESP8266;
    g_uart2_esp_handle.Init.BaudRate = baudrate;
    g_uart2_esp_handle.Init.WordLength = UART_WORDLENGTH_8B;
    g_uart2_esp_handle.Init.StopBits = UART_STOPBITS_1;
    g_uart2_esp_handle.Init.Parity = UART_PARITY_NONE;
    g_uart2_esp_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    g_uart2_esp_handle.Init.Mode = UART_MODE_TX_RX;

    HAL_UART_Init(&g_uart2_esp_handle);
    HAL_UART_Receive_IT(&g_uart2_esp_handle,
                    &usart2_rx_byte,
                    1);
}

/* USART3 */
void usart3_init(uint32_t baudrate)
{
    g_uart3_handle.Instance = USART3;
    g_uart3_handle.Init.BaudRate = baudrate;
    g_uart3_handle.Init.WordLength = UART_WORDLENGTH_8B;
    g_uart3_handle.Init.StopBits = UART_STOPBITS_1;
    g_uart3_handle.Init.Parity = UART_PARITY_NONE;
    g_uart3_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    g_uart3_handle.Init.Mode = UART_MODE_TX_RX;

    HAL_UART_Init(&g_uart3_handle);
    HAL_UART_Receive_IT(&g_uart3_handle,
                        g_usart3_rx_buffer,
                        1);
}

/******************************** MSP ********************************/
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio_init_struct;
    __HAL_RCC_AFIO_CLK_ENABLE();

    if (huart->Instance == USART_UX)
    {
        USART_TX_GPIO_CLK_ENABLE();
        USART_RX_GPIO_CLK_ENABLE();
        USART_UX_CLK_ENABLE();

        gpio_init_struct.Pin = USART_TX_GPIO_PIN;
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull = GPIO_PULLUP;
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(USART_TX_GPIO_PORT, &gpio_init_struct);

        gpio_init_struct.Pin = USART_RX_GPIO_PIN;
        gpio_init_struct.Mode = GPIO_MODE_AF_INPUT;
        HAL_GPIO_Init(USART_RX_GPIO_PORT, &gpio_init_struct);

        HAL_NVIC_SetPriority(USART_UX_IRQn, 3, 3);
        HAL_NVIC_EnableIRQ(USART_UX_IRQn);
    }
    else if (huart->Instance == USART_ESP8266)
    {
        USART_TX_GPIO_CLK_ENABLE_ESP8266();
        USART_RX_GPIO_CLK_ENABLE_ESP8266();
        USART_ESP8266_CLK_ENABLE();

        gpio_init_struct.Pin = USART_TX_GPIO_PIN_ESP8266;
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull = GPIO_PULLUP;
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(USART_TX_GPIO_PORT_ESP8266, &gpio_init_struct);

        gpio_init_struct.Pin = USART_RX_GPIO_PIN_ESP8266;
        gpio_init_struct.Mode = GPIO_MODE_INPUT;
        HAL_GPIO_Init(USART_RX_GPIO_PORT_ESP8266, &gpio_init_struct);

        HAL_NVIC_SetPriority(USART_ESP8266_IRQn, 3, 3);
        HAL_NVIC_EnableIRQ(USART_ESP8266_IRQn);
    }
    else if (huart->Instance == USART3)
    {
        USART3_TX_GPIO_CLK_ENABLE();
        USART3_RX_GPIO_CLK_ENABLE();
        USART3_UX_CLK_ENABLE();

        gpio_init_struct.Pin = USART3_TX_GPIO_PIN;
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull = GPIO_PULLUP;
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(USART3_TX_GPIO_PORT, &gpio_init_struct);

        gpio_init_struct.Pin = USART3_RX_GPIO_PIN;
        gpio_init_struct.Mode = GPIO_MODE_INPUT;
        HAL_GPIO_Init(USART3_RX_GPIO_PORT, &gpio_init_struct);

        HAL_NVIC_SetPriority(USART3_UX_IRQn, 3, 3);
        HAL_NVIC_EnableIRQ(USART3_UX_IRQn);
    }
    
}

/******************************** 接收回调 ********************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    #if 0
    /* ================= USART1：PC ================= */
    if (huart->Instance == USART_UX)
    {
        if (!(g_usart_rx_sta & 0x8000))
        {
            if (g_rx_buffer[0] == '\n')
                g_usart_rx_sta |= 0x8000;
            else
                g_usart_rx_buf[g_usart_rx_sta++] = g_rx_buffer[0];
        }

        HAL_UART_Receive_IT(&g_uart1_handle,
                            g_rx_buffer,
                            RXBUFFERSIZE);
    }
#endif
    
if (huart->Instance == USART_UX)
{
    usart1_stream[usart1_len++] = usart1_rx_byte;

    if (usart1_len >= sizeof(usart1_stream))
    {
        usart1_len = 0;
    }

    /* 帧尾：0D 0A */
    if (usart1_len >= 2 &&
        usart1_stream[usart1_len - 2] == 0x0D &&
        usart1_stream[usart1_len - 1] == 0x0A)
    {
        usart1_msg_ready = 1;
    }

    HAL_UART_Receive_IT(&g_uart1_handle, &usart1_rx_byte, 1);
}


    /* ================= USART2：esp32 ================= */
else if (huart->Instance == USART_ESP8266)
{
    usart2_stream[usart2_len++] = usart2_rx_byte;

    if (usart2_len >= sizeof(usart2_stream))
    {
        usart2_len = 0;
    }

    /* 帧尾：0D 0A */
    if (usart2_len >= 2 &&
        usart2_stream[usart2_len - 2] == 0x0D &&
        usart2_stream[usart2_len - 1] == 0x0A)
    {
        usart2_msg_ready = 1;
    }

    HAL_UART_Receive_IT(&g_uart2_esp_handle,
                        &usart2_rx_byte,
                        1);
}

    /* ================= USART3：空气模块 ================= */
    else if (huart->Instance == USART3)
    {
        static uint8_t  rx_cnt = 0;
        static uint32_t last_rx_tick = 0;
        uint32_t now = HAL_GetTick();

        /* ===== 接收超时保护（关键） ===== */
        if (rx_cnt != 0 && (now - last_rx_tick > 50))
        {
            rx_cnt = 0;
        }

        last_rx_tick = now;

        /* ===== 存数据 ===== */
        g_usart3_rx_buf[rx_cnt++] = g_usart3_rx_buffer[0];

        /* ===== 收满一帧 ===== */
        if (rx_cnt >= AIRMOD_FRAME_LEN)
        {
            rx_cnt = 0;

            if (Kanfur_Airmod_Decode(g_usart3_rx_buf))
            {
                airmod_update_flag = 1;

                /* ===== 新增：在线 & 时间戳 ===== */
                airmod_last_tick = HAL_GetTick();
                airmod_online = 1;
            }
        }

        /* ===== 继续接收 1 字节 ===== */
        HAL_UART_Receive_IT(&g_uart3_handle,
                            g_usart3_rx_buffer,
                            1);
    }
}

#if 0
//防掉线
#define AIRMOD_TIMEOUT_MS   3000   // 3 秒无数据判定掉线

void AirMod_Check_And_Recover(void)
{
    if (airmod_online)
    {
        // 正常在线，检查是否超时
        if (HAL_GetTick() - airmod_last_tick > AIRMOD_TIMEOUT_MS)
        {
            airmod_online = 0;   // 判定掉线
        }
    }

    if (!airmod_online)
    {
        // ===== 掉线处理 =====

        // 1. 反初始化串口
        HAL_UART_DeInit(&g_uart3_handle);

        HAL_Delay(50);

        // 2. 重新初始化串口
        usart3_init(9600);

        // 3. 清状态
        airmod_update_flag = 0;
        airmod_last_tick = HAL_GetTick();

        // 4. 等待新数据
        // 注意：真正 online 要等收到数据后再置
    }
}

#endif

/******************************** IRQ ********************************/
void USART_UX_IRQHandler(void)
{
#if SYS_SUPPORT_OS
    OSIntEnter();
#endif
    HAL_UART_IRQHandler(&g_uart1_handle);
#if SYS_SUPPORT_OS
    OSIntExit();
#endif
}

void USART_ESP8266_IRQHandler(void)
{
#if SYS_SUPPORT_OS
    OSIntEnter();
#endif
    HAL_UART_IRQHandler(&g_uart2_esp_handle);
#if SYS_SUPPORT_OS
    OSIntExit();
#endif
}

void USART3_IRQHandler(void)
{
#if SYS_SUPPORT_OS
    OSIntEnter();
#endif
    HAL_UART_IRQHandler(&g_uart3_handle);
#if SYS_SUPPORT_OS
    OSIntExit();
#endif
}

/******************************** 工具函数 ********************************/
void Nextion_Send(UART_HandleTypeDef *huart, const char *cmd)
{
    uint8_t end[3] = {0xFF, 0xFF, 0xFF};
    HAL_UART_Transmit(huart, (uint8_t *)cmd, strlen(cmd), 100);
    HAL_UART_Transmit(huart, end, 3, 100);
}

#endif
