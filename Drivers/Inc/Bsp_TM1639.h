/**
 ******************************************************************************
 * @file      bsp_TM1639.h
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-09-14
 * @brief     该文件包含了TM1639的一些API，通过这些API实现对片上的TM1639进行造作
 * @History
 * Date           Author    version    		Notes
 * 2018-09-14       ZSY     V1.0.0      first version.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _BSP_TM1639_H_
#define _BSP_TM1639_H_

#if defined STM32F1
#include "stm32f1xx.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#endif

/*!< Definition */
#define TM_MODE_DISP_DIG    0x01
#define TM_MODE_DISP_LED    0x02

#define DIG0                0xC0
#define DIG1                0xC2
#define DIG2                0xC4
#define DIG3                0xC6
#define DIG4                0xC8
#define DIG5                0xCA
#define DIG6                0xCC
#define DIG7                0xCE

#define LEVEL_OFF           0x80
#define LEVEL_1             0x88
#define LEVEL_2             0x89
#define LEVEL_4             0x8A
#define LEVEL_10            0x8B
#define LEVEL_11            0x8C
#define LEVEL_12            0x8D
#define LEVEL_13            0x8E
#define LEVEL_14            0x8F

/*!< APIs */
void TM1639_Control(uint8_t Cmd);
void TM1639_DispOn(void);
void TM1639_DispOff(void);
void TM1639_Disp(uint8_t * Data, uint8_t Mode);

#endif
