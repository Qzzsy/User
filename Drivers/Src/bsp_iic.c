/**
 ******************************************************************************
 * @file      iic.c
 * @author    门禁开发小组
 * @version   V1.0.5
 * @date      2018-06-24
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
 * 2018-06-24     ZSY       V1.0.5          更改驱动框架，实现不同的IIC设备利用统一的接口实现数据传输.
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
        
        /* 超时检测，防止卡死 */
        if (time > (Handle->Ops->Timeout))
        {
            IIC_Stop(Handle);
            
            /* 超时意味着失败 */
            return IIC_OPER_FAILT;
        }	
    }
    
    Handle->Ops->Set_SCL(RESET);
    
    /* 有应答信号说明成功 */
    return IIC_OPER_OK; 
}

/**
 * @func    IIC_SendByte
 * @brief   IIC 发送一个字节的数据
 * @param   Data 将要发送的数据					
 * @retval  无
 */
void IIC_SendByte(IIC_Handle_t Handle)
{
    __IO uint8_t i, Data = *(Handle->Msg->Data + Handle->Msg->Offiset);
    
    Data |= Handle->Msg->Flags;

    /* Set GPIO to Output mode */
    Handle->Ops->Set_SDA_DIR(SDA_OUT);
    
    Handle->Ops->Set_SCL(RESET);
    
    /* 循环发送一个字节的数据 */
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
 * @brief   i2c 从设备读取一个字节的数据
 * @param   ack 主机应答的类型
                I2C_NEED_ACK 说明当前还不是最后一个字节，此时发送 ack
                I2C_NEEDNT_ACK 说明当前的传输是最后一个字节的数据，此时发送nack
 * @retval	receive 读取到的数据
 */
void IIC_ReadByte(IIC_Handle_t Handle)
{
    uint8_t i, Receive = 0;
    
    /* Set Gpio to input mode */
    Handle->Ops->Set_SDA_DIR(SDA_IN);
    
    Handle->Ops->Set_SDA(SET);
    
    /* 循环读取一个字节的数据 */
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
        IIC_NoAck(Handle);	//发送nACK
    }
    else       
    {
        IIC_Ack(Handle); 		//发送ACK   
    }
    *(Handle->Msg->Data + Handle->Msg->Offiset) = Receive;  
}

/**
 * @func    IIC_CheckDevice
 * @brief   检验从设备是否存在
 * @param   _Address 从设备地址
 * @retval	receive 读取到的数据
 */
uint8_t IIC_CheckDevice(IIC_Handle_t Handle)
{
	if (Handle->Ops->Get_SDA())
	{
		IIC_Start(Handle);		/* 发送启动信号 */

		/* 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传 */
        Handle->Msg->Flags = IIC_DRV_WR;
		IIC_SendByte(Handle);
		if (IIC_WaitAck(Handle) == IIC_OPER_OK)	/* 检测设备的ACK应答 */
        {
            IIC_Stop(Handle);			/* 发送停止信号 */
            return IIC_OPER_OK;
        }
	}
	return IIC_OPER_FAILT;	/* IIC总线异常 */
}

/**
 * @func    IIC_Read
 * @brief   主机从指定的地址读取指定长度的数据
 * @param   Handle 设备的句柄
 * @retval  Operation successfully returned IIC_OPER_OK, otherwise IIC_OPER_FAILT returned
 */
uint8_t IIC_Read(IIC_Handle_t Handle)
{
    /* 第1步：发起I2C总线启动信号 */
    IIC_Start(Handle);
    
    /* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
    /* 此处是写指令 */
    Handle->Msg->Flags = IIC_DRV_WR;

    /* 发送器件地址 */
    IIC_SendByte(Handle);	
    
    /* 第3步：等待ACK */
    if (IIC_WaitAck(Handle) != IIC_OPER_OK)
    {
        goto _return;	/* 器件无应答 */
    }

    /* 第4步：发送字节地址，判断地址的大小 */
    for (Handle->Msg->Offiset = 0; Handle->Msg->Offiset < Handle->Msg->SubAddrSize; Handle->Msg->Offiset++)
    {
        IIC_SendByte(Handle);
        if (IIC_WaitAck(Handle) != IIC_OPER_OK)
        {
            goto _return;	/* 器件无应答 */
        }
    }
    
    /* 第6步：重新启动I2C总线。下面开始读取数据 */
    IIC_Start(Handle);
    
    /* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
    Handle->Msg->Flags = IIC_DRV_R;
    IIC_SendByte(Handle);	/* 此处是读指令 */
    
    /* 第8步：等待ACK */
    if (IIC_WaitAck(Handle) != IIC_OPER_OK)
    {
        goto _return;	/* 器件无应答 */
    }
    
    Handle->Msg->Offiset = 0;
    /* 第9步：循环读取数据 */
    for (Handle->Msg->Offiset = 0; Handle->Msg->Offiset < Handle->Msg->DataSize - 1; Handle->Msg->Offiset++)
    {
        Handle->Msg->Flags = IIC_NEED_ACK;
        IIC_ReadByte(Handle);	/* 读1个字节 */
    }
    
    Handle->Msg->Flags = IIC_NEEDNT_ACK;
    IIC_ReadByte(Handle);	/* 读1个字节 */
    
    /* 发送I2C总线停止信号 */
    IIC_Stop(Handle);
    
    /* 执行成功 */
    return IIC_OPER_OK;	
    
_return: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */

    return IIC_OPER_FAILT;
}

/**
 * @func    IIC_Write
 * @brief   主机往指定的地址写入指定长度的数据
 * @param   Handle 设备句柄
 * @retval  Operation successfully returned IIC_OPER_OK, otherwise IIC_OPER_FAILT returned
 */
uint8_t IIC_Write(IIC_Handle_t Handle)
{
    uint16_t i, m;
    
    /* 第1步：发起I2C总线启动信号 */
    IIC_Start(Handle);
    
    /* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
    /* 此处是写指令 */
    Handle->Msg->Flags = IIC_DRV_WR;

    /* 发送器件地址 */
    IIC_SendByte(Handle);	
    
    /* 第3步：等待ACK */
    if (IIC_WaitAck(Handle) != IIC_OPER_OK)
    {
        goto _return;    /* 器件无应答 */
    }

    for (Handle->Msg->Offiset = 0; Handle->Msg->Offiset < Handle->Msg->DataSize; Handle->Msg->Offiset++)
    {
        /* 发送器件地址 */
        IIC_SendByte(Handle);    
        /* 等待ACK */
        if (IIC_WaitAck(Handle) != IIC_OPER_OK)
        {
            goto _return;    /* 器件无应答 */
        }
    }
    
    /* 命令执行成功，发送I2C总线停止信号 */
    IIC_Stop(Handle);
    return IIC_OPER_OK;
    
_return: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */

    return IIC_OPER_FAILT;
}
