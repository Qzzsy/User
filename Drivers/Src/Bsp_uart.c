/**
 ******************************************************************************
 * @file      Bsp_uart.c
 * @author    ZSY
 * @version   V1.0.2
 * @date      2019-06-10
 * @brief     ���ļ��ṩ�˴��ڲ�����ص�API��ʹ�õײ���Ӧ�ò���ӷ��룬����mystring��usart�ļ�
 * @History
 * Date           Author    version    		Notes
 * 2018-08-31       ZSY     V1.0.0      first version.
 * 2018-10-08       ZSY     V1.0.1      ʵ�ִ��ڿ����жϽ������ݣ�DMAģʽ.
 * 2019-06-10       ZSY     V1.0.2      ��Ӷ�USART2��USART3��֧��.
 */

/* Includes ------------------------------------------------------------------*/
#include "Bsp_uart.h"
#include "zu_string.h"
#include "usart.h"
#include "zu_config.h"
#include "zu_def.h"

#ifdef USE_USART1
extern DMA_HandleTypeDef hdma_usart1_rx;
zu_serial_t * p_usart1_dev;
#endif
#ifdef USE_USART2
extern DMA_HandleTypeDef hdma_usart2_rx;
zu_serial_t * p_usart2_dev;
#endif
#ifdef USE_USART3
extern DMA_HandleTypeDef hdma_usart3_rx;
zu_serial_t * p_usart3_dev;
#endif

#ifdef USE_CONSOLE
static UART_HandleTypeDef * ConsoleOutHandle;
#endif

#ifdef USE_CONSOLE
/**
 * @func    ConsoleOut
 * @brief   �������������Ҫ�������������������ʾ
 * @param   Sendbuf ���͵Ļ���
 * @param   Lenght ���͵����ݳ���
 * @retval  ��
 */
void console_out(const char * Sendbuf, unsigned long Lenght)
{
    HAL_UART_Transmit(ConsoleOutHandle, (uint8_t *)Sendbuf, Lenght, 10);
}
#endif

/**
 * @func    SetUartIDLE_IT
 * @brief   ���ÿ����ж�
 * @param   hUart ���
 * @param   Status ״̬
 * @retval  ��
 */
void set_usart_IDLE_IT(zu_serial_t *dev)
{
    if (dev->flags & ZU_ENABLE)
    {
        __HAL_UART_ENABLE_IT(dev->huart, UART_IT_IDLE);
    }
    else if (dev->flags & ZU_DISABLE)
    {
        __HAL_UART_DISABLE_IT(dev->huart, UART_IT_IDLE);
    }
}

/**
 * @func    set_usart_DMA_recv_buff
 * @brief   ����DMA���յĻ�����
 * @param   hUart ���
 * @param   pBuf ������ָ��
 * @param   BufSize ��������С
 * @retval  ��
 */
void set_usart_DMA_recv_buff(zu_serial_t * dev)
{  
    if (dev->usart_buf.recv_buf != NULL && dev->usart_buf.recv_buf_size != 0)
    {
        //DMA ���յ�ַ����
        HAL_UART_Receive_DMA(dev->huart, dev->usart_buf.recv_buf, dev->usart_buf.recv_buf_size);
    }
}

/**
 * @func    UART_RxIdleCallback
 * @brief   ���ڿ����жϻص�����
 * @param   hUart ���
 * @retval  ��
 */
