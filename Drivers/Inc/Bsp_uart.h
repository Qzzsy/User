/**
 ******************************************************************************
 * @file      Bsp_uart.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-08-31
 * @brief     ���ļ��ṩ�˴��ڲ�����ص�API��ʹ�õײ���Ӧ�ò���ӷ���
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

HAL_StatusTypeDef SetConsoleDevice(UART_HandleTypeDef * hUart);
UART_HandleTypeDef * Uart1_GetHandle(void);
UART_HandleTypeDef * Uart2_GetHandle(void);
void SetUartIDLE_IT(UART_HandleTypeDef * hUart, uint8_t Status);
void SetUartDMARecvBuff(UART_HandleTypeDef * hUart, void * pBuf, uint32_t BufSize);
void SetUartRecvHook(void (*RecvProcess)(const void *Data, uint32_t Size));
#endif

       
