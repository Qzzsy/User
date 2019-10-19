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

#include "uconfig.h"
#include "udef.h"

#if defined (STM32F1)
#include "stm32f1xx.h"
#elif defined (STM32F4)
#include "stm32f4xx.h"
#endif

#define UART_MODE_IDLE_INT            (1 << 0)
#define UART_MODE_RECV_INT            (1 << 1)

typedef struct 
{
    void * recv_buf;
    uint32_t recv_buf_size;
    void * send_buf;
    uint32_t send_buf_size;
}uart_buf_t;

struct serial_device
{
    char                        name[OBJECT_NAME_MAX];
    UART_HandleTypeDef          *huart;
    void                        (*recv_process)(const void *, uint32_t);
    uart_buf_t                  uart_buf;
    uint16_t                    mode;
    uint8_t                     flags;
};
typedef struct serial_device serial_t;

void console_out(const char * Sendbuf, uint32_t Lenght);
HAL_StatusTypeDef SetConsoleDevice(UART_HandleTypeDef * hUart);
void set_uart_IDLE_IT(serial_t *dev);
void set_uart_DMA_recv_buff(serial_t * dev);
err_t uart_init(serial_t *dev);
#endif

       
