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

///**
// * @func    Bsp_eeCheckOk
// * @brief   �жϴ���EERPOM�Ƿ�����
// * @retval  AT24XX_OK ������ AT24XX_FAULT ����
// */
uint8_t Bsp_eeCheckOk(void)
{
//    if (IIC_CheckDevice(EE_DEV_ADDR) == IIC_OPER_OK)
//    {
//        return AT24XX_OK;
//    }
//    else
//    {
//        /* ʧ�ܺ��мǷ���I2C����ֹͣ�ź� */
//        IIC_Stop();
//        return AT24XX_FAULT;
//    }
}

///**
// * @func    Bsp_eeReadBytes
// * @brief   �Ӵ���EEPROMָ����ַ����ʼ��ȡ��������
// * @param   _usAddress : ��ʼ��ַ
// * @param   _usSize : ���ݳ��ȣ���λΪ�ֽ�
// * @param   _pReadBuf : ��Ŷ��������ݵĻ�����ָ��
// * @retval  AT24XX_FAULT ��ʾʧ�ܣ�AT24XX_OK��ʾ�ɹ�
// */
uint8_t Bsp_eeReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize)
{
//    uint16_t i;
//    
//    /* ���ô���EEPROM�漴��ȡָ�����У�������ȡ�����ֽ� */
//    
//    /* ��1��������I2C���������ź� */
//    IIC_Start();
//    
//    /* ��2������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
//    IIC_SendByte(EE_DEV_ADDR | IIC_DRV_WR);	/* �˴���дָ�� */
//    
//    /* ��3��������ACK */
//    if (IIC_WaitAck() != IIC_OPER_OK)
//    {
//        goto _return;	/* EEPROM������Ӧ�� */
//    }
//    
//    /* ��4���������ֽڵ�ַ��24C02ֻ��256�ֽڣ����1���ֽھ͹��ˣ������24C04���ϣ���ô�˴���Ҫ���������ַ */
//    if (EE_ADDR_BYTES == 1)
//    {
//        IIC_SendByte((uint8_t)_usAddress);
//        if (IIC_WaitAck() != IIC_OPER_OK)
//        {
//            goto _return;	/* EEPROM������Ӧ�� */
//        }
//    }
//    else
//    {
//        IIC_SendByte(_usAddress >> 8);
//        if (IIC_WaitAck() != IIC_OPER_OK)
//        {
//            goto _return;	/* EEPROM������Ӧ�� */
//        }
//    
//        IIC_SendByte(_usAddress);
//        if (IIC_WaitAck() != IIC_OPER_OK)
//        {
//            goto _return;	/* EEPROM������Ӧ�� */
//        }
//    }
//    
//    /* ��6������������I2C���ߡ����濪ʼ��ȡ���� */
//    IIC_Start();
//    
//    /* ��7������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
//    IIC_SendByte(EE_DEV_ADDR | IIC_DRV_R);	/* �˴��Ƕ�ָ�� */
//    
//    /* ��8��������ACK */
//    if (IIC_WaitAck() != IIC_OPER_OK)
//    {
//        goto _return;	/* EEPROM������Ӧ�� */
//    }
//    
//    /* ��9����ѭ����ȡ���� */
//    for (i = 0; i < _usSize - 1; i++)
//    {
//        _pReadBuf[i] = IIC_ReadByte(IIC_NEED_ACK);	/* ��1���ֽ� */
//    }
//    
//    _pReadBuf[i] = IIC_ReadByte(IIC_NEEDNT_ACK);	/* ��1���ֽ� */
//    
//    /* ����I2C����ֹͣ�ź� */
//    IIC_Stop();
//    
//    /* ִ�гɹ� */
//    return AT24XX_OK;	
//    
//_return: /* ����ִ��ʧ�ܺ��мǷ���ֹͣ�źţ�����Ӱ��I2C�����������豸 */

//    return AT24XX_FAULT;
}

