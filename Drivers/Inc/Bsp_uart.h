/**
 ******************************************************************************
 * @file      Bsp_uart.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-08-31
 * @brief     该文件提供了串口操作相关的API，使得底层与应用层更加分离
 * @History
 * Date           Author    version    		Notes
 * 2018-08-31       ZSY     V1.0.0      first version.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _BSP_UART_H_
#define _BSP_UART_H_

#if defined STM32F1
#include "stm32f1xx.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#endif
#include "zu_config.h"
#include "zu_def.h"

#define ZU_MODE_IDLE_INT            (1 << 0)
#define ZU_MODE_RECV_INT            (1 << 1)

typedef struct 
{
    void * recv_buf;
    uint32_t recv_buf_size;
    void * send_buf;
    uint32_t send_buf_size;
}usart_buf_t;

struct zu_serial_device
{
    char                        name[ZU_NAME_MAX];
    UART_HandleTypeDef          *huart;
    void                        (*recv_process)(const void *, uint32_t);
    usart_buf_t                 usart_buf;
    uint16_t                    mode;
    uint8_t                     flags;
};
typedef struct zu_serial_device zu_serial_t;

void console_out(const char * Sendbuf, unsigned long Lenght);
HAL_StatusTypeDef SetConsoleDevice(UART_HandleTypeDef * hUart);
void set_usart_IDLE_IT(zu_serial_t *dev);
void set_usart_DMA_recv_buff(zu_serial_t * dev);
zu_err_t usart_init(zu_serial_t *dev);
#endif

       
