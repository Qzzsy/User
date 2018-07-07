#ifndef _DRV_LCD_H_
#define _DRV_LCD_H_

/* Includes ------------------------------------------------------------------*/
#if defined STM32F1
#include "stm32f10x.h"
#elif STM32F4
#include "stm32f4xx.h"	 
#endif
/* Public macro Definition ---------------------------------------------------*/

/* ����LCD�Ĵ�С */
#define LCD_HEIGHT          480
#define LCD_WDITH           320

/* �궨��LCD��ɨ�跽�� */
#define DIR_HORIZONTAL_NORMAL           0x01
#define DIR_HORIZONTAL_REVERSE          0x02
#define DIR_VERTICAL_NORMAL             0x03
#define DIR_VERTICAL_REVERSE            0x04

/* ����LCD��������ĺ궨�� */
#define LCD_LIGHT_GPIO          LCD_BL_Pin
#define LCD_LIGHT_PORT          LCD_BL_GPIO_Port
#define LCD_LIGHT_RCC           RCC_APB2Periph_GPIOA

#if defined USE_FULL_LL_DRIVER
#define LCD_LIGHT_ON    LL_GPIO_SetOutputPin(LCD_LIGHT_PORT, LCD_LIGHT_GPIO);
#define LCD_LIGHT_OFF   LL_GPIO_ResetOutputPin(LCD_LIGHT_PORT, LCD_LIGHT_GPIO)
#elif defined USE_HAL_DRIVER
#define LCD_LIGHT_ON    HAL_GPIO_WritePin(LCD_LIGHT_PORT, LCD_LIGHT_GPIO, GPIO_PIN_SET);
#define LCD_LIGHT_OFF   HAL_GPIO_WritePin(LCD_LIGHT_PORT, LCD_LIGHT_GPIO, GPIO_PIN_RESET)
#endif

/* �����Ƿ�ʹ���ֶ�����LCD_ID */
#define USING_LCD_ID
#ifdef USING_LCD_ID
#define LCD_ID      0x61529
#endif
/* End public macro Definition -----------------------------------------------*/

/* UserCode start ------------------------------------------------------------*/

/* LCD��ַ�ṹ�� */    
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

typedef union
{
    __IO uint16_t Value;
    
    struct
    {
        __IO uint16_t B : 5;
        __IO uint16_t G : 6;
        __IO uint16_t R : 5;
    }RGB;
    
}RGB_t;


/* LCD��Ҫ������ */
typedef struct
{					    
	uint16_t pWidth;               //LCD ��ȣ�������Ļ����ı�
	uint16_t pHeight;              //LCD �߶ȣ�������Ļ����ı�				    
	uint16_t Width;                 //LCD ���
	uint16_t Height;                //LCD �߶�
	uint32_t Id;                    //LCD ID
	uint8_t  Dir;                   //���������������ƣ�0��������1��������	
	uint16_t WramCmd;               //��ʼдgramָ��
	uint16_t SetXCmd;               //����x����ָ��
	uint16_t SetYCmd;               //����y����ָ�� 
	uint16_t MemoryAccContCmd;		
	uint16_t DirHorNormalData;
	uint16_t DirHorReverseData;
	uint16_t DirVerNormalData;
	uint16_t DirVerReverseData;
	uint16_t DispOnCmd;
	uint16_t DispOffCmd;
}BspLCD_Dev_t; 

/* Member method APIs --------------------------------------------------------*/
/* ����LCD�ķ�����Ա */
typedef struct
{
    void (*Init)(void);
    
    void (*SetDispDir)(uint8_t Dir);
    
    void (*SetDispWin)(uint16_t xCur, uint16_t yCur, 
                        uint16_t Width,uint16_t Height);
    
    void (*SetDispCur)(uint16_t xPos, uint16_t yPos);
    
    uint16_t (*BGR2RGB)(uint16_t c);
    
    void (*DispOn)(void);
    
    void (*DispOff)(void);
    
    uint16_t (*GetXSize)(void);
    
    uint16_t (*GetYSize)(void);
    
    void (*ClrScr)(uint16_t pColor);
    
    void (*PutPixel)(uint16_t xCur, uint16_t yCur, 
                    uint16_t pColor);
    
    void (*PutPixelNoPos)(uint16_t pColor);
    
    uint16_t (*GetPixel)(uint16_t xCur, uint16_t yCur);

    void (*Fill)(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t pColor);

    void (*FillColor)(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t * pColor);
    
}BspLCD_Func_t;

extern BspLCD_Dev_t BspLCD_Dev;
extern BspLCD_Func_t BspLCD;

#define BLUE                0x001f
#define RED                 0xf800
#define GREEN               0x07e0
#define WHITE               0xffff
#define BLACK               0x0000

static inline void BspLCD_PutPixelNoXY(uint16_t pColor)
{
    extern BspLCD_t * BspLCD_RW;
    BspLCD_RW->RAM = pColor;
}
void BspLCD_FuncInit(void);
/* End Member Method APIs ---------------------------------------------------*/
/* UserCode end -------------------------------------------------------------*/

#endif  
	 
	 



