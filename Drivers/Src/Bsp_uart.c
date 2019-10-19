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
#include "ustring.h"
#include "usart.h"
#include "UserConf.h"
#include "udef.h"

#ifdef USING_USART1
#ifdef USING_USART1_DMA_RX
extern DMA_HandleTypeDef hdma_usart1_rx;
#endif
serial_t * p_usart1_dev;
#endif
#ifdef USING_USART2
extern DMA_HandleTypeDef hdma_usart2_rx;
serial_t * p_usart2_dev;
#endif
#ifdef USING_USART3
extern DMA_HandleTypeDef hdma_usart3_rx;
serial_t * p_usart3_dev;
#endif

#ifdef USING_CONSOLE
static UART_HandleTypeDef * ConsoleOutHandle;
#endif

#ifdef USING_CONSOLE
/**
 * @func    ConsoleOut
 * @brief   �������������Ҫ�������������������ʾ
 * @param   Sendbuf ���͵Ļ���
 * @param   Lenght ���͵����ݳ���
 * @retval  ��
 */
void console_out(const char * Sendbuf, uint32_t Lenght)
{
    HAL_UART_Transmit(ConsoleOutHandle, (uint8_t *)Sendbuf, Lenght, 100);
}
#endif

/* �����ж���Ҫ���DMAʹ�� */
#ifdef USING_USART_DMA      
/**
 * @func    SetUartIDLE_IT
 * @brief   ���ÿ����ж�
 * @param   hUart ���
 * @param   Status ״̬
 * @retval  ��
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
 * @brief   ����DMA���յĻ�����
 * @param   hUart ���
 * @param   pBuf ������ָ��
 * @param   BufSize ��������С
 * @retval  ��
 */
void set_uart_DMA_recv_buff(serial_t * dev)
{  
    if (dev->uart_buf.recv_buf != NULL && dev->uart_buf.recv_buf_size != 0)
    {
        //DMA ���յ�ַ����
        HAL_UART_Receive_DMA(dev->huart, dev->uart_buf.recv_buf, dev->uart_buf.recv_buf_size);
        //__HAL_LOCK(dev->huart->hdmarx);
//        dev->huart->hdmarx->Instance->CCR |= (1 << 7);
        //__HAL_UNLOCK(dev->huart->hdmarx);
        
    }
}


/**
 * @func    UART_RxIdleCallback
 * @brief   ���ڿ����жϻص�����
 * @param   hUart ���
 * @retval  ��
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
#elif defined (STM32F1) || defined (STM32F0)
            rxSize = dev->uart_buf.recv_buf_size - hdma_usart1_rx.Instance->CNDTR;
#endif
            
            if (dev->recv_process != NULL)
                dev->recv_process(dev->uart_buf.recv_buf, rxSize);
        }
#endif
#ifdef USING_USART2
        if (dev->huart->Instance == USART2)
        {
            __HAL_UART_CLEAR_IDLEFLAG(dev->huart);
            HAL_UART_DMAStop(dev->huart);
#if defined (STM32F4)
            rxSize = dev->usart_buf.recv_buf_size - hdma_usart2_rx.Instance->NDTR;
#elif defined (STM32F1) || defined (STM32F0)
            rxSize = dev->uart_buf.recv_buf_size - hdma_usart2_rx.Instance->CNDTR;
#endif
            
            if (dev->recv_process != NULL)
                dev->recv_process(dev->uart_buf.recv_buf, rxSize);
        }
#endif
#ifdef USING_USART3
        if (dev->huart->Instance == USART3)
        {
            __HAL_UART_CLEAR_IDLEFLAG(dev->huart);
            HAL_UART_DMAStop(dev->huart);
#if defined (STM32F4)
            rxSize = dev->usart_buf.recv_buf_size - hdma_usart3_rx.Instance->NDTR;
#elif defined (STM32F1)
            rxSize = dev->usart_buf.recv_buf_size - hdma_usart3_rx.Instance->CNDTR;
#endif
            
            if (dev->recv_process != NULL)
                dev->recv_process(dev->uart_buf.recv_buf, rxSize);
        }
#endif
        HAL_UART_Receive_DMA(dev->huart, dev->uart_buf.recv_buf, dev->uart_buf.recv_buf_size);
    }
    __HAL_UART_CLEAR_FEFLAG(dev->huart);
    __HAL_UART_CLEAR_PEFLAG(dev->huart);
    __HAL_UART_CLEAR_NEFLAG(dev->huart);
    __HAL_UART_CLEAR_OREFLAG(dev->huart);
}
#endif

#ifdef USING_USART1
/**
 * @func    USART1_IRQHandler
 * @brief   ����ͨ���ж����
 * @retval  ��
 */
void USART1_IRQHandler(void)
{
#ifdef USING_USART1_DMA_RX
    UART_rx_IDLE_callback(p_usart1_dev);
#endif
}
#endif

#ifdef USING_USART2
/**
 * @func    USART2_IRQHandler
 * @brief   ����ͨ���ж����
 * @retval  ��
 */
void USART2_IRQHandler(void)
{
    UART_rx_IDLE_callback(p_usart2_dev);
}
#endif

#ifdef USING_USART3
/**
 * @func    USART3_IRQHandler
 * @brief   ����ͨ���ж����
 * @retval  ��
 */
void USART3_IRQHandler(void)
{
    UART_rx_IDLE_callback(p_usart3_dev);
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
    
#ifdef USING_USART1
    if (ustrcmp("usart1\0", dev->name) == 0)
    {
        p_usart1_dev = dev;
        
        dev->huart = &huart1;

#ifdef USING_CONSOLE    
        if (CONSOLE_DEVICE == "usart1\0")
        {
            ConsoleOutHandle = dev->huart;
        }
#endif
        
        /* USART1 interrupt Init */
        HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
#endif
#ifdef USING_USART2
    if (ustrcmp("usart2\0", dev->name) == 0)
    {
        p_usart2_dev = dev;
        dev->huart = &huart2;

#ifdef USING_CONSOLE
        if (CONSOLE_DEVICE == "usart2\0")
        {
            ConsoleOutHandle = dev->huart;
        }
#endif
    }
#endif
#ifdef USING_USART3
    if (ustrcmp("usart3\0", dev->name) == 0)
    {
        p_usart3_dev = dev;
        dev->huart = &huart3;
        
#ifdef USING_CONSOLE
        if (CONSOLE_DEVICE == "usart3\0")
        {
            ConsoleOutHandle = dev->huart;
        }
#endif
    }
#endif

    if (dev->mode & UART_MODE_IDLE_INT)
    {
        dev->flags |= UENABLE;
#ifdef USING_USART_DMA
        set_uart_IDLE_IT(dev);
        set_uart_DMA_recv_buff(dev);
#endif
    }
    
    return UEOK;
}


