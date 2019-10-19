/**
 ******************************************************************************
 * @file      bsp_spi_bus.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-10-03
 * @brief     SPI总线实现程序，实现SPI总线驱动框架
 * @note      
 * @History
 * Date           Author    version    		Notes
 * 2018-10-04       ZSY     V1.0.0      first version.
 */
/* Includes ------------------------------------------------------------------*/
#include "bsp_spi_bus.h"

/* 若使用LL库，请定义USER_EFFI为1 */
#ifndef USING_EFFI
#define USING_EFFI 1
#endif

#if (USING_EFFI == 1)
#include "stm32f1xx_ll_spi.h"
#endif

#ifndef NULL
#define NULL 0
#endif

#define READ        (1 << 0)
#define WRITE       (1 << 1)
#define TIMEOUT 200

static uint16_t spi_bus_id = 0;
static uint8_t spi_bus_busy = SPI_BUS_NOBUSY;


/**
 * @func    spi_bus_take
 * @brief   占用SPI总线
 * @note    
 * @retval  无
 */
void spi_bus_take(void)
{
    spi_bus_busy = SPI_BUS_BUSY;
}

/**
 * @func    spi_bus_release
 * @brief   释放占用的SPI总线
 * @note    
 * @retval  无
 */
void spi_bus_release(void)
{
    spi_bus_busy = SPI_BUS_NOBUSY;
}

/**
 * @func    spi_get_bus_busy
 * @brief   判断SPI总线忙。
 * @note    
 * @retval  无
 */
uint8_t spi_get_bus_busy(void)
{
    return spi_bus_busy;
}

/**
 * @func    spix_wr
 * @brief   SPI读写数据
 * @param   _spi_device_handle SPI句柄
 * @param   *message 发送的消息
 * @note    只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
 * @retval  无
 */
uint32_t spix_wr(spi_device_handle_t _spi_device_handle, spi_message_t *message)
{
    __IO uint8_t Retry = 0, result = HAL_OK;

    if (message->cs_take == 1)
    {
        _spi_device_handle->bus->ops->cs_take();
        spi_bus_take();
    }
#if USING_EFFI == 1
    uint16_t i = 0;
    while (i < message->length)
    {
        /* 检查指定的SPI标志位设置与否:发送缓存空标志位 */
        while (LL_SPI_IsActiveFlag_TXE((SPI_TypeDef *)_spi_device_handle->device) == RESET)
        {
            Retry++;
            if (Retry > message->timeout)
            {
                result = (uint8_t)SPI_ERROR;
                goto __exit;
            }
        }
        
        if (message->flag & WRITE)
        {
            LL_SPI_TransmitData8((SPI_TypeDef *)_spi_device_handle->device, *((uint8_t *)message->send_buf + i));
        }
        else
        {
            LL_SPI_TransmitData8((SPI_TypeDef *)_spi_device_handle->device, 0xff);
        }
            

        Retry = 0;
        /* 检查指定的SPI标志位设置与否:发送缓存空标志位 */
        while (LL_SPI_IsActiveFlag_RXNE((SPI_TypeDef *)_spi_device_handle->device) == RESET)
        {
            Retry++;
            if (Retry > message->timeout)
            {
                result = (uint8_t)SPI_ERROR;
                goto __exit;
            }
        }

        if (message->flag & READ)
        {
            *((uint8_t *)message->recv_buf + i) = LL_SPI_ReceiveData8((SPI_TypeDef *)_spi_device_handle->device);
        }
        else
        {
            LL_SPI_ReceiveData8((SPI_TypeDef *)_spi_device_handle->device);
        }

        i++;
    }
#else
    SPI_HandleTypeDef *hSPI = (SPI_HandleTypeDef *)_spi_device_handle->device;

    if (message->flag & READ && message->flag & WRITE)
    {
        result = HAL_SPI_TransmitReceive(hSPI, (uint8_t *)message->send_buf, message->recv_buf, message->length, message->timeout);
        if (result != HAL_OK)
        {
            result = (uint8_t)SPI_ERROR;
            goto __exit;
        }
    }
    else if (message->flag & WRITE)
    {
        result = HAL_SPI_Transmit(hSPI, (uint8_t *)message->send_buf, message->length, message->timeout);
        if (result != HAL_OK)
        {
            result = (uint8_t)SPI_ERROR;
            goto __exit;
        }
    }
    else if (message->flag & READ)
    {
        result = HAL_SPI_Receive(hSPI, message->recv_buf, message->length, message->timeout);
        if (result != HAL_OK)
        {
            result = (uint8_t)SPI_ERROR;
            goto __exit;
        }
    }
#endif
__exit:
    if (message->cs_release == 1)
    {
        _spi_device_handle->bus->ops->cs_reslease();
        spi_bus_release();
    }
    return result;
}

