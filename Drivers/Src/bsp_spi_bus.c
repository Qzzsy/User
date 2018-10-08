/**
 ******************************************************************************
 * @file      bsp_spi_bus.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-10-03
 * @brief     SPI����ʵ�ֳ���ʵ��SPI�����������
 * @note      
 * @History
 * Date           Author    version    		Notes
 * 2018-10-04       ZSY     V1.0.0      first version.
 */
/* Includes ------------------------------------------------------------------*/
#include "bsp_spi_bus.h"

/* ��ʹ��LL�⣬�붨��USER_EFFIΪ1 */
#ifndef USER_EFFI
#define USER_EFFI 0
#endif

#ifndef NULL
#define NULL 0
#endif

#define READ (1 << 0)
#define WRITE (1 << 1)
#define TIMEOUT 200

static uint16_t spiBusID = 0;
static uint8_t spiBusBusy = SPI_BUS_NOBUSY;

/**
 * @func    spiBusTake
 * @brief   ռ��SPI����
 * @note    
 * @retval  ��
 */
void spiBusTake(void)
{
    spiBusBusy = SPI_BUS_BUSY;
}

/**
 * @func    spiBusRelease
 * @brief   �ͷ�ռ�õ�SPI����
 * @note    
 * @retval  ��
 */
void spiBusRelease(void)
{
    spiBusBusy = SPI_BUS_NOBUSY;
}

/**
 * @func    spiGetBusBusy
 * @brief   �ж�SPI����æ��
 * @note    
 * @retval  ��
 */
uint8_t spiGetBusBusy(void)
{
    return spiBusBusy;
}

/**
 * @func    SpixRW
 * @brief   SPI��д����
 * @param   _spiDeviceHandle SPI���
 * @param   *Message ���͵���Ϣ
 * @note    ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
 * @retval  ��
 */
uint32_t SpixRW(spiDeviceHandle_t _spiDeviceHandle, spiMessage_t *Message)
{
    __IO uint8_t Retry = 0, result = HAL_OK;

    if (Message->csTake == 1)
    {
        _spiDeviceHandle->Bus->Ops->csTake();
        spiBusTake();
    }
#if USER_EFFI == 1
    uint16_t i = 0;
    while (i < Message->Length)
    {
        /* ���ָ����SPI��־λ�������:���ͻ���ձ�־λ */
        while (LL_SPI_IsActiveFlag_TXE((SPI_TypeDef *)_spiDeviceHandle->Device) == RESET)
        {
            Retry++;
            if (Retry > Message->TimeOut)
            {
                result = (uint8_t)SPI_ERROR;
                goto __exit;
            }
        }
        LL_SPI_TransmitData8((SPI_TypeDef *)_spiDeviceHandle->Device, *((uint8_t *)Message->SendBuf + i));

        Retry = 0;
        /* ���ָ����SPI��־λ�������:���ͻ���ձ�־λ */
        while (LL_SPI_IsActiveFlag_RXNE((SPI_TypeDef *)_spiDeviceHandle->Device) == RESET)
        {
            Retry++;
            if (Retry > Message->TimeOut)
            {
                result = (uint8_t)SPI_ERROR;
                goto __exit;
            }
        }

        *((uint8_t *)Message->RecvBuf + i) = LL_SPI_ReceiveData8((SPI_TypeDef *)_spiDeviceHandle->Device);

        i++;
    }
#else
    SPI_HandleTypeDef *hSPI = (SPI_HandleTypeDef *)_spiDeviceHandle->Device;

    if (Message->Flag & READ && Message->Flag & WRITE)
    {
        result = HAL_SPI_TransmitReceive(hSPI, (uint8_t *)Message->SendBuf, Message->RecvBuf, Message->Length, Message->TimeOut);
        if (result != HAL_OK)
        {
            result = (uint8_t)SPI_ERROR;
            goto __exit;
        }
    }
    else if (Message->Flag & WRITE)
    {
        result = HAL_SPI_Transmit(hSPI, (uint8_t *)Message->SendBuf, Message->Length, Message->TimeOut);
        if (result != HAL_OK)
        {
            result = (uint8_t)SPI_ERROR;
            goto __exit;
        }
    }
    else if (Message->Flag & READ)
    {
        result = HAL_SPI_Receive(hSPI, Message->RecvBuf, Message->Length, Message->TimeOut);
        if (result != HAL_OK)
        {
            result = (uint8_t)SPI_ERROR;
            goto __exit;
        }
    }
#endif
__exit:
    if (Message->csRelease == 1)
    {
        _spiDeviceHandle->Bus->Ops->csReslease();
        spiBusRelease();
    }
    return result;
}

