/**
 ******************************************************************************
 * @Copyright     (C) 2017 - 2018 guet-sctc-hardwarepart Team
 * @filename      bsp_iic.h
 * @author        �Ž�����С��
 * @version       V1.0.4
 * @date          2018-06-20
 * @Description   bsp_iic�ļ����ڴ��ļ��ڶ�����һЩiic�����ź궨�壬��Ҫ����iic��ʱ
                  ������ʱ�ڴ��ļ��ڽ��������޸ļ��ɣ�������ļ�����iic���⿪�ŵ�API
 * @Others
 * @History
 * Date           Author    version    		Notes
 * 2017-11-01     ZSY       V1.0.0      first version.
 * 2017-11-02     ZSY       V1.0.1      �����˺�IIC_ACK_TIMEOUT��IIC_OPER_OK��
                                        IIC_OPER_FAILT
 * 2018-01-09     ZSY       V1.0.2      �Ű��ʽ������.
 * 2018-01-26     ZSY       V1.0.3      ���˽�к͹��к궨��.
 * 2018-06-20     ZSY       V1.0.4          ��߼�����.
 */
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _IIC_H_
#define _IIC_H_

/* Includes ------------------------------------------------------------------*/
#ifdef STM32F1
#include "stm32f10x.h"
#elif defined STM32F4
#include "stm32f4xx.h"	
#endif

/* Public macro Definition ---------------------------------------------------*/

/* ���������� */
#define IIC_OPER_OK         (0)		//�����ɹ�
#define IIC_OPER_FAILT      (1)		//����ʧ��

#define IIC_NEED_ACK        (1)
#define IIC_NEEDNT_ACK      (0)

#define IIC_DRV_WR          (0)     //IICд����
#define IIC_DRV_R           (1)     //IIC������
/* End public macro Definition -----------------------------------------------*/

/* UserCode start ------------------------------------------------------------*/
/* Member method APIs --------------------------------------------------------*/
/* config iic gpio */
void IIC_GPIO_Config(void);

void IIC_Start(void); 
void IIC_Stop(void);
uint8_t IIC_WaitAck(void);
void IIC_SendByte(uint8_t Data);
uint8_t IIC_ReadByte(uint8_t Ack);
uint8_t IIC_CheckDevice(uint8_t _Address);

/* End Member Method APIs ---------------------------------------------------*/

#endif







