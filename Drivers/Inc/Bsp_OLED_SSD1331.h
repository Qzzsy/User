/**
 ******************************************************************************
 * @file      Bsp_OLED_SSD1331.h
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-09-17
 * @brief     实现对SSD1331的驱动
 * @History
 * Date           Author    version    		Notes
 * 2018-09-17       ZSY     V1.0.0      first version.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _BSP_OLED_SSD1331_H_
#define _BSP_OLED_SSD1331_H_

#if defined STM32F1
#include "stm32f1xx.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#endif

/*!< APIs */
void OLED_SSD1331_Init(void);
void ssd1331PutPixel(uint16_t _xPos, uint16_t _yPos, uint16_t _hwColor);
void ssd1331ClrScreen(uint16_t Color);
void ssd1331SetWin(uint16_t _xPos, uint16_t _yPos, uint16_t _Width, uint16_t _Height);
void ssd1331PutPixelNoPos(uint16_t _hwColor);

#endif

