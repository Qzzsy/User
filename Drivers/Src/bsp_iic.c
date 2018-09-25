/**
 ******************************************************************************
 * @file      iic.c
 * @author    �Ž�����С��
 * @version   V1.0.5
 * @date      2018-06-24
 * @brief     �ļ��ڰ���һЩiic���������ã�iic�Ļ�����д�����Լ�ͨ��ʱ��
 * @History
 * Date           Author    version    		Notes
 * 2017-11-01     ZSY       V1.0.0          first version.
 * 2017-11-02     ZSY       V1.0.1          �޸���Ӧ���źŵķ�����ʽ�������˶�Ӧ�ĺ�
                                            ���壬��iic_ack��iic_no_ack�����static
                                            �����η��������ڱ��ļ���ʹ��
 * 2018-01-09     ZSY       V1.0.2          �Ű��ʽ������.
 * 2018-01-26     ZSY       V1.0.3          ���˽�к͹��к궨��.
 * 2018-06-20     ZSY       V1.0.4          ��߼�����.
 * 2018-06-24     ZSY       V1.0.5          ����������ܣ�ʵ�ֲ�ͬ��IIC�豸����ͳһ�Ľӿ�ʵ�����ݴ���.
 */
	
/* Includes ------------------------------------------------------------------*/
#include "bsp_iic.h"

#define WRITE_ADDR              (1 << 0)
#define WRITE_DATA              (1 << 1)

/* User functions ------------------------------------------------------------*/

/**
 * @func    IIC_Start
 * @brief   IIC start transport
                IIC device is about to start a new transfer process
 * @retval  none
 */
static void IIC_Start(IIC_Handle_t iicHandle)  
{ 
    /* Set GPIO to Output mode */
    iicHandle->Ops->Set_SDA_DIR(SDA_OUT);
    
    /* IIC produce a start signal */
    iicHandle->Ops->Set_SDA(SET);
    iicHandle->Ops->Set_SCL(SET);
    iicHandle->Ops->uDelay(iicHandle->Ops->Delay_us);
    iicHandle->Ops->Set_SDA(RESET);
    iicHandle->Ops->uDelay(iicHandle->Ops->Delay_us);
    iicHandle->Ops->Set_SCL(RESET);
} 

/**
 * @func    IIC_Stop
 * @brief   iic stop transport
                IIC device is about to stop the current transport process
 * @retval  none
 */
static void IIC_Stop(IIC_Handle_t iicHandle)  
{ 
    /* Set GPIO to Output mode */
    iicHandle->Ops->Set_SDA_DIR(SDA_OUT);
    
    /* iic produce a stop signal */
    iicHandle->Ops->Set_SCL(RESET);
    iicHandle->Ops->Set_SDA(RESET);
    iicHandle->Ops->uDelay(iicHandle->Ops->Delay_us);
    iicHandle->Ops->Set_SCL(SET);
    iicHandle->Ops->uDelay(iicHandle->Ops->Delay_us);
    iicHandle->Ops->Set_SDA(SET);
}

/**
 * @func    IIC_Ack
 * @brief   IIC master produce a ack signal
 * @retval  none
 */
static void IIC_Ack(IIC_Handle_t iicHandle) 
{ 
    iicHandle->Ops->Set_SCL(RESET);
    
    /* Set GPIO to Output mode */
    iicHandle->Ops->Set_SDA_DIR(SDA_OUT);
    
    iicHandle->Ops->Set_SDA(RESET);
    iicHandle->Ops->uDelay(iicHandle->Ops->Delay_us);
    iicHandle->Ops->Set_SCL(SET);
    iicHandle->Ops->uDelay(iicHandle->Ops->Delay_us);
    iicHandle->Ops->Set_SCL(RESET);
} 

/**
 * @func    IIC_NoAck
 * @brief   IIC slave don't produce a ack signal??transport will stop
 * @retval  none
 */
static void IIC_NoAck(IIC_Handle_t iicHandle)
{
    iicHandle->Ops->Set_SCL(RESET);
    
    /* Set GPIO to Output mode */
    iicHandle->Ops->Set_SDA_DIR(SDA_OUT);
    
    /* IIC produce no ack signal */
    iicHandle->Ops->Set_SDA(SET);
    iicHandle->Ops->uDelay(iicHandle->Ops->Delay_us);
    iicHandle->Ops->Set_SCL(SET);
    iicHandle->Ops->uDelay(iicHandle->Ops->Delay_us);
    iicHandle->Ops->Set_SCL(RESET);
}

/**
 * @func    IIC_WaitAck
 * @brief   IIC master waiting a ack signal from slave
                The CPU produces a clock and reads the device's ACK signal
 * @retval  return IIC_OPER_OK for correct response, IIC_OPER_FAILT for no device response
 */