/**
 * @func    spiGetBaudRatePrescaler
 * @brief   ��ȡSPI�Ĳ�����
 * @param   maxHz ���ʱ��Ƶ��
 * @note    
 * @retval  SPI_BaudRatePrescaler ��Ƶϵ��
 */
static inline uint32_t spiGetBaudRatePrescaler(uint32_t maxHz)
{
    uint16_t SPI_BaudRatePrescaler;
#if USER_EFFI == 1
    /* STM32F40x SPI MAX 42Mhz  SPI1 max 84MHz*/
    if (maxHz >= SystemCoreClock / 2 && SystemCoreClock / 2 <= 36000000)
    {
        SPI_BaudRatePrescaler = LL_SPI_BAUDRATEPRESCALER_DIV2;
    }
    else if (maxHz >= SystemCoreClock / 4)
    {
        SPI_BaudRatePrescaler = LL_SPI_BAUDRATEPRESCALER_DIV4;
    }
    else if (maxHz >= SystemCoreClock / 8)
    {
        SPI_BaudRatePrescaler = LL_SPI_BAUDRATEPRESCALER_DIV8;
    }
    else if (maxHz >= SystemCoreClock / 16)
    {
        SPI_BaudRatePrescaler = LL_SPI_BAUDRATEPRESCALER_DIV16;
    }
    else if (maxHz >= SystemCoreClock / 32)
    {
        SPI_BaudRatePrescaler = LL_SPI_BAUDRATEPRESCALER_DIV32;
    }
    else if (maxHz >= SystemCoreClock / 64)
    {
        SPI_BaudRatePrescaler = LL_SPI_BAUDRATEPRESCALER_DIV64;
    }
    else if (maxHz >= SystemCoreClock / 128)
    {
        SPI_BaudRatePrescaler = LL_SPI_BAUDRATEPRESCALER_DIV128;
    }
    else
    {
        /* min prescaler 256 */
        SPI_BaudRatePrescaler = LL_SPI_BAUDRATEPRESCALER_DIV256;
    }
#else
    /* STM32F40x SPI MAX 42Mhz  SPI1 max 84MHz*/
    if (maxHz >= SystemCoreClock / 2 && SystemCoreClock / 2 <= 36000000)
    {
        SPI_BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    }
    else if (maxHz >= SystemCoreClock / 4)
    {
        SPI_BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    }
    else if (maxHz >= SystemCoreClock / 8)
    {
        SPI_BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    }
    else if (maxHz >= SystemCoreClock / 16)
    {
        SPI_BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    }
    else if (maxHz >= SystemCoreClock / 32)
    {
        SPI_BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    }
    else if (maxHz >= SystemCoreClock / 64)
    {
        SPI_BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    }
    else if (maxHz >= SystemCoreClock / 128)
    {
        SPI_BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
    }
    else
    {
        /* min prescaler 256 */
        SPI_BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    }
#endif
    return SPI_BaudRatePrescaler;
}

/**
 * @func    spiConfigure
 * @brief   SPI����
 * @param   _spiDeviceHandle SPI���
 * @note    
 * @retval  ������룬�ɹ���ʧ��
 */
