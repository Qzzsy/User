#ifndef _BATVOLTAGE_H_
#define _BATVOLTAGE_H_

#ifdef STM32F1
#include "stm32f1xx.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#endif

uint16_t GetBatVoltage(void);
void BatElecDisp(void);

#endif
