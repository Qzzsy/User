/**
 ******************************************************************************
 * @Copyright     (C) 2017 - 2018 guet-sctc-hardwarepart Team
 * @filename      queue.h
 * @author        ZSY
 * @version       V1.0.0
 * @date          2018-10-09
 * @Description   CRC计算接口文件
 * @Others
 * @History
 * Date           Author    version    		Notes
 * 2018-10-09      ZSY      V1.0.0      first version.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAL_CRC_H_
#define __CAL_CRC_H_

#if defined STM32F1
#include "STM32F1xx.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#endif

typedef struct
{
    unsigned long crc;
} CRC32_CTX_t;

void CalCrc8_Maxim(const void *pBuf, uint16_t pLen, uint8_t *pOut);
void CalCrc16(const void *pDataIn, int iLenIn, unsigned short *pCRCOut);
void CalCrc32Init(CRC32_CTX_t *ctx);
void CalCrc32Update(CRC32_CTX_t *ctx, const unsigned char *data, size_t len);
void CalCrc32Final(CRC32_CTX_t *ctx, unsigned char *md);

#endif