uint8_t spiConfigure(spiDeviceHandle_t _spiDeviceHandle)
{
    assert_param(_spiDeviceHandle);
    assert_param(_spiDeviceHandle->Bus);
    assert_param(_spiDeviceHandle->Config);
    assert_param(_spiDeviceHandle->Device);

    if (_spiDeviceHandle->Bus != NULL)
    {
        if (_spiDeviceHandle->Bus->ID == spiBusID)
        {
            /* set SPI bus owner */
            spiBusID = _spiDeviceHandle->Bus->ID;

#if USER_EFFI == 1
            LL_SPI_InitTypeDef SPI_InitStruct;

            LL_SPI_StructInit(&SPI_InitStruct);

            /* data_width */
            if (_spiDeviceHandle->Config->DataWidth <= 8)
            {
                SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
            }
            else if (_spiDeviceHandle->Config->DataWidth <= 16)
            {
                SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_16BIT;
            }
            else
            {
                return (uint8_t)SPI_ERROR;
            }
            /* baudrate */
            SPI_InitStruct.BaudRate = spiGetBaudRatePrescaler(_spiDeviceHandle->Config->MaxFreq);
            /* CPOL */
            if (_spiDeviceHandle->Config->Mode & SPI_CPOL)
            {
                SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_HIGH;
            }
            else
            {
                SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
            }
            /* CPHA */
            if (_spiDeviceHandle->Config->Mode & SPI_CPHA)
            {
                SPI_InitStruct.ClockPhase = LL_SPI_PHASE_2EDGE;
            }
            else
            {
                SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
            }
            /* MSB or LSB */
            if (_spiDeviceHandle->Config->Mode & SPI_MSB)
            {
                SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
            }
            else
            {
                SPI_InitStruct.BitOrder = LL_SPI_LSB_FIRST;
            }

            SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
            SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
            SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;

            SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
            SPI_InitStruct.CRCPoly = 10;

            LL_SPI_DeInit(_spiDeviceHandle->Device);

            LL_SPI_Init(_spiDeviceHandle->Device, &SPI_InitStruct);

            LL_SPI_SetStandard(_spiDeviceHandle->Device, LL_SPI_PROTOCOL_MOTOROLA);
#else
            SPI_HandleTypeDef *hspi = _spiDeviceHandle->Device;

            /* data_width */
            if (_spiDeviceHandle->Config->DataWidth <= 8)
            {
                hspi->Init.DataSize = SPI_DATASIZE_8BIT;
            }
            else if (_spiDeviceHandle->Config->DataWidth <= 16)
            {
                hspi->Init.DataSize = SPI_DATASIZE_16BIT;
            }
            else
            {
                return (uint8_t)SPI_ERROR;
            }

            /* baudrate */
            hspi->Init.BaudRatePrescaler = spiGetBaudRatePrescaler(_spiDeviceHandle->Config->MaxFreq);
            /* CPOL */
            if (_spiDeviceHandle->Config->Mode & SPI_CPOL)
            {
                hspi->Init.CLKPolarity = SPI_POLARITY_HIGH;
            }
            else
            {
                hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
            }

            /* CPHA */
            if (_spiDeviceHandle->Config->Mode & SPI_CPHA)
            {
                hspi->Init.CLKPhase = SPI_PHASE_2EDGE;
            }
            else
            {
                hspi->Init.CLKPhase = SPI_PHASE_1EDGE;
            }
            /* MSB or LSB */
            if (_spiDeviceHandle->Config->Mode & SPI_MSB)
            {
                hspi->Init.FirstBit = SPI_FIRSTBIT_MSB;
            }
            else
            {
                hspi->Init.FirstBit = SPI_FIRSTBIT_LSB;
            }

            hspi->Init.NSS = SPI_NSS_SOFT;
            hspi->Init.TIMode = SPI_TIMODE_DISABLE;
            hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
            hspi->Init.CRCPolynomial = 10;
            if (HAL_SPI_Init(hspi) != HAL_OK)
            {
                _Error_Handler(__FILE__, __LINE__);
            }
#endif
        }
    }

#if USER_EFFI == 1
    LL_SPI_Enable((SPI_TypeDef *)_spiDeviceHandle->Device);
#else
    __HAL_SPI_ENABLE((SPI_HandleTypeDef *)_spiDeviceHandle->Device);
#endif
    return SPI_OK;
}

/**
 * @func    spiSendThenSend
 * @brief   SPI��������
 * @param   _spiDeviceHandle SPI���
 * @param   sendBuf1 ��һ�����ݻ���
 * @param   sendLength1 ��һ�����ݴ�С
 * @param   sendBuf2 �ڶ������ݻ���
 * @param   sendLength2 �ڶ������ݴ�С
 * @note    
 * @retval  ������룬�ɹ���ʧ��
 */
uint8_t spiSendThenSend(spiDeviceHandle_t _spiDeviceHandle,
                        const void *sendBuf1,
                        uint32_t sendLength1,
                        const void *sendBuf2,
                        uint32_t sendLength2)
{
    spiMessage_t Message;
    uint32_t result;

    assert_param(_spiDeviceHandle);
    assert_param(_spiDeviceHandle->Bus);
    assert_param(_spiDeviceHandle->Config);
    assert_param(_spiDeviceHandle->Device);

    result = spiConfigure(_spiDeviceHandle);
    /* not the same owner as current, re-configure SPI bus */
    if (result != SPI_OK)
    {
        /* configure SPI bus failed */
        result = SPI_ERROR;
        goto __exit;
    }

    /* send data1 */
    Message.Flag = WRITE;
    Message.SendBuf = sendBuf1;
    Message.RecvBuf = NULL;
    Message.Length = sendLength1;
    Message.csTake = 1;
    Message.csRelease = 0;
    Message.TimeOut = TIMEOUT;

    result = SpixRW(_spiDeviceHandle, &Message);
    if (result != SPI_OK)
    {
        result = SPI_ERROR;
        goto __exit;
    }

    /* send data2 */
    Message.Flag = WRITE;
    Message.SendBuf = sendBuf2;
    Message.RecvBuf = NULL;
    Message.Length = sendLength2;
    Message.csTake = 0;
    Message.csRelease = 1;
    Message.TimeOut = TIMEOUT;

    result = SpixRW(_spiDeviceHandle, &Message);
    if (result != SPI_OK)
    {
        result = SPI_ERROR;
        goto __exit;
    }

    result = SPI_OK;

__exit:

    return result;
}

