/**
 ******************************************************************************
 * @file      Bsp_uart.c
 * @author    ZSY
 * @version   V1.0.2
 * @date      2019-06-10
 * @brief     该文件提供了串口操作相关的API，使得底层与应用层更加分离，依赖mystring和usart文件
 * @History
 * Date           Author    version    		Notes
 * 2018-08-31       ZSY     V1.0.0      first version.
 * 2018-10-08       ZSY     V1.0.1      实现串口空闲中断接受数据，DMA模式.
 * 2019-06-10       ZSY     V1.0.2      添加对USART2和USART3的支持.
 */

/* Includes ------------------------------------------------------------------*/
#include "Bsp_uart.h"
#include "ustring.h"
#include "usart.h"
#include "uconfig.h"
#include "udef.h"

#ifdef USING_USART1
#ifdef USING_USART1_DMA_RX
extern DMA_HandleTypeDef hdma_usart1_rx;
#endif
serial_t * p_usart1_dev;
#endif
#ifdef USING_USART2
extern DMA_HandleTypeDef hdma_usart2_rx;
zu_serial_t * p_usart2_dev;
#endif
#ifdef USING_USART3
extern DMA_HandleTypeDef hdma_usart3_rx;
zu_serial_t * p_usart3_dev;
#endif

#ifdef USING_CONSOLE
static UART_HandleTypeDef * ConsoleOutHandle;
#endif

#ifdef USING_CONSOLE
/**
 * @func    ConsoleOut
 * @brief   流输出方法，主要用于输出到串口助手显示
 * @param   Sendbuf 发送的缓存
 * @param   Lenght 发送的数据长度
 * @retval  无
 */
void console_out(const char * Sendbuf, uint32_t Lenght)
{
    HAL_UART_Transmit(ConsoleOutHandle, (uint8_t *)Sendbuf, Lenght, 10);
}
#endif

/* 空闲中断需要配合DMA使用 */
#ifdef USING_USART_DMA      
/**
 * @func    SetUartIDLE_IT
 * @brief   配置空闲中断
 * @param   hUart 句柄
 * @param   Status 状态
 * @retval  无
 */
void set_uart_IDLE_IT(serial_t *dev)
{
    if (dev->flags & UENABLE)
    {
        __HAL_UART_ENABLE_IT(dev->huart, UART_IT_IDLE);
    }
    else if (dev->flags & UDISABLE)
    {
        __HAL_UART_DISABLE_IT(dev->huart, UART_IT_IDLE);
    }
}

/**
 * @func    set_usart_DMA_recv_buff
 * @brief   设置DMA接收的缓存区
 * @param   hUart 句柄
 * @param   pBuf 缓存区指针
 * @param   BufSize 缓存区大小
 * @retval  无
 */
void set_uart_DMA_recv_buff(serial_t * dev)
{  
    if (dev->uart_buf.recv_buf != NULL && dev->uart_buf.recv_buf_size != 0)
    {
        //DMA 接收地址设置
        HAL_UART_Receive_DMA(dev->huart, dev->uart_buf.recv_buf, dev->uart_buf.recv_buf_size);
    }
}


/**
 * @func    UART_RxIdleCallback
 * @brief   串口空闲中断回调方法
 * @param   hUart 句柄
 * @retval  无
 */
