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
        
        /* 超时检测，防止卡死 */
        if (time > (iicHandle->Timeout))
        {
            IIC_Stop(iicHandle);
            
            /* 超时意味着失败 */
            return IIC_OPER_FAILT;
        }	
    }
    
    iicHandle->Ops->Set_SCL(RESET);
    
    /* 有应答信号说明成功 */
    return IIC_OPER_OK; 
}

/**
 * @func    IIC_SendByte
 * @brief   IIC 发送一个字节的数据
 * @param   Data 将要发送的数据					
 * @retval  无
 */
static void IIC_SendByte(IIC_Handle_t iicHandle)
{
    __IO uint8_t i, Data;

    Data = *(iicHandle->Msg->Data);

    /* Set GPIO to Output mode */
    iicHandle->Ops->Set_SDA_DIR(SDA_OUT);
    
    iicHandle->Ops->Set_SCL(RESET);
    
    /* 循环发送一个字节的数据 */
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
 * @brief   i2c 从设备读取一个字节的数据
 * @param   ack 主机应答的类型
                I2C_NEED_ACK 说明当前还不是最后一个字节，此时发送 ack
                I2C_NEEDNT_ACK 说明当前的传输是最后一个字节的数据，此时发送nack
 * @retval	receive 读取到的数据
 */
static void IIC_ReadByte(IIC_Handle_t iicHandle)
{
    uint8_t i, Receive = 0;
    
    /* Set Gpio to input mode */
    iicHandle->Ops->Set_SDA_DIR(SDA_IN);
    
    iicHandle->Ops->Set_SDA(SET);
    
    /* 循环读取一个字节的数据 */
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
        IIC_NoAck(iicHandle);	//发送nACK
    }
    else if (iicHandle->Msg->Flags & IIC_NEED_ACK)
    {
        IIC_Ack(iicHandle); 		//发送ACK   
    }
    *(iicHandle->Msg->Data) = Receive;  
}

/**
 * @func    IIC_CheckDevice
 * @brief   检验从设备是否存在
 * @param   _Address 从设备地址
 * @retval	receive 读取到的数据
 */
uint8_t IIC_CheckDevice(IIC_Handle_t iicHandle)
{
    uint8_t Temp;
	if (iicHandle->Ops->Get_SDA())
	{
		IIC_Start(iicHandle);		/* 发送启动信号 */

		/* 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传 */
        Temp = iicHandle->SlaveAddr | IIC_DRV_WR;
        iicHandle->Msg->Data = &Temp;
		IIC_SendByte(iicHandle);

		if (IIC_WaitAck(iicHandle) == IIC_OPER_OK)	/* 检测设备的ACK应答 */
        {
            IIC_Stop(iicHandle);			/* 发送停止信号 */
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
uint8_t IIC_Read(IIC_Handle_t iicHandle)
{
    uint8_t Temp;
    assert_param(iicHandle);
    /* 第1步：发起I2C总线启动信号 */
    IIC_Start(iicHandle);
    
    /* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
    /* 此处是写指令 */
    Temp = iicHandle->SlaveAddr | IIC_DRV_WR;
    iicHandle->Msg->Data = &Temp;

    /* 发送器件地址 */
    IIC_SendByte(iicHandle);	
    
    /* 第3步：等待ACK */
    if (IIC_WaitAck(iicHandle) != IIC_OPER_OK)
    {
        goto _return;	/* 器件无应答 */
    }

    /* 第4步：发送字节地址，判断地址的大小 */
    for (uint32_t i = 0; i < iicHandle->Msg->SubAddrSize; i++)
    {
        iicHandle->Msg->Data = ((uint8_t *)(&(iicHandle->Msg->SubAddr))) + i;
        IIC_SendByte(iicHandle);
        if (IIC_WaitAck(iicHandle) != IIC_OPER_OK)
        {
            goto _return;	/* 器件无应答 */
        }
    }
    
    /* 第6步：重新启动I2C总线。下面开始读取数据 */
    IIC_Start(iicHandle);
    
    /* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
    Temp = iicHandle->SlaveAddr | IIC_DRV_R;
    iicHandle->Msg->Data = &Temp;
    IIC_SendByte(iicHandle);	/* 此处是读指令 */
    
    /* 第8步：等待ACK */
    if (IIC_WaitAck(iicHandle) != IIC_OPER_OK)
    {
        goto _return;	/* 器件无应答 */
    }
    
    /* 第9步：循环读取数据 */
    for (uint32_t i = 0; i < iicHandle->BufSize; i++)
    {
        iicHandle->Msg->Flags = IIC_NEED_ACK;
        iicHandle->Msg->Data = iicHandle->Buf + i;
        IIC_ReadByte(iicHandle);	/* 读1个字节 */

        if (i == iicHandle->BufSize - 1)
        {
            i++;
            iicHandle->Msg->Flags = IIC_NEEDNT_ACK;
            iicHandle->Msg->Data = iicHandle->Buf + i;
            IIC_ReadByte(iicHandle);	/* 读1个字节 */
        }
    }
    
    /* 发送I2C总线停止信号 */
    IIC_Stop(iicHandle);
    
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
uint8_t IIC_Write(IIC_Handle_t iicHandle)
{
    uint8_t Temp;
    assert_param(iicHandle);

    /* 第1步：发起I2C总线启动信号 */
    IIC_Start(iicHandle);
    
    /* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
    /* 此处是写指令 */
    Temp = iicHandle->SlaveAddr | IIC_DRV_WR;
    iicHandle->Msg->Data = &Temp;

    /* 发送器件地址 */
    IIC_SendByte(iicHandle);	
    
    /* 第3步：等待ACK */
    if (IIC_WaitAck(iicHandle) != IIC_OPER_OK)
    {
        goto _return;    /* 器件无应答 */
    }

    /* 第4步：发送字节地址，判断地址的大小 */
    for (uint32_t i = 0; i < iicHandle->Msg->SubAddrSize; i++)
    {
        iicHandle->Msg->Data = ((uint8_t *)(&(iicHandle->Msg->SubAddr))) + i;
        IIC_SendByte(iicHandle);
        if (IIC_WaitAck(iicHandle) != IIC_OPER_OK)
        {
            goto _return;	/* 器件无应答 */
        }
    }

    for (uint32_t i = 0; i < iicHandle->BufSize; i++)
    {
        iicHandle->Msg->Data = iicHandle->Buf + i;
        /* 发送器件地址 */
        IIC_SendByte(iicHandle);    
        /* 等待ACK */
        if (IIC_WaitAck(iicHandle) != IIC_OPER_OK)
        {
            goto _return;    /* 器件无应答 */
        }
    }
    
    /* 命令执行成功，发送I2C总线停止信号 */
    IIC_Stop(iicHandle);
    return IIC_OPER_OK;
    
_return: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */

    return IIC_OPER_FAILT;
}
