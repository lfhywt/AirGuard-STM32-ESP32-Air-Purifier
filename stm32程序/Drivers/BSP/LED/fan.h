#ifndef __FAN_H
#define __FAN_H

#include "stm32f1xx_hal.h"


typedef enum {
    FAN_OFF = 0,
    FAN_ON  = 1
} FAN_STATE_E;

extern  volatile FAN_STATE_E g_fan_state;


void PA12_GPIO_Init(void);

void PA12_ON(void);     // Æô¶¯
void PA12_OFF(void);    // Í£Ö¹
void PA12_Toggle(void);


#endif
