/**
 ******************************************************************************
 * @Copyright     (C) 2017 - 2018 guet-sctc-hardwarepart Team
 * @filename      iap.h
 * @author        ZSY
 * @version       V1.0.0
 * @date          2018-10-25
 * @Description   实现IAP功能。
 * @Others
 * @History
 * Date           Author    version    		    Notes
 * 2018-06-22      ZSY      V1.0.0          first version.
 */
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _IAP_H_
#define _IAP_H_

#if defined STM32F1
#include "stm32f1xx.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#endif

/* APIs */
void Bootloader(void);
void SetBootloaderHooks(void (*SendData)(const void *Data, uint32_t Size),
                        void (*GetDataSize)(uint32_t *Size),
                        void (*ReadData)(void *rData));

#endif