static inline void USART_rx_IDLE_callback(zu_serial_t * dev)
{
    __IO uint32_t rxSize = 0;
    if(__HAL_UART_GET_FLAG(dev->huart, UART_FLAG_IDLE))
    {
#ifdef USE_USART1
        if (dev->huart->Instance == USART1)
        {
            __HAL_UART_CLEAR_IDLEFLAG(dev->huart);
            HAL_UART_DMAStop(dev->huart);
            //hdma_usart1_rx.Instance->NDTR = hUart1DMABuf.RecvBufSize;
            rxSize = dev->usart_buf.recv_buf_size - hdma_usart1_rx.Instance->NDTR;
            
            if (dev->recv_process != NULL)
                dev->recv_process(dev->usart_buf.recv_buf, rxSize);
        }
#endif
#ifdef USE_USART2
        if (huart->Instance == USART2)
        {
            UartDMABuf = &hUart2DMABuf;
            __HAL_UART_CLEAR_IDLEFLAG(huart);
            HAL_UART_DMAStop(huart);
            //hdma_usart1_rx.Instance->NDTR = hUart1DMABuf.RecvBufSize;
            rxSize = UartDMABuf->RecvBufSize - hdma_usart2_rx.Instance->NDTR;
            
            if (Hooks.Usart2RecvProcess != NULL)
                Hooks.Usart2RecvProcess(UartDMABuf->RecvBuf, rxSize);
        }
#endif
#ifdef USE_USART3
        if (huart->Instance == USART3)
        {
            UartDMABuf = &hUart3DMABuf;
            __HAL_UART_CLEAR_IDLEFLAG(huart);
            HAL_UART_DMAStop(huart);
            //hdma_usart1_rx.Instance->NDTR = hUart1DMABuf.RecvBufSize;
            rxSize = UartDMABuf->RecvBufSize - hdma_usart3_rx.Instance->NDTR;
            
            if (Hooks.Usart3RecvProcess != NULL)
                Hooks.Usart3RecvProcess(UartDMABuf->RecvBuf, rxSize);
        }
#endif
        HAL_UART_Receive_DMA(dev->huart, dev->usart_buf.recv_buf, dev->usart_buf.recv_buf_size);
    }
}

#ifdef USE_USART1
/**
 * @func    USART1_IRQHandler
 * @brief   ����ͨ���ж����
 * @retval  ��
 */
void USART1_IRQHandler(void)
{
    USART_rx_IDLE_callback(p_usart1_dev);
}
#endif

#ifdef USE_USART2
/**
 * @func    USART2_IRQHandler
 * @brief   ����ͨ���ж����
 * @retval  ��
 */
void USART2_IRQHandler(void)
{
    USART_rx_IDLE_callback(usart2_dev);
}
#endif

#ifdef USE_USART3
/**
 * @func    USART3_IRQHandler
 * @brief   ����ͨ���ж����
 * @retval  ��
 */
void USART3_IRQHandler(void)
{
    USART_rx_IDLE_callback(usart3_dev);
}
#endif

zu_err_t usart_init(zu_serial_t *dev)
{
    if (dev == ZU_NULL)
    {
        return -ZU_ERROR;
    }
    if (dev->usart_buf.recv_buf == ZU_NULL || dev->usart_buf.recv_buf_size == 0)
    {
        return -ZU_ERROR;
    }
    
#ifdef USE_USART1
    if (zu_strcmp("usart1", dev->name) == 0)
    {
        dev->huart = &huart1;
        if (dev->mode & ZU_MODE_IDLE_INT)
        {
            dev->flags |= ZU_ENABLE;
            set_usart_IDLE_IT(dev);
        }
        p_usart1_dev = dev;
#ifdef USE_CONSOLE    
        if (CONSOLE_DEVICE == "usart1")
        {
            ConsoleOutHandle = dev->huart;
        }
#endif
    }
#endif
#ifdef USE_USART2
    if (zu_strcmp("usart2", dev->name) == 0)
    {
        dev->huart = &huart1;
        if (dev->mode & ZU_MODE_IDLE_INT)
        {
            dev->flags |= ZU_ENABLE;
            set_usart_IDLE_IT(dev);
        }
        p_usart2_dev = dev;
    
#ifdef USE_CONSOLE
        if (CONSOLE_DEVICE == "usart2")
        {
            ConsoleOutHandle = dev->huart;
        }
    }
#endif
#endif
#ifdef USE_USART3
    if (zu_strcmp("usart3", dev->name) == 0)
    {
        dev->huart = &huart2;
        if (dev->mode & ZU_MODE_IDLE_INT)
        {
            dev->flags |= ZU_ENABLE;
            set_usart_IDLE_IT(dev);
        }
        p_usart3_dev = dev;
        
#ifdef USE_CONSOLE
        if (CONSOLE_DEVICE == "usart3")
        {
            ConsoleOutHandle = dev->huart;
        }
#endif
    }
#endif
}
