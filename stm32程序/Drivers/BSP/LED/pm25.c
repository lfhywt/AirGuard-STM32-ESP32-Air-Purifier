#include "./BSP/LED/pm25.h"



AIRMOD_T airmod;

volatile uint32_t airmod_last_tick = 0;   // 最近一次收到数据的时间
volatile uint8_t  airmod_online = 0;      // 传感器在线标志


volatile uint8_t airmod_update_flag = 0;

uint8_t UART_RX_BUFF[AIRMOD_FRAME_LEN];

uint8_t Kanfur_Airmod_Decode(uint8_t *buf)
{
    uint8_t  i;
    uint8_t  sum = 0;

    // 帧头
    if (buf[0] != 0x3C || buf[1] != 0x02)
        return 0;

    // 校验和：前 16 字节
    for (i = 0; i < 16; i++)
    {
        sum += buf[i];
    }

    if (sum != buf[16])
        return 0;

    // 解析数据
    airmod.CO2  = ((uint16_t)buf[2]  << 8) | buf[3];
    airmod.JQ   = ((uint16_t)buf[4]  << 8) | buf[5];
    airmod.VOC  = ((uint16_t)buf[6]  << 8) | buf[7];
    airmod.PM25 = ((uint16_t)buf[8]  << 8) | buf[9];
    airmod.PM10 = ((uint16_t)buf[10] << 8) | buf[11];

    airmod.TEMP_MINUS = (buf[12] & 0x80) ? 1 : 0;
    airmod.TEMP_I     = buf[12] & 0x7F;
    airmod.TEMP_F     = buf[13];

    airmod.TEMP_FLOAT =
        (float)airmod.TEMP_I +
        (float)airmod.TEMP_F * 0.1f;

    if (airmod.TEMP_MINUS)
        airmod.TEMP_FLOAT = -airmod.TEMP_FLOAT;

    airmod.HUMI_I = buf[14];
    airmod.HUMI_F = buf[15];

    airmod.HUMI_FLOAT =
        (float)airmod.HUMI_I +
        (float)airmod.HUMI_F * 0.1f;

    return 1;
}