static inline void UART_rx_IDLE_callback(serial_t * dev)
{
    __IO uint32_t rxSize = 0;
    if(__HAL_UART_GET_FLAG(dev->huart, UART_FLAG_IDLE))
    {
#ifdef USING_USART1
        if (dev->huart->Instance == USART1)
        {
            __HAL_UART_CLEAR_IDLEFLAG(dev->huart);
            HAL_UART_DMAStop(dev->huart);
#if defined (STM32F4)
            rxSize = dev->usart_buf.recv_buf_size - hdma_usart1_rx.Instance->NDTR;
#elif defined (STM32F1)
            rxSize = dev->uart_buf.recv_buf_size - hdma_usart1_rx.Instance->CNDTR;
#endif
            
            if (dev->recv_process != NULL)
                dev->recv_process(dev->uart_buf.recv_buf, rxSize);
        }
#endif
#ifdef USING_USART2
        if (huart->Instance == USART2)
        {
            UartDMABuf = &hUart2DMABuf;
            __HAL_UART_CLEAR_IDLEFLAG(huart);
            HAL_UART_DMAStop(huart);
#if defined (STM32F4)
            rxSize = dev->usart_buf.recv_buf_size - hdma_usart2_rx.Instance->NDTR;
#elif defined (STM32F1)
            rxSize = dev->usart_buf.recv_buf_size - hdma_usart2_rx.Instance->CNDTR;
#endif
            
            if (Hooks.Usart2RecvProcess != NULL)
                Hooks.Usart2RecvProcess(UartDMABuf->RecvBuf, rxSize);
        }
#endif
#ifdef USING_USART3
        if (huart->Instance == USART3)
        {
            UartDMABuf = &hUart3DMABuf;
            __HAL_UART_CLEAR_IDLEFLAG(huart);
            HAL_UART_DMAStop(huart);
#if defined (STM32F4)
            rxSize = dev->usart_buf.recv_buf_size - hdma_usart3_rx.Instance->NDTR;
#elif defined (STM32F1)
            rxSize = dev->usart_buf.recv_buf_size - hdma_usart3_rx.Instance->CNDTR;
#endif
            
            if (Hooks.Usart3RecvProcess != NULL)
                Hooks.Usart3RecvProcess(UartDMABuf->RecvBuf, rxSize);
        }
#endif
        HAL_UART_Receive_DMA(dev->huart, dev->uart_buf.recv_buf, dev->uart_buf.recv_buf_size);
    }
}
#endif

#ifdef USING_USART1
/**
 * @func    USART1_IRQHandler
 * @brief   串口通用中断入口
 * @retval  无
 */
void USART1_IRQHandler(void)
{
#ifdef USING_USART1_DMA_RX
    UART_rx_IDLE_callback(p_usart1_dev);
#endif
    __HAL_UART_CLEAR_IDLEFLAG(p_usart1_dev->huart);
    __HAL_UART_CLEAR_OREFLAG(p_usart1_dev->huart);
    __HAL_UART_CLEAR_NEFLAG(p_usart1_dev->huart);
    __HAL_UART_CLEAR_FEFLAG(p_usart1_dev->huart);
}
#endif

#ifdef USING_USART2
/**
 * @func    USART2_IRQHandler
 * @brief   串口通用中断入口
 * @retval  无
 */
void USART2_IRQHandler(void)
{
    USART_rx_IDLE_callback(usart2_dev);
}
#endif

#ifdef USING_USART3
/**
 * @func    USART3_IRQHandler
 * @brief   串口通用中断入口
 * @retval  无
 */
void USART3_IRQHandler(void)
{
    USART_rx_IDLE_callback(usart3_dev);
}
#endif

err_t uart_init(serial_t *dev)
{
    if (dev == UNULL)
    {
        return -UERROR;
    }
    if (dev->uart_buf.recv_buf == UNULL || dev->uart_buf.recv_buf_size == 0)
    {
        return -UERROR;
    }
    
    p_usart1_dev = dev;
    
#ifdef USING_USART1
    if (ustrcmp("usart1", dev->name) == 0)
    {
        dev->huart = &huart1;

#ifdef USING_CONSOLE    
        if (CONSOLE_DEVICE == "usart1")
        {
            ConsoleOutHandle = dev->huart;
        }
#endif
    }
#endif
#ifdef USING_USART2
    if (zu_strcmp("usart2", dev->name) == 0)
    {
        dev->huart = &huart2;

#ifdef USE_CONSOLE
        if (CONSOLE_DEVICE == "usart2")
        {
            ConsoleOutHandle = dev->huart;
        }
    }
#endif
#endif
#ifdef USING_USART3
    if (zu_strcmp("usart3", dev->name) == 0)
    {
        dev->huart = &huart3;
        
#ifdef USING_CONSOLE
        if (CONSOLE_DEVICE == "usart3")
        {
            ConsoleOutHandle = dev->huart;
        }
#endif
    }
#endif

    if (dev->mode & UART_MODE_IDLE_INT)
    {
        dev->flags |= UENABLE;
        //set_uart_IDLE_IT(dev);
#ifdef USING_USART_DMA
        set_uart_DMA_recv_buff(dev);
#endif
    }
    
    return UEOK;
}


