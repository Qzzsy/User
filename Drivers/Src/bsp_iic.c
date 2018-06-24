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

/* Private macro Definition --------------------------------------------------*/
		   
/* �궨��i2c��Ӳ���ӿ� */
#define IIC_SCL_PIN   IIC_SCL_Pin
#define IIC_SDA_PIN   IIC_SDA_Pin   

#define IIC_SCL_PORT	IIC_SCL_GPIO_Port
#define IIC_SDA_PORT	IIC_SDA_GPIO_Port

#ifdef STM32F1
/* IO�������� */
#define SET_IIC_SDA_IN()  {GPIOB->CRH &= 0XFFFF0FFF; GPIOB->CRH |= (uint32_t)8 << 12;}
#define SET_IIC_SDA_OUT() {GPIOB->CRH &= 0XFFFF0FFF; GPIOB->CRH |= (uint32_t)3 << 12;}
#elif defined STM32F4
/* IO�������� */
#define SET_IIC_SDA_IN()  {IIC_SDA_PORT->MODER &= ~(3 << (9 * 2)); IIC_SDA_PORT->MODER |= (0 << (9 * 2));}	//PB12??????
#define SET_IIC_SDA_OUT() {IIC_SDA_PORT->MODER &= ~(3 << (9 * 2)); IIC_SDA_PORT->MODER |= (1 << (9 * 2));} 
#endif
/* ����iic�ӿڵĸߵ��߼���ƽ��� */
#define IIC_SCL_WRITE_H   LL_GPIO_SetOutputPin(IIC_SCL_PORT, IIC_SCL_PIN)
#define IIC_SCL_WRITE_L   LL_GPIO_ResetOutputPin(IIC_SCL_PORT, IIC_SCL_PIN)
						
#define IIC_SDA_WRITE_H   LL_GPIO_SetOutputPin(IIC_SDA_PORT, IIC_SDA_PIN)
#define IIC_SDA_WRITE_L   LL_GPIO_ResetOutputPin(IIC_SDA_PORT, IIC_SDA_PIN)

/* ����iic��sda�ߵĶ��빦��	*/
#define	IIC_SDA_READ      LL_GPIO_IsInputPinSet(IIC_SDA_PORT, IIC_SDA_PIN)

/* Ӧ���ź�ACK�ȴ���ʱʱ��	*/
#define IIC_ACK_TIMEOUT   200

/* End private macro Definition ----------------------------------------------*/

/* global variable Declaration -----------------------------------------------*/

/* User function Declaration -------------------------------------------------*/

/* User functions ------------------------------------------------------------*/

/**
 * @func    BspIIC_Delay
 * @brief   BspIIC��ʱ����
 * @param   nCount ʱ��
 * @retval  ��
 */
void BspIIC_Delay(__IO uint32_t Number)
{
    uint32_t i = 0;

    while (Number--)
    {
        i = 200;
        while (i--);
    }
}

/**
 * @func    IIC_Start
 * @brief   IIC start transport
                IIC device is about to start a new transfer process
 * @retval  none
 */
void IIC_Start(IIC_Handle_t Handle)  
{ 
    /* Set GPIO to Output mode */
    Handle->Ops->Set_SDA_DIR(SDA_OUT);
    
    /* IIC produce a start signal */
    Handle->Ops->Set_SDA(SET);
    Handle->Ops->Set_SCL(SET);
    Handle->Ops->uDelay(Handle->Ops->Delay_us);
    Handle->Ops->Set_SDA(RESET);
    Handle->Ops->uDelay(Handle->Ops->Delay_us);
    Handle->Ops->Set_SCL(RESET);
} 

/**
 * @func    IIC_Stop
 * @brief   iic stop transport
                IIC device is about to stop the current transport process
 * @retval  none
 */
void IIC_Stop(IIC_Handle_t Handle)  
{ 
    /* Set GPIO to Output mode */
    Handle->Ops->Set_SDA_DIR(SDA_OUT);
    
    /* iic produce a stop signal */
    Handle->Ops->Set_SCL(RESET);
    Handle->Ops->Set_SDA(RESET);
    Handle->Ops->uDelay(Handle->Ops->Delay_us);
    Handle->Ops->Set_SCL(SET);
    Handle->Ops->uDelay(Handle->Ops->Delay_us);
    Handle->Ops->Set_SDA(SET);
}

/**
 * @func    IIC_Ack
 * @brief   IIC master produce a ack signal
 * @retval  none
 */
