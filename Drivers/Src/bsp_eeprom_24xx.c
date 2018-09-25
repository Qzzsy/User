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
/* Private macro Definition --------------------------------------------------*/

static IIC_Handle_t Bsp_eeHandle = NULL;

void Bsp_eeSet_SDA(uint32_t State)
{
    if (State == SET)
    {
        IIC_SDA_WRITE_H;
    }
    else
    {
        IIC_SDA_WRITE_L;
    }
}

void Bsp_eeSet_SCL(uint32_t State)
{
    if (State == SET)
    {
        IIC_SCL_WRITE_H;
    }
    else
    {
        IIC_SCL_WRITE_L;
    }
}

void Bsp_eeSet_SDA_Dir(uint32_t State)
{
    if (State == SDA_IN)
    {
        SET_IIC_SDA_IN();
    }
    else if (State == SDA_OUT)
    {
        SET_IIC_SDA_OUT();
    }
}

uint8_t Bsp_eeGet_SDA(void)
{
    return IIC_SDA_READ;
}
/**
 * @func    BspIIC_Delay
 * @brief   BspIIC延时方法
 * @param   nCount 时间
 * @retval  无
 */
void Bsp_eeDelay(__IO uint32_t Number)
{
    uint32_t i = 0;

    while (Number--)
    {
        i = 200;
        while (i--);
    }
}

/**
* @func    Bsp_eeCheckOk
* @brief   判断串行EERPOM是否正常
* @retval  AT24XX_OK 正常， AT24XX_FAULT 错误
*/
static uint8_t Bsp_eeCheckOk(void)
{
   if (IIC_CheckDevice(Bsp_eeHandle) == IIC_OPER_OK)
   {
       return AT24XX_OK;
   }
   else
   {
       return AT24XX_FAULT;
   }
}

uint8_t Bsp_eeInit(void)
{
    static IIC_Device_t eeDevice;
    static IIC_Ops_t Ops;
    static IIC_Msg_t Msg;

    Ops.uDelay = Bsp_eeDelay;
    Ops.Set_SDA = Bsp_eeSet_SDA;
    Ops.Set_SCL = Bsp_eeSet_SCL;
    Ops.Set_SDA_DIR = Bsp_eeSet_SDA_Dir;
    Ops.Get_SDA = Bsp_eeGet_SDA;
    Ops.Get_SCL = NULL;
    Ops.Delay_us = 4;

    eeDevice.Ops = &Ops;
    eeDevice.Msg = &Msg;
    eeDevice.SlaveAddr = EE_DEV_ADDR;
    eeDevice.Timeout = 200;
    Bsp_eeHandle = &eeDevice;
    
    return Bsp_eeCheckOk();
}

void Bsp_eeDeinit(void)
{
    Bsp_eeHandle = NULL;
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

    Bsp_eeHandle->Buf = _pReadBuf;
    Bsp_eeHandle->BufSize = _usSize;
    Bsp_eeHandle->Msg->SubAddr = _usAddress;
    Bsp_eeHandle->Msg->SubAddrSize = EE_ADDR_BYTES;

    if (IIC_Read(Bsp_eeHandle) != IIC_OPER_OK)
    {
        return AT24XX_FAULT;
    }
    
    /* 执行成功 */
    return AT24XX_OK;	
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
   
   /*
       写串行EEPROM不像读操作可以连续读取很多字节，每次写操作只能在同一个page。
       对于24xx02，page size = 8
       简单的处理方法为：按字节写操作模式，每写1个字节，都发送地址
       为了提高连续写的效率: 本函数采用page wirte操作。
   */
   
    for (i = 0; i < _usSize; i++)
    {
        Bsp_eeHandle->Buf = _pWriteBuf + i;
        Bsp_eeHandle->BufSize = 1;
        Bsp_eeHandle->Msg->SubAddr = _usAddress + i;
        Bsp_eeHandle->Msg->SubAddrSize = EE_ADDR_BYTES;
        
        if (IIC_Write(Bsp_eeHandle) != IIC_OPER_OK)
        {
            return AT24XX_FAULT;
        }
        HAL_Delay(1);
    }
   
    return AT24XX_OK;
}

