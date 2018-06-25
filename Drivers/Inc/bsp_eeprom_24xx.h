/**
 ******************************************************************************
 * @Copyright     (C) 2017 - 2018 guet-sctc-hardwarepart Team
 * @filename      bsp_eeprom_24xx.h
 * @author        �Ž�����С��
 * @version       V1.0.0
 * @date          2018-06-21
 * @Description   ����EEPROM 24xx02����ģ��
 * @Others
 * @History
 * Date           Author    version    		Notes
 * 2018-06-21     ZSY       V1.0.0      first version.
 */
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _BSP_EEPROM_24XX_H
#define _BSP_EEPROM_24XX_H

#if defined STM32F1
#include "STM32f10x.h"
#elif defined STM32F4
#include "STM32f4xx.h"
#endif

#define AT24C02
//#define AT24C128

#define AT24XX_OK           0
#define AT24XX_FAULT        1	

#ifdef AT24C02
	#define EE_MODEL_NAME       "AT24C02"
	#define EE_DEV_ADDR         0xA0		/* �豸��ַ */
	#define EE_PAGE_SIZE        8			/* ҳ���С(�ֽ�) */
	#define EE_SIZE             256			/* ������(�ֽ�) */
	#define EE_ADDR_BYTES       1			/* ��ַ�ֽڸ��� */
#endif

#ifdef AT24C128
	#define EE_MODEL_NAME		"AT24C128"
	#define EE_DEV_ADDR			0xA0		/* �豸��ַ */
	#define EE_PAGE_SIZE		64			/* ҳ���С(�ֽ�) */
	#define EE_SIZE				(16*1024)	/* ������(�ֽ�) */
	#define EE_ADDR_BYTES		2			/* ��ַ�ֽڸ��� */
#endif

uint8_t Bsp_eeInit(void);
uint8_t Bsp_eeReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize);
uint8_t Bsp_eeWriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize);

#endif


