/**
 ******************************************************************************
 * @file      Bsp_uart.c
 * @author    ZSY
 * @version   V1.0.1
 * @date      2018-10-08
 * @brief     该文件提供了串口操作相关的API，使得底层与应用层更加分离
 * @History
 * Date           Author    version    		Notes
 * 2018-08-31       ZSY     V1.0.0      first version.
 * 2018-10-08       ZSY     V1.0.1      实现串口空闲中断接受数据，DMA模式.
 */

/* Includes ------------------------------------------------------------------*/
#include "Bsp_uart.h"
#include "mystring.h"
#include "usart.h"

/* 定义使用的串口端口 */
#define USE_UART1
//#define USE_UART2
//#define USE_CONSOLE 

#define RECV_BUF_SIZE   8192

#ifdef USE_UART1
extern DMA_HandleTypeDef hdma_usart1_rx;
#endif

/* 内存操作方法 */
typedef struct
{
    void (*RecvProcess)(void *Data, uint32_t Size);
} UartHooks_t;

/* 初始化内存操作方法 */
static UartHooks_t Hooks =
    {
        .RecvProcess = NULL};

typedef struct 
{
    void * RecvBuf;
    uint32_t RecvBufSize;
    void * SendBuf;
    uint32_t SendBufSize;
}UartDMABuf_t;

uint8_t RecvBuf[RECV_BUF_SIZE] = {'\0'};

static UartDMABuf_t hUart1DMABuf =
{
    .RecvBuf = RecvBuf,
    .RecvBufSize = RECV_BUF_SIZE
};
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
 * @func    SetUartIDLE_IT
 * @brief   配置空闲中断
 * @param   hUart 句柄
 * @param   Status 状态
 * @retval  无
 */
void SetUartIDLE_IT(UART_HandleTypeDef * hUart, uint8_t Status)
{
    if (Status == ENABLE)
    {
        __HAL_UART_ENABLE_IT(hUart, UART_IT_IDLE);
    }
    else if (Status == DISABLE)
    {
        __HAL_UART_DISABLE_IT(hUart, UART_IT_IDLE);
    }
}

/**
 * @func    SetUartDMARecvBuff
 * @brief   设置DMA接收的缓存区
 * @param   hUart 句柄
 * @param   pBuf 缓存区指针
 * @param   BufSize 缓存区大小
 * @retval  无
 */
void SetUartDMARecvBuff(UART_HandleTypeDef * hUart, void * pBuf, uint32_t BufSize)
{  
    if (pBuf != NULL && BufSize != 0)
    {
        if (hUart->Instance == USART1)
        {
            hUart1DMABuf.RecvBuf = pBuf;
            hUart1DMABuf.RecvBufSize = BufSize;
        }
    }
    //DMA 接收地址设置
    HAL_UART_Receive_DMA(hUart, hUart1DMABuf.RecvBuf, hUart1DMABuf.RecvBufSize);
}

/**
 * @func    UART_RxIdleCallback
 * @brief   串口空闲中断回调方法
 * @param   hUart 句柄
 * @retval  无
 */
static inline void UART_RxIdleCallback(UART_HandleTypeDef *huart)
{
    if(__HAL_UART_GET_FLAG(huart,UART_FLAG_IDLE))
    {
        if (huart->Instance == USART1)
        {
            __IO uint32_t rxSize = 0;
            __HAL_UART_CLEAR_IDLEFLAG(huart);
            HAL_UART_DMAStop(huart);
            //hdma_usart1_rx.Instance->NDTR = hUart1DMABuf.RecvBufSize;
            rxSize = hUart1DMABuf.RecvBufSize - hdma_usart1_rx.Instance->NDTR;
            
            if (Hooks.RecvProcess != NULL)
                Hooks.RecvProcess(hUart1DMABuf.RecvBuf, rxSize);

            HAL_UART_Receive_DMA(huart, hUart1DMABuf.RecvBuf, hUart1DMABuf.RecvBufSize);
        }
    }
}

#ifdef USE_CONSOLE
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
#endif

#ifdef USE_UART1
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

#ifdef USE_UART2
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

/**
 * @func    SetUartRecvHook
 * @brief   配置接收处理函数的回调函数
 * @param   RecvProcess 函数指针
 * @retval  无
 */
void SetUartRecvHook(void (*RecvProcess)(const void *Data, uint32_t Size))
{
    if (RecvProcess != NULL)
    {
        Hooks.RecvProcess = RecvProcess;
    }
}

/**
 * @func    USART1_IRQHandler
 * @brief   串口通用中断入口
 * @retval  无
 */
void USART1_IRQHandler(void)
{
    UART_RxIdleCallback(&huart1);
}


