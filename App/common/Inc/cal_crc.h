/**
 ******************************************************************************
 * @Copyright     (C) 2017 - 2018 guet-sctc-hardwarepart Team
 * @filename      cal_crc.h
 * @author        ZSY
 * @version       V1.0.1
 * @date          2019-11-01
 * @Description   CRC计算接口文件
 * @Others
 * @History
 * Date           Author    version    		Notes
 * 2018-10-09      ZSY      V1.0.0      first version.
 * 2019-11-01      ZSY      V1.0.1      添加对CRC16/Modbus的支持
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAL_CRC_H_
#define __CAL_CRC_H_

#if defined STM32F1
#include "STM32F10x.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#endif

typedef struct
{
    unsigned long crc;
} crc32_ctx_t;

void cal_crc8_maxim(const void *pBuf, uint16_t pLen, uint8_t *pOut);
void cal_crc16(const void *pDataIn, int iLenIn, unsigned short *pCRCOut);
void cal_crc16_modbus(const void *data_in, int len, unsigned short *out);
void cal_crc32_init(crc32_ctx_t *ctx);
void cal_crc32_update(crc32_ctx_t *ctx, const unsigned char *data, uint32_t len);
void cal_crc32_final(crc32_ctx_t *ctx, uint32_t *md);

#endif
