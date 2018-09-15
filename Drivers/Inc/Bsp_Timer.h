/**
 ******************************************************************************
 * @file      bsp_timer.h
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-09-14
 * @brief     该文件包含了Timer的一些API，通过这些API实现对片上的Timer进行造作
 * @History
 * Date           Author    version    		Notes
 * 2018-09-14       ZSY     V1.0.0      first version.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _BSP_TIMER_H_
#define _BSP_TIMER_H_

#if defined STM32F1
#include "stm32f1xx.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#endif
#include "tim.h"

/*!< APIs */
void TimerStart(TIM_HandleTypeDef * tim_baseHandle);
void TimerStartIT(TIM_HandleTypeDef * tim_baseHandle);
void TimerStartPWM(TIM_HandleTypeDef * tim_baseHandle, uint16_t Channel);
void PwmSetCompare(TIM_HandleTypeDef * tim_baseHandle, uint16_t Channel, uint16_t IndexWave);

#endif


