/*
*********************************************************************************************************
*
*	ģ������ : SPI��������
*	�ļ����� : bsp_spi_bus.h
*	��    �� : V1.2
*	˵    �� : SPI���ߵײ��������ṩSPI���á��շ����ݡ����豸����SPI֧�֡�
*	�޸ļ�¼ :
*		�汾��  ����        ����    ˵��
*       v1.0    2014-10-24 armfly  �װ档������FLASH��TSC2046��VS1053��AD7705��ADS1256��SPI�豸������
*									���շ����ݵĺ������л��ܷ��ࡣ�������ͬ�ٶȵ��豸��Ĺ������⡣
*		V1.1	2015-02-25 armfly  Ӳ��SPIʱ��û�п���GPIOBʱ�ӣ��ѽ����
*		V1.2	2015-07-23 armfly  �޸� bsp_SPI_Init() ���������ӿ���SPIʱ�ӵ���䡣�淶Ӳ��SPI�����SPI�ĺ궨��.
*
*	Copyright (C), 2015-2016, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp_spi_bus.h"
/*
*********************************************************************************************************
*	�� �� ��: bsp_InitSPIBus
*	����˵��: ����SPI���ߡ� ֻ���� SCK�� MOSI�� MISO���ߵ����á�������ƬѡCS��Ҳ����������оƬ���е�INT��BUSY��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitSPIBus(void)
{
#ifdef SOFT_SPI		/* ���SPI */
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* ��GPIOʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_SCK | RCC_MOSI | RCC_MISO, ENABLE);

	/* ���ü����������IO */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ���������費ʹ�� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	/* IO������ٶ� */

	GPIO_InitStructure.GPIO_Pin = PIN_SCK;
	GPIO_Init(PORT_SCK, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PIN_MOSI;
	GPIO_Init(PORT_MOSI, &GPIO_InitStructure);

	/* ����GPIOΪ��������ģʽ(ʵ����CPU��λ���������״̬) */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* �������������� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	/* IO������ٶ� */

	GPIO_InitStructure.GPIO_Pin = PIN_MISO;
	GPIO_Init(PORT_MISO, &GPIO_InitStructure);
#endif

#ifdef HARD_SPI
	/* Ӳ��SPI */
	GPIO_InitTypeDef  GPIO_InitStructure;

	/* ����GPIOʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);


	/* ���� SCK, MISO �� MOSI Ϊ���ù��� */
	//GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI3);
	//GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI3);
	//GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI3);
	/* ���� SCK, MISO �� MOSI Ϊ���ù��� */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* ��SPIʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	bsp_SPI_Init(SPI_Direction_2Lines_FullDuplex | SPI_Mode_Master | SPI_DataSize_8b
		| SPI_CPOL_Low | SPI_CPHA_1Edge | SPI_NSS_Soft | SPI_BaudRatePrescaler_64 | SPI_FirstBit_MSB);	
	
	/* Activate the SPI mode (Reset I2SMOD bit in I2SCFGR register) */
	SPI_HARD->I2SCFGR &= SPI_Mode_Select;		/* ѡ��SPIģʽ������I2Sģʽ */

	/*---------------------------- SPIx CRCPOLY Configuration --------------------*/
	/* Write to SPIx CRCPOLY */
	SPI_HARD->CRCPR = 7;		/* һ�㲻�� */


	SPI_Cmd(SPI_HARD, DISABLE);			/* �Ƚ�ֹSPI  */

	SPI_Cmd(SPI_HARD, ENABLE);			/* ʹ��SPI  */
#endif	
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_SPI_Init
*	����˵��: ����STM32�ڲ�SPIӲ���Ĺ���ģʽ�� �򻯿⺯�������ִ��Ч�ʡ� ������SPI�ӿڼ��л���
*	��    ��: _cr1 �Ĵ���ֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
#ifdef HARD_SPI		/* Ӳ��SPI */
void bsp_SPI_Init(uint16_t _cr1)
{
	SPI_HARD->CR1 = ((SPI_HARD->CR1 & CR1_CLEAR_Mask) | _cr1);
	  
	//SPI_Cmd(SPI_HARD, DISABLE);			/* �Ƚ�ֹSPI  */	    
    SPI_HARD->CR1 &= CR1_SPE_Reset;	/* Disable the selected SPI peripheral */

	//SPI_Cmd(SPI_HARD, ENABLE);			/* ʹ��SPI  */		    
    SPI_HARD->CR1 |= CR1_SPE_Set;	  /* Enable the selected SPI peripheral */
}
#endif

#ifdef SOFT_SPI		/* ���SPI */
/*
*********************************************************************************************************
*	�� �� ��: bsp_SpiDelay
*	����˵��: ʱ���ӳ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_spiDelay(void)
{
#if 1
	uint32_t i;

	/*
		�ӳ�5ʱ�� F407 (168MHz��Ƶ�� GPIOģ�⣬ʵ�� SCK ���� = 480ns (��Լ2M)
	*/
	for (i = 0; i < 5; i++);
#else
	/*
		������ӳ���䣬 F407 (168MHz��Ƶ�� GPIOģ�⣬ʵ�� SCK ���� = 200ns (��Լ5M)
	*/
#endif
}
#endif

