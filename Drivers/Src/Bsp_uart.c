/**
 ******************************************************************************
 * @file      Bsp_uart.c
 * @author    ZSY
 * @version   V1.0.1
 * @date      2018-10-08
 * @brief     ���ļ��ṩ�˴��ڲ�����ص�API��ʹ�õײ���Ӧ�ò���ӷ���
 * @History
 * Date           Author    version    		Notes
 * 2018-08-31       ZSY     V1.0.0      first version.
 * 2018-10-08       ZSY     V1.0.1      ʵ�ִ��ڿ����жϽ������ݣ�DMAģʽ.
 */

/* Includes ------------------------------------------------------------------*/
#include "Bsp_uart.h"
#include "mystring.h"
#include "usart.h"

/* ����ʹ�õĴ��ڶ˿� */
#define USE_UART1
//#define USE_UART2
//#define USE_CONSOLE 

#define RECV_BUF_SIZE   8192

#ifdef USE_UART1
extern DMA_HandleTypeDef hdma_usart1_rx;
#endif

/* �ڴ�������� */
typedef struct
{
    void (*RecvProcess)(void *Data, uint32_t Size);
} UartHooks_t;

/* ��ʼ���ڴ�������� */
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
 * @func    SetUartIDLE_IT
 * @brief   ���ÿ����ж�
 * @param   hUart ���
 * @param   Status ״̬
 * @retval  ��
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
 * @brief   ����DMA���յĻ�����
 * @param   hUart ���
 * @param   pBuf ������ָ��
 * @param   BufSize ��������С
 * @retval  ��
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
    //DMA ���յ�ַ����
    HAL_UART_Receive_DMA(hUart, hUart1DMABuf.RecvBuf, hUart1DMABuf.RecvBufSize);
}

/**
 * @func    UART_RxIdleCallback
 * @brief   ���ڿ����жϻص�����
 * @param   hUart ���
 * @retval  ��
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
#endif

#ifdef USE_UART1
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

#ifdef USE_UART2
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

/**
 * @func    SetUartRecvHook
 * @brief   ���ý��մ������Ļص�����
 * @param   RecvProcess ����ָ��
 * @retval  ��
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
 * @brief   ����ͨ���ж����
 * @retval  ��
 */
void USART1_IRQHandler(void)
{
    UART_RxIdleCallback(&huart1);
}


