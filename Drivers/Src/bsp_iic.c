/**
 ******************************************************************************
 * @file      iic.c
 * @author    门禁开发小组
 * @version   V1.0.4
 * @date      2018-06-20
 * @brief     文件内包含一些iic的引脚配置，iic的基本读写操作以及通信时序
 * @History
 * Date           Author    version    		Notes
 * 2017-11-01     ZSY       V1.0.0          first version.
 * 2017-11-02     ZSY       V1.0.1          修改了应答信号的反馈形式，增加了对应的宏
                                            定义，对iic_ack和iic_no_ack添加了static
                                            的修饰符，仅限于本文件内使用
 * 2018-01-09     ZSY       V1.0.2          排版格式化操作.
 * 2018-01-26     ZSY       V1.0.3          添加私有和公有宏定义.
 * 2018-06-20     ZSY       V1.0.4          提高兼容性.
 */
	
/* Includes ------------------------------------------------------------------*/
#include "bsp_iic.h"

/* Private macro Definition --------------------------------------------------*/
		   
/* 宏定义i2c的硬件接口 */
#define IIC_SCL_PIN   IIC_SCL_Pin
#define IIC_SDA_PIN   IIC_SDA_Pin   

#define IIC_SCL_PORT	IIC_SCL_GPIO_Port
#define IIC_SDA_PORT	IIC_SDA_GPIO_Port

#ifdef STM32F1
/* IO方向设置 */
#define SET_IIC_SDA_IN()  {GPIOB->CRH &= 0XFFFF0FFF; GPIOB->CRH |= (uint32_t)8 << 12;}
#define SET_IIC_SDA_OUT() {GPIOB->CRH &= 0XFFFF0FFF; GPIOB->CRH |= (uint32_t)3 << 12;}
#elif defined STM32F4
/* IO方向设置 */
#define SET_IIC_SDA_IN()  {IIC_SDA_PORT->MODER &= ~(3 << (9 * 2)); IIC_SDA_PORT->MODER |= (0 << (9 * 2));}	//PB12??????
#define SET_IIC_SDA_OUT() {IIC_SDA_PORT->MODER &= ~(3 << (9 * 2)); IIC_SDA_PORT->MODER |= (1 << (9 * 2));} 
#endif
/* 设置iic接口的高低逻辑电平输出 */
#define IIC_SCL_WRITE_H   LL_GPIO_SetOutputPin(IIC_SCL_PORT, IIC_SCL_PIN)
#define IIC_SCL_WRITE_L   LL_GPIO_ResetOutputPin(IIC_SCL_PORT, IIC_SCL_PIN)
						
#define IIC_SDA_WRITE_H   LL_GPIO_SetOutputPin(IIC_SDA_PORT, IIC_SDA_PIN)
#define IIC_SDA_WRITE_L   LL_GPIO_ResetOutputPin(IIC_SDA_PORT, IIC_SDA_PIN)

/* 设置iic的sda线的读入功能	*/
#define	IIC_SDA_READ      LL_GPIO_IsInputPinSet(IIC_SDA_PORT, IIC_SDA_PIN)

/* 应答信号ACK等待超时时间	*/
#define IIC_ACK_TIMEOUT   200

/* End private macro Definition ----------------------------------------------*/

/* global variable Declaration -----------------------------------------------*/



/* User function Declaration -------------------------------------------------*/



/* User functions ------------------------------------------------------------*/

/**
 * @func    BspIIC_Delay
 * @brief   BspIIC延时方法
 * @param   nCount 时间
 * @retval  无
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
// * @brief   设置触屏的IIC引脚,用软件模拟的方法实现IIC功能
// * @note    需要开启对应的时钟和复用
// * @retval  无
// */
//void IIC_GPIO_Config(void) 
//{
//    GPIO_InitTypeDef GPIO_InitStructure;

//    /* 使能GPIOB和GPIOC的时钟 */
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//    
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIO
//    /* config IIC scl line */
//    GPIO_InitStructure.GPIO_Pin = IIC_SCL_PIN;  		//PC5  
//    GPIO_Init(IIC_SCL_PORT, &GPIO_InitStructure);
//    
//    /* config IIC sda line */
//    GPIO_InitStructure.GPIO_Pin =  IIC_SDA_PIN; 		//PC4
//    GPIO_Init(IIC_SDA_PORT, &GPIO_InitStructure);
//    
//    /* 初始化完成后设置为高 */
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
        
        /* 超时检测，防止卡死 */
        if (time > IIC_ACK_TIMEOUT)
        {
            IIC_Stop();
            
            /* 超时意味着失败 */
            return IIC_OPER_FAILT;
        }	
    }
    
    IIC_SCL_WRITE_L;
    
    /* 有应答信号说明成功 */
    return IIC_OPER_OK; 
}

/**
 * @func    IIC_SendByte
 * @brief   IIC 发送一个字节的数据
 * @param   Data 将要发送的数据					
 * @retval  无
 */
void IIC_SendByte(uint8_t Data)
{
    __IO uint8_t i;
    
    /* Set GPIO to Output mode */
    SET_IIC_SDA_OUT();
    
    IIC_SCL_WRITE_L;
    
    /* 循环发送一个字节的数据 */
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
 * @brief   i2c 从设备读取一个字节的数据
 * @param   ack 主机应答的类型
                I2C_NEED_ACK 说明当前还不是最后一个字节，此时发送 ack
                I2C_NEEDNT_ACK 说明当前的传输是最后一个字节的数据，此时发送nack
 * @retval	receive 读取到的数据
 */
uint8_t IIC_ReadByte(uint8_t Ack)
{
    unsigned char i, Receive = 0;
    
    /* Set Gpio to input mode */
    SET_IIC_SDA_IN();
    
    IIC_SDA_WRITE_H;
    
    /* 循环读取一个字节的数据 */
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
        IIC_NoAck();	//发送nACK
    }
    else       
    {
        IIC_Ack(); 		//发送ACK   
    }
      
    return Receive;
}

/**
 * @func    IIC_CheckDevice
 * @brief   检验从设备是否存在
 * @param   _Address 从设备地址
 * @retval	receive 读取到的数据
 */
uint8_t IIC_CheckDevice(uint8_t _Address)
{
	if (IIC_SDA_READ)
	{
		IIC_Start();		/* 发送启动信号 */

		/* 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传 */
		IIC_SendByte(_Address | IIC_DRV_WR);
		if (IIC_WaitAck() == IIC_OPER_OK)	/* 检测设备的ACK应答 */
        {
            IIC_Stop();			/* 发送停止信号 */
            return IIC_OPER_OK;
        }
	}
	return IIC_OPER_FAILT;	/* IIC总线异常 */
}


