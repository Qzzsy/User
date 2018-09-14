/**
 ******************************************************************************
 * @file      bsp_Adc.h
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-09-14
 * @brief     该文件包含了ADC的一些API，通过这些API实现对片上的ADC进行造作
 * @History
 * Date           Author    version    		Notes
 * 2018-09-14       ZSY     V1.0.0      first version.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _BSP_ADC_H_
#define _BSP_ADC_H_

#if defined STM32F1
#include "stm32f1xx.h"
#elif STM32F4
#include "stm32f4xx.h"
#endif

/*!< APIs */
uint8_t SetAdcConvChannel(ADC_HandleTypeDef * _Handle, uint32_t Channel, uint32_t SamplingTime);
uint16_t GetAdcValue(ADC_HandleTypeDef * _Handle);
ADC_HandleTypeDef * GetAdc1Handle(void);
ADC_HandleTypeDef * GetAdc2Handle(void);

#endif
