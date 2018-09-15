#ifndef _OLED_SSD1331_H_
#define _OLED_SSD1331_H_

#if defined STM32F1
#include "stm32f1xx.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#endif

void OLED_SSD1331_Init(void);
void ssd1331PutPixel(uint16_t _xPos, uint16_t _yPos, uint16_t _hwColor);
void ssd1331ClrScreen(uint16_t Color);
void ssd1331SetWin(uint16_t _xPos, uint16_t _yPos, uint16_t _Width, uint16_t _Height);
void ssd1331PutPixelNoPos(uint16_t _hwColor);

#endif

