#ifndef _BSP_RTP_TOUCH_H_
#define _BSP_RTP_TOUCH_H_

#ifdef STM32F1
#include "stm32f10cx.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#endif

#define RTP_OK              0
#define RTP_FAULT           1

#define RTP_PRESS           (1 << 0)
#define RTP_LIFT_UP         (1 << 1)

uint8_t RTP_Scan(void);
void RTP_Adjust(void);

#endif