///**
// * @func    Bsp_eeWriteBytes
// * @brief   ����EEPROMָ����ַд���������ݣ�����ҳд�������д��Ч��
// * @param   _usAddress : ��ʼ��ַ
// * @param   _usSize : ���ݳ��ȣ���λΪ�ֽ�
// * @param   _pWriteBuf : ��Ŷ��������ݵĻ�����ָ��
// * @retval  AT24XX_FAULT ��ʾʧ�ܣ�AT24XX_OK ��ʾ�ɹ�
// */
uint8_t Bsp_eeWriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize)
{
//    uint16_t i, m;
//    uint16_t usAddr;
//    
//    /*
//        д����EEPROM�������������������ȡ�ܶ��ֽڣ�ÿ��д����ֻ����ͬһ��page��
//        ����24xx02��page size = 8
//        �򵥵Ĵ�����Ϊ�����ֽ�д����ģʽ��ÿд1���ֽڣ������͵�ַ
//        Ϊ���������д��Ч��: ����������page wirte������
//    */
//    
//    usAddr = _usAddress;
//    for (i = 0; i < _usSize; i++)
//    {
//        /* �����͵�1���ֽڻ���ҳ���׵�ַʱ����Ҫ���·��������źź͵�ַ */
//        if ((i == 0) || (usAddr & (EE_PAGE_SIZE - 1)) == 0)
//        {
//            /*���ڣ�������ֹͣ�źţ������ڲ�д������*/
//            IIC_Stop();
//    
//            /* ͨ���������Ӧ��ķ�ʽ���ж��ڲ�д�����Ƿ����, һ��С�� 10ms
//                CLKƵ��Ϊ200KHzʱ����ѯ����Ϊ30������
//            */
//            for (m = 0; m < 1000; m++)
//            {
//                /* ��1��������I2C���������ź� */
//                IIC_Start();
//    
//                /* ��2������������ֽڣ���7bit�ǵ�ַ��bit0�Ƕ�д����λ��0��ʾд��1��ʾ�� */
//                IIC_SendByte(EE_DEV_ADDR | IIC_DRV_WR);	/* �˴���дָ�� */
//    
//                /* ��3��������һ��ʱ�ӣ��ж������Ƿ���ȷӦ�� */
//                if (IIC_WaitAck() == IIC_OPER_OK)
//                {
//                    break;
//                }
//            }
//            if (m  == 1000)
//            {
//                goto _return;	/* EEPROM����д��ʱ */
//            }
//    
//            /* ��4���������ֽڵ�ַ��24C02ֻ��256�ֽڣ����1���ֽھ͹��ˣ������24C04���ϣ���ô�˴���Ҫ���������ַ */
//            if (EE_ADDR_BYTES == 1)
//            {
//                IIC_SendByte((uint8_t)usAddr);
//                if (IIC_WaitAck() != IIC_OPER_OK)
//                {
//                    goto _return;	/* EEPROM������Ӧ�� */
//                }
//            }
//            else
//            {
//                IIC_SendByte(usAddr >> 8);
//                if (IIC_WaitAck() != IIC_OPER_OK)
//                {
//                    goto _return;	/* EEPROM������Ӧ�� */
//                }
//    
//                IIC_SendByte(usAddr);
//                if (IIC_WaitAck() != IIC_OPER_OK)
//                {
//                    goto _return;	/* EEPROM������Ӧ�� */
//                }
//            }
//        }
//    
//        /* ��6������ʼд������ */
//        IIC_SendByte(_pWriteBuf[i]);
//    
//        /* ��7��������ACK */
//        if (IIC_WaitAck() != 0)
//        {
//            goto _return;	/* EEPROM������Ӧ�� */
//        }
//    
//        usAddr++;	/* ��ַ��1 */
//    }
//    
//    /* ����ִ�гɹ�������I2C����ֹͣ�ź� */
//    IIC_Stop();
//    return AT24XX_OK;
//    
//_return: /* ����ִ��ʧ�ܺ��мǷ���ֹͣ�źţ�����Ӱ��I2C�����������豸 */

//    return AT24XX_FAULT;
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