/**
 * @func    spiSendThenRecv
 * @brief   SPI�������ݲ���������
 * @param   _spiDeviceHandle SPI���
 * @param   sendBuf ���ͻ���
 * @param   sendLength �������ݴ�С
 * @param   recvBuf ���ջ���
 * @param   recvLength �������ݴ�С
 * @note    
 * @retval  ������룬�ɹ���ʧ��
 */
uint8_t spiSendThenRecv(spiDeviceHandle_t _spiDeviceHandle,
                        const void *sendBuf,
                        uint32_t sendLength,
                        void *recvBuf,
                        uint32_t recvLength)
{
    spiMessage_t Message;
    uint32_t result;

    assert_param(_spiDeviceHandle);
    assert_param(_spiDeviceHandle->Bus);
    assert_param(_spiDeviceHandle->Config);
    assert_param(_spiDeviceHandle->Device);

    result = spiConfigure(_spiDeviceHandle);
    /* not the same owner as current, re-configure SPI bus */
    if (result != SPI_OK)
    {
        /* configure SPI bus failed */
        result = SPI_ERROR;
        goto __exit;
    }

    /* send data1 */
    Message.Flag = WRITE;
    Message.SendBuf = sendBuf;
    Message.RecvBuf = NULL;
    Message.Length = sendLength;
    Message.csTake = 1;
    Message.csRelease = 0;
    Message.TimeOut = TIMEOUT;

    result = SpixRW(_spiDeviceHandle, &Message);
    if (result != SPI_OK)
    {
        result = SPI_ERROR;
        goto __exit;
    }

    /* send data2 */
    Message.Flag = READ;
    Message.SendBuf = NULL;
    Message.RecvBuf = recvBuf;
    Message.Length = recvLength;
    Message.csTake = 0;
    Message.csRelease = 1;
    Message.TimeOut = TIMEOUT;

    result = SpixRW(_spiDeviceHandle, &Message);
    if (result != SPI_OK)
    {
        result = SPI_ERROR;
        goto __exit;
    }

__exit:

    return result;
}

/**
 * @func    spiTransfer
 * @brief   SPI��������
 * @param   _spiDeviceHandle SPI���
 * @param   sendBuf ���ͻ���
 * @param   recvBuf ���ջ���
 * @param   Length ���ݴ�С
 * @note    
 * @retval  ������룬�ɹ���ʧ��
 */
uint8_t spiTransfer(spiDeviceHandle_t _spiDeviceHandle,
                    const void *sendBuf,
                    void *recvBuf,
                    uint32_t Length)
{
    spiMessage_t Message;
    uint32_t result;

    assert_param(_spiDeviceHandle);
    assert_param(_spiDeviceHandle->Bus);
    assert_param(_spiDeviceHandle->Config);
    assert_param(_spiDeviceHandle->Device);

    result = spiConfigure(_spiDeviceHandle);
    if (result != SPI_OK)
    {
        result = SPI_ERROR;
        goto __exit;
    }

    if (sendBuf == NULL)
    {
        Message.Flag = READ;
    }
    else if (recvBuf == NULL)
    {
        Message.Flag = WRITE;
    }
    else if (recvBuf == NULL && sendBuf == NULL)
    {
        Message.Flag = WRITE | READ;
    }
    else
    {
        result = SPI_ERROR;
        goto __exit;
    }
    /* send data1 */
    Message.SendBuf = sendBuf;
    Message.RecvBuf = recvBuf;
    Message.Length = Length;
    Message.csTake = 1;
    Message.csRelease = 1;
    Message.TimeOut = TIMEOUT;

    result = SpixRW(_spiDeviceHandle, &Message);
    if (result != SPI_OK)
    {
        result = SPI_ERROR;
        goto __exit;
    }

__exit:

    return result;
}
