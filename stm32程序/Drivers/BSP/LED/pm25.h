#ifndef __PM25_H
#define __PM25_H


#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <string.h>

#define AIRMOD_FRAME_LEN 17

extern volatile uint8_t airmod_update_flag;
extern volatile uint32_t airmod_last_tick;   // 最近一次收到数据的时间
extern volatile uint8_t  airmod_online;      // 传感器在线标志

typedef struct
{
    uint16_t CO2;
    uint16_t JQ;
    uint16_t VOC;
    uint16_t PM25;
    uint16_t PM10;

    uint8_t  TEMP_MINUS;
    uint8_t  TEMP_I;
    uint8_t  TEMP_F;
    float    TEMP_FLOAT;

    uint8_t  HUMI_I;
    uint8_t  HUMI_F;
    float    HUMI_FLOAT;
} AIRMOD_T;

extern AIRMOD_T airmod;

extern uint8_t UART_RX_BUFF[AIRMOD_FRAME_LEN];

uint8_t Kanfur_Airmod_Decode(uint8_t *buf);


#endif
