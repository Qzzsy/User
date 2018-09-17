/**
 ******************************************************************************
 * @Copyright     (C) 2017 - 2018 guet-sctc-hardwarepart Team
 * @filename      GUIDr.h
 * @author        ZSY
 * @version       V1.0.2
 * @date          2018-09-15
 * @Description   ������Һ���Ļ���������ʵ�ֺ��֡�Ӣ�ĺ���Ӣ�Ļ����ʾ��ʵ�ָ��������
 *                2Dͼ����ơ�
 * @Others
 * @History
 * Date           Author    version    		    Notes
 * 2018-06-22      ZSY      V1.0.0          first version.
 * 2018-06-22      ZSY      V1.0.1          ��Ӷ��ⲿ�ֿ��֧��.
 * 2018-09-15      ZSY      V1.0.2          ���Ĳ��ֽṹ��������.
 */
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _GUIDR_H_
#define _GUIDR_H_

#include "Font.h"

/* ��Ļ�Ŀ�Ⱥ͸߶� */
#define LCD_WIDTH           96
#define LCD_HEIGHT          64

/* �������ݻ���, ������ڵ��ڵ�����ģ��Ҫ�Ĵ洢�ռ�*/ 
#define BYTES_PER_FONT      512 

#if defined USE_CN_INT_LIB || defined USE_ASCII_INT_LIB
/* �ֿ�λ���ڲ���FLASH */
#define USE_SMALL_LIB_FONT              (1)     //1Ϊʹ���ڲ��ֿ�
#endif

#ifdef USE_CN_EXT_LIB
/* �ֿ�λ���ⲿ��FLASH */
#define USE_GBK_LIB_FONT                (0)     //1Ϊʹ��GBK�ֿ�
/* �ֿ�λ���ⲿ��FLASH */
#define USE_GB2312_LIB_FONT             (0)     //1Ϊʹ��GB2312�ֿ�
#endif

#if USE_SMALL_LIB_FONT == 1
/* �����ֿ�������� */
#define ChAR_NUM_MAX          200
#endif

/* Ӣ�ĵ��ֿ�ĵ�ַ��������ڣ��뽫��ע�͵���������ִ��� */
#ifdef USE_ASCII_EXT_LIB
/* Ӣ���ֿ����ַ */
#define FONT_ASCII16_BASE_ADDR                 0x20000000
#define FONT_ASCII24_BASE_ADDR                 0x20000000
#define FONT_ASCII32_BASE_ADDR                 0x20000000
#define FONT_ASCII40_BASE_ADDR                 0x20000000
#define FONT_ASCII48_BASE_ADDR                 0x20000000
#endif

/* ���ֵ��ֿ�ĵ�ַ��������ڣ��뽫��ע�͵���������ִ��� */
#if USE_GBK_LIB_FONT == 1

/* �����ֿ����ַ */
#define GBK_FONT_CN16_BASE_ADDR                 0x20000000
#define GBK_FONT_CN24_BASE_ADDR                 0x20000000
#define GBK_FONT_CN32_BASE_ADDR                 0x20000000
#define GBK_FONT_CN40_BASE_ADDR                 0x20000000
#define GBK_FONT_CN48_BASE_ADDR                 0x20000000
#endif

#if USE_GB2312_LIB_FONT == 1
/* �����ֿ����ַ */
#define GB2312_FONT_CN16_BASE_ADDR                 0x20000000
#define GB2312_FONT_CN24_BASE_ADDR                 0x20000000
#define GB2312_FONT_CN32_BASE_ADDR                 0x20000000
#define GB2312_FONT_CN40_BASE_ADDR                 0x20000000
#define GB2312_FONT_CN48_BASE_ADDR                 0x20000000
#endif

/* �ַ������뷽ʽ�ĺ궨�� */
#define LEFT_ALIGN          1 << 0
#define RIGHT_ALIGN         1 << 1
#define TOP_ALIGN           1 << 2
#define BOTTOM_ALIGN        1 << 3
#define CENTER_ALIGN        1 << 4

/* ������ɫĬ��ֵ */
#define GUI_TextDefaultColor  \
{ \
    0x0000, \
    0xffff, \
};

/* �����СĬ��ֵ */
#define GUI_TextDefaultFont \
{ \
    {16, 24, 12, "A24"}, \
    {24, 24, 24, "H24"}, \
};

#ifndef NULL
    #define NULL            0
#endif

/* �������ص�Ľṹ�壬���ڲ�����ص� */
typedef union _GUI_RGB565Color
{
    unsigned short int Color;
    struct
    {
        unsigned short int  B : 5;
        unsigned short int  G : 6;
        unsigned short int  R : 5;
    }RGB;
}GUI_RGB565Color_t;

/* ��չ�������� */
#ifdef USING_CN_16_CHAR
extern const paCharsInfo_t FontCn16;
#endif
#ifdef USING_CN_24_CHAR
extern const paCharsInfo_t FontCn24;
#endif
#ifdef USING_CN_32_CHAR
extern const paCharsInfo_t FontCn32;
#endif
#ifdef USING_CN_40_CHAR
extern const paCharsInfo_t FontCn40;
#endif
#ifdef USING_CN_48_CHAR
extern const paCharsInfo_t FontCn48;
#endif

