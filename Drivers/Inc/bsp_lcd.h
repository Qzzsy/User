#ifndef _DRV_LCD_H_
#define _DRV_LCD_H_

/* Includes ------------------------------------------------------------------*/
#ifdef STM32F4
#include "stm32f4xx.h"	 
#endif
/* Public macro Definition ---------------------------------------------------*/

/* 定义LCD的大小 */
#define LCD_HEIGHT          480
#define LCD_WDITH           320

/* 宏定义LCD的扫描方向 */
#define DIR_HORIZONTAL_NORMAL           0x01
#define DIR_HORIZONTAL_REVERSE          0x02
#define DIR_VERTICAL_NORMAL             0x03
#define DIR_VERTICAL_REVERSE            0x04

/* 定义LCD背光操作的宏定义 */
#define LCD_LIGHT_GPIO          LCD_BL_Pin
#define LCD_LIGHT_PORT          LCD_BL_GPIO_Port
#define LCD_LIGHT_RCC           RCC_APB2Periph_GPIOA

#define LCD_LIGHT_ON    LL_GPIO_SetOutputPin(LCD_LIGHT_PORT, LCD_LIGHT_GPIO);
#define LCD_LIGHT_OFF   LL_GPIO_ResetOutputPin(LCD_LIGHT_PORT, LCD_LIGHT_GPIO)

/* 定义是否使用手动输入LCD_ID */
#define USING_LCD_ID
#ifdef USING_LCD_ID
#define LCD_ID      0x9341
#endif
/* End public macro Definition -----------------------------------------------*/

/* UserCode start ------------------------------------------------------------*/

/* LCD地址结构体 */    
typedef struct
{
    __IO uint16_t REG;
    __IO uint16_t RAM;
}BspLCD_t;

typedef union
{
    __IO uint16_t xyPos;
    struct
    {
        __IO uint8_t lBit;
        __IO uint8_t hBit;
    }Pos;
}BspLCD_Pos_t;


/* LCD重要参数集 */
typedef struct
{					    
	uint16_t pWidth;               //LCD 宽度，不随屏幕方向改变
	uint16_t pHeight;              //LCD 高度，不随屏幕方向改变				    
	uint16_t Width;                 //LCD 宽度
	uint16_t Height;                //LCD 高度
	uint32_t Id;                    //LCD ID
	uint8_t  Dir;                   //横屏还是竖屏控制：0，竖屏；1，横屏。	
	uint16_t WramCmd;               //开始写gram指令
	uint16_t SetXCmd;               //设置x坐标指令
	uint16_t SetYCmd;               //设置y坐标指令 
	uint16_t MemoryAccContCmd;		
	uint16_t DirHorNormalData;
	uint16_t DirHorReverseData;
	uint16_t DirVerNormalData;
	uint16_t DirVerReverseData;
	uint16_t DispOnCmd;
	uint16_t DispOffCmd;
}BspLCD_Dev_t; 

/* Member method APIs --------------------------------------------------------*/
/* 访问LCD的方法成员 */
typedef struct
{
    void (*Init)(void);
    
    void (*SetDispDir)(uint8_t Dir);
    
    void (*SetDispWin)(uint16_t xCur, uint16_t yCur, 
                        uint16_t Width,uint16_t Height);
    
    void (*SetDispCur)(uint16_t xPos, uint16_t yPos);
    
    uint16_t (*BGRToRGB)(uint16_t c);
    
    void (*DispOn)(void);
    
    void (*DispOff)(void);
    
    uint16_t (*GetXSize)(void);
    
    uint16_t (*GetYSize)(void);
    
    void (*ClrScr)(uint16_t pColor);
    
    void (*PutPixel)(uint16_t xCur, uint16_t yCur, 
                    uint16_t pColor);
    
    void (*PutPixelNoPos)(uint16_t pColor);
    
    uint16_t (*GetPixel)(uint16_t xCur, uint16_t yCur);
    
}BspLCD_Func_t;

extern BspLCD_Dev_t BspLCD_Dev;
extern BspLCD_Func_t BspLCD;
void BspLCD_FuncInit(void);
/* End Member Method APIs ---------------------------------------------------*/
/* UserCode end -------------------------------------------------------------*/

#endif  
	 
	 



