/**
 ******************************************************************************
 * @file      iic.c
 * @author    �Ž�����С��
 * @version   V1.0.4
 * @date      2018-06-20
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
    uint32_t i=0;

    while (Number--)
    {
        i = 200;
        while (i--);
    }
}

///**
// * @func    IIC_GPIO_Config
// * @brief   ���ô�����IIC����,�����ģ��ķ���ʵ��IIC����
// * @note    ��Ҫ������Ӧ��ʱ�Ӻ͸���
// * @retval  ��
// */
//void IIC_GPIO_Config(void) 
//{
//    GPIO_InitTypeDef GPIO_InitStructure;

//    /* ʹ��GPIOB��GPIOC��ʱ�� */
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//    
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIO
//    /* config IIC scl line */
//    GPIO_InitStructure.GPIO_Pin = IIC_SCL_PIN;  		//PC5  
//    GPIO_Init(IIC_SCL_PORT, &GPIO_InitStructure);
//    
//    /* config IIC sda line */
//    GPIO_InitStructure.GPIO_Pin =  IIC_SDA_PIN; 		//PC4
//    GPIO_Init(IIC_SDA_PORT, &GPIO_InitStructure);
//    
//    /* ��ʼ����ɺ�����Ϊ�� */
//    IIC_SCL_WRITE_H;
//    IIC_SDA_WRITE_H;
//    
//    /* IIC stop transport */
//    IIC_Stop();
//}

/**
 * @func    IIC_Start
 * @brief   IIC start transport
                IIC device is about to start a new transfer process
 * @retval  none
 */
void IIC_Start(void)  
{ 
    /* Set GPIO to Output mode */
    SET_IIC_SDA_OUT();
    
    /* IIC produce a start signal */
    IIC_SDA_WRITE_H;
    IIC_SCL_WRITE_H;
    BspIIC_Delay(4);
    IIC_SDA_WRITE_L;
    BspIIC_Delay(4);
    IIC_SCL_WRITE_L; 
} 

/**
 * @func    IIC_Stop
 * @brief   iic stop transport
                IIC device is about to stop the current transport process
 * @retval  none
 */
void IIC_Stop(void)  
{ 
    /* Set GPIO to Output mode */
    SET_IIC_SDA_OUT();
    
    /* iic produce a stop signal */
    IIC_SDA_WRITE_L;
    IIC_SCL_WRITE_L;
    BspIIC_Delay(4);
    IIC_SCL_WRITE_H;
    BspIIC_Delay(4);
    IIC_SDA_WRITE_H;
}

/**
 * @func    IIC_Ack
 * @brief   IIC master produce a ack signal
 * @retval  none
 */
static void IIC_Ack(void) 
{ 
    IIC_SCL_WRITE_L;
    
    /* Set GPIO to Output mode */
    SET_IIC_SDA_OUT();
    
    IIC_SDA_WRITE_L;
    BspIIC_Delay(2);
    IIC_SCL_WRITE_H;
    BspIIC_Delay(2);
    IIC_SCL_WRITE_L;
} 

/**
 * @func    IIC_NoAck
 * @brief   IIC slave don't produce a ack signal??transport will stop
 * @retval  none
 */
static void IIC_NoAck(void)
{
    IIC_SCL_WRITE_L;
    
    /* Set GPIO to Output mode */
    SET_IIC_SDA_OUT();
    
    /* IIC produce no ack signal */
    IIC_SDA_WRITE_H;
    BspIIC_Delay(2);
    IIC_SCL_WRITE_H;
    BspIIC_Delay(2);
    IIC_SCL_WRITE_L;	
}

/**
 * @func    IIC_WaitAck
 * @brief   IIC master waiting a ack signal from slave
                The CPU produces a clock and reads the device's ACK signal
 * @retval  return IIC_OPER_OK for correct response, IIC_OPER_FAILT for no device response
 */