/* API �ӿ� */
void GuiSetTextFont(const paCharsInfo_t * CharInfo);
void GuiSetTextColor(unsigned short int WordColor, unsigned short int BackColor);
void GuiSetDeviceAPI(void (* PutPixelNoPos)(unsigned short int pColor),
                     void (* PutPixel)(unsigned short int xCur, unsigned short int yCur, unsigned short int pColor),
                     void (* SetDispWin)(unsigned short int xCur, unsigned short int yCur, unsigned short int Width, unsigned short int Height)
                     );
#if defined USE_ASCII_EXT_LIB || defined USE_CN_EXT_LIB
void GuiSetFlashReadAPI(void (* ReadData)(uint32_t Address, uint8_t * pDataBuf, uint32_t BufSize));
#endif
void GuiClrScr(unsigned short int pColor);
void GuiDrawStringAt(const char * Cn, unsigned short int x, unsigned short int y);
void GuiDrawTranStringAt(const char * Cn, unsigned short int x, unsigned short int y);
unsigned char GuiDrawNumberAt(double Number, unsigned short int x, unsigned short int y, unsigned char PointNum);
unsigned char GuiDrawTranNumberAt(double Number, unsigned short int x, unsigned short int y, unsigned char PointNum);                    
void GuiDrawText(const char * Cn, unsigned short int xCur, unsigned short int yCur, unsigned short int xEnd, unsigned short int yEnd, unsigned short int Align);   
void GuiDrawTranText(const char * Cn, unsigned short int xCur, unsigned short int yCur, unsigned short int xEnd, unsigned short int yEnd, unsigned short int Align);                   
                     
void GuiDrawHorGradientColorBar(unsigned short int xCur, unsigned short int yCur,
                                unsigned short int Width, unsigned short int Height,
                                unsigned short int FirstColor, unsigned short int SecondColor);                     

void GuiDrawHorRadiationColorBar(unsigned short int xCur, unsigned short int yCur,
                                unsigned short int Width, unsigned short int Height,
                                unsigned short int FirstColor, unsigned short int SecondColor);

void GuiDrawHorThreeGradientColorBar(unsigned short int xCur, unsigned short int yCur,
                                    unsigned short int Width, unsigned short int Height,
                                    unsigned short int FirstColor, unsigned short int SecondColor,
                                    unsigned short int ThreeColor);

void GuiDrawVerGradientColorBar(unsigned short int xCur, unsigned short int yCur,
                                unsigned short int Width, unsigned short int Height,
                                unsigned short int FirstColor, unsigned short int SecondColor);

void GuiDrawVerRadiationColorBar(unsigned short int xCur, unsigned short int yCur,
                                unsigned short int Width, unsigned short int Height,
                                unsigned short int FirstColor, unsigned short int MiddleColor);

void GuiDrawVerThreeGradientColorBar(unsigned short int xCur, unsigned short int yCur,
                                    unsigned short int Width, unsigned short int Height,
                                    unsigned short int FirstColor, unsigned short int SecondColor,
                                    unsigned short int ThreeColor);
void GuiGetBarColorItems(unsigned short int *Buf, unsigned short int nLen);
void GuiDrawLine(unsigned short int xStart , unsigned short int yStart , unsigned short int xEnd , unsigned short int yEnd , unsigned short int pColor);
void GuiDrawHorLine(unsigned short int xCur, unsigned short int yCur, unsigned short int Lenght, unsigned short int pColor);
void GuiDrawVerLine(unsigned short int xCur, unsigned short int yCur, unsigned short int Lenght, unsigned short int pColor);
void GuiDrawHorColorLine(unsigned short int xCur, unsigned short int yCur, unsigned short int dWidth, const unsigned short int *pColor);
void GuiDrawPoints(const unsigned short int *x, unsigned short int *y, unsigned short int Size, unsigned short int pColor);
void GuiDrawRect(unsigned short int xCur, unsigned short int yCur, unsigned short int dWidth, unsigned short int dHeight, unsigned short int pColor);
void GuiDrawCircle(unsigned short int xCur, unsigned short int yCur, unsigned short int rRadius, unsigned short int pColor);
void GuiDrawRectRound(unsigned short int xCur, unsigned short int yCur, unsigned short int dWidth, unsigned short int dHeight, unsigned short int pColor, unsigned short int Round);
void GuiDrawFillRect(unsigned short int xCur, unsigned short int yCur, unsigned short int dWidth, unsigned short int dHeight, unsigned short int pColor);
void GuiDrawFillCircle(unsigned short int xCur, unsigned short int yCur, unsigned short int rRadius, unsigned short int pColor);
void GuiDrawFillRectRound(unsigned short int xCur, unsigned short int yCur, unsigned short int dWidth, unsigned short int dHeight, unsigned short int pColor, unsigned short int Round);
void GuiDrawTopTriangle(unsigned short int xCur, unsigned short int yCur, unsigned short int Edges, unsigned short int pColor);
void GuiDrawFillTopTriangle(unsigned short int xCur, unsigned short int yCur, unsigned short int Edges, unsigned short int pColor);
void GuiDrawBMP(unsigned short int xCur, unsigned short int yCur, unsigned short int dWidth, unsigned short int dHeight, const unsigned short int *Ptr);
void GuiGetWarmColdColor(unsigned short int SrcColor, unsigned short int * WarmColor, unsigned short int * ColdColor);

#endif

