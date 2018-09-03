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

/* Includes ------------------------------------------------------------------*/
#include "Usart.h"
#include "Bsp_uart.h"
#include "mystring.h"

/* ����ʹ�õĴ��ڶ˿� */
#define USER_UART1
#define USER_UART2

static UART_HandleTypeDef * ConsoleOutHandle;

/**
 * @func    ConsoleOut
 * @brief   �������������Ҫ�������������������ʾ
 * @param   Sendbuf ���͵Ļ���
 * @param   Lenght ���͵����ݳ���
 * @retval  ��
 */
void ConsoleOut(const char * Sendbuf, unsigned long Lenght)
{
    HAL_UART_Transmit(ConsoleOutHandle, (uint8_t *)Sendbuf, Lenght, 10);
}

/**
 * @func    SetConsoleDevice
 * @brief   ����������ľ��
 * @param   hUart ���
 * @retval  HAL_OK
 */
HAL_StatusTypeDef SetConsoleDevice(UART_HandleTypeDef * hUart)
{
    ConsoleOutHandle = hUart;
    SetConsoleOutFunc(&ConsoleOut);
    return HAL_OK;
}

#ifdef USER_UART1
/**
 * @func    Uart1_GetHandle
 * @brief   ��ȡ����1�ľ��
 * @retval  ���
 */
UART_HandleTypeDef * Uart1_GetHandle(void)
{
  return &huart1;
}
#endif

#ifdef USER_UART2
/**
 * @func    Uart2_GetHandle
 * @brief   ��ȡ����2�ľ��
 * @retval  ���
 */
UART_HandleTypeDef * Uart2_GetHandle(void)
{
  return &huart2;
}
#endif



