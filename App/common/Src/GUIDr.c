/**
 ******************************************************************************
 * @file      GUIDr.c
 * @author    ZSY
 * @version   V1.0.2
 * @date      2018-09-15
 * @brief     本文件实现LCD的一些绘制文字，图形等操作，移植本文件不需要更改本文件的内容 
 * @History
 * Date           Author    version    		   Notes
 * 2018-06-22      ZSY      V1.0.0          first version.
 * 2018-06-22      ZSY      V1.0.1          添加对外部字库的支持.
 * 2018-09-15      ZSY      V1.0.2          更改部分结构，简化流程.
 */

/* Includes ------------------------------------------------------------------*/
#include "GUIDr.h"
#include "math.h"
#include "stdlib.h"
#include "MyString.h"

/* Private macro Definition --------------------------------------------------*/
/* 定义两个控制变量 */
#define CHAR_TRANS          0x00
#define CHAR_NO_TRANS       0x01
#define GBK_FONT            0x00
#define GB2312_FONT         0x01
#define NONE_FONT           0xff

#define GUI_ERROR           (uint32_t)(-1)
#define GUI_OK              (uint32_t)(-2)
#define GUI_NO_FIND_FONT    (uint32_t)(-3)

/* 扩展声明变量 */
typedef unsigned char       uint8_t;
/* 系统已经定义有 */
typedef char                int8_t;
typedef unsigned short int  uint16_t;
typedef short int           int16_t;
typedef unsigned int        uint32_t;
typedef int                 int32_t;

typedef struct _LCD_Info
{
    uint16_t Width;
    uint16_t Hight;
}LCD_Info_t;

/* 控制字体颜色的结构体 */
typedef struct _GUI_TextColor
{
    uint16_t WordColor;
    uint16_t BackColor;
}GUI_TextColor_t;

typedef struct _GUI_TextPos
{
    uint16_t x;
    uint16_t y;
}GUI_TextPos_t;

/* 控制显示的文字内容的信息结构体 */
typedef struct _GUI_CnInfo
{
    uint16_t        Cn;
    GUI_TextPos_t   tPos;
    uint16_t        SumBytes;
    uint8_t         TransFlag;
    GUI_TextColor_t Color;
    uint8_t        *FontDataBuf;
    paCnInfo_t      paCnInfo;
    uint32_t         ERROR_CODE;
}GUI_CnInfo_t;

/* 底层应该提供的4个API接口 */
typedef struct
{
    void (* PutPixelNoPos)(uint16_t pColor);
    void (* PutPixel)(uint16_t xCur, uint16_t yCur, uint16_t pColor);
    void (* SetDispWin)(uint16_t xCur, uint16_t yCur, uint16_t Width, uint16_t Height);
#if defined USE_ASCII_EXT_LIB || defined USE_CN_EXT_LIB
    void (* ReadData)(uint32_t Address, uint8_t * pDataBuf, uint32_t BufSize);
#endif
}GUI_DeviceAPI_t;

/* 外部声明，引用变量 */
#ifdef USING_CN_16_CHAR
extern const CnChar_t HanZi16Index[];
extern const Cn16Data_t HanZi16Data[];
extern const unsigned char ASCII08x16[];
#endif
#ifdef USING_CN_24_CHAR
extern const CnChar_t HanZi24Index[];
extern const Cn24Data_t HanZi24Data[];
extern const unsigned char ASCII12x24[];
#endif
#ifdef USING_CN_32_CHAR
extern const CnChar_t HanZi32Index[];
extern const Cn32Data_t HanZi32Data[];
extern const unsigned char ASCII16x32[];
#endif
#ifdef USING_CN_40_CHAR
extern const CnChar_t HanZi40Index[];
extern const Cn40Data_t HanZi40Data[];
extern const unsigned char ASCII20x40[];
#endif
#ifdef USING_CN_48_CHAR
extern const CnChar_t HanZi48Index[];
extern const Cn48Data_t HanZi48Data[];
extern const unsigned char ASCII24x48[];
#endif

/* 定义控制字体颜色的变量，并设置为默认值 */
static GUI_TextColor_t _GUI_TextColor = GUI_TextDefaultColor;

/* 默认字体内容缓存区，防止因找不到字体使显示错乱 */
static uint8_t _GUI_FontDefaultDataBuf[BYTES_PER_FONT] = {'\0'};

#if defined USE_CN_EXT_LIB || defined USE_ASCII_EXT_LIB
/* 默认字体内容缓存区，使用外部字库时用到 */
static uint8_t _GUI_FontDataBufFromFlash[BYTES_PER_FONT] = {'\0'};
#endif

/* 定义控制字体的一些信息的变量，并设置为默认值 */
static paCharsInfo_t _paCharInfo = GUI_TextDefaultFont;

/* 定义底层驱动的API接口变量 */
static GUI_DeviceAPI_t _GUI_DeviceAPI;

/* 定义一个缓存区，取屏幕宽或高的最大值作为集合的大小，用于临时储存色条数据 */
#if (LCD_HEIGHT > LCD_WIDTH)
static uint16_t _GUI_BarColorItems[LCD_HEIGHT] = {'\0'};
#elif (LCD_HEIGHT < LCD_WIDTH)
static uint16_t _GUI_BarColorItems[LCD_WIDTH] = {'\0'};
#endif

/**
 * @func    _GetASCII_FontData
 * @brief   从内存里获取点阵的数据
 * @param   GUI_CnInfo 显示文字的信息的结构体
 * @note
 * @retval  无
 */