/*
*********************************************************************************************************
*	�� �� ��: bsp_spiWrite0
*	����˵��: ��SPI���߷���һ���ֽڡ�SCK�����زɼ�����, SCK����ʱΪ�͵�ƽ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_spiWrite0(uint8_t _ucByte)
{
#ifdef SOFT_SPI		/* ���SPI */
	uint8_t i;

	for(i = 0; i < 8; i++)
	{
		if (_ucByte & 0x80)
		{
			MOSI_1();
		}
		else
		{
			MOSI_0();
		}
		bsp_spiDelay();
		SCK_1();
		_ucByte <<= 1;
		bsp_spiDelay();
		SCK_0();
	}
	bsp_spiDelay();
#endif

#ifdef HARD_SPI		/* Ӳ��SPI */
	/* �ȴ����ͻ������� */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* ����һ���ֽ� */
	SPI_I2S_SendData(SPI1, _ucByte);

	/* �ȴ����ݽ������ */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* ��ȡ���յ������� */
	SPI_I2S_ReceiveData(SPI1);
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_spiRead0
*	����˵��: ��SPI���߽���8��bit���ݡ� SCK�����زɼ�����, SCK����ʱΪ�͵�ƽ��
*	��    ��: ��
*	�� �� ֵ: ����������
*********************************************************************************************************
*/
uint8_t bsp_spiRead0(void)
{
#ifdef SOFT_SPI		/* ���SPI */
	uint8_t i;
	uint8_t read = 0;

	for (i = 0; i < 8; i++)
	{
		read = read<<1;

		if (MISO_IS_HIGH())
		{
			read++;
		}
		SCK_1();
		bsp_spiDelay();
		SCK_0();
		bsp_spiDelay();
	}
	return read;
#endif

#ifdef HARD_SPI		/* Ӳ��SPI */
	uint8_t read;

	/* �ȴ����ͻ������� */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* ����һ���ֽ� */
	SPI_I2S_SendData(SPI1, 0);

	/* �ȴ����ݽ������ */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* ��ȡ���յ������� */
	read = SPI_I2S_ReceiveData(SPI1);

	/* ���ض��������� */
	return read;
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_spiWrite1
*	����˵��: ��SPI���߷���һ���ֽڡ�  SCK�����زɼ�����, SCK����ʱΪ�ߵ�ƽ
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_spiWrite1(uint8_t _ucByte)
{
#ifdef SOFT_SPI		/* ���SPI */
	uint8_t i;

	for(i = 0; i < 8; i++)
	{
		if (_ucByte & 0x80)
		{
			MOSI_1();
		}
		else
		{
			MOSI_0();
		}
		SCK_0();
		_ucByte <<= 1;
		bsp_spiDelay();
		SCK_1();				/* SCK�����زɼ�����, SCK����ʱΪ�ߵ�ƽ */
		bsp_spiDelay();
	}
#endif

#ifdef HARD_SPI		/* Ӳ��SPI */
	/* �ȴ����ͻ������� */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* ����һ���ֽ� */
	SPI_I2S_SendData(SPI1, _ucByte);

	/* �ȴ����ݽ������ */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* ��ȡ���յ������� */
	SPI_I2S_ReceiveData(SPI1);
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_spiRead1
*	����˵��: ��SPI���߽���8��bit���ݡ�  SCK�����زɼ�����, SCK����ʱΪ�ߵ�ƽ
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint8_t bsp_spiRead1(void)
{
#ifdef SOFT_SPI		/* ���SPI */
	uint8_t i;
	uint8_t read = 0;

	for (i = 0; i < 8; i++)
	{
		SCK_0();
		bsp_spiDelay();
		read = read<<1;
		if (MISO_IS_HIGH())
		{
			read++;
		}
		SCK_1();
		bsp_spiDelay();
	}
	return read;
#endif

#ifdef HARD_SPI		/* Ӳ��SPI */
	uint8_t read;

	/* �ȴ����ͻ������� */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* ����һ���ֽ� */
	SPI_I2S_SendData(SPI1, 0);

	/* �ȴ����ݽ������ */
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	/* ��ȡ���յ������� */
	read = SPI_I2S_ReceiveData(SPI1);

	/* ���ض��������� */
	return read;
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_SpiBusEnter
*	����˵��: ռ��SPI����
*	��    ��: ��
*	�� �� ֵ: 0 ��ʾ��æ  1��ʾæ
*********************************************************************************************************
*/
void bsp_SpiBusEnter(void)
{
	g_spi_busy = 1;
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_SpiBusExit
*	����˵��: �ͷ�ռ�õ�SPI����
*	��    ��: ��
*	�� �� ֵ: 0 ��ʾ��æ  1��ʾæ
*********************************************************************************************************
*/
void bsp_SpiBusExit(void)
{
	g_spi_busy = 0;
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_SpiBusBusy
*	����˵��: �ж�SPI����æ�������Ǽ������SPIоƬ��Ƭѡ�ź��Ƿ�Ϊ1
*	��    ��: ��
*	�� �� ֵ: 0 ��ʾ��æ  1��ʾæ
*********************************************************************************************************
*/
uint8_t bsp_SpiBusBusy(void)
{
	return g_spi_busy;
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_SetSpiSck
*	����˵��: �������ģʽ������SCK GPIO��״̬���ں���CS=0֮ǰ�����ã����ڲ�ͬ�����SPI�豸���л���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
#ifdef SOFT_SPI		/* ���SPI */
void bsp_SetSpiSck(uint8_t _data)
{
	if (_data == 0)
	{
		SCK_0();
	}
	else
	{
		SCK_1();
	}
}
#endif



uint8_t spiConfigure(spiDeviceHandle_t spiDeviceHandle)
{
    rt_err_t result;

    RT_ASSERT(device != RT_NULL);

    /* set configuration */
    device->config.data_width = cfg->data_width;
    device->config.mode       = cfg->mode & RT_SPI_MODE_MASK ;
    device->config.max_hz     = cfg->max_hz ;

    if (device->bus != RT_NULL)
    {
        result = rt_mutex_take(&(device->bus->lock), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            if (device->bus->owner == device)
            {
                device->bus->ops->configure(device, &device->config);
            }

            /* release lock */
            rt_mutex_release(&(device->bus->lock));
        }
    }

    return RT_EOK;
}

uint32_t spiSendThenSend(spiDeviceHandle_t _spiDeviceHandle,
                         const void           *sendBuf1,
                         uint32_t             sendLength1,
                         const void           *sendBuf2,
                         uint32_t             sendLength2)
{
    spiMessage_t Message;
    uint32_t result;

    assert_param(_spiDeviceHandle);
    assert_param(_spiDeviceHandle->bus);
    assert_param(_spiDeviceHandle->Config);
    assert_param(_spiDeviceHandle->Device);

    if (_spiDeviceHandle->Bus->Owner != _spiDeviceHandle->Device)
    {
        /* not the same owner as current, re-configure SPI bus */
        result = _spiDeviceHandle->Bus->Ops->configure(_spiDeviceHandle);
        if (result == SPI_OK)
        {
            /* set SPI bus owner */
            _spiDeviceHandle->Bus->Owner = _spiDeviceHandle->Device;
        }
        else
        {
            /* configure SPI bus failed */
            result = SPI_ERROR;
            goto __exit;
        }
    }

    /* send data1 */
    Message.send_buf   = sendBuf1;
    Message.recv_buf   = NULL;
    Message.length     = sendLength1;
    Message.cs_take    = 1;
    Message.cs_release = 0;

    result = _spiDeviceHandle->Bus->Ops->xfer(_spiDeviceHandle);
    if (result == 0)
    {
        result = SPI_ERROR;
        goto __exit;
    }

    /* send data2 */
    Message.send_buf   = sendBuf2;
    Message.recv_buf   = NULL;
    Message.length     = sendLength2;
    Message.cs_take    = 0;
    Message.cs_release = 1;

    result = device->bus->ops->xfer(device, &message);
    if (result == 0)
    {
        result = SPI_ERROR;
        goto __exit;
    }

    result = SPI_OK;

__exit:

    return result;
}

uint32_t spiSendThenRecv(spiDeviceHandle_t _spiDeviceHandle,
                               const void           *sendBuf,
                               uint32_t             sendLength,
                               void                 *recvBuf,
                               uint32_t             recvLength)
{
    spiMessage_t Message;
    uint32_t result;

    assert_param(_spiDeviceHandle);
    assert_param(_spiDeviceHandle->bus);
    assert_param(_spiDeviceHandle->Config);
    assert_param(_spiDeviceHandle->Device);

    if (_spiDeviceHandle->Bus->Owner != _spiDeviceHandle->Device)
    {
        /* not the same owner as current, re-configure SPI bus */
        result = _spiDeviceHandle->Bus->Ops->configure(_spiDeviceHandle);
        if (result == SPI_OK)
        {
            /* set SPI bus owner */
            _spiDeviceHandle->Bus->Owner = _spiDeviceHandle->Device;
        }
        else
        {
            /* configure SPI bus failed */
            result = SPI_ERROR;
            goto __exit;
        }
    }

    /* send data1 */
    Message.send_buf   = sendBuf;
    Message.recv_buf   = NULL;
    Message.length     = sendLength;
    Message.cs_take    = 1;
    Message.cs_release = 0;

    result = _spiDeviceHandle->Bus->Ops->xfer(_spiDeviceHandle);
    if (result == 0)
    {
        result = SPI_ERROR;
        goto __exit;
    }

    /* send data2 */
    Message.send_buf   = NULL;
    Message.recv_buf   = recvBuf;
    Message.length     = recvLength;
    Message.cs_take    = 0;
    Message.cs_release = 1;

    result = device->bus->ops->xfer(device, &message);
    if (result == 0)
    {
        result = SPI_ERROR;
        goto __exit;
    }

    result = SPI_OK;

__exit:

    return result;
}

uint32_t spiTransfer(spiDeviceHandle_t _spiDeviceHandle,
                          const void           *sendBuf,
                          void                 *recvBuf,
                          uint32_t             length)
{
    spiMessage_t Message;
    uint32_t result;

    assert_param(_spiDeviceHandle);
    assert_param(_spiDeviceHandle->bus);
    assert_param(_spiDeviceHandle->Config);
    assert_param(_spiDeviceHandle->Device);

    if (_spiDeviceHandle->Bus->Owner != _spiDeviceHandle->Device)
    {
        /* not the same owner as current, re-configure SPI bus */
        result = _spiDeviceHandle->Bus->Ops->configure(_spiDeviceHandle);
        if (result == SPI_OK)
        {
            /* set SPI bus owner */
            _spiDeviceHandle->Bus->Owner = _spiDeviceHandle->Device;
        }
        else
        {
            /* configure SPI bus failed */
            result = SPI_ERROR;
            goto __exit;
        }
    }

    /* send data1 */
    Message.send_buf   = sendBuf;
    Message.recv_buf   = recvBuf;
    Message.length     = sendLength;
    Message.cs_take    = 1;
    Message.cs_release = 0;

    result = _spiDeviceHandle->Bus->Ops->xfer(_spiDeviceHandle);
    if (result == 0)
    {
        result = SPI_ERROR;
        goto __exit;
    }

    result = SPI_OK;

__exit:

    return result;
}


/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