/**
 * @func    spi_get_baud_rate_prescaler
 * @brief   获取SPI的波特率
 * @param   maxHz 最大时钟频率
 * @note    
 * @retval  SPI_BaudRatePrescaler 分频系数
 */
static inline uint32_t spi_get_baud_rate_prescaler(uint32_t maxHz)
{
    uint16_t SPI_baud_rate_prescaler;
#if USING_EFFI == 1
    /* STM32F40x SPI MAX 42Mhz  SPI1 max 84MHz*/
    if (maxHz >= SystemCoreClock / 2 && SystemCoreClock / 2 <= 36000000)
    {
        SPI_baud_rate_prescaler = LL_SPI_BAUDRATEPRESCALER_DIV2;
    }
    else if (maxHz >= SystemCoreClock / 4)
    {
        SPI_baud_rate_prescaler = LL_SPI_BAUDRATEPRESCALER_DIV4;
    }
    else if (maxHz >= SystemCoreClock / 8)
    {
        SPI_baud_rate_prescaler = LL_SPI_BAUDRATEPRESCALER_DIV8;
    }
    else if (maxHz >= SystemCoreClock / 16)
    {
        SPI_baud_rate_prescaler = LL_SPI_BAUDRATEPRESCALER_DIV16;
    }
    else if (maxHz >= SystemCoreClock / 32)
    {
        SPI_baud_rate_prescaler = LL_SPI_BAUDRATEPRESCALER_DIV32;
    }
    else if (maxHz >= SystemCoreClock / 64)
    {
        SPI_baud_rate_prescaler = LL_SPI_BAUDRATEPRESCALER_DIV64;
    }
    else if (maxHz >= SystemCoreClock / 128)
    {
        SPI_baud_rate_prescaler = LL_SPI_BAUDRATEPRESCALER_DIV128;
    }
    else
    {
        /* min prescaler 256 */
        SPI_baud_rate_prescaler = LL_SPI_BAUDRATEPRESCALER_DIV256;
    }
#else
    /* STM32F40x SPI MAX 42Mhz  SPI1 max 84MHz*/
    if (maxHz >= SystemCoreClock / 2 && SystemCoreClock / 2 <= 36000000)
    {
        SPI_baud_rate_prescaler = SPI_BAUDRATEPRESCALER_2;
    }
    else if (maxHz >= SystemCoreClock / 4)
    {
        SPI_baud_rate_prescaler = SPI_BAUDRATEPRESCALER_4;
    }
    else if (maxHz >= SystemCoreClock / 8)
    {
        SPI_baud_rate_prescaler = SPI_BAUDRATEPRESCALER_8;
    }
    else if (maxHz >= SystemCoreClock / 16)
    {
        SPI_baud_rate_prescaler = SPI_BAUDRATEPRESCALER_16;
    }
    else if (maxHz >= SystemCoreClock / 32)
    {
        SPI_baud_rate_prescaler = SPI_BAUDRATEPRESCALER_32;
    }
    else if (maxHz >= SystemCoreClock / 64)
    {
        SPI_baud_rate_prescaler = SPI_BAUDRATEPRESCALER_64;
    }
    else if (maxHz >= SystemCoreClock / 128)
    {
        SPI_baud_rate_prescaler = SPI_BAUDRATEPRESCALER_128;
    }
    else
    {
        /* min prescaler 256 */
        SPI_baud_rate_prescaler = SPI_BAUDRATEPRESCALER_256;
    }
#endif
    return SPI_baud_rate_prescaler;
}

/**
 * @func    spi_configure
 * @brief   SPI配置
 * @param   _spi_device_handle SPI句柄
 * @note    
 * @retval  错误代码，成功或失败
 */
