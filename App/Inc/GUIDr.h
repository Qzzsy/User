/**
 ******************************************************************************
 * @Copyright     (C) 2017 - 2018 guet-sctc-hardwarepart Team
 * @filename      GUIDr.h
 * @author        ZSY
 * @version       V1.0.0
 * @date          2018-06-22
 * @Description   集成了液晶的基本操作，实现汉字、英文和中英文混合显示，实现各类基本的
 *                2D图像绘制。
 * @Others
 * @History
 * Date           Author    version    		Notes
 * 2018-06-22      ZSY      V1.0.0      first version.
 */
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _GUIDR_H_
#define _GUIDR_H_

#include "Font.h"

#define LCD_WIDTH           320
#define LCD_HEIGHT          240

/* 点阵数据缓存, 必须大于等于单个字模需要的存储空间*/ 
#define BYTES_PER_FONT      512 

#define USE_SMALL_FONT
#define USE_GBK_FONT
#define USE_GB2312_FONT

#define LEFT_ALIGN          1 << 0
#define RIGHT_ALIGN         1 << 1
#define TOP_ALIGN           1 << 2
#define BOTTOM_ALIGN        1 << 3
#define CENTER_ALIGN        1 << 4

#ifdef USE_SMALL_FONT
/* 中文字库最大容量 */
#define CharNumMax          200
#endif

/* 字体颜色默认值 */
#define GUI_TextDefaultColor  \
{ \
    0x0000, \
    0xffff, \
};

/* 字体大小默认值 */
#define GUI_TextDefaultFont \
{ \
    {16, 24, 12, "A24"}, \
    {24, 24, 24, "H24"}, \
};

/* 扩展声明变量 */
typedef unsigned char       uint8_t;
/* 系统已经定义有 */
//typedef char int8_t;
typedef unsigned short int  uint16_t;
typedef short int           int16_t;
typedef unsigned int        uint32_t;
typedef int                 int32_t;

/* 定义像素点的结构体，用于拆解像素点 */
typedef union _GUI_RGB565Color
{
    uint16_t Color;
    struct
    {
        uint16_t  B : 5;
        uint16_t  G : 6;
        uint16_t  R : 5;
    }RGB;
}GUI_RGB565Color_t;

/* 扩展字体类型 */
#ifdef USING_CN_16_CHAR
extern const paCharInfo_t FontCn16;
#endif
#ifdef USING_CN_16_CHAR
extern const paCharInfo_t FontCn24;
#endif
#ifdef USING_CN_16_CHAR
extern const paCharInfo_t FontCn32;
#endif
#ifdef USING_CN_16_CHAR
extern const paCharInfo_t FontCn40;
#endif
#ifdef USING_CN_16_CHAR
extern const paCharInfo_t FontCn48;
#endif

/* API 接口 */
void GuiSetTextFont(const paCharInfo_t * CharInfo);
void GuiSetTextColor(uint16_t WordColor, uint16_t BackColor);
void GuiSetDeviceAPI(void (* PutPixelNoPos)(uint16_t pColor),
                     void (* PutPixel)(uint16_t xCur, uint16_t yCur, uint16_t pColor),
                     void (* SetDispWin)(uint16_t xCur, uint16_t yCur, uint16_t Width, uint16_t Height)
                     );
void GuiClrScr(uint16_t pColor);
void GuiDrawStringAt(const char * Cn, uint16_t x, uint16_t y);
void GuiDrawTranStringAt(const char * Cn, uint16_t x, uint16_t y);
uint8_t GuiDrawNumberAt(double Number, uint16_t x, uint16_t y, uint8_t PointNum);
uint8_t GuiDrawTranNumberAt(double Number, uint16_t x, uint16_t y, uint8_t PointNum);                    
void GuiDrawText(const char * Cn, uint16_t xCur, uint16_t yCur, uint16_t xEnd, uint16_t yEnd, uint16_t Align);   
void GuiDrawTranText(const char * Cn, uint16_t xCur, uint16_t yCur, uint16_t xEnd, uint16_t yEnd, uint16_t Align);                   
                     
void GuiDrawHorGradientColorBar(uint16_t xCur, uint16_t yCur,
                                uint16_t Width, uint16_t Height,
                                uint16_t FirstColor, uint16_t SecondColor);                     

void GuiDrawHorRadiationColorBar(uint16_t xCur, uint16_t yCur,
                                uint16_t Width, uint16_t Height,
                                uint16_t FirstColor, uint16_t SecondColor);

void GuiDrawHorThreeGradientColorBar(uint16_t xCur, uint16_t yCur,
                                    uint16_t Width, uint16_t Height,
                                    uint16_t FirstColor, uint16_t SecondColor,
                                    uint16_t ThreeColor);

void GuiDrawVerGradientColorBar(uint16_t xCur, uint16_t yCur,
                                uint16_t Width, uint16_t Height,
                                uint16_t FirstColor, uint16_t SecondColor);

void GuiDrawVerRadiationColorBar(uint16_t xCur, uint16_t yCur,
                                uint16_t Width, uint16_t Height,
                                uint16_t FirstColor, uint16_t MiddleColor);

void GuiDrawVerThreeGradientColorBar(uint16_t xCur, uint16_t yCur,
                                    uint16_t Width, uint16_t Height,
                                    uint16_t FirstColor, uint16_t SecondColor,
                                    uint16_t ThreeColor);
void GuiGetBarColorItems(uint16_t *Buf, uint16_t nLen);
void GuiDrawLine(uint16_t xStart , uint16_t yStart , uint16_t xEnd , uint16_t yEnd , uint16_t pColor);
void GuiDrawHorLine(uint16_t xCur, uint16_t yCur, uint16_t Lenght, uint16_t pColor);
void GuiDrawVerLine(uint16_t xCur, uint16_t yCur, uint16_t Lenght, uint16_t pColor);
void GuiDrawHorColorLine(uint16_t xCur, uint16_t yCur, uint16_t dWidth, const uint16_t *pColor);
void GuiDrawPoints(const uint16_t *x, uint16_t *y, uint16_t Size, uint16_t pColor);
void GuiDrawRect(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor);
void GuiDrawCircle(uint16_t xCur, uint16_t yCur, uint16_t rRadius, uint16_t pColor);
void GuiDrawRectRound(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor, uint16_t Round);
void GuiDrawFillRect(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor);
void GuiDrawFillCircle(uint16_t xCur, uint16_t yCur, uint16_t rRadius, uint16_t pColor);
void GuiDrawFillRectRound(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor, uint16_t Round);
void GuiDrawTopTriangle(uint16_t xCur, uint16_t yCur, uint16_t Edges, uint16_t pColor);
void GuiDrawFillTopTriangle(uint16_t xCur, uint16_t yCur, uint16_t Edges, uint16_t pColor);
void GuiDrawBMP(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, const uint16_t *Ptr);
void GuiGetWarmColdColor(uint16_t SrcColor, uint16_t * WarmColor, uint16_t * ColdColor);

#endif

