#ifndef _BSP_ADC_H_
#define _BSP_ADC_H_

#if defined STM32F1
#include "stm32f1xx.h"
#elif STM32F4
#include "stm32f4xx.h"
#endif

uint8_t SetAdcConvChannel(ADC_HandleTypeDef * _Handle, uint32_t Channel, uint32_t SamplingTime);
uint16_t GetAdcValue(ADC_HandleTypeDef * _Handle);
ADC_HandleTypeDef * GetAdc1Handle(void);
ADC_HandleTypeDef * GetAdc2Handle(void);

#endif
