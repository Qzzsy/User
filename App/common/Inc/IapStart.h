#ifndef _IAP_START_H_
#define _IAP_START_H_

#if defined STM32F1
#include "STM32F1xx.h"
#elif defined STM32F4
#include "STM32F4xx.h"
#endif

void IapStartInit();
void SetUpdateFlag();
void SysReset();

#endif