uint8_t spi_configure(spi_device_handle_t _spi_device_handle)
{
    assert_param(_spi_device_handle);
    assert_param(_spi_device_handle->bus);
    assert_param(_spi_device_handle->config);
    assert_param(_spi_device_handle->device);

    if (_spi_device_handle->bus != NULL)
    {
        if (_spi_device_handle->bus->id == spi_bus_id)
        {
            /* set SPI bus owner */
            spi_bus_id = _spi_device_handle->bus->id;

#if USING_EFFI == 1
            LL_SPI_InitTypeDef SPI_InitStruct;

            LL_SPI_StructInit(&SPI_InitStruct);

            /* data_width */
            if (_spi_device_handle->config->data_width <= 8)
            {
                SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
            }
            else if (_spi_device_handle->config->data_width <= 16)
            {
                SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_16BIT;
            }
            else
            {
                return (uint8_t)SPI_ERROR;
            }
            /* baudrate */
            SPI_InitStruct.BaudRate = spi_get_baud_rate_prescaler(_spi_device_handle->config->max_freq);
            /* CPOL */
            if (_spi_device_handle->config->mode & SPI_CPOL)
            {
                SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_HIGH;
            }
            else
            {
                SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
            }
            /* CPHA */
            if (_spi_device_handle->config->mode & SPI_CPHA)
            {
                SPI_InitStruct.ClockPhase = LL_SPI_PHASE_2EDGE;
            }
            else
            {
                SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
            }
            /* MSB or LSB */
            if (_spi_device_handle->config->mode & SPI_MSB)
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

            LL_SPI_DeInit(_spi_device_handle->device);

            LL_SPI_Init(_spi_device_handle->device, &SPI_InitStruct);

            //LL_SPI_SetStandard(_spi_device_handle->device, LL_SPI_PROTOCOL_MOTOROLA);
#else
            SPI_HandleTypeDef *hspi = _spi_device_handle->device;

            /* data_width */
            if (_spi_device_handle->config->data_width <= 8)
            {
                hspi->Init.DataSize = SPI_DATASIZE_8BIT;
            }
            else if (_spi_device_handle->config->data_width <= 16)
            {
                hspi->Init.DataSize = SPI_DATASIZE_16BIT;
            }
            else
            {
                return (uint8_t)SPI_ERROR;
            }

            /* baudrate */
            hspi->Init.BaudRatePrescaler = spi_get_baud_rate_prescaler(_spi_device_handle->config->max_freq);
            /* CPOL */
            if (_spi_device_handle->config->mode & SPI_CPOL)
            {
                hspi->Init.CLKPolarity = SPI_POLARITY_HIGH;
            }
            else
            {
                hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
            }

            /* CPHA */
            if (_spi_device_handle->config->mode & SPI_CPHA)
            {
                hspi->Init.CLKPhase = SPI_PHASE_2EDGE;
            }
            else
            {
                hspi->Init.CLKPhase = SPI_PHASE_1EDGE;
            }
            /* MSB or LSB */
            if (_spi_device_handle->config->mode & SPI_MSB)
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
                //_Error_Handler(__FILE__, __LINE__);
            }
#endif
        }
    }

#if USING_EFFI == 1
    LL_SPI_Enable((SPI_TypeDef *)_spi_device_handle->device);
#else
    __HAL_SPI_ENABLE((SPI_HandleTypeDef *)_spi_device_handle->device);
#endif
    return SPI_OK;
}

/**
 * @func    spi_send_then_send
 * @brief   SPI发送数据
 * @param   _spi_device_handle SPI句柄
 * @param   send_buf1 第一个数据缓存
 * @param   send_length1 第一个数据大小
 * @param   send_buf2 第二个数据缓存
 * @param   send_length2 第二个数据大小
 * @note    
 * @retval  错误代码，成功或失败
 */
