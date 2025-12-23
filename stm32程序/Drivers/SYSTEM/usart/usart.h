/**
 ****************************************************************************************************
 * @file        usart.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.1
 * @date        2023-06-05
 * @brief       串口初始化代码(一般是串口1)，支持printf
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F103开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20211103
 * 第一次发布
 * V1.1 20230605
 * 删除USART_UX_IRQHandler()函数的超时处理和修改HAL_UART_RxCpltCallback()
 *
 ****************************************************************************************************
 */

#ifndef __USART_H
#define __USART_H

#include "stdio.h"
#include "./SYSTEM/sys/sys.h"
#include <string.h>


/******************************************************************************************/
/* 引脚 和 串口 定义 
 * 默认是针对USART1的.
 * 注意: 通过修改这几个宏定义,可以支持USART1~UART5任意一个串口.
 */
#define USART_TX_GPIO_PORT                  GPIOA
#define USART_TX_GPIO_PIN                   GPIO_PIN_9
#define USART_TX_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)   /* PA口时钟使能 */

#define USART_RX_GPIO_PORT                  GPIOA
#define USART_RX_GPIO_PIN                   GPIO_PIN_10
#define USART_RX_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)   /* PA口时钟使能 */

#define USART_UX                            USART1
#define USART_UX_IRQn                       USART1_IRQn
#define USART_UX_IRQHandler                 USART1_IRQHandler
#define USART_UX_CLK_ENABLE()               do{ __HAL_RCC_USART1_CLK_ENABLE(); }while(0)  /* USART1 时钟使能 */
/******************************** USART2 ********************************/
#define USART_TX_GPIO_PORT_ESP8266                  GPIOA
#define USART_TX_GPIO_PIN_ESP8266                   GPIO_PIN_2   //TX
#define USART_TX_GPIO_CLK_ENABLE_ESP8266()          do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)   /* PA口时钟使能 */

#define USART_RX_GPIO_PORT_ESP8266                  GPIOA
#define USART_RX_GPIO_PIN_ESP8266                   GPIO_PIN_3  //RX
#define USART_RX_GPIO_CLK_ENABLE_ESP8266()          do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)   /* PA口时钟使能 */

#define USART_ESP8266                            USART2
#define USART_ESP8266_IRQn                       USART2_IRQn
#define USART_ESP8266_IRQHandler                 USART2_IRQHandler
#define USART_ESP8266_CLK_ENABLE()               do{ __HAL_RCC_USART2_CLK_ENABLE(); }while(0)  /* USART1 时钟使能 */


/******************************** USART3 ********************************/

/* USART3 TX: PB10 */
#define USART3_TX_GPIO_PORT                 GPIOB
#define USART3_TX_GPIO_PIN                  GPIO_PIN_10
#define USART3_TX_GPIO_CLK_ENABLE()         do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)

/* USART3 RX: PB11 */
#define USART3_RX_GPIO_PORT                 GPIOB
#define USART3_RX_GPIO_PIN                  GPIO_PIN_11
#define USART3_RX_GPIO_CLK_ENABLE()         do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)

/* USART3 本体 */
#define USART3_UX                           USART3
#define USART3_UX_IRQn                      USART3_IRQn
#define USART3_UX_IRQHandler                USART3_IRQHandler
#define USART3_UX_CLK_ENABLE()              do{ __HAL_RCC_USART3_CLK_ENABLE(); }while(0)
/******************************************************************************************/

#define USART_REC_LEN               200         /* 定义最大接收字节数 200 */
#define USART_EN_RX                 1           /* 使能（1）/禁止（0）串口1接收 */
#define RXBUFFERSIZE   1                        /* 缓存大小 */

extern UART_HandleTypeDef g_uart1_handle;       /* HAL UART句柄 */

extern uint8_t  g_usart_rx_buf[USART_REC_LEN];  /* 接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 */
extern uint16_t g_usart_rx_sta;                 /* 接收状态标记 */
extern uint8_t g_rx_buffer[RXBUFFERSIZE];       /* HAL库USART接收Buffer */
extern uint8_t  usart1_rx_byte;
extern uint8_t  usart1_stream[128];
extern uint16_t usart1_len;
extern uint8_t usart1_msg_ready;


/* USART2 stream（新增） */
extern uint8_t  usart2_rx_byte;
extern uint8_t  usart2_stream[128];
extern uint16_t usart2_len;
extern uint8_t  usart2_msg_ready;


/* USART2 - ESP8266 */
extern UART_HandleTypeDef g_uart2_esp_handle;
extern uint8_t  g_esp_rx_buf[USART_REC_LEN];
extern uint16_t g_esp_rx_sta;
extern uint8_t  g_esp_rx_buffer[RXBUFFERSIZE];





/* USART3 */
extern UART_HandleTypeDef g_uart3_handle;
extern uint8_t  g_usart3_rx_buf[USART_REC_LEN];
extern uint16_t g_usart3_rx_sta;
extern uint8_t  g_usart3_rx_buffer[RXBUFFERSIZE];





/* 初始化函数 */
void usart_init(uint32_t bound);          // USART1
void esp8266_usart_init(uint32_t bound);  // USART2
void usart3_init(uint32_t bound);//usart3
void Nextion_Send(UART_HandleTypeDef *huart, const char *cmd); //屏幕发送
void AirMod_Check_And_Recover(void);
#endif