static void IIC_Ack(IIC_Handle_t Handle) 
{ 
    Handle->Ops->Set_SCL(RESET);
    
    /* Set GPIO to Output mode */
    Handle->Ops->Set_SDA_DIR(SDA_OUT);
    
    Handle->Ops->Set_SDA(RESET);
    Handle->Ops->uDelay(Handle->Ops->Delay_us);
    Handle->Ops->Set_SCL(SET);
    Handle->Ops->uDelay(Handle->Ops->Delay_us);
    Handle->Ops->Set_SCL(RESET);
} 

/**
 * @func    IIC_NoAck
 * @brief   IIC slave don't produce a ack signal??transport will stop
 * @retval  none
 */
static void IIC_NoAck(IIC_Handle_t Handle)
{
    Handle->Ops->Set_SCL(RESET);
    
    /* Set GPIO to Output mode */
    Handle->Ops->Set_SDA_DIR(SDA_OUT);
    
    /* IIC produce no ack signal */
    Handle->Ops->Set_SDA(SET);
    Handle->Ops->uDelay(Handle->Ops->Delay_us);
    Handle->Ops->Set_SCL(SET);
    Handle->Ops->uDelay(Handle->Ops->Delay_us);
    Handle->Ops->Set_SCL(RESET);
}

/**
 * @func    IIC_WaitAck
 * @brief   IIC master waiting a ack signal from slave
                The CPU produces a clock and reads the device's ACK signal
 * @retval  return IIC_OPER_OK for correct response, IIC_OPER_FAILT for no device response
 */
uint8_t IIC_WaitAck(IIC_Handle_t Handle)
{ 
    __IO uint16_t time = 0;
    
    /* Set GPIO to input mode */
    Handle->Ops->Set_SDA_DIR(SDA_IN);
    
    Handle->Ops->Set_SDA(SET);
    Handle->Ops->uDelay(Handle->Ops->Delay_us);
    Handle->Ops->Set_SCL(SET);
    Handle->Ops->uDelay(Handle->Ops->Delay_us);
    
    /* waiting slave ack signal	*/
    while (Handle->Ops->Get_SDA())
    {
        time++;
        
        /* ��ʱ��⣬��ֹ���� */
        if (time > (Handle->Ops->Timeout))
        {
            IIC_Stop(Handle);
            
            /* ��ʱ��ζ��ʧ�� */
            return IIC_OPER_FAILT;
        }	
    }
    
    Handle->Ops->Set_SCL(RESET);
    
    /* ��Ӧ���ź�˵���ɹ� */
    return IIC_OPER_OK; 
}

/**
 * @func    IIC_SendByte
 * @brief   IIC ����һ���ֽڵ�����
 * @param   Data ��Ҫ���͵�����					
 * @retval  ��
 */
void IIC_SendByte(IIC_Handle_t Handle)
{
    __IO uint8_t i, Data = *(Handle->Msg->Data + Handle->Msg->Offiset);
    
    Data |= Handle->Msg->Flags;

    /* Set GPIO to Output mode */
    Handle->Ops->Set_SDA_DIR(SDA_OUT);
    
    Handle->Ops->Set_SCL(RESET);
    
    /* ѭ������һ���ֽڵ����� */
    for (i = 0; i < 8; i++)
    {		
        if (Data & 0x80)
        {
            Handle->Ops->Set_SDA(SET);
        }
        else
        {
            Handle->Ops->Set_SDA(RESET);
        }
        Handle->Ops->uDelay(Handle->Ops->Delay_us);
        Handle->Ops->Set_SCL(SET);
        Handle->Ops->uDelay(Handle->Ops->Delay_us);
        Handle->Ops->Set_SCL(RESET);
        Handle->Ops->uDelay(Handle->Ops->Delay_us);
        Data <<= 1;	
    }
    Handle->Ops->Set_SDA(SET);
}

/**
 * @func    IIC_ReadByte
 * @brief   i2c ���豸��ȡһ���ֽڵ�����
 * @param   ack ����Ӧ�������
                I2C_NEED_ACK ˵����ǰ���������һ���ֽڣ���ʱ���� ack
                I2C_NEEDNT_ACK ˵����ǰ�Ĵ��������һ���ֽڵ����ݣ���ʱ����nack
 * @retval	receive ��ȡ��������
 */
void IIC_ReadByte(IIC_Handle_t Handle)
{
    uint8_t i, Receive = 0;
    
    /* Set Gpio to input mode */
    Handle->Ops->Set_SDA_DIR(SDA_IN);
    
    Handle->Ops->Set_SDA(SET);
    
    /* ѭ����ȡһ���ֽڵ����� */
    for (i = 0; i < 8; i++)
    {
        Handle->Ops->Set_SCL(RESET);
        Handle->Ops->uDelay(Handle->Ops->Delay_us);
        Handle->Ops->Set_SCL(SET);
        Receive <<= 1;
        if (Handle->Ops->Get_SDA())
        {
            Receive++;   
        }
        Handle->Ops->uDelay(Handle->Ops->Delay_us);
    }		
    
    if (Handle->Msg->Flags & IIC_NEEDNT_ACK) 
    {	   
        IIC_NoAck(Handle);	//����nACK
    }
    else       
    {
        IIC_Ack(Handle); 		//����ACK   
    }
    *(Handle->Msg->Data + Handle->Msg->Offiset) = Receive;  
}