static uint8_t IIC_WaitAck(IIC_Handle_t iicHandle)
{ 
    __IO uint16_t time = 0;
    
    /* Set GPIO to input mode */
    iicHandle->Ops->Set_SDA_DIR(SDA_IN);
    
    iicHandle->Ops->Set_SDA(SET);
    iicHandle->Ops->uDelay(iicHandle->Ops->Delay_us);
    iicHandle->Ops->Set_SCL(SET);
    iicHandle->Ops->uDelay(iicHandle->Ops->Delay_us);
    
    /* waiting slave ack signal	*/
    while (iicHandle->Ops->Get_SDA())
    {
        time++;
        
        /* ��ʱ��⣬��ֹ���� */
        if (time > (iicHandle->Timeout))
        {
            IIC_Stop(iicHandle);
            
            /* ��ʱ��ζ��ʧ�� */
            return IIC_OPER_FAILT;
        }	
    }
    
    iicHandle->Ops->Set_SCL(RESET);
    
    /* ��Ӧ���ź�˵���ɹ� */
    return IIC_OPER_OK; 
}

/**
 * @func    IIC_SendByte
 * @brief   IIC ����һ���ֽڵ�����
 * @param   Data ��Ҫ���͵�����					
 * @retval  ��
 */
static void IIC_SendByte(IIC_Handle_t iicHandle)
{
    __IO uint8_t i, Data;

    Data = *(iicHandle->Msg->Data);

    /* Set GPIO to Output mode */
    iicHandle->Ops->Set_SDA_DIR(SDA_OUT);
    
    iicHandle->Ops->Set_SCL(RESET);
    
    /* ѭ������һ���ֽڵ����� */
    for (i = 0; i < 8; i++)
    {		
        if (Data & 0x80)
        {
            iicHandle->Ops->Set_SDA(SET);
        }
        else
        {
            iicHandle->Ops->Set_SDA(RESET);
        }
        iicHandle->Ops->uDelay(iicHandle->Ops->Delay_us);
        iicHandle->Ops->Set_SCL(SET);
        iicHandle->Ops->uDelay(iicHandle->Ops->Delay_us);
        iicHandle->Ops->Set_SCL(RESET);
        iicHandle->Ops->uDelay(iicHandle->Ops->Delay_us);
        Data <<= 1;	
    }
    iicHandle->Ops->Set_SDA(SET);
}

/**
 * @func    IIC_ReadByte
 * @brief   i2c ���豸��ȡһ���ֽڵ�����
 * @param   ack ����Ӧ�������
                I2C_NEED_ACK ˵����ǰ���������һ���ֽڣ���ʱ���� ack
                I2C_NEEDNT_ACK ˵����ǰ�Ĵ��������һ���ֽڵ����ݣ���ʱ����nack
 * @retval	receive ��ȡ��������
 */
static void IIC_ReadByte(IIC_Handle_t iicHandle)
{
    uint8_t i, Receive = 0;
    
    /* Set Gpio to input mode */
    iicHandle->Ops->Set_SDA_DIR(SDA_IN);
    
    iicHandle->Ops->Set_SDA(SET);
    
    /* ѭ����ȡһ���ֽڵ����� */
    for (i = 0; i < 8; i++)
    {
        iicHandle->Ops->Set_SCL(RESET);
        iicHandle->Ops->uDelay(iicHandle->Ops->Delay_us);
        iicHandle->Ops->Set_SCL(SET);
        Receive <<= 1;
        if (iicHandle->Ops->Get_SDA())
        {
            Receive++;   
        }
        iicHandle->Ops->uDelay(iicHandle->Ops->Delay_us);
    }		
    
    if (iicHandle->Msg->Flags & IIC_NEEDNT_ACK) 
    {	   
        IIC_NoAck(iicHandle);	//����nACK
    }
    else if (iicHandle->Msg->Flags & IIC_NEED_ACK)
    {
        IIC_Ack(iicHandle); 		//����ACK   
    }
    *(iicHandle->Msg->Data) = Receive;  
}

/**
 * @func    IIC_CheckDevice
 * @brief   ������豸�Ƿ����
 * @param   _Address ���豸��ַ
 * @retval	receive ��ȡ��������
 */
uint8_t IIC_CheckDevice(IIC_Handle_t iicHandle)
{
    uint8_t Temp;
	if (iicHandle->Ops->Get_SDA())
	{
		IIC_Start(iicHandle);		/* ���������ź� */

		/* �����豸��ַ+��д����bit��0 = w�� 1 = r) bit7 �ȴ� */
        Temp = iicHandle->SlaveAddr | IIC_DRV_WR;
        iicHandle->Msg->Data = &Temp;
		IIC_SendByte(iicHandle);

		if (IIC_WaitAck(iicHandle) == IIC_OPER_OK)	/* ����豸��ACKӦ�� */
        {
            IIC_Stop(iicHandle);			/* ����ֹͣ�ź� */
            return IIC_OPER_OK;
        }
	}
	return IIC_OPER_FAILT;	/* IIC�����쳣 */
}

/**
 * @func    IIC_Read
 * @brief   ������ָ���ĵ�ַ��ȡָ�����ȵ�����
 * @param   Handle �豸�ľ��
 * @retval  Operation successfully returned IIC_OPER_OK, otherwise IIC_OPER_FAILT returned
 */
