#ifndef _BSP_TIMER_H_
#define _BSP_TIMER_H_

#if defined STM32F1
#include "stm32f1xx.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#endif
#include "tim.h"

void TimerStart(TIM_HandleTypeDef* tim_baseHandle);

#endif