/**
 * @func    IIC_CheckDevice
 * @brief   ������豸�Ƿ����
 * @param   _Address ���豸��ַ
 * @retval	receive ��ȡ��������
 */
uint8_t IIC_CheckDevice(IIC_Handle_t Handle)
{
	if (Handle->Ops->Get_SDA())
	{
		IIC_Start(Handle);		/* ���������ź� */

		/* �����豸��ַ+��д����bit��0 = w�� 1 = r) bit7 �ȴ� */
        Handle->Msg->Flags = IIC_DRV_WR;
		IIC_SendByte(Handle);
		if (IIC_WaitAck(Handle) == IIC_OPER_OK)	/* ����豸��ACKӦ�� */
        {
            IIC_Stop(Handle);			/* ����ֹͣ�ź� */
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
uint8_t IIC_Read(IIC_Handle_t Handle)
{
    /* ��1��������I2C���������ź� */
    IIC_Start(Handle);
    
    /* ��2������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
    /* �˴���дָ�� */
    Handle->Msg->Flags = IIC_DRV_WR;

    /* ����������ַ */
    IIC_SendByte(Handle);	
    
    /* ��3�����ȴ�ACK */
    if (IIC_WaitAck(Handle) != IIC_OPER_OK)
    {
        goto _return;	/* ������Ӧ�� */
    }

    /* ��4���������ֽڵ�ַ���жϵ�ַ�Ĵ�С */
    for (Handle->Msg->Offiset = 0; Handle->Msg->Offiset < Handle->Msg->SubAddrSize; Handle->Msg->Offiset++)
    {
        IIC_SendByte(Handle);
        if (IIC_WaitAck(Handle) != IIC_OPER_OK)
        {
            goto _return;	/* ������Ӧ�� */
        }
    }
    
    /* ��6������������I2C���ߡ����濪ʼ��ȡ���� */
    IIC_Start(Handle);
    
    /* ��7������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
    Handle->Msg->Flags = IIC_DRV_R;
    IIC_SendByte(Handle);	/* �˴��Ƕ�ָ�� */
    
    /* ��8�����ȴ�ACK */
    if (IIC_WaitAck(Handle) != IIC_OPER_OK)
    {
        goto _return;	/* ������Ӧ�� */
    }
    
    Handle->Msg->Offiset = 0;
    /* ��9����ѭ����ȡ���� */
    for (Handle->Msg->Offiset = 0; Handle->Msg->Offiset < Handle->Msg->DataSize - 1; Handle->Msg->Offiset++)
    {
        Handle->Msg->Flags = IIC_NEED_ACK;
        IIC_ReadByte(Handle);	/* ��1���ֽ� */
    }
    
    Handle->Msg->Flags = IIC_NEEDNT_ACK;
    IIC_ReadByte(Handle);	/* ��1���ֽ� */
    
    /* ����I2C����ֹͣ�ź� */
    IIC_Stop(Handle);
    
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
uint8_t IIC_Write(IIC_Handle_t Handle)
{
    uint16_t i, m;
    
    /* ��1��������I2C���������ź� */
    IIC_Start(Handle);
    
    /* ��2������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
    /* �˴���дָ�� */
    Handle->Msg->Flags = IIC_DRV_WR;

    /* ����������ַ */
    IIC_SendByte(Handle);	
    
    /* ��3�����ȴ�ACK */
    if (IIC_WaitAck(Handle) != IIC_OPER_OK)
    {
        goto _return;    /* ������Ӧ�� */
    }

    for (Handle->Msg->Offiset = 0; Handle->Msg->Offiset < Handle->Msg->DataSize; Handle->Msg->Offiset++)
    {
        /* ����������ַ */
        IIC_SendByte(Handle);    
        /* �ȴ�ACK */
        if (IIC_WaitAck(Handle) != IIC_OPER_OK)
        {
            goto _return;    /* ������Ӧ�� */
        }
    }
    
    /* ����ִ�гɹ�������I2C����ֹͣ�ź� */
    IIC_Stop(Handle);
    return IIC_OPER_OK;
    
_return: /* ����ִ��ʧ�ܺ��мǷ���ֹͣ�źţ�����Ӱ��I2C�����������豸 */

    return IIC_OPER_FAILT;
}
