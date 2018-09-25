/**
 ******************************************************************************
 * @file      bsp_eeprom_24xx.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-06-21
 * @brief     ʵ��24xxϵ��EEPROM�Ķ�д������д��������ҳдģʽ���д��Ч�ʡ�
 * @History
 * Date           Author    version    		Notes
 * 2018-06-21     ZSY       V1.0.0          first version.
 */

/* Includes ------------------------------------------------------------------*/
#include "bsp_eeprom_24xx.h"
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
 * @brief   BspIIC��ʱ����
 * @param   nCount ʱ��
 * @retval  ��
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
* @brief   �жϴ���EERPOM�Ƿ�����
* @retval  AT24XX_OK ������ AT24XX_FAULT ����
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
* @brief   �Ӵ���EEPROMָ����ַ����ʼ��ȡ��������
* @param   _usAddress : ��ʼ��ַ
* @param   _usSize : ���ݳ��ȣ���λΪ�ֽ�
* @param   _pReadBuf : ��Ŷ��������ݵĻ�����ָ��
* @retval  AT24XX_FAULT ��ʾʧ�ܣ�AT24XX_OK��ʾ�ɹ�
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
    
    /* ִ�гɹ� */
    return AT24XX_OK;	
}

/**
* @func    Bsp_eeWriteBytes
* @brief   ����EEPROMָ����ַд���������ݣ�����ҳд�������д��Ч��
* @param   _usAddress : ��ʼ��ַ
* @param   _usSize : ���ݳ��ȣ���λΪ�ֽ�
* @param   _pWriteBuf : ��Ŷ��������ݵĻ�����ָ��
* @retval  AT24XX_FAULT ��ʾʧ�ܣ�AT24XX_OK ��ʾ�ɹ�
*/
uint8_t Bsp_eeWriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize)
{
   uint16_t i, m;
   
   /*
       д����EEPROM�������������������ȡ�ܶ��ֽڣ�ÿ��д����ֻ����ͬһ��page��
       ����24xx02��page size = 8
       �򵥵Ĵ�����Ϊ�����ֽ�д����ģʽ��ÿд1���ֽڣ������͵�ַ
       Ϊ���������д��Ч��: ����������page wirte������
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