uint8_t IIC_Read(IIC_Handle_t iicHandle)
{
    uint8_t Temp;
    assert_param(iicHandle);
    /* ��1��������I2C���������ź� */
    IIC_Start(iicHandle);
    
    /* ��2������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
    /* �˴���дָ�� */
    Temp = iicHandle->SlaveAddr | IIC_DRV_WR;
    iicHandle->Msg->Data = &Temp;

    /* ����������ַ */
    IIC_SendByte(iicHandle);	
    
    /* ��3�����ȴ�ACK */
    if (IIC_WaitAck(iicHandle) != IIC_OPER_OK)
    {
        goto _return;	/* ������Ӧ�� */
    }

    /* ��4���������ֽڵ�ַ���жϵ�ַ�Ĵ�С */
    for (uint32_t i = 0; i < iicHandle->Msg->SubAddrSize; i++)
    {
        iicHandle->Msg->Data = ((uint8_t *)(&(iicHandle->Msg->SubAddr))) + i;
        IIC_SendByte(iicHandle);
        if (IIC_WaitAck(iicHandle) != IIC_OPER_OK)
        {
            goto _return;	/* ������Ӧ�� */
        }
    }
    
    /* ��6������������I2C���ߡ����濪ʼ��ȡ���� */
    IIC_Start(iicHandle);
    
    /* ��7������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
    Temp = iicHandle->SlaveAddr | IIC_DRV_R;
    iicHandle->Msg->Data = &Temp;
    IIC_SendByte(iicHandle);	/* �˴��Ƕ�ָ�� */
    
    /* ��8�����ȴ�ACK */
    if (IIC_WaitAck(iicHandle) != IIC_OPER_OK)
    {
        goto _return;	/* ������Ӧ�� */
    }
    
    /* ��9����ѭ����ȡ���� */
    for (uint32_t i = 0; i < iicHandle->BufSize; i++)
    {
        iicHandle->Msg->Flags = IIC_NEED_ACK;
        iicHandle->Msg->Data = iicHandle->Buf + i;
        IIC_ReadByte(iicHandle);	/* ��1���ֽ� */

        if (i == iicHandle->BufSize - 1)
        {
            i++;
            iicHandle->Msg->Flags = IIC_NEEDNT_ACK;
            iicHandle->Msg->Data = iicHandle->Buf + i;
            IIC_ReadByte(iicHandle);	/* ��1���ֽ� */
        }
    }
    
    /* ����I2C����ֹͣ�ź� */
    IIC_Stop(iicHandle);
    
    /* ִ�гɹ� */
    return IIC_OPER_OK;	
    
_return: /* ����ִ��ʧ�ܺ��мǷ���ֹͣ�źţ�����Ӱ��I2C�����������豸 */

    return IIC_OPER_FAILT;
}

/**
 * @func    IIC_Write
 * @brief   ������ָ���ĵ�ַд��ָ�����ȵ�����
 * @param   Handle �豸���
 * @retval  Operation successfully returned IIC_OPER_OK, otherwise IIC_OPER_FAILT returned
 */
uint8_t IIC_Write(IIC_Handle_t iicHandle)
{
    uint8_t Temp;
    assert_param(iicHandle);

    /* ��1��������I2C���������ź� */
    IIC_Start(iicHandle);
    
    /* ��2������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
    /* �˴���дָ�� */
    Temp = iicHandle->SlaveAddr | IIC_DRV_WR;
    iicHandle->Msg->Data = &Temp;

    /* ����������ַ */
    IIC_SendByte(iicHandle);	
    
    /* ��3�����ȴ�ACK */
    if (IIC_WaitAck(iicHandle) != IIC_OPER_OK)
    {
        goto _return;    /* ������Ӧ�� */
    }

    /* ��4���������ֽڵ�ַ���жϵ�ַ�Ĵ�С */
    for (uint32_t i = 0; i < iicHandle->Msg->SubAddrSize; i++)
    {
        iicHandle->Msg->Data = ((uint8_t *)(&(iicHandle->Msg->SubAddr))) + i;
        IIC_SendByte(iicHandle);
        if (IIC_WaitAck(iicHandle) != IIC_OPER_OK)
        {
            goto _return;	/* ������Ӧ�� */
        }
    }

    for (uint32_t i = 0; i < iicHandle->BufSize; i++)
    {
        iicHandle->Msg->Data = iicHandle->Buf + i;
        /* ����������ַ */
        IIC_SendByte(iicHandle);    
        /* �ȴ�ACK */
        if (IIC_WaitAck(iicHandle) != IIC_OPER_OK)
        {
            goto _return;    /* ������Ӧ�� */
        }
    }
    
    /* ����ִ�гɹ�������I2C����ֹͣ�ź� */
    IIC_Stop(iicHandle);
    return IIC_OPER_OK;
    
_return: /* ����ִ��ʧ�ܺ��мǷ���ֹͣ�źţ�����Ӱ��I2C�����������豸 */

    return IIC_OPER_FAILT;
}