uint8_t spi_send_then_send(spi_device_handle_t _spi_device_handle,
                        const void *send_buf1,
                        uint32_t send_length1,
                        const void *send_buf2,
                        uint32_t send_length2)
{
    spi_message_t message;
    uint32_t result;

    assert_param(_spi_device_handle);
    assert_param(_spi_device_handle->bus);
    assert_param(_spi_device_handle->config);
    assert_param(_spi_device_handle->device);

    result = spi_configure(_spi_device_handle);
    /* not the same owner as current, re-configure SPI bus */
    if (result != SPI_OK)
    {
        /* configure SPI bus failed */
        result = SPI_ERROR;
        goto __exit;
    }

    /* send data1 */
    message.flag = WRITE;
    message.send_buf = send_buf1;
    message.recv_buf = NULL;
    message.length = send_length1;
    message.cs_take = 1;
    message.cs_release = 0;
    message.timeout = TIMEOUT;

    result = spix_wr(_spi_device_handle, &message);
    if (result != SPI_OK)
    {
        result = SPI_ERROR;
        goto __exit;
    }

    /* send data2 */
    message.flag = WRITE;
    message.send_buf = send_buf2;
    message.recv_buf = NULL;
    message.length = send_length2;
    message.cs_take = 0;
    message.cs_release = 1;
    message.timeout = TIMEOUT;

    result = spix_wr(_spi_device_handle, &message);
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
 * @func    spi_send_then_recv
 * @brief   SPI发送数据并接收数据
 * @param   _spi_device_handle SPI句柄
 * @param   send_buf 发送缓存
 * @param   send_length 发送数据大小
 * @param   recv_buf 接收缓存
 * @param   recv_length 接收数据大小
 * @note    
 * @retval  错误代码，成功或失败
 */
uint8_t spi_send_then_recv(spi_device_handle_t _spi_device_handle,
                        const void *send_buf,
                        uint32_t send_length,
                        void *recv_buf,
                        uint32_t recv_length)
{
    spi_message_t message;
    uint32_t result;

    assert_param(_spi_device_handle);
    assert_param(_spi_device_handle->bus);
    assert_param(_spi_device_handle->config);
    assert_param(_spi_device_handle->device);

    result = spi_configure(_spi_device_handle);
    /* not the same owner as current, re-configure SPI bus */
    if (result != SPI_OK)
    {
        /* configure SPI bus failed */
        result = SPI_ERROR;
        goto __exit;
    }

    /* send data1 */
    message.flag = WRITE;
    message.send_buf = send_buf;
    message.recv_buf = NULL;
    message.length = send_length;
    message.cs_take = 1;
    message.cs_release = 0;
    message.timeout = TIMEOUT;

    result = spix_wr(_spi_device_handle, &message);
    if (result != SPI_OK)
    {
        result = SPI_ERROR;
        goto __exit;
    }

    /* send data2 */
    message.flag = READ;
    message.send_buf = NULL;
    message.recv_buf = recv_buf;
    message.length = recv_length;
    message.cs_take = 0;
    message.cs_release = 1;
    message.timeout = TIMEOUT;

    result = spix_wr(_spi_device_handle, &message);
    if (result != SPI_OK)
    {
        result = SPI_ERROR;
        goto __exit;
    }

__exit:

    return result;
}

/**
 * @func    spi_transfer
 * @brief   SPI传输数据
 * @param   _spi_device_handle SPI句柄
 * @param   send_buf 发送缓存
 * @param   recv_buf 接收缓存
 * @param   length 数据大小
 * @note    
 * @retval  错误代码，成功或失败
 */
uint8_t spi_transfer(spi_device_handle_t _spi_device_handle,
                    const void *send_buf,
                    void *recv_buf,
                    uint32_t length)
{
    spi_message_t message;
    uint32_t result;

    assert_param(_spi_device_handle);
    assert_param(_spi_device_handle->bus);
    assert_param(_spi_device_handle->config);
    assert_param(_spi_device_handle->device);

    result = spi_configure(_spi_device_handle);
    if (result != SPI_OK)
    {
        result = SPI_ERROR;
        goto __exit;
    }

    if (send_buf == NULL)
    {
        message.flag = READ;
    }
    else if (recv_buf == NULL)
    {
        message.flag = WRITE;
    }
    else if (recv_buf == NULL && send_buf == NULL)
    {
        message.flag = WRITE | READ;
    }
    else
    {
        result = SPI_ERROR;
        goto __exit;
    }
    /* send data1 */
    message.send_buf = send_buf;
    message.recv_buf = recv_buf;
    message.length = length;
    message.cs_take = 1;
    message.cs_release = 1;
    message.timeout = TIMEOUT;

    result = spix_wr(_spi_device_handle, &message);
    if (result != SPI_OK)
    {
        result = SPI_ERROR;
        goto __exit;
    }

__exit:

    return result;
}
