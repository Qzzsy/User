/**
 ******************************************************************************
 * @file      bsp_eeprom_24xx.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-06-21
 * @brief     实现24xx系列EEPROM的读写操作。写操作采用页写模式提高写入效率。
 * @History
 * Date           Author    version    		Notes
 * 2018-06-21     ZSY       V1.0.0          first version.
 */

/* Includes ------------------------------------------------------------------*/
#include "bsp_eeprom_24xx.h"
#include "bsp_iic.h"   

/* Private macro Definition --------------------------------------------------*/

/**
 * @func    Bsp_eeCheckOk
 * @brief   判断串行EERPOM是否正常
 * @retval  AT24XX_OK 正常， AT24XX_FAULT 错误
 */
uint8_t Bsp_eeCheckOk(void)
{
    if (IIC_CheckDevice(EE_DEV_ADDR) == IIC_OPER_OK)
    {
        return AT24XX_OK;
    }
    else
    {
        /* 失败后，切记发送I2C总线停止信号 */
        IIC_Stop();
        return AT24XX_FAULT;
    }
}

/**
 * @func    Bsp_eeReadBytes
 * @brief   从串行EEPROM指定地址处开始读取若干数据
 * @param   _usAddress : 起始地址
 * @param   _usSize : 数据长度，单位为字节
 * @param   _pReadBuf : 存放读到的数据的缓冲区指针
 * @retval  AT24XX_FAULT 表示失败，AT24XX_OK表示成功
 */
uint8_t Bsp_eeReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize)
{
    uint16_t i;
    
    /* 采用串行EEPROM随即读取指令序列，连续读取若干字节 */
    
    /* 第1步：发起I2C总线启动信号 */
    IIC_Start();
    
    /* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
    IIC_SendByte(EE_DEV_ADDR | IIC_DRV_WR);	/* 此处是写指令 */
    
    /* 第3步：发送ACK */
    if (IIC_WaitAck() != IIC_OPER_OK)
    {
        goto _return;	/* EEPROM器件无应答 */
    }
    
    /* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
    if (EE_ADDR_BYTES == 1)
    {
        IIC_SendByte((uint8_t)_usAddress);
        if (IIC_WaitAck() != IIC_OPER_OK)
        {
            goto _return;	/* EEPROM器件无应答 */
        }
    }
    else
    {
        IIC_SendByte(_usAddress >> 8);
        if (IIC_WaitAck() != IIC_OPER_OK)
        {
            goto _return;	/* EEPROM器件无应答 */
        }
    
        IIC_SendByte(_usAddress);
        if (IIC_WaitAck() != IIC_OPER_OK)
        {
            goto _return;	/* EEPROM器件无应答 */
        }
    }
    
    /* 第6步：重新启动I2C总线。下面开始读取数据 */
    IIC_Start();
    
    /* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
    IIC_SendByte(EE_DEV_ADDR | IIC_DRV_R);	/* 此处是读指令 */
    
    /* 第8步：发送ACK */
    if (IIC_WaitAck() != IIC_OPER_OK)
    {
        goto _return;	/* EEPROM器件无应答 */
    }
    
    /* 第9步：循环读取数据 */
    for (i = 0; i < _usSize - 1; i++)
    {
        _pReadBuf[i] = IIC_ReadByte(IIC_NEED_ACK);	/* 读1个字节 */
    }
    
    _pReadBuf[i] = IIC_ReadByte(IIC_NEEDNT_ACK);	/* 读1个字节 */
    
    /* 发送I2C总线停止信号 */
    IIC_Stop();
    
    /* 执行成功 */
    return AT24XX_OK;	
    
_return: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */

    return AT24XX_FAULT;
}

/**
 * @func    Bsp_eeWriteBytes
 * @brief   向串行EEPROM指定地址写入若干数据，采用页写操作提高写入效率
 * @param   _usAddress : 起始地址
 * @param   _usSize : 数据长度，单位为字节
 * @param   _pWriteBuf : 存放读到的数据的缓冲区指针
 * @retval  AT24XX_FAULT 表示失败，AT24XX_OK 表示成功
 */
uint8_t Bsp_eeWriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize)
{
    uint16_t i, m;
    uint16_t usAddr;
    
    /*
        写串行EEPROM不像读操作可以连续读取很多字节，每次写操作只能在同一个page。
        对于24xx02，page size = 8
        简单的处理方法为：按字节写操作模式，每写1个字节，都发送地址
        为了提高连续写的效率: 本函数采用page wirte操作。
    */
    
    usAddr = _usAddress;
    for (i = 0; i < _usSize; i++)
    {
        /* 当发送第1个字节或是页面首地址时，需要重新发起启动信号和地址 */
        if ((i == 0) || (usAddr & (EE_PAGE_SIZE - 1)) == 0)
        {
            /*　第０步：发停止信号，启动内部写操作　*/
            IIC_Stop();
    
            /* 通过检查器件应答的方式，判断内部写操作是否完成, 一般小于 10ms
                CLK频率为200KHz时，查询次数为30次左右
            */
            for (m = 0; m < 1000; m++)
            {
                /* 第1步：发起I2C总线启动信号 */
                IIC_Start();
    
                /* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
                IIC_SendByte(EE_DEV_ADDR | IIC_DRV_WR);	/* 此处是写指令 */
    
                /* 第3步：发送一个时钟，判断器件是否正确应答 */
                if (IIC_WaitAck() == IIC_OPER_OK)
                {
                    break;
                }
            }
            if (m  == 1000)
            {
                goto _return;	/* EEPROM器件写超时 */
            }
    
            /* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
            if (EE_ADDR_BYTES == 1)
            {
                IIC_SendByte((uint8_t)usAddr);
                if (IIC_WaitAck() != IIC_OPER_OK)
                {
                    goto _return;	/* EEPROM器件无应答 */
                }
            }
            else
            {
                IIC_SendByte(usAddr >> 8);
                if (IIC_WaitAck() != IIC_OPER_OK)
                {
                    goto _return;	/* EEPROM器件无应答 */
                }
    
                IIC_SendByte(usAddr);
                if (IIC_WaitAck() != IIC_OPER_OK)
                {
                    goto _return;	/* EEPROM器件无应答 */
                }
            }
        }
    
        /* 第6步：开始写入数据 */
        IIC_SendByte(_pWriteBuf[i]);
    
        /* 第7步：发送ACK */
        if (IIC_WaitAck() != 0)
        {
            goto _return;	/* EEPROM器件无应答 */
        }
    
        usAddr++;	/* 地址增1 */
    }
    
    /* 命令执行成功，发送I2C总线停止信号 */
    IIC_Stop();
    return AT24XX_OK;
    
_return: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */

    return AT24XX_FAULT;
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
