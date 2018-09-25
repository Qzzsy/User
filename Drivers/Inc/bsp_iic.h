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
 * 2018-06-20     ZSY       V1.0.4      ��߼�����.
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

#define IIC_NEED_ACK        (1 << 1)
#define IIC_NEEDNT_ACK      (1 << 2)

#define IIC_DRV_WR          (0)     //IICд����
#define IIC_DRV_R           (1)     //IIC������

#define SDA_IN              (0)
#define SDA_OUT             (1)

#ifndef uint32_t
#define uint32_t unsigned int
#endif
/* End public macro Definition -----------------------------------------------*/

typedef struct
{
    uint16_t Speed;
    uint32_t Delay_us;
    void (* Set_SDA)(uint32_t State);
    void (* Set_SCL)(uint32_t State);
    void (* Set_SDA_DIR)(uint32_t State);
    uint8_t (*Get_SDA)(void);
    uint8_t (*Get_SCL)(void);
    void (*uDelay)(uint32_t us);
}IIC_Ops_t;

typedef struct
{
    uint32_t SubAddr;
    uint8_t SubAddrSize;
    uint8_t *Data;
    uint8_t Flags;
}IIC_Msg_t;

typedef struct
{
    IIC_Ops_t *Ops;
    IIC_Msg_t *Msg;
    uint8_t *Buf;
    uint32_t BufSize;
    uint8_t Flags;
    uint32_t Timeout;   /* in tick */
    uint8_t SlaveAddr;
}IIC_Device_t;

typedef IIC_Device_t * IIC_Handle_t;

/* UserCode start ------------------------------------------------------------*/
/* Member method APIs --------------------------------------------------------*/
uint8_t IIC_Read(IIC_Handle_t Handle);
uint8_t IIC_Write(IIC_Handle_t Handle);
uint8_t IIC_CheckDevice(IIC_Handle_t Handle);

/* End Member Method APIs ---------------------------------------------------*/

#endif