static inline void _GetASCII_FontData(GUI_CnInfo_t * GUI_CnInfo)
{
    uint8_t WordNun; 

    GUI_CnInfo->ERROR_CODE = GUI_OK;

#if defined USE_ASCII_EXT_LIB || defined USE_CN_EXT_LIB
    uint32_t FlashAddr = 0;
#endif
/* 屏蔽部分没有显示的ASCII码 */
    if (GUI_CnInfo->Cn < 32)
    {
        GUI_CnInfo->ERROR_CODE = GUI_ERROR;
        return ;
    }
    
    /* 从ASCII码的32开始有显示，所以此处减掉32 */
    WordNun = GUI_CnInfo->Cn - 32;
#ifdef USE_ASCII_INT_LIB        
    /* 使用16的点阵 */
#ifdef USING_CN_16_CHAR
    if (my_strncmp("A16", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 8*16 ASCII字符 */
    {
        /* 指针直接取地址 */
        GUI_CnInfo->FontDataBuf = (uint8_t *)&ASCII08x16[(uint16_t )WordNun * GUI_CnInfo->SumBytes];
        return ;
    }
#endif /* USING_CN_16_CHAR */   
    
    /* 使用24的点阵 */
#ifdef USING_CN_24_CHAR  
    if (my_strncmp("A24", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 16*24 ASCII字符 */
    {          
        GUI_CnInfo->FontDataBuf = (uint8_t *)&ASCII12x24[(uint16_t )WordNun * GUI_CnInfo->SumBytes];
        return ;
    }
#endif /* USING_CN_24_CHAR */
    
    /* 使用32的点阵 */
#ifdef USING_CN_32_CHAR  
    if (my_strncmp("A32", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 20*32 ASCII字符 */
    {  
        GUI_CnInfo->FontDataBuf = (uint8_t *)&ASCII16x32[(uint16_t )WordNun * GUI_CnInfo->SumBytes];
        return ;
    }
#endif /* USING_CN_32_CHAR */
    
    /* 使用40的点阵 */
#ifdef USING_CN_40_CHAR  
    if (my_strncmp("A40", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 24*40 ASCII字符 */
    {  
        GUI_CnInfo->FontDataBuf = (uint8_t *)&ASCII20x40[(uint16_t )WordNun * GUI_CnInfo->SumBytes];
        return ;
    }
#endif /* USING_CN_40_CHAR */
    
    /* 使用48的点阵 */
#ifdef USING_CN_48_CHAR   
    if (my_strncmp("A48", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 28*48 ASCII字符 */
    { 
        GUI_CnInfo->FontDataBuf = (uint8_t *)&ASCII24x48[(uint16_t )WordNun * GUI_CnInfo->SumBytes];
        return ;
    }
#endif /* USING_CN_48_CHAR */
#elif defined USE_ASCII_EXT_LIB
    /* 使用16的点阵 */
#ifdef USING_CN_16_CHAR
    if (my_strncmp("A16", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 8*16 ASCII字符 */
    {
#ifdef FONT_ASCII16_BASE_ADDR
        /* 计算地址 */
        FlashAddr = FONT_ASCII16_BASE_ADDR + (uint16_t)WordNun * GUI_CnInfo->SumBytes;
        goto _ReadASCII_Data; 
#else
        GUI_CnInfo->ERROR_CODE = GUI_NO_FIND_FONT;
        return ;
#endif
    }
#endif /* USING_CN_16_CHAR */   
    
    /* 使用24的点阵 */
#ifdef USING_CN_24_CHAR  
    if (my_strncmp("A24", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 16*24 ASCII字符 */
    {          
#ifdef FONT_ASCII24_BASE_ADDR
        FlashAddr = FONT_ASCII24_BASE_ADDR + (uint16_t)WordNun * GUI_CnInfo->SumBytes;
        goto _ReadASCII_Data;
#else
        GUI_CnInfo->ERROR_CODE = GUI_NO_FIND_FONT;
        return ;
#endif
    }
#endif /* USING_CN_24_CHAR */
    
    /* 使用32的点阵 */
#ifdef USING_CN_32_CHAR  
    if (my_strncmp("A32", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 20*32 ASCII字符 */
    {  
#ifdef FONT_ASCII32_BASE_ADDR
        FlashAddr = FONT_ASCII32_BASE_ADDR + (uint16_t)WordNun * GUI_CnInfo->SumBytes;
        goto _ReadASCII_Data;
#else
        GUI_CnInfo->ERROR_CODE = GUI_NO_FIND_FONT;
        return ;
#endif
    }
#endif /* USING_CN_32_CHAR */
    
    /* 使用40的点阵 */
#ifdef USING_CN_40_CHAR  
    if (my_strncmp("A40", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 24*40 ASCII字符 */
    {  
#ifdef FONT_ASCII40_BASE_ADDR
        FlashAddr = FONT_ASCII40_BASE_ADDR + (uint16_t)WordNun * GUI_CnInfo->SumBytes;
        goto _ReadASCII_Data;
#else
        GUI_CnInfo->ERROR_CODE = GUI_NO_FIND_FONT;
        return ;
#endif
    }
#endif /* USING_CN_40_CHAR */
    
    /* 使用48的点阵 */
#ifdef USING_CN_48_CHAR   
    if (my_strncmp("A48", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 28*48 ASCII字符 */
    { 
#ifdef FONT_ASCII48_BASE_ADDR
        FlashAddr = FONT_ASCII48_BASE_ADDR + (uint16_t)WordNun * GUI_CnInfo->SumBytes;
        goto _ReadASCII_Data;
#else
        GUI_CnInfo->ERROR_CODE = GUI_NO_FIND_FONT;
        return ;
#endif
    }
#endif /* USING_CN_48_CHAR */
_ReadASCII_Data:
    _GUI_DeviceAPI.ReadData(FlashAddr, _GUI_FontDataBufFromFlash, GUI_CnInfo->SumBytes);
    GUI_CnInfo->FontDataBuf = _GUI_FontDataBufFromFlash;
    return ;
#endif
    GUI_CnInfo->ERROR_CODE = GUI_ERROR;
    return ;
}

/**
 * @func    _GetCN_FontData
 * @brief   从内存里获取点阵的数据
 * @param   GUI_CnInfo 显示文字的信息的结构体
 * @note
 * @retval  无
 */
static inline void _GetCN_FontData(GUI_CnInfo_t * GUI_CnInfo)
{
    uint16_t i = 0;
#if defined USE_ASCII_EXT_LIB || defined USE_CN_EXT_LIB
    uint32_t FlashAddr = 0;
    uint8_t FontType = NONE_FONT;
#endif
#ifdef USE_CN_INT_LIB
#ifdef USING_CN_16_CHAR  
    if(my_strncmp("H16", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 16*16 中文字符 */
    { 
        for (i = 0; i < ChAR_NUM_MAX; i++)        //循环查询内码，查找汉字的数据
        {
            if((HanZi16Index[i].Index[1] == ((GUI_CnInfo->Cn >> 8) & 0xff))		
                & (HanZi16Index[i].Index[0] == (GUI_CnInfo->Cn & 0xff)))
            {
                GUI_CnInfo->FontDataBuf = (uint8_t *)HanZi16Data[i].Msk;
                return ;
            }
        }
        
        goto _ERROR;
    }
#endif /* USING_CN_16_CHAR */
#ifdef USING_CN_24_CHAR          
    if (my_strncmp("H24", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 24*24 中文字符 */
    {
        for (i = 0; i < ChAR_NUM_MAX; i++)
        {
            if((HanZi24Index[i].Index[1] == ((GUI_CnInfo->Cn >> 8) & 0xff))		
                & (HanZi24Index[i].Index[0] == (GUI_CnInfo->Cn & 0xff)))
            {
                GUI_CnInfo->FontDataBuf = (uint8_t *)HanZi24Data[i].Msk;
                return ;
            }
        }
        
        goto _ERROR;
    }
#endif /* USING_CN_24_CHAR */
#ifdef USING_CN_32_CHAR  
    if (my_strncmp("H32", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 32*32 中文字符 */
    {
        for (i = 0; i < ChAR_NUM_MAX; i++)
        {
            if((HanZi32Index[i].Index[1] == ((GUI_CnInfo->Cn >> 8) & 0xff))		
                & (HanZi32Index[i].Index[0] == (GUI_CnInfo->Cn & 0xff)))
            {
                GUI_CnInfo->FontDataBuf = (uint8_t *)HanZi32Data[i].Msk;
                return ;
            }
        }
        
        goto _ERROR;
    }
#endif /* USING_CN_32_CHAR */
#ifdef USING_CN_40_CHAR  
    if (my_strncmp("H40", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 40*40 中文字符 */
    {
        for (i = 0; i < ChAR_NUM_MAX; i++)
        {
            if((HanZi40Index[i].Index[1] == ((GUI_CnInfo->Cn >> 8) & 0xff))		
                & (HanZi40Index[i].Index[0] == (GUI_CnInfo->Cn & 0xff)))
            {
                GUI_CnInfo->FontDataBuf = (uint8_t *)HanZi40Data[i].Msk;
                return ;
            }
        }
        
        goto _ERROR;
    }
#endif /* USING_CN_40_CHAR */
#ifdef USING_CN_48_CHAR  
    if (my_strncmp("H48", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 48*48 中文字符 */
    {
        for (i = 0; i < ChAR_NUM_MAX; i++)
        {
            if((HanZi48Index[i].Index[1] == ((GUI_CnInfo->Cn >> 8) & 0xff))		
                & (HanZi48Index[i].Index[0] == (GUI_CnInfo->Cn & 0xff)))
            {
                GUI_CnInfo->FontDataBuf = (uint8_t *)HanZi48Data[i].Msk;
                return ;
            }
        }
        
        goto _ERROR;
    }
#endif /* USING_CN_48_CHAR */
#elif defined USE_CN_EXT_LIB
#ifdef USING_CN_16_CHAR  
    if(my_strncmp("H16", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 16*16 中文字符 */
    { 
#ifdef GBK_FONT_CN16_BASE_ADDR
        FlashAddr = GBK_FONT_CN16_BASE_ADDR;
        FontType = GBK_FONT;
        goto _ReadCN_Data;
#elif define GB2312_FONT_CN16_BASE_ADDR
        FlashAddr = GB2312_FONT_CN16_BASE_ADDR;
        FontType = GB2312_FONT;
        goto _ReadCN_Data;
#else
        goto _ERROR;
#endif
    }
#endif /* USING_CN_16_CHAR */
#ifdef USING_CN_24_CHAR          
    if (my_strncmp("H24", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 24*24 中文字符 */
    {
#ifdef GBK_FONT_CN24_BASE_ADDR
        FlashAddr = GBK_FONT_CN24_BASE_ADDR;
        FontType = GBK_FONT;
        goto _ReadCN_Data;
#elif define GB2312_FONT_CN24_BASE_ADDR
        FlashAddr = GB2312_FONT_CN24_BASE_ADDR;
        FontType = GB2312_FONT;
        goto _ReadCN_Data;
#else
        goto _ERROR;
#endif
    }
#endif /* USING_CN_24_CHAR */
#ifdef USING_CN_32_CHAR  
    if (my_strncmp("H32", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 32*32 中文字符 */
    {
#ifdef GBK_FONT_CN32_BASE_ADDR
        FlashAddr = GBK_FONT_CN32_BASE_ADDR;
        FontType = GBK_FONT;
        goto _ReadCN_Data;
#elif define GB2312_FONT_CN32_BASE_ADDR
        FlashAddr = GB2312_FONT_CN32_BASE_ADDR;
        FontType = GB2312_FONT;
        goto _ReadCN_Data;
#else
        goto _ERROR;
#endif
    }
#endif /* USING_CN_32_CHAR */
#ifdef USING_CN_40_CHAR  
    if (my_strncmp("H40", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 40*40 中文字符 */
    {
#ifdef GBK_FONT_CN40_BASE_ADDR
        FlashAddr = GBK_FONT_CN40_BASE_ADDR;
        FontType = GBK_FONT;
        goto _ReadCN_Data;
#elif define GB2312_FONT_CN40_BASE_ADDR
        FlashAddr = GB2312_FONT_CN40_BASE_ADDR;
        FontType = GB2312_FONT;
        goto _ReadCN_Data;
#else
        goto _ERROR;
#endif
    }
#endif /* USING_CN_40_CHAR */
#ifdef USING_CN_48_CHAR  
    if (my_strncmp("H48", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 48*48 中文字符 */
    {
#ifdef GBK_FONT_CN48_BASE_ADDR
        FlashAddr = GBK_FONT_CN48_BASE_ADDR;
        FontType = GBK_FONT;
        goto _ReadCN_Data;
#elif define GB2312_FONT_CN48_BASE_ADDR
        FlashAddr = GB2312_FONT_CN48_BASE_ADDR;
        FontType = GB2312_FONT;
        goto _ReadCN_Data;
#else
        goto _ERROR;
#endif
    }
#endif /* USING_CN_48_CHAR */
_ReadCN_Data:
    if (FontType == GBK_FONT)
    {
        /* 根据汉字内码的计算公式锁定起始地址 */
        code2 = c >> 8;
        code1 = c & 0xFF;
        
        /* 由于字符编码是安顺序存储的，先存储到高位（区号），然后是低位（位号）。而我们用的是小端格式，
            一个汉字两个字节，获取的16位变量，正好相反，16位变量的高位是位号，低位是区号。
        */
        FlashAddr = ((code1 - 0xA1) * 94 + (code2 - 0xa1)) * GUI_CnInfo->SumBytes + FlashAddr;
    }
    else if (FontType == GB2312_FONT)
    {
        /* 根据汉字内码的计算公式锁定起始地址 */
        code2 = c >> 8;
        code1 = c & 0xFF;
        
        /* 由于字符编码是安顺序存储的，先存储到高位（区号），然后是低位（位号）。而我们用的是小端格式，
            一个汉字两个字节，获取的16位变量，正好相反，16位变量的高位是位号，低位是区号。
        */
        FlashAddr = ((code1 - 0x81) * 190 + (code2 - 0x40) - (code2 / 128)) * GUI_CnInfo->SumBytes + FlashAddr;
    }

    _GUI_DeviceAPI.ReadData(FlashAddr, _GUI_FontDataBufFromFlash, GUI_CnInfo->SumBytes);
    GUI_CnInfo->FontDataBuf = _GUI_FontDataBufFromFlash;
#endif
_ERROR:
    GUI_CnInfo->ERROR_CODE = GUI_ERROR;
    return ;
}

/**
 * @func    _GetDataFromMemory
 * @brief   从内存里获取点阵的信息
 * @param   GUI_CnInfo 显示文字的信息的结构体
 * @note
 * @retval  无
 */
static inline void _GetDataFromMemory(GUI_CnInfo_t * GUI_CnInfo)
{
    /* 判断是否超出缓存大小，超出大小限制大小，防止数组越界 */
    if ( GUI_CnInfo->SumBytes > BYTES_PER_FONT)
    {
         GUI_CnInfo->SumBytes = BYTES_PER_FONT;
    }
    
    /* 判断是中文的还是英文的 */    
    if (GUI_CnInfo->Cn < 0x80)                                                                
    {
        _GetASCII_FontData(GUI_CnInfo);
    }
    /* 中文显示 */
    else
    {
        _GetCN_FontData(GUI_CnInfo);
    }
}

/**
 * @func    _DispChar
 * @brief   显示内容
 * @param   GUI_CnInfo 显示文字的信息的结构体
 * @note
 * @retval  无
 */
static inline void _DispChar(GUI_CnInfo_t * GUI_CnInfo)
{
    uint16_t i, Cnt, Color;
    uint8_t * _FontDataBuf = NULL;
    /* 获取字体的点阵 */
    _GetDataFromMemory(GUI_CnInfo);

    _FontDataBuf = GUI_CnInfo->FontDataBuf;

    if (GUI_CnInfo->ERROR_CODE == GUI_ERROR)
    {
        _FontDataBuf = _GUI_FontDefaultDataBuf;
        return ;
    }
    /* 判断是否需要背景透明显示 */
    if (GUI_CnInfo->TransFlag == CHAR_NO_TRANS)
    {
        /* 设置窗口 */
        _GUI_DeviceAPI.SetDispWin(GUI_CnInfo->tPos.x, GUI_CnInfo->tPos.y, GUI_CnInfo->paCnInfo.Width, GUI_CnInfo->paCnInfo.Hight);
       
        /* 循环取数据进行显示 */
        for (Cnt = 0; Cnt < GUI_CnInfo->SumBytes; Cnt++)
        {
            /* 获取点阵数据，一个字节 */
            Color = _FontDataBuf[Cnt];
            for (i = 0; i < 8; i++)
            {
                /* 判断画点 */
                if((Color << i) & 0x80)
                {
                    _GUI_DeviceAPI.PutPixelNoPos(GUI_CnInfo->Color.WordColor);
                }
                else
                {
                    _GUI_DeviceAPI.PutPixelNoPos(GUI_CnInfo->Color.BackColor);
                }
            }
        }
    }
    
    /* 需要背景透明显示 */
    else if (GUI_CnInfo->TransFlag == CHAR_TRANS)
    {
        uint16_t x, y;
        
        /* 记录起始坐标 */
        x = GUI_CnInfo->tPos.x;
        y = GUI_CnInfo->tPos.y;
        
        for (Cnt = 0; Cnt < GUI_CnInfo->SumBytes; Cnt++)
        {
            /* 获取点阵数据，一个字节 */
            Color = _FontDataBuf[Cnt];
            for (i = 0; i < 8; i++)
            {
                if((Color << i) & 0x80)
                {
                    /* 调用画点函数进行绘制 */
                    _GUI_DeviceAPI.PutPixel(x, y, GUI_CnInfo->Color.WordColor);
                }
                x++;
                /* 判断是否需要换行 */
                if(x == (GUI_CnInfo->tPos.x + GUI_CnInfo->paCnInfo.Width))
                {
                    x = GUI_CnInfo->tPos.x;
                    y++;
                }
                
            }
        }
    }
}

/**
 * @func    _GuiDrawString
 * @brief   显示一个字符串内容
 * @param   Cn 字符串数据
 * @param   x 起始的x坐标
 * @param   y 起始的y坐标
 * @param   GUI_CnInfoIn 显示文字的信息的结构体
 * @note
 * @retval  无
 */
static inline void _GuiDrawString(const char * Cn, uint16_t xCur, uint16_t yCur, GUI_CnInfo_t * GUI_CnInfoIn)
{
    uint16_t Ch;
    GUI_CnInfo_t * GUI_CnInfo = GUI_CnInfoIn;
    
    /* 循环取出字符串中的字符进行显示 */
    while (*Cn != '\0')
    {
        /* 先取一个字节 */
        Ch = *Cn;
        
        /* 判断是否为英文字符，中文字符的编码从0x80开始 */
        if (Ch < 0x80)
        {
            /* 载入一些信息，显示需要 */
            GUI_CnInfo->Cn = Ch;
            GUI_CnInfo->SumBytes = _paCharInfo.paAsciiInfo.Hight * _paCharInfo.paAsciiInfo.PerLineBytes;
                       
            /* 载入显示字体的信息 */
            GUI_CnInfo->paCnInfo = _paCharInfo.paAsciiInfo;
            
            /* 载入显示的坐标 */
            GUI_CnInfo->tPos.x = xCur;
            GUI_CnInfo->tPos.y = yCur;
            
            /* 一个英文字符由一个字节的编码构成 */
            Cn++;
            
            /* x坐标跳转一个字符的宽度 */
            xCur += GUI_CnInfo->paCnInfo.Width;
        }
        else
        {
            GUI_CnInfo->Cn = *(uint16_t *)Cn;
            GUI_CnInfo->SumBytes = _paCharInfo.paHanziInfo.Hight * _paCharInfo.paHanziInfo.PerLineBytes;
            GUI_CnInfo->paCnInfo = _paCharInfo.paHanziInfo;
            
            /* 载入显示的坐标 */
            GUI_CnInfo->tPos.x = xCur;
            GUI_CnInfo->tPos.y = yCur;
            
            /* 一个汉字由两个字节的编码构成 */
            Cn += 2;
            
            /* x坐标跳转一个字符的宽度 */
            xCur += GUI_CnInfo->paCnInfo.Width;
        }

        GUI_CnInfo->Color = _GUI_TextColor;

        /* 显示内容 */
        _DispChar(GUI_CnInfo);
    }
}

/**
 * @func    _GuiGetColorBarData
 * @brief   获取渐变颜色值
 * @param   ColorNum 像素点的数量
 * @param   FirstColor 第一种颜色
 * @param   SecondColor 第二种颜色
 * @param   ColorData 像素点缓存区
 * @note
 * @retval  无
 */
static inline void _GuiGetColorBarData(uint16_t ColorNum,
                                        uint16_t FirstColor, uint16_t SecondColor,
                                        uint16_t * ColorData)
{
    double R_Coefficient, G_Coefficient, B_Coefficient;
    uint16_t i;
    uint8_t R, G, B;
    GUI_RGB565Color_t RGB_FirstColor;
    GUI_RGB565Color_t RGB_SecondColor;
    
    /* 拆解像素 */
    RGB_FirstColor.Color = FirstColor;
    RGB_SecondColor.Color = SecondColor;
    
    /* 计算系数 */
    R_Coefficient = abs(RGB_SecondColor.RGB.R - RGB_FirstColor.RGB.R) / (double)(ColorNum - 1);
    G_Coefficient = abs(RGB_SecondColor.RGB.G - RGB_FirstColor.RGB.G) / (double)(ColorNum - 1);
    B_Coefficient = abs(RGB_SecondColor.RGB.B - RGB_FirstColor.RGB.B) / (double)(ColorNum - 1);
    
    /* 循环计算像素点 */
    for (i = 0; i < ColorNum; i++)
    {
        /* 计算红色 */
        if(RGB_SecondColor.RGB.R > RGB_FirstColor.RGB.R)
        {
            R = RGB_FirstColor.RGB.R + i * R_Coefficient;
        }
        else
        {
            R = RGB_FirstColor.RGB.R - i * R_Coefficient;
        }
        
        /* 计算绿色 */
        if (RGB_SecondColor.RGB.G > RGB_FirstColor.RGB.G)
        {
            G = RGB_FirstColor.RGB.G + i * G_Coefficient;
        }
        else
        {
            G = RGB_FirstColor.RGB.G - i * G_Coefficient;
        }
        
        /* 计算蓝色 */
        if (RGB_SecondColor.RGB.B > RGB_FirstColor.RGB.B)
        {
            B = RGB_FirstColor.RGB.B + i * B_Coefficient;
        }
        else
        {
            B = RGB_FirstColor.RGB.B - i * B_Coefficient;
        }
        
        /* 合成像素点 */
        ColorData[i] = (uint16_t)(R << 11) | (uint16_t)(G << 5) | B;
    } 
}

/**
 * @func    ColorInverted
 * @brief   将像素点数组进行倒叙放置
 * @param   *pbDest 目标地址
 * @param   *pbSrc 源地址
 * @param   nLen 个数
 * @note
 * @retval  无
 */
static inline void _ColorInverted(uint16_t *pbDest, const uint16_t *pbSrc, uint32_t nLen)
{
    uint32_t i;
    for (i = 0; i < nLen; i++)
    {
        pbDest[i] = pbSrc[nLen - i - 1];
    }
}

/**
 * @func    _GuiDrawLine
 * @brief   采用 Bresenham 算法，在2点间画一条直线。
 * @param   xStart 起始点x坐标
 * @param   yStart 起始点y坐标
 * @param   xEnd 终止点x坐标
 * @param   yEnd 终止点y坐标
 * @param   pColor 画笔颜色
 * @note
 * @retval  无
 */
static inline void _GuiDrawLine(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t pColor)
{
    int32_t dx, dy;
    int32_t tx, ty;
    int32_t inc1, inc2;
    int32_t d, iTag;
    int32_t x, y;
    
    /* 采用 Bresenham 算法，在2点间画一条直线 */
    _GUI_DeviceAPI.PutPixel(xStart, yStart, pColor);
    
    /* 如果两点重合，结束后面的动作。*/
    if (xStart == xEnd && yStart == yEnd)
    {
        return;
    }
    
    iTag = 0 ;
    
    /* dx = abs ( xEnd - xStart ); */
    if (xEnd >= xStart)
    {
        dx = xEnd - xStart;
    }
    else
    {
        dx = xStart - xEnd;
    }
    
    /* dy = abs ( yEnd - yStart ); */
    if (yEnd >= yStart)
    {
        dy = yEnd - yStart;
    }
    else
    {
        dy = yStart - yEnd;
    }
    
    /*如果dy为计长方向，则交换纵横坐标。*/
    if (dx < dy)   
    {
        uint16_t temp;
      
        iTag = 1;
        temp = xStart; 
        xStart = yStart; 
        yStart = temp;
        temp = xEnd; 
        xEnd = yEnd; 
        yEnd = temp;
        temp = dx; 
        dx = dy; 
        dy = temp;
    } 
    
    /* 确定是增1还是减1 */
    tx = xEnd > xStart ? 1 : -1;   
    ty = yEnd > yStart ? 1 : -1;
    x = xStart;
    y = yStart;
    inc1 = 2 * dy;
    inc2 = 2 * (dy - dx);
    d = inc1 - dx;
    
    /* 循环画点 */
    while (x != xEnd)     
    {
        if (d < 0)
        {
            d += inc1;
        }
        else
        {
            y += ty;
            d += inc2;
        }
        if (iTag)
        {
            _GUI_DeviceAPI.PutPixel(y, x, pColor);
        }
        else
        {
            _GUI_DeviceAPI.PutPixel(x, y, pColor);
        }
        x += tx ;
    }
}

/**
 * @func    _GuiDrawHorLine
 * @brief   绘制一条高度为1的水平直线
 * @param   xCur 起始点x坐标
 * @param   yCur 起始点y坐标
 * @param   Lenght 直线的长度
 * @param   pColor 画笔颜色
 * @note
 * @retval  无
 */
static inline void _GuiDrawHorLine(uint16_t xCur, uint16_t yCur, uint16_t Lenght, uint16_t pColor)
{
    _GuiDrawLine(xCur, yCur, xCur + Lenght, yCur, pColor);
}

/**
 * @func    _GuiDrawHorColorLine
 * @brief   绘制一条彩色水平线
 * @param   xCur 起始点x坐标
 * @param   yCur 起始点y坐标
 * @param   dWidth 直线的宽度
 * @param   *pColor 颜色缓冲区
 * @note
 * @retval  无
 */
static inline void _GuiDrawHorColorLine(uint16_t xCur, uint16_t yCur, uint16_t dWidth, const uint16_t *pColor)
{
    uint16_t i;
	
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, dWidth, 1);
    
    /* 写显存 */
    for (i = 0; i < dWidth; i++)
    {
        _GUI_DeviceAPI.PutPixelNoPos(*(pColor++));
    }
}

/**
 * @func    _GuiDrawVerLine
 * @brief   绘制一条宽度为1的垂直直线
 * @param   xCur 起始点x坐标
 * @param   yCur 起始点y坐标
 * @param   Lenght 直线的长度
 * @param   pColor 画笔颜色
 * @note
 * @retval  无
 */
static inline void _GuiDrawVerLine(uint16_t xCur, uint16_t yCur, uint16_t Lenght, uint16_t pColor)
{
    _GuiDrawLine(xCur, yCur, xCur, yCur + Lenght, pColor);
}

/**
 * @func    _GuiDrawRect
 * @brief   绘制水平放置的矩形。
 * @param   xCur 起始点x坐标
 * @param   yCur 起始点y坐标
 * @param   dWidth 矩形的宽度
 * @param   dHeight 矩形的高度
 * @param   pColor 画笔颜色
 * @note    ---------------->---
           |(xCur，yCur)        |
           V                    V  dHeight
           |                    |
           ---------------->---
                dWidth
 * @retval  无
 */
static inline void _GuiDrawRect(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor)
{
    /* 顶 */
    _GuiDrawLine(xCur, yCur, xCur + dWidth - 1, yCur, pColor);	
    
    /* 底 */
    _GuiDrawLine(xCur, yCur + dHeight - 1, xCur + dWidth - 1, yCur + dHeight - 1, pColor);	
    
    /* 左 */
    _GuiDrawLine(xCur, yCur, xCur, yCur + dHeight - 1, pColor);	
    
    /* 右 */
    _GuiDrawLine(xCur + dWidth - 1, yCur, xCur + dWidth - 1, yCur + dHeight, pColor);	
}

/**
 * @func    _GuiDrawCircle
 * @brief   绘制一个圆，笔宽为1个像素
 * @param   xCur 圆心x坐标
 * @param   yCur 圆心y坐标
 * @param   rRadius 圆的半径
 * @param   pColor 画笔颜色
 * @note
 * @retval  无
 */
static inline void _GuiDrawCircle(uint16_t xCur, uint16_t yCur, uint16_t rRadius, uint16_t pColor)
{
    int32_t  D;			/* Decision Variable */
    uint32_t  nxCur;		/* 当前 X 值 */
    uint32_t  nyCur;		/* 当前 Y 值 */
    
    D = 3 - (rRadius << 1);
    nxCur = 0;
    nyCur = rRadius;
    
    /* 采用圆弧插补方式画圆 */
    while (nxCur <= nyCur)
    {
        _GUI_DeviceAPI.PutPixel(xCur + nxCur, yCur + nyCur, pColor);
        _GUI_DeviceAPI.PutPixel(xCur + nxCur, yCur - nyCur, pColor);
        _GUI_DeviceAPI.PutPixel(xCur - nxCur, yCur + nyCur, pColor);
        _GUI_DeviceAPI.PutPixel(xCur - nxCur, yCur - nyCur, pColor);
        _GUI_DeviceAPI.PutPixel(xCur + nyCur, yCur + nxCur, pColor);
        _GUI_DeviceAPI.PutPixel(xCur + nyCur, yCur - nxCur, pColor);
        _GUI_DeviceAPI.PutPixel(xCur - nyCur, yCur + nxCur, pColor);
        _GUI_DeviceAPI.PutPixel(xCur - nyCur, yCur - nxCur, pColor);
        
        if (D < 0)
        {
            D += (nxCur << 2) + 6;
        }
        else
        {
            D += ((nxCur - nyCur) << 2) + 10;
            nyCur--;
        }
        nxCur++;
    }
}

/**
 * @func    _GuiDrawRectRound
 * @brief   绘制水平放置的圆弧矩形。
 * @param   xCur 起始点x坐标
 * @param   yCur 起始点y坐标
 * @param   dWidth 矩形的宽度
 * @param   dHeight 矩形的高度
 * @param   pColor 画笔颜色
 * @param   Round 圆弧大小
 * @note    
 * @retval  无
 */
static inline void _GuiDrawRectRound(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor, uint16_t Round)
{
    
    int32_t  D;			/* Decision Variable */
    uint32_t  nxCur;		/* 当前 X 值 */
    uint32_t  nyCur;		/* 当前 Y 值 */
    
    D = 3 - (Round << 1);
    nxCur = 0;
    nyCur = Round;
    
    /* 先画出矩形，四个角除外 */
    /* 顶 */
    _GuiDrawLine(xCur + Round, yCur, xCur + dWidth - Round, yCur, pColor);	
    
    /* 底 */
    _GuiDrawLine(xCur + Round, yCur + dHeight - 1, xCur + dWidth - Round, yCur + dHeight - 1, pColor);	
    
    /* 左 */
    _GuiDrawLine(xCur, yCur + Round, xCur, yCur + dHeight - Round, pColor);	
    
    /* 右 */
    _GuiDrawLine(xCur + dWidth - 1, yCur + Round, xCur + dWidth - 1, yCur + dHeight - Round, pColor);    
        
    /* 补充四个圆弧 */
    while (nxCur <= nyCur)
    {
        /* 左上角 */
        _GUI_DeviceAPI.PutPixel(xCur + Round - nxCur, yCur + Round - nyCur, pColor);        
        _GUI_DeviceAPI.PutPixel(xCur + Round - nyCur, yCur + Round - nxCur, pColor);
        /* 右上角 */
        _GUI_DeviceAPI.PutPixel(xCur - Round + dWidth - 1 + nxCur, yCur + Round - nyCur, pColor);
        _GUI_DeviceAPI.PutPixel(xCur - Round + dWidth - 1 + nyCur, yCur + Round - nxCur, pColor);
        
        /* 左下角 */
        _GUI_DeviceAPI.PutPixel(xCur + Round - nxCur, yCur - Round + dHeight - 1 + nyCur, pColor);
        _GUI_DeviceAPI.PutPixel(xCur + Round - nyCur, yCur - Round + dHeight - 1 + nxCur, pColor);
        /* 右下角 */
        _GUI_DeviceAPI.PutPixel(xCur - Round + dWidth - 1 + nxCur, yCur - Round + dHeight - 1 + nyCur, pColor);
        _GUI_DeviceAPI.PutPixel(xCur - Round + dWidth - 1 + nyCur, yCur - Round + dHeight - 1 + nxCur, pColor);        
        
        /* 圆弧插补 */
        if (D < 0)
        {
            D += (nxCur << 2) + 6;
        }
        else
        {
            D += ((nxCur - nyCur) << 2) + 10;
            nyCur--;
        }
        nxCur++;
    }
}

/**
 * @func    _GuiDrawFillRect
 * @brief   填充矩形。
 * @param   xCur 起始点x坐标
 * @param   yCur 起始点y坐标
 * @param   dWidth 矩形的宽度
 * @param   dHeight 矩形的高度
 * @param   pColor 画笔颜色
 * @note     ---------------->---
            |(xCur，yCur)        |
            V                    V  dHeight
            |                    |
            ---------------->---
                dWidth
 * @retval  无
 */
static inline void _GuiDrawFillRect(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor)
{
    uint32_t i;
    
    /* 设置窗口 */
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, dWidth, dHeight);
    
    /* 循环填充 */
    for (i = 0; i < dWidth * dHeight; i++)
    {
        _GUI_DeviceAPI.PutPixelNoPos(pColor);
    }
}

/**
 * @func    _GuiDrawFillCircle
 * @brief   填充圆形。
 * @param   xCur 原点x坐标
 * @param   yCur 原点y坐标
 * @param   rRadius 圆形的半径
 * @param   pColor 画笔颜色
 * @note    
 * @retval  无
 */
static inline void _GuiDrawFillCircle(uint16_t xCur, uint16_t yCur, uint16_t rRadius, uint16_t pColor)
{    
    int32_t  D;			/* Decision Variable */
    uint32_t  nxCur;		/* 当前 X 值 */
    uint32_t  nyCur;		/* 当前 Y 值 */
    
    D = 3 - (rRadius << 1);
    nxCur = 0;
    nyCur = rRadius;
    
    /* 圆弧插补计算轮廓 */
    while (nxCur <= nyCur)
    {
        /* 采用直线逐行填充 */
        _GuiDrawHorLine(xCur - nyCur, yCur + nxCur, 2 * nyCur, pColor);
        _GuiDrawHorLine(xCur - nyCur, yCur - nxCur, 2 * nyCur, pColor);
        
        _GuiDrawHorLine(xCur - nxCur, yCur - nyCur, 2 * nxCur, pColor);
        _GuiDrawHorLine(xCur - nxCur, yCur + nyCur, 2 * nxCur, pColor);
        
        if (D < 0)
        {
            D += (nxCur << 2) + 6;
        }
        else
        {
            D += ((nxCur - nyCur) << 2) + 10;
            nyCur--;
        }
        nxCur++;
    }
}

/**
 * @func    _GuiDrawFillRectRound
 * @brief   在LCD上填充一个圆弧的矩形
 * @param   xCur 起始点x坐标
 * @param   yCur 起始点y坐标
 * @param   dWidth 矩形的宽度
 * @param   dHeight 矩形的高度
 * @param   pColor 填充的颜色
 * @param   Round 圆弧的大小，半径
 * @note
 * @retval  无
 */
void _GuiDrawFillRectRound(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor, uint16_t Round)
{    
    int32_t  D;			/* Decision Variable */
    uint32_t  nxCur;		/* 当前 X 值 */
    uint32_t  nyCur;		/* 当前 Y 值 */
    
    D = 3 - (Round << 1);
    nxCur = 0;
    nyCur = Round;
    
    if (dHeight > 2 * Round)
    {
        /* 首先填充一个矩形 */
        _GuiDrawFillRect(xCur, yCur + Round, dWidth, dHeight - 2 * Round, pColor);
    }
    
    /* 补充其余部分 */
    while (nxCur <= nyCur)
    {
        /* 用画线条的方式填充上半部分 */
        _GuiDrawHorLine(xCur + Round - nxCur, yCur + Round - nyCur, dWidth - 2 * Round + 2 * nxCur, pColor);
        _GuiDrawHorLine(xCur + Round - nyCur, yCur + Round - nxCur, dWidth - 2 * Round + 2 * nyCur, pColor);
        
        /* 用画线条的方式填充下半部分 */
        _GuiDrawHorLine(xCur + Round - nxCur, yCur - Round + dHeight - 1 + nyCur, dWidth - 2 * Round + 2 * nxCur, pColor);
        _GuiDrawHorLine(xCur + Round - nyCur, yCur - Round + dHeight - 1 + nxCur, dWidth - 2 * Round + 2 * nyCur, pColor);

        /* 圆弧插补法计算圆弧的x，y */
        if (D < 0)
        {
            D += (nxCur << 2) + 6;
        }
        else
        {
            D += ((nxCur - nyCur) << 2) + 10;
            nyCur--;
        }
        nxCur++;
    }
}

/**
 * @func    _GuiDrawTriangle
 * @brief   在LCD上显示一个三角形
 * @param   xCur 三角形的中心x坐标
 * @param   yCur 三角形的中心y坐标
 * @param   Edges 三角形边长
 * @param   pColor 画笔颜色
 * @note
 * @retval  无
 */
static inline void _GuiDrawTopTriangle(uint16_t xCur, uint16_t yCur, uint16_t Edges, uint16_t pColor)
{
    uint16_t x1, x2, x3, y1, y2, y3, Height;
    
    x1 = xCur - (Edges >> 1);
    x2 = xCur + (Edges >> 1);
    x3 = xCur;
    
    Height = (Edges >> 1) * 1.732;
    
    y1 = y2 = yCur - Height / 3;
    
    y3 = yCur + ((Height / 3) << 1);
    
    _GuiDrawLine(x1, y1, x2, y2, pColor);
    _GuiDrawLine(x2, y2, x3, y3, pColor);
    _GuiDrawLine(x1, y1, x3, y3, pColor);
}

/**
 * @func    _GuiDrawTriangle
 * @brief   在LCD上填充显示一个三角形
 * @param   xCur 三角形的中心x坐标
 * @param   yCur 三角形的中心y坐标
 * @param   Edges 三角形边长
 * @param   pColor 画笔颜色
 * @note
 * @retval  无
 */
static inline void _GuiDrawFillTopTriangle(uint16_t xCur, uint16_t yCur, uint16_t Edges, uint16_t pColor)  
{  
    uint16_t x1, x2, x3, y1, y2, y3, Height, i;
    
    (void)y2;
    (void)y3;
    
    float dxy, xs, xe;
    
    x1 = xCur - (Edges >> 1);
    x2 = xCur + (Edges >> 1);
    x3 = xCur;
    
    Height = (Edges >> 1) * 1.732;
    
    y1 = y2 = yCur - Height / 3;
    
    y3 = yCur + ((Height / 3) << 1);
    
    /* 计算左右误差 */
    dxy = (x3 - x1) * 1.0 / Height;  
  
    /* 开始进行填充 */
    xs = x1;
    xe = x2;
    
    for(i = 0; i <= Height; i++)
    {  
        _GuiDrawHorLine(xs,  y1 + i, xe - xs, pColor);
  
        xs += dxy;  
        xe -= dxy;  
    }  
} 

/**
 * @func    _GuiDrawBmp
 * @brief   在LCD上显示一个BMP位图，位图点阵扫描次序：从左到右，从上到下
 * @param   xCur 起始点x坐标
 * @param   yCur 起始点y坐标
 * @param   dWidth 图片宽度
 * @param   dHeight 图片高度
 * @param   *Ptr 图片点阵指针
 * @note
 * @retval  无
 */
static inline void _GuiDrawBmp(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, const uint16_t *Ptr)
{
    uint32_t Index = 0;
    const uint16_t *p;
  
    /* 设置图片的位置和大小， 即设置显示窗口 */
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, dWidth, dHeight);
    
    p = Ptr;
    for (Index = 0; Index < dHeight * dWidth; Index++)
    {
        /* 循环画点 */
        _GUI_DeviceAPI.PutPixelNoPos(*(p++));
    }
}























/*****************************************************************************************************
******************************************************************************************************/

/**
 * @func    GuiSetTextFont
 * @brief   设置GUI显示的文字的大小
 * @param   paCharInfo 字体信息结构体
 * @note
 * @retval  无
 */
void GuiSetTextFont(const paCharsInfo_t* paCharInfo)
{
    /* 结构体直接复制，此处是允许这样操作的 */
    _paCharInfo = *paCharInfo;
}

/**
 * @func    GuiSetTextColor
 * @brief   设置GUI显示的文字的前景色和背景色
 * @param   WordColor 字体前景色
 * @param   BackColor 字体背景色
 * @note
 * @retval  无
 */
void GuiSetTextColor(uint16_t WordColor, uint16_t BackColor)
{
    _GUI_TextColor.WordColor = WordColor;
    _GUI_TextColor.BackColor = BackColor;
}

/**
 * @func    GuiSetDeviceAPI
 * @brief   设置GUI与底层驱动的API接口
 * @param   void (* LCDPutPixelNoPos)(uint16_t pColor) 直接画点函数
 * @param   void (* LCDPutPixel)(uint16_t xCur, uint16_t yCur, uint16_t pColor) 根据给定坐标画点函数
 * @param   void (* LCDSetDispCur)(uint16_t xPos, uint16_t yPos) 设置光标
 * @param   void (* LCDSetDispWin)(uint16_t xCur, uint16_t yCur, uint16_t Width, uint16_t Height) 设置显示窗口
 * @note
 * @retval  无
 */
void GuiSetDeviceAPI(void (* PutPixelNoPos)(uint16_t pColor),
                     void (* PutPixel)(uint16_t xCur, uint16_t yCur, uint16_t pColor),
                     void (* SetDispWin)(uint16_t xCur, uint16_t yCur, uint16_t Width, uint16_t Height))
{
    /* 逐一复制 */
    _GUI_DeviceAPI.PutPixelNoPos = PutPixelNoPos;
    _GUI_DeviceAPI.PutPixel = PutPixel;
    _GUI_DeviceAPI.SetDispWin = SetDispWin;
}

#if defined USE_ASCII_EXT_LIB || defined USE_CN_EXT_LIB
/**
 * @func    GuiSetFlashReadAPI
 * @brief   设置GUI与底层驱动的API接口
 * @param   void (* ReadData)(uint32_t Address, uint8_t * pDataBuf, uint32_t BufSize) 从Flash中读取数据
 * @note
 * @retval  无
 */
void GuiSetFlashReadAPI(void (* ReadData)(uint32_t Address, uint8_t * pDataBuf, uint32_t BufSize))
{
    _GUI_DeviceAPI.ReadData = ReadData;
}
#endif
/**
 * @func    GuiClrScr
 * @brief   清屏
 * @param   pColor 清屏的颜色
 * @note
 * @retval  无
 */
void GuiClrScr(uint16_t pColor)
{
    uint32_t n;
    
    /* 设置坐标 */
    _GUI_DeviceAPI.SetDispWin(0, 0, LCD_WIDTH, LCD_HEIGHT);
    
    /* 计算总的像素点 */
    n = LCD_WIDTH * LCD_HEIGHT;
    
    /* 循环刷新 */
    while (n--)
    {
        _GUI_DeviceAPI.PutPixelNoPos(pColor);
    }
}

/**
 * @func    GuiDrawStringAt
 * @brief   显示一个字符串内容，背景不透明
 * @param   Cn 字符串数据
 * @param   xCur 起始的x坐标
 * @param   yCur 起始的y坐标
 * @note
 * @retval  无
 */
void GuiDrawStringAt(const char * Cn, uint16_t xCur, uint16_t yCur)
{
    GUI_CnInfo_t GUI_CnInfo;
    
    /* 背景不透明标识 */
    GUI_CnInfo.TransFlag = CHAR_NO_TRANS;
    
    /* 显示字符串 */
    _GuiDrawString(Cn, xCur, yCur, &GUI_CnInfo);
}

/**
 * @func    GuiDrawTranStringAt
 * @brief   显示一个字符串内容，背景透明
 * @param   Cn 字符串数据
 * @param   xCur 起始的x坐标
 * @param   yCur 起始的y坐标
 * @note
 * @retval  无
 */
void GuiDrawTranStringAt(const char * Cn, uint16_t xCur, uint16_t yCur)
{    
    GUI_CnInfo_t GUI_CnInfo;
    
    /* 背景透明标识 */
    GUI_CnInfo.TransFlag = CHAR_TRANS;
    
    /* 显示字符串 */
    _GuiDrawString(Cn, xCur, yCur, &GUI_CnInfo);
}

/**
 * @func    GuiDrawNumberAt
 * @brief   显示一个数字，背景不透明
 * @param   Number 要实现的数字
 * @param   xCur 起始的x坐标
 * @param   yCur 起始的y坐标
 * @param   PointNum 保留多少位小数点
 * @note
 * @retval  无
 */
uint8_t GuiDrawNumberAt(double Number, uint16_t xCur, uint16_t yCur, uint8_t PointNum)
{
    uint8_t i = 0;
    char Buf[100] = {'\0'};
    char ConsBuf[10] = {'\0'};
    
    /* PointNum大于0，说明需要显示小数点后的数 */
    if (PointNum > 0)
    {
        /* 构造字符串 */
        my_sprintf(ConsBuf, "%%.%df", PointNum);
        my_sprintf(Buf, ConsBuf, Number);
    }
    else
    {
        /* 构造字符串 */
        my_sprintf(Buf, "%d", (int32_t)Number);
    }
    
    /* 显示字符串 */
    GuiDrawStringAt(Buf, xCur, yCur);
    
    while (Buf[i++] != '\0');
    return --i;
}

/**
 * @func    GuiDrawTranNumberAt
 * @brief   显示一个数字，背景透明
 * @param   Number 要实现的数字
 * @param   xCur 起始的x坐标
 * @param   yCur 起始的y坐标
 * @param   PointNum 保留多少位小数点
 * @note
 * @retval  无
 */
uint8_t GuiDrawTranNumberAt(double Number, uint16_t xCur, uint16_t yCur, uint8_t PointNum)
{
    uint8_t i;
    char Buf[100] = {'\0'};
    char ConsBuf[10] = {'\0'};
    
    /* PointNum大于0，说明需要显示小数点后的数 */
    if (PointNum > 0)
    {
        /* 构造字符串 */
        my_sprintf(ConsBuf, "%%.%df", PointNum);
        my_sprintf(Buf, ConsBuf, Number);
    }
    else
    {
        /* 构造字符串 */
        my_sprintf(Buf, "%d", (int32_t)Number);
    }
    
    /* 显示字符串 */
    GuiDrawTranStringAt(Buf, xCur, yCur);
    
    while (Buf[i++] != '\0');
    return --i;
}

/**
 * @func    GuiDrawText
 * @brief   显示文字，背景不透明，需要指定对齐方式
 * @param   Cn 显示的字符串
 * @param   xStart 起始的x坐标
 * @param   yStart 起始的y坐标
 * @param   xEnd 终点的x坐标
 * @param   yEnd 终点的y坐标
 * @param   Align 对齐方式
 * @note
 * @retval  无
 */
void GuiDrawText(const char * Cn, uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t Align)
{
    uint16_t nLen = 0;
    
    /* 计算字符的数量 */
    while (*(Cn + nLen) != '\0')
    {
        nLen++;
    }
    
    /* 没有指定对齐方式，则左对齐显示 */
    if (Align == 0)
    {
        /* 显示字符串 */
        GuiDrawStringAt(Cn, xStart, yStart);
    }
    
    if ((Align & LEFT_ALIGN) && (Align & CENTER_ALIGN))
    {
        /* 显示字符串 */
        GuiDrawStringAt(Cn, xStart,
                            yStart + ((yEnd - yStart) - _paCharInfo.paAsciiInfo.Hight) / 2);
    }
    else if ((Align & RIGHT_ALIGN) && (Align & CENTER_ALIGN))
    {
        /* 显示字符串 */
        GuiDrawStringAt(Cn, xEnd - _paCharInfo.paAsciiInfo.Width * nLen,
                            yStart + ((yEnd - yStart) - _paCharInfo.paAsciiInfo.Hight) / 2);
    }
    else if ((Align & TOP_ALIGN) && (Align & CENTER_ALIGN))
    {
        /* 显示字符串 */
        GuiDrawStringAt(Cn, xStart + ((xEnd - xStart) - _paCharInfo.paAsciiInfo.Width * nLen) / 2,
                            yStart);
    }
    else if ((Align & BOTTOM_ALIGN) && (Align & CENTER_ALIGN))
    {
        /* 显示字符串 */
        GuiDrawStringAt(Cn, xStart + ((xEnd - xStart) - _paCharInfo.paAsciiInfo.Width * nLen) / 2,
                            yEnd - _paCharInfo.paAsciiInfo.Hight);
    }
    else if (Align & CENTER_ALIGN)
    {        /* 显示字符串 */
        GuiDrawStringAt(Cn, xStart + ((xEnd - xStart) - _paCharInfo.paAsciiInfo.Width * nLen) / 2,
                            yStart + ((yEnd - yStart) - _paCharInfo.paAsciiInfo.Hight) / 2);
    }
    else if (Align & LEFT_ALIGN)
    {
        /* 显示字符串 */
        GuiDrawStringAt(Cn, xStart, yStart);
    }
    else if (Align & RIGHT_ALIGN)
    {
        /* 显示字符串 */
        GuiDrawStringAt(Cn, xEnd - _paCharInfo.paAsciiInfo.Width * nLen, yStart);
    }
    else if (Align & TOP_ALIGN)
    {
        /* 显示字符串 */
        GuiDrawStringAt(Cn, xStart, yStart);
    }
    else if (Align & BOTTOM_ALIGN)
    {
        /* 显示字符串 */
        GuiDrawStringAt(Cn, xStart, yEnd - _paCharInfo.paAsciiInfo.Hight);
    }
}

/**
 * @func    GuiDrawTranText
 * @brief   显示文字，背景透明，需要指定对齐方式
 * @param   Cn 显示的字符串
 * @param   xStart 起始的x坐标
 * @param   yStart 起始的y坐标
 * @param   xEnd 终点的x坐标
 * @param   yEnd 终点的y坐标
 * @param   Align 对齐方式
 * @note
 * @retval  无
 */
void GuiDrawTranText(const char * Cn, uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t Align)
{
    uint16_t nLen = 0;
    
    /* 计算字符的个数 */
    while (*(Cn + nLen) != '\0')
    {
        nLen++;
    }
    
    /* 没有指定对齐方式，则左对齐显示 */
    if (Align == 0)
    {
        /* 显示字符串 */
        GuiDrawStringAt(Cn, xStart, yStart);
    }
    
    if ((Align & LEFT_ALIGN) && (Align & CENTER_ALIGN))
    {
        /* 显示字符串 */
        GuiDrawTranStringAt(Cn, xStart,
                                yStart + ((yEnd - yStart) - _paCharInfo.paAsciiInfo.Hight) / 2);
    }
    else if ((Align & RIGHT_ALIGN) && (Align & CENTER_ALIGN))
    {
        /* 显示字符串 */
        GuiDrawTranStringAt(Cn, xEnd - _paCharInfo.paAsciiInfo.Width * nLen,
                                yStart + ((yEnd - yStart) - _paCharInfo.paAsciiInfo.Hight) / 2);
    }
    else if ((Align & TOP_ALIGN) && (Align & CENTER_ALIGN))
    {
        /* 显示字符串 */
        GuiDrawTranStringAt(Cn, xStart + ((xEnd - xStart) - _paCharInfo.paAsciiInfo.Width * nLen) / 2,
                                yStart);
    }
    else if ((Align & BOTTOM_ALIGN) && (Align & CENTER_ALIGN))
    {
        /* 显示字符串 */
        GuiDrawTranStringAt(Cn, xStart + ((xEnd - xStart) - _paCharInfo.paAsciiInfo.Width * nLen) / 2,
                                yEnd - _paCharInfo.paAsciiInfo.Hight);
    }
    else if ((Align & TOP_ALIGN) && (Align & LEFT_ALIGN))
    {
        /* 显示字符串 */
        GuiDrawTranStringAt(Cn, xStart,
                                yStart);        
    }
    else if ((Align & TOP_ALIGN) && (Align & RIGHT_ALIGN))
    {
        /* 显示字符串 */
        GuiDrawTranStringAt(Cn, xEnd - (_paCharInfo.paAsciiInfo.Width * nLen),
                                yStart);        
    }
    else if ((Align & BOTTOM_ALIGN) && (Align & LEFT_ALIGN))
    {
        /* 显示字符串 */
        GuiDrawTranStringAt(Cn, xStart,
                                yEnd - _paCharInfo.paAsciiInfo.Hight);        
    }
    else if ((Align & BOTTOM_ALIGN) && (Align & RIGHT_ALIGN))
    {
        /* 显示字符串 */
        GuiDrawTranStringAt(Cn, xEnd - (_paCharInfo.paAsciiInfo.Width * nLen),
                                yEnd - _paCharInfo.paAsciiInfo.Hight);        
    }
    else if (Align & CENTER_ALIGN)
    {        
        /* 显示字符串 */
        GuiDrawTranStringAt(Cn, xStart + ((xEnd - xStart) - _paCharInfo.paAsciiInfo.Width * nLen) / 2,
                                yStart + ((yEnd - yStart) - _paCharInfo.paAsciiInfo.Hight) / 2);
    }
    else if (Align & LEFT_ALIGN)
    {
        /* 显示字符串 */
        GuiDrawTranStringAt(Cn, xStart, yStart);
    }
    else if (Align & RIGHT_ALIGN)
    {
        /* 显示字符串 */
        GuiDrawTranStringAt(Cn, xEnd - _paCharInfo.paAsciiInfo.Width * nLen, yStart);
    }
    else if (Align & TOP_ALIGN)
    {
        /* 显示字符串 */
        GuiDrawTranStringAt(Cn, xStart, yStart);
    }
    else if (Align & BOTTOM_ALIGN)
    {
        /* 显示字符串 */
        GuiDrawTranStringAt(Cn, xStart, yEnd - _paCharInfo.paAsciiInfo.Hight);
    }
}

/**
 * @func    GuiDrawHorGradientColorBar
 * @brief   绘制一条水平方向的渐变色条
 * @param   xCur 起始x坐标
 * @param   yCur 起始y坐标
 * @param   Width 色条宽度
 * @param   Height 色条高度
 * @param   FirstColor 第一种颜色
 * @param   SecondColor 第二种颜色
 * @note
 * @retval  无
 */
void GuiDrawHorGradientColorBar(uint16_t xCur, uint16_t yCur,
                                uint16_t Width, uint16_t Height,
                                uint16_t FirstColor, uint16_t SecondColor)
{
    uint16_t DataBuf[LCD_WIDTH] = {'\0'};
    uint16_t i, Color;
    
    /* 安全检测，防止数组越界从而产生硬件崩溃 */
    if (Width > LCD_WIDTH)
    {
        Width = LCD_WIDTH;
    }   
    
    /* 获取颜色序列 */
    _GuiGetColorBarData(Width, FirstColor, SecondColor, DataBuf);
    
    /* 复制色条的数据到临时缓存 */
    my_memcpy(_GUI_BarColorItems, DataBuf, Width * 2);
    
    /* 坐标设置 */
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, Width, Height);								
    
    /* 循环画点 */
    for (i = 0; i < Height * Width; i++)
    {
        Color = DataBuf[i % Width];
        _GUI_DeviceAPI.PutPixelNoPos(Color);
    }	
}

/**
 * @func    GuiDrawHorRadiationColorBar
 * @brief   绘制一条水平方向的中间辐射色条
 * @param   xCur 起始x坐标
 * @param   yCur 起始y坐标
 * @param   Width 色条宽度
 * @param   Height 色条高度
 * @param   FirstColor 第一种颜色
 * @param   SecondColor 第二种颜色
 * @note
 * @retval  无
 */
void GuiDrawHorRadiationColorBar(uint16_t xCur, uint16_t yCur,
                                uint16_t Width, uint16_t Height,
                                uint16_t FirstColor, uint16_t MiddleColor)
{
    uint16_t DataBuf[LCD_WIDTH] = {'\0'};
    uint16_t i, Color;
    
    /* 安全检测，防止数组越界从而产生硬件崩溃 */
    if (Width > LCD_WIDTH)
    {
        Width = LCD_WIDTH;
    }    
    
    /* 判断是否为2整除 */
    if (!(Width % 2))
    {
        /* 获取像素序列 */
        _GuiGetColorBarData(Width / 2, FirstColor, MiddleColor, DataBuf);
        
        /* 对像素点进行倒叙 */
        _ColorInverted(DataBuf + Width / 2, DataBuf, Width / 2);
    }
    else
    {
        /* 获取像素序列 */
        _GuiGetColorBarData(Width / 2, FirstColor, MiddleColor, DataBuf);
        
        /* 对中间的点进行手动填充颜色 */
        DataBuf[Width / 2]  = MiddleColor;
        
        /* 对像素点进行倒叙 */
        _ColorInverted(DataBuf + Width / 2 + 1, DataBuf, Width / 2);
    }
    
    /* 复制色条的数据到临时缓存 */
    my_memcpy(_GUI_BarColorItems, DataBuf, Width * 2);
    
    /* 坐标设置 */
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, Width, Height);								
    
    /* 循环画点 */
    for (i = 0; i < Height * Width; i++)
    {
        Color = DataBuf[i % Width];
        _GUI_DeviceAPI.PutPixelNoPos(Color);
    } 
}

/**
 * @func    GuiDrawHorThreeGradientColorBar
 * @brief   绘制一条水平方向的三色渐变的色条
 * @param   xCur 起始x坐标
 * @param   yCur 起始y坐标
 * @param   Width 色条宽度
 * @param   Height 色条高度
 * @param   FirstColor 第一种颜色
 * @param   SecondColor 第二种颜色
 * @param   ThreeColor 第三种颜色
 * @note
 * @retval  无
 */
void GuiDrawHorThreeGradientColorBar(uint16_t xCur, uint16_t yCur,
                                    uint16_t Width, uint16_t Height,
                                    uint16_t FirstColor, uint16_t SecondColor,
                                    uint16_t ThreeColor)
{
    uint16_t DataBuf[LCD_WIDTH] = {'\0'};
    uint16_t i, Color;
    
    /* 安全检测，防止数组越界从而产生硬件崩溃 */
    if (Width > LCD_WIDTH)
    {
        Width = LCD_WIDTH;
    }
    
    if (!(Width % 2))
    {
        /* 获取第一第二种颜色的像素点序列 */
        _GuiGetColorBarData(Width / 2, FirstColor, SecondColor, DataBuf);
        
        /* 获取第二第三种颜色的像素点序列 */
        _GuiGetColorBarData(Width / 2, SecondColor, ThreeColor, DataBuf + Width / 2);
    }
    else
    {
        /* 获取第一第二种颜色的像素点序列 */
        _GuiGetColorBarData(Width / 2, FirstColor, SecondColor, DataBuf);
        
        /* 对于中间的点进行手动填充 */
        DataBuf[Width / 2]  = SecondColor;
        
        /* 获取第二第三种颜色的像素点序列 */
        _GuiGetColorBarData(Width / 2, SecondColor, ThreeColor, DataBuf + Width / 2 + 1);
    }
    
    /* 复制色条的数据到临时缓存 */
    my_memcpy(_GUI_BarColorItems, DataBuf, Width * 2);
    
    /* 坐标设置 */
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, Width, Height);								
    
    /* 循环画点 */
    for (i = 0; i < Height * Width; i++)
    {
        Color = DataBuf[i % Width];
        _GUI_DeviceAPI.PutPixelNoPos(Color);
    }  
}

/**
 * @func    GuiDrawVerGradientColorBar
 * @brief   绘制一条竖直方向的渐变色条
 * @param   xCur 起始x坐标
 * @param   yCur 起始y坐标
 * @param   Width 色条宽度
 * @param   Height 色条高度
 * @param   FirstColor 第一种颜色
 * @param   SecondColor 第二种颜色
 * @note
 * @retval  无
 */
void GuiDrawVerGradientColorBar(uint16_t xCur, uint16_t yCur,
                                uint16_t Width, uint16_t Height,
                                uint16_t FirstColor, uint16_t SecondColor)
{
    uint16_t DataBuf[LCD_HEIGHT] = {'\0'};
    uint16_t i, Color;
    
    /* 安全检测 */
    if (Height > LCD_HEIGHT)
    {
        Height = LCD_HEIGHT;
    }
    
    _GuiGetColorBarData(Height, FirstColor, SecondColor, DataBuf);
    
    /* 复制色条的数据到临时缓存 */
    my_memcpy(_GUI_BarColorItems, DataBuf, Height * 2);
    
    /* 坐标设置 */
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, Width, Height);								
    
    for (i = 0; i < Height * Width; i++)
    {
        Color = DataBuf[i / Width];
        _GUI_DeviceAPI.PutPixelNoPos(Color);
    }	
}

/**
 * @func    GuiDrawVerRadiationColorBar
 * @brief   绘制一条竖直方向的中间辐射色条
 * @param   xCur 起始x坐标
 * @param   yCur 起始y坐标
 * @param   Width 色条宽度
 * @param   Height 色条高度
 * @param   FirstColor 第一种颜色
 * @param   SecondColor 第二种颜色
 * @note
 * @retval  无
 */
void GuiDrawVerRadiationColorBar(uint16_t xCur, uint16_t yCur,
                                uint16_t Width, uint16_t Height,
                                uint16_t FirstColor, uint16_t MiddleColor)
{    
    uint16_t DataBuf[LCD_HEIGHT] = {'\0'};
    uint16_t i, Color;
    
    /* 安全检测 */
    if (Height > LCD_HEIGHT)
    {
        Height = LCD_HEIGHT;
    }
    
    if (!(Height % 2))
    {
        _GuiGetColorBarData(Height / 2, FirstColor, MiddleColor, DataBuf);
        _ColorInverted(DataBuf + Height / 2, DataBuf, Height / 2);
    }
    else
    {
        _GuiGetColorBarData(Height / 2, FirstColor, MiddleColor, DataBuf);
        DataBuf[Height / 2]  = MiddleColor;
        _ColorInverted(DataBuf + Height / 2 + 1, DataBuf, Height / 2);
    }
    
    /* 复制色条的数据到临时缓存 */
    my_memcpy(_GUI_BarColorItems, DataBuf, Height * 2);
    
    /* 坐标设置 */
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, Width, Height);								
    
    for (i = 0; i < Height * Width; i++)
    {
        Color = DataBuf[i / Width];
        _GUI_DeviceAPI.PutPixelNoPos(Color);
    } 
}

/**
 * @func    GuiDrawHorThreeGradientColorBar
 * @brief   绘制一条竖直方向的三色渐变的色条
 * @param   xCur 起始x坐标
 * @param   yCur 起始y坐标
 * @param   Width 色条宽度
 * @param   Height 色条高度
 * @param   FirstColor 第一种颜色
 * @param   SecondColor 第二种颜色
 * @param   ThreeColor 第三种颜色
 * @note
 * @retval  无
 */
void GuiDrawVerThreeGradientColorBar(uint16_t xCur, uint16_t yCur,
                                    uint16_t Width, uint16_t Height,
                                    uint16_t FirstColor, uint16_t SecondColor,
                                    uint16_t ThreeColor)
{
    uint16_t DataBuf[LCD_HEIGHT] = {'\0'};
    uint16_t i, Color;
    
    /* 安全检测 */
    if (Height > LCD_HEIGHT)
    {
        Height = LCD_HEIGHT;
    }
    
    if (!(Width % 2))
    {
        _GuiGetColorBarData(Height / 2, FirstColor, SecondColor, DataBuf);
        _GuiGetColorBarData(Height / 2, SecondColor, ThreeColor, DataBuf + Height / 2);
    }
    else
    {
        _GuiGetColorBarData(Height / 2, FirstColor, SecondColor, DataBuf);
        DataBuf[Height / 2]  = SecondColor;
        _GuiGetColorBarData(Height / 2, SecondColor, ThreeColor, DataBuf + Height / 2 + 1);
    }
    
    /* 复制色条的数据到临时缓存 */
    my_memcpy(_GUI_BarColorItems, DataBuf, Height * 2);
    
    /* 坐标设置 */
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, Width, Height);								
    
    for (i = 0; i < Height * Width; i++)
    {
        Color = DataBuf[i / Width];
        _GUI_DeviceAPI.PutPixelNoPos(Color);
    } 
}

/**
 * @func    GuiGetBarColorItems
 * @brief   获取最近绘制的彩色色条的颜色集合
 * @param   *Buf 输入缓存
 * @param   nLen 缓存的长度
 * @note
 * @retval  无
 */
void GuiGetBarColorItems(uint16_t *Buf, uint16_t nLen)
{
#if (LCD_HEIGHT > LCD_WIDTH)
    /* 安全检测 */
    if (nLen > LCD_HEIGHT)
    {
        nLen = LCD_HEIGHT;
    }
#elif (LCD_HEIGHT < LCD_WIDTH)
    /* 安全检测 */
    if (nLen > LCD_WIDTH)
    {
        nLen = LCD_WIDTH;
    }
#endif
    /* 复制色条的数据到输出缓存 */
    my_memcpy(Buf, _GUI_BarColorItems, nLen * 2);
}

/**
 * @func    GuiDrawLine
 * @brief   采用 Bresenham 算法，在2点间画一条直线。
 * @param   xStart 起始点x坐标
 * @param   yStart 起始点y坐标
 * @param   xEnd 终止点x坐标
 * @param   yEnd 终止点y坐标
 * @param   pColor 画笔颜色
 * @note
 * @retval  无
 */
void GuiDrawLine(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t pColor)
{
    _GuiDrawLine(xStart, yStart, xEnd, yEnd, pColor);
}

/**
 * @func    GuiDrawHorLine
 * @brief   绘制一条高度为1的水平直线
 * @param   xCur 起始点x坐标
 * @param   yCur 起始点y坐标
 * @param   Lenght 直线的长度
 * @param   pColor 画笔颜色
 * @note
 * @retval  无
 */
void GuiDrawHorLine(uint16_t xCur, uint16_t yCur, uint16_t Lenght, uint16_t pColor)
{
    if (Lenght == 0)
        return ;
    _GuiDrawHorLine(xCur, yCur, Lenght, pColor);
}

/**
 * @func    GuiDrawVerLine
 * @brief   绘制一条宽度为1的垂直直线
 * @param   xCur 起始点x坐标
 * @param   yCur 起始点y坐标
 * @param   Lenght 直线的长度
 * @param   pColor 画笔颜色
 * @note
 * @retval  无
 */
void GuiDrawVerLine(uint16_t xCur, uint16_t yCur, uint16_t Lenght, uint16_t pColor)
{
    if (Lenght == 0)
        return ;
    _GuiDrawVerLine(xCur, yCur, Lenght, pColor);
}

/**
 * @func    GuiDrawHorColorLine
 * @brief   绘制一条彩色水平线
 * @param   xCur 起始点x坐标
 * @param   yCur 起始点y坐标
 * @param   dWidth 直线的宽度
 * @param   *pColor 颜色缓冲区
 * @note
 * @retval  无
 */
void GuiDrawHorColorLine(uint16_t xCur, uint16_t yCur, uint16_t dWidth, const uint16_t *pColor)
{
    if (dWidth == 0)
        return ;
    _GuiDrawHorColorLine(xCur, yCur, dWidth, pColor);
}

/**
 * @func    GuiDrawVerLine
 * @brief   绘制一条宽度为1的曲线直线
 * @param   *x x点的集合
 * @param   *y y点的集合
 * @param   Size 点的集合的大小
 * @param   pColor 画笔颜色
 * @note
 * @retval  无
 */
void GuiDrawPoints(const uint16_t *x, uint16_t *y, uint16_t Size, uint16_t pColor)
{
	uint16_t i;

	for (i = 0 ; i < Size - 1; i++)
	{
		_GuiDrawLine(x[i], y[i], x[i + 1], y[i + 1], pColor);
	}
}

/**
 * @func    _GuiDrawCircle
 * @brief   绘制一个圆，笔宽为1个像素
 * @param   xCur 圆心x坐标
 * @param   yCur 圆心y坐标
 * @param   rRadius 圆的半径
 * @param   pColor 画笔颜色
 * @note
 * @retval  无
 */
void GuiDrawCircle(uint16_t xCur, uint16_t yCur, uint16_t rRadius, uint16_t pColor)
{
	_GuiDrawCircle(xCur, yCur, rRadius, pColor);
}

/**
 * @func    _GuiDrawRect
 * @brief   绘制水平放置的矩形。
 * @param   xCur 起始点x坐标
 * @param   yCur 起始点y坐标
 * @param   dWidth 矩形的宽度
 * @param   dHeight 矩形的高度
 * @param   pColor 画笔颜色
 * @note    ---------------->---
           |(xCur，yCur)        |
           V                    V  dHeight
           |                    |
           ---------------->---
                dWidth
 * @retval  无
 */
void GuiDrawRect(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor)
{
    if (dWidth == 0 || dHeight == 0)
        return ;
    _GuiDrawRect(xCur, yCur, dWidth, dHeight, pColor);
}

/**
 * @func    _GuiDrawFillCircle
 * @brief   填充圆形。
 * @param   xCur 原点x坐标
 * @param   yCur 原点y坐标
 * @param   rRadius 圆形的半径
 * @param   pColor 画笔颜色
 * @note    
 * @retval  无
 */
void GuiDrawFillCircle(uint16_t xCur, uint16_t yCur, uint16_t rRadius, uint16_t pColor)
{
    _GuiDrawFillCircle(xCur, yCur, rRadius, pColor);
}

/**
 * @func    _GuiDrawRectRound
 * @brief   绘制水平放置的圆弧矩形。
 * @param   xCur 起始点x坐标
 * @param   yCur 起始点y坐标
 * @param   dWidth 矩形的宽度
 * @param   dHeight 矩形的高度
 * @param   pColor 画笔颜色
 * @param   Round 圆弧大小
 * @note    
 * @retval  无
 */
void GuiDrawRectRound(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor, uint16_t Round)
{
    if (dWidth == 0 || dHeight == 0)
        return ;
    _GuiDrawRectRound(xCur, yCur, dWidth, dHeight, pColor, Round);
}

/**
 * @func    _GuiDrawFillRect
 * @brief   填充矩形。
 * @param   xCur 起始点x坐标
 * @param   yCur 起始点y坐标
 * @param   dWidth 矩形的宽度
 * @param   dHeight 矩形的高度
 * @param   pColor 画笔颜色
 * @note     ---------------->---
            |(xCur，yCur)        |
            V                    V  dHeight
            |                    |
            ---------------->---
                dWidth
 * @retval  无
 */
void GuiDrawFillRect(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor)
{
    if (dWidth == 0 || dHeight == 0)
        return ;
	_GuiDrawFillRect(xCur, yCur, dWidth, dHeight, pColor);
}

/**
 * @func    GuiDrawFillRectRound
 * @brief   在LCD上填充一个圆弧的矩形
 * @param   xCur 起始点x坐标
 * @param   yCur 起始点y坐标
 * @param   dWidth 矩形的宽度
 * @param   dHeight 矩形的高度
 * @param   pColor 填充的颜色
 * @param   Round 圆弧的大小，半径
 * @note
 * @retval  无
 */
void GuiDrawFillRectRound(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor, uint16_t Round)
{
    if (dWidth == 0 || dHeight == 0)
        return ;
    _GuiDrawFillRectRound(xCur, yCur, dWidth, dHeight, pColor, Round);
}

/**
 * @func    GuiDrawTopTriangle
 * @brief   在LCD上显示一个三角形
 * @param   xCur 三角形的中心x坐标
 * @param   yCur 三角形的中心y坐标
 * @param   Edges 三角形边长
 * @param   pColor 画笔颜色
 * @note
 * @retval  无
 */
void GuiDrawTopTriangle(uint16_t xCur, uint16_t yCur, uint16_t Edges, uint16_t pColor)
{
    _GuiDrawTopTriangle(xCur, yCur, Edges, pColor);
}

/**
 * @func    GuiDrawFillTopTriangle
 * @brief   在LCD上填充显示一个三角形
 * @param   xCur 三角形的中心x坐标
 * @param   yCur 三角形的中心y坐标
 * @param   Edges 三角形边长
 * @param   pColor 画笔颜色
 * @note
 * @retval  无
 */
void GuiDrawFillTopTriangle(uint16_t xCur, uint16_t yCur, uint16_t Edges, uint16_t pColor)  
{  
    _GuiDrawFillTopTriangle(xCur, yCur, Edges, pColor);
}

/**
 * @func    GuiDrawBMP
 * @brief   在LCD上显示一个BMP位图，位图点阵扫描次序：从左到右，从上到下
 * @param   xCur 起始点x坐标
 * @param   yCur 起始点y坐标
 * @param   dWidth 图片宽度
 * @param   dHeight 图片高度
 * @param   *Ptr 图片点阵指针
 * @note
 * @retval  无
 */
void GuiDrawBMP(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, const uint16_t *Ptr)
{
	_GuiDrawBmp(xCur, yCur, dWidth, dHeight, Ptr);
}

/**
 * @func    GuiGetWarmColdColor
 * @brief   根据输入的颜色，获取该颜色的冷暖颜色
 * @param   SrcColor 输入的颜色
 * @param   * WarmColor 输出暖颜色
 * @param   * ColdColor 输出冷颜色
 * @note    具体计算过程就不分析了，基本原理就是红色跟蓝色的变化
 * @retval  无
 */
void GuiGetWarmColdColor(uint16_t SrcColor, uint16_t * WarmColor, uint16_t * ColdColor)
{
    GUI_RGB565Color_t SrcColorData, WarmColorData, ColdColorData;
    
    /* 利用结构体和共用体拆分颜色 */
    SrcColorData.Color = SrcColor;
    
    /* 暖色计算 */
    if (SrcColorData.RGB.R > 16)
    {
        WarmColorData.RGB.R = 31;
        WarmColorData.RGB.G = SrcColorData.RGB.G;
        if (SrcColorData.RGB.B > (15 - (31 - SrcColorData.RGB.R)))
        {
            WarmColorData.RGB.B = SrcColorData.RGB.B - (15 - (31 - SrcColorData.RGB.R));
        }
        else
        {
            WarmColorData.RGB.B = 0;
        }
    }
    else
    {
        WarmColorData.RGB.R = SrcColorData.RGB.R + 15;
        WarmColorData.RGB.G = SrcColorData.RGB.G;
        WarmColorData.RGB.B = SrcColorData.RGB.B;
    }

    /*冷色计算 */
    if (SrcColorData.RGB.B > 16)
    {
        ColdColorData.RGB.B = 31;
        ColdColorData.RGB.G = SrcColorData.RGB.G;
        if (SrcColorData.RGB.R > (15 - (31 - SrcColorData.RGB.B)))
        {
            ColdColorData.RGB.R = SrcColorData.RGB.R - (15 - (31 - SrcColorData.RGB.B));
        }
        else
        {
            ColdColorData.RGB.R = 0;
        }
    }
    else
    {
        ColdColorData.RGB.R = SrcColorData.RGB.R;
        ColdColorData.RGB.G = SrcColorData.RGB.G;
        ColdColorData.RGB.B = SrcColorData.RGB.B + 15;
    }
    
    /* 输出颜色 */
    *WarmColor = WarmColorData.Color;
    *ColdColor = ColdColorData.Color;
}


