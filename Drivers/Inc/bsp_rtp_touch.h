#ifndef _BSP_RTP_TOUCH_H_
#define _BSP_RTP_TOUCH_H_

#ifdef STM32F1
#include "stm32f10cx.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#endif

uint8_t RTP_Scan(void);

#endif


