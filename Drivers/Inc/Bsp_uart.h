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

#include "stm32f4xx.h"

HAL_StatusTypeDef SetConsoleDevice(UART_HandleTypeDef * hUart);
UART_HandleTypeDef * Uart1_GetHandle(void);
UART_HandleTypeDef * Uart2_GetHandle(void);

#endif

                                                  