uint8_t IIC_WaitAck(void)
{ 
    __IO uint16_t time = 0;
    
    /* Set GPIO to input mode */
    SET_IIC_SDA_IN();
    
    IIC_SDA_WRITE_H;  
    BspIIC_Delay(1);
    IIC_SCL_WRITE_H;
    BspIIC_Delay(1);
    
    /* waiting slave ack signal	*/
    while (IIC_SDA_READ)
    {
        time++;
        
        /* ��ʱ��⣬��ֹ���� */
        if (time > IIC_ACK_TIMEOUT)
        {
            IIC_Stop();
            
            /* ��ʱ��ζ��ʧ�� */
            return IIC_OPER_FAILT;
        }	
    }
    
    IIC_SCL_WRITE_L;
    
    /* ��Ӧ���ź�˵���ɹ� */
    return IIC_OPER_OK; 
}

/**
 * @func    IIC_SendByte
 * @brief   IIC ����һ���ֽڵ�����
 * @param   Data ��Ҫ���͵�����					
 * @retval  ��
 */
void IIC_SendByte(uint8_t Data)
{
    __IO uint8_t i;
    
    /* Set GPIO to Output mode */
    SET_IIC_SDA_OUT();
    
    IIC_SCL_WRITE_L;
    
    /* ѭ������һ���ֽڵ����� */
    for (i = 0; i < 8; i++)
    {		
        if (Data & 0x80)
        {
            IIC_SDA_WRITE_H;
        }
        else
        {
            IIC_SDA_WRITE_L;
        }
        BspIIC_Delay(2);
        IIC_SCL_WRITE_H;	
        BspIIC_Delay(2);
        IIC_SCL_WRITE_L;
        BspIIC_Delay(2);
        Data <<= 1;	
    }
    IIC_SDA_WRITE_H;
}

/**
 * @func    IIC_ReadByte
 * @brief   i2c ���豸��ȡһ���ֽڵ�����
 * @param   ack ����Ӧ�������
                I2C_NEED_ACK ˵����ǰ���������һ���ֽڣ���ʱ���� ack
                I2C_NEEDNT_ACK ˵����ǰ�Ĵ��������һ���ֽڵ����ݣ���ʱ����nack
 * @retval	receive ��ȡ��������
 */
uint8_t IIC_ReadByte(uint8_t Ack)
{
    unsigned char i, Receive = 0;
    
    /* Set Gpio to input mode */
    SET_IIC_SDA_IN();
    
    IIC_SDA_WRITE_H;
    
    /* ѭ����ȡһ���ֽڵ����� */
    for (i = 0; i < 8; i++)
    {
        IIC_SCL_WRITE_L; 
        BspIIC_Delay(2);
        IIC_SCL_WRITE_H;
        Receive <<= 1;
        if (IIC_SDA_READ)
        {
            Receive++;   
        }
        BspIIC_Delay(2); 
    }		
    
    if (Ack == IIC_NEEDNT_ACK) 
    {	   
        IIC_NoAck();	//����nACK
    }
    else       
    {
        IIC_Ack(); 		//����ACK   
    }
      
    return Receive;
}

/**
 * @func    IIC_CheckDevice
 * @brief   ������豸�Ƿ����
 * @param   _Address ���豸��ַ
 * @retval	receive ��ȡ��������
 */
uint8_t IIC_CheckDevice(uint8_t _Address)
{
	if (IIC_SDA_READ)
	{
		IIC_Start();		/* ���������ź� */

		/* �����豸��ַ+��д����bit��0 = w�� 1 = r) bit7 �ȴ� */
		IIC_SendByte(_Address | IIC_DRV_WR);
		if (IIC_WaitAck() == IIC_OPER_OK)	/* ����豸��ACKӦ�� */
        {
            IIC_Stop();			/* ����ֹͣ�ź� */
            return IIC_OPER_OK;
        }
	}
	return IIC_OPER_FAILT;	/* IIC�����쳣 */
}


