#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "udef.h"

#ifdef STM32F1
#include "STM32F1xx.h"
#elif STM32F4
#include "stm32f4xx.h"
#endif

void mem_check(void);



#endif

