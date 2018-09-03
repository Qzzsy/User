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

/* Includes ------------------------------------------------------------------*/
#include "Usart.h"
#include "Bsp_uart.h"
#include "mystring.h"

/* 定义使用的串口端口 */
#define USER_UART1
#define USER_UART2

static UART_HandleTypeDef * ConsoleOutHandle;

/**
 * @func    ConsoleOut
 * @brief   流输出方法，主要用于输出到串口助手显示
 * @param   Sendbuf 发送的缓存
 * @param   Lenght 发送的数据长度
 * @retval  无
 */
void ConsoleOut(const char * Sendbuf, unsigned long Lenght)
{
    HAL_UART_Transmit(ConsoleOutHandle, (uint8_t *)Sendbuf, Lenght, 10);
}

/**
 * @func    SetConsoleDevice
 * @brief   设置流输出的句柄
 * @param   hUart 句柄
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
 * @brief   获取串口1的句柄
 * @retval  句柄
 */
UART_HandleTypeDef * Uart1_GetHandle(void)
{
  return &huart1;
}
#endif

#ifdef USER_UART2
/**
 * @func    Uart2_GetHandle
 * @brief   获取串口2的句柄
 * @retval  句柄
 */
UART_HandleTypeDef * Uart2_GetHandle(void)
{
  return &huart2;
}
#endif



