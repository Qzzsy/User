
#ifndef _LED_H_
#define _LED_H_

#if defined STM32F1
#include "stm32f1xx.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#endif

void LED0_On(void);
void LED0_Off(void);
void LED1_On(void);
void LED1_Off(void);

#endif

