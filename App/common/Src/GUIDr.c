/**
 ******************************************************************************
 * @file      GUIDr.c
 * @author    ZSY
 * @version   V1.0.2
 * @date      2018-09-15
 * @brief     ���ļ�ʵ��LCD��һЩ�������֣�ͼ�εȲ�������ֲ���ļ�����Ҫ���ı��ļ������� 
 * @History
 * Date           Author    version    		   Notes
 * 2018-06-22      ZSY      V1.0.0          first version.
 * 2018-06-22      ZSY      V1.0.1          ��Ӷ��ⲿ�ֿ��֧��.
 * 2018-09-15      ZSY      V1.0.2          ���Ĳ��ֽṹ��������.
 */

/* Includes ------------------------------------------------------------------*/
#include "GUIDr.h"
#include "math.h"
#include "stdlib.h"
#include "MyString.h"

/* Private macro Definition --------------------------------------------------*/
/* �����������Ʊ��� */
#define CHAR_TRANS          0x00
#define CHAR_NO_TRANS       0x01
#define GBK_FONT            0x00
#define GB2312_FONT         0x01
#define NONE_FONT           0xff

#define GUI_ERROR           (uint32_t)(-1)
#define GUI_OK              (uint32_t)(-2)
#define GUI_NO_FIND_FONT    (uint32_t)(-3)

/* ��չ�������� */
typedef unsigned char       uint8_t;
/* ϵͳ�Ѿ������� */
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

/* ����������ɫ�Ľṹ�� */
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

/* ������ʾ���������ݵ���Ϣ�ṹ�� */
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

/* �ײ�Ӧ���ṩ��4��API�ӿ� */
typedef struct
{
    void (* PutPixelNoPos)(uint16_t pColor);
    void (* PutPixel)(uint16_t xCur, uint16_t yCur, uint16_t pColor);
    void (* SetDispWin)(uint16_t xCur, uint16_t yCur, uint16_t Width, uint16_t Height);
#if defined USE_ASCII_EXT_LIB || defined USE_CN_EXT_LIB
    void (* ReadData)(uint32_t Address, uint8_t * pDataBuf, uint32_t BufSize);
#endif
}GUI_DeviceAPI_t;

/* �ⲿ���������ñ��� */
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

/* �������������ɫ�ı�����������ΪĬ��ֵ */
static GUI_TextColor_t _GUI_TextColor = GUI_TextDefaultColor;

/* Ĭ���������ݻ���������ֹ���Ҳ�������ʹ��ʾ���� */
static uint8_t _GUI_FontDefaultDataBuf[BYTES_PER_FONT] = {'\0'};

#if defined USE_CN_EXT_LIB || defined USE_ASCII_EXT_LIB
/* Ĭ���������ݻ�������ʹ���ⲿ�ֿ�ʱ�õ� */
static uint8_t _GUI_FontDataBufFromFlash[BYTES_PER_FONT] = {'\0'};
#endif

/* ������������һЩ��Ϣ�ı�����������ΪĬ��ֵ */
static paCharsInfo_t _paCharInfo = GUI_TextDefaultFont;

/* ����ײ�������API�ӿڱ��� */
static GUI_DeviceAPI_t _GUI_DeviceAPI;

/* ����һ����������ȡ��Ļ���ߵ����ֵ��Ϊ���ϵĴ�С��������ʱ����ɫ������ */
#if (LCD_HEIGHT > LCD_WIDTH)
static uint16_t _GUI_BarColorItems[LCD_HEIGHT] = {'\0'};
#elif (LCD_HEIGHT < LCD_WIDTH)
static uint16_t _GUI_BarColorItems[LCD_WIDTH] = {'\0'};
#endif

/**
 * @func    _GetASCII_FontData
 * @brief   ���ڴ����ȡ���������
 * @param   GUI_CnInfo ��ʾ���ֵ���Ϣ�Ľṹ��
 * @note
 * @retval  ��
 */
static inline void _GetASCII_FontData(GUI_CnInfo_t * GUI_CnInfo)
{
    uint8_t WordNun; 

    GUI_CnInfo->ERROR_CODE = GUI_OK;

#if defined USE_ASCII_EXT_LIB || defined USE_CN_EXT_LIB
    uint32_t FlashAddr = 0;
#endif
/* ���β���û����ʾ��ASCII�� */
    if (GUI_CnInfo->Cn < 32)
    {
        GUI_CnInfo->ERROR_CODE = GUI_ERROR;
        return ;
    }
    
    /* ��ASCII���32��ʼ����ʾ�����Դ˴�����32 */
    WordNun = GUI_CnInfo->Cn - 32;
#ifdef USE_ASCII_INT_LIB        
    /* ʹ��16�ĵ��� */
#ifdef USING_CN_16_CHAR
    if (my_strncmp("A16", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 8*16 ASCII�ַ� */
    {
        /* ָ��ֱ��ȡ��ַ */
        GUI_CnInfo->FontDataBuf = (uint8_t *)&ASCII08x16[(uint16_t )WordNun * GUI_CnInfo->SumBytes];
        return ;
    }
#endif /* USING_CN_16_CHAR */   
    
    /* ʹ��24�ĵ��� */
#ifdef USING_CN_24_CHAR  
    if (my_strncmp("A24", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 16*24 ASCII�ַ� */
    {          
        GUI_CnInfo->FontDataBuf = (uint8_t *)&ASCII12x24[(uint16_t )WordNun * GUI_CnInfo->SumBytes];
        return ;
    }
#endif /* USING_CN_24_CHAR */
    
    /* ʹ��32�ĵ��� */
#ifdef USING_CN_32_CHAR  
    if (my_strncmp("A32", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 20*32 ASCII�ַ� */
    {  
        GUI_CnInfo->FontDataBuf = (uint8_t *)&ASCII16x32[(uint16_t )WordNun * GUI_CnInfo->SumBytes];
        return ;
    }
#endif /* USING_CN_32_CHAR */
    
    /* ʹ��40�ĵ��� */
#ifdef USING_CN_40_CHAR  
    if (my_strncmp("A40", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 24*40 ASCII�ַ� */
    {  
        GUI_CnInfo->FontDataBuf = (uint8_t *)&ASCII20x40[(uint16_t )WordNun * GUI_CnInfo->SumBytes];
        return ;
    }
#endif /* USING_CN_40_CHAR */
    
    /* ʹ��48�ĵ��� */
#ifdef USING_CN_48_CHAR   
    if (my_strncmp("A48", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 28*48 ASCII�ַ� */
    { 
        GUI_CnInfo->FontDataBuf = (uint8_t *)&ASCII24x48[(uint16_t )WordNun * GUI_CnInfo->SumBytes];
        return ;
    }
#endif /* USING_CN_48_CHAR */
#elif defined USE_ASCII_EXT_LIB
    /* ʹ��16�ĵ��� */
#ifdef USING_CN_16_CHAR
    if (my_strncmp("A16", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 8*16 ASCII�ַ� */
    {
#ifdef FONT_ASCII16_BASE_ADDR
        /* �����ַ */
        FlashAddr = FONT_ASCII16_BASE_ADDR + (uint16_t)WordNun * GUI_CnInfo->SumBytes;
        goto _ReadASCII_Data; 
#else
        GUI_CnInfo->ERROR_CODE = GUI_NO_FIND_FONT;
        return ;
#endif
    }
#endif /* USING_CN_16_CHAR */   
    
    /* ʹ��24�ĵ��� */
#ifdef USING_CN_24_CHAR  
    if (my_strncmp("A24", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 16*24 ASCII�ַ� */
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
    
    /* ʹ��32�ĵ��� */
#ifdef USING_CN_32_CHAR  
    if (my_strncmp("A32", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 20*32 ASCII�ַ� */
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
    
    /* ʹ��40�ĵ��� */
#ifdef USING_CN_40_CHAR  
    if (my_strncmp("A40", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 24*40 ASCII�ַ� */
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
    
    /* ʹ��48�ĵ��� */
#ifdef USING_CN_48_CHAR   
    if (my_strncmp("A48", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 28*48 ASCII�ַ� */
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
 * @brief   ���ڴ����ȡ���������
 * @param   GUI_CnInfo ��ʾ���ֵ���Ϣ�Ľṹ��
 * @note
 * @retval  ��
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
    if(my_strncmp("H16", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 16*16 �����ַ� */
    { 
        for (i = 0; i < ChAR_NUM_MAX; i++)        //ѭ����ѯ���룬���Һ��ֵ�����
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
    if (my_strncmp("H24", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 24*24 �����ַ� */
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
    if (my_strncmp("H32", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 32*32 �����ַ� */
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
    if (my_strncmp("H40", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 40*40 �����ַ� */
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
    if (my_strncmp("H48", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 48*48 �����ַ� */
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
    if(my_strncmp("H16", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 16*16 �����ַ� */
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
    if (my_strncmp("H24", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 24*24 �����ַ� */
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
    if (my_strncmp("H32", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 32*32 �����ַ� */
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
    if (my_strncmp("H40", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 40*40 �����ַ� */
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
    if (my_strncmp("H48", GUI_CnInfo->paCnInfo.Char, 3) == 0) /* 48*48 �����ַ� */
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
        /* ���ݺ�������ļ��㹫ʽ������ʼ��ַ */
        code2 = c >> 8;
        code1 = c & 0xFF;
        
        /* �����ַ������ǰ�˳��洢�ģ��ȴ洢����λ�����ţ���Ȼ���ǵ�λ��λ�ţ����������õ���С�˸�ʽ��
            һ�����������ֽڣ���ȡ��16λ�����������෴��16λ�����ĸ�λ��λ�ţ���λ�����š�
        */
        FlashAddr = ((code1 - 0xA1) * 94 + (code2 - 0xa1)) * GUI_CnInfo->SumBytes + FlashAddr;
    }
    else if (FontType == GB2312_FONT)
    {
        /* ���ݺ�������ļ��㹫ʽ������ʼ��ַ */
        code2 = c >> 8;
        code1 = c & 0xFF;
        
        /* �����ַ������ǰ�˳��洢�ģ��ȴ洢����λ�����ţ���Ȼ���ǵ�λ��λ�ţ����������õ���С�˸�ʽ��
            һ�����������ֽڣ���ȡ��16λ�����������෴��16λ�����ĸ�λ��λ�ţ���λ�����š�
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
 * @brief   ���ڴ����ȡ�������Ϣ
 * @param   GUI_CnInfo ��ʾ���ֵ���Ϣ�Ľṹ��
 * @note
 * @retval  ��
 */
static inline void _GetDataFromMemory(GUI_CnInfo_t * GUI_CnInfo)
{
    /* �ж��Ƿ񳬳������С��������С���ƴ�С����ֹ����Խ�� */
    if ( GUI_CnInfo->SumBytes > BYTES_PER_FONT)
    {
         GUI_CnInfo->SumBytes = BYTES_PER_FONT;
    }
    
    /* �ж������ĵĻ���Ӣ�ĵ� */    
    if (GUI_CnInfo->Cn < 0x80)                                                                
    {
        _GetASCII_FontData(GUI_CnInfo);
    }
    /* ������ʾ */
    else
    {
        _GetCN_FontData(GUI_CnInfo);
    }
}

/**
 * @func    _DispChar
 * @brief   ��ʾ����
 * @param   GUI_CnInfo ��ʾ���ֵ���Ϣ�Ľṹ��
 * @note
 * @retval  ��
 */
static inline void _DispChar(GUI_CnInfo_t * GUI_CnInfo)
{
    uint16_t i, Cnt, Color;
    uint8_t * _FontDataBuf = NULL;
    /* ��ȡ����ĵ��� */
    _GetDataFromMemory(GUI_CnInfo);

    _FontDataBuf = GUI_CnInfo->FontDataBuf;

    if (GUI_CnInfo->ERROR_CODE == GUI_ERROR)
    {
        _FontDataBuf = _GUI_FontDefaultDataBuf;
        return ;
    }
    /* �ж��Ƿ���Ҫ����͸����ʾ */
    if (GUI_CnInfo->TransFlag == CHAR_NO_TRANS)
    {
        /* ���ô��� */
        _GUI_DeviceAPI.SetDispWin(GUI_CnInfo->tPos.x, GUI_CnInfo->tPos.y, GUI_CnInfo->paCnInfo.Width, GUI_CnInfo->paCnInfo.Hight);
       
        /* ѭ��ȡ���ݽ�����ʾ */
        for (Cnt = 0; Cnt < GUI_CnInfo->SumBytes; Cnt++)
        {
            /* ��ȡ�������ݣ�һ���ֽ� */
            Color = _FontDataBuf[Cnt];
            for (i = 0; i < 8; i++)
            {
                /* �жϻ��� */
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
    
    /* ��Ҫ����͸����ʾ */
    else if (GUI_CnInfo->TransFlag == CHAR_TRANS)
    {
        uint16_t x, y;
        
        /* ��¼��ʼ���� */
        x = GUI_CnInfo->tPos.x;
        y = GUI_CnInfo->tPos.y;
        
        for (Cnt = 0; Cnt < GUI_CnInfo->SumBytes; Cnt++)
        {
            /* ��ȡ�������ݣ�һ���ֽ� */
            Color = _FontDataBuf[Cnt];
            for (i = 0; i < 8; i++)
            {
                if((Color << i) & 0x80)
                {
                    /* ���û��㺯�����л��� */
                    _GUI_DeviceAPI.PutPixel(x, y, GUI_CnInfo->Color.WordColor);
                }
                x++;
                /* �ж��Ƿ���Ҫ���� */
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
 * @brief   ��ʾһ���ַ�������
 * @param   Cn �ַ�������
 * @param   x ��ʼ��x����
 * @param   y ��ʼ��y����
 * @param   GUI_CnInfoIn ��ʾ���ֵ���Ϣ�Ľṹ��
 * @note
 * @retval  ��
 */
static inline void _GuiDrawString(const char * Cn, uint16_t xCur, uint16_t yCur, GUI_CnInfo_t * GUI_CnInfoIn)
{
    uint16_t Ch;
    GUI_CnInfo_t * GUI_CnInfo = GUI_CnInfoIn;
    
    /* ѭ��ȡ���ַ����е��ַ�������ʾ */
    while (*Cn != '\0')
    {
        /* ��ȡһ���ֽ� */
        Ch = *Cn;
        
        /* �ж��Ƿ�ΪӢ���ַ��������ַ��ı����0x80��ʼ */
        if (Ch < 0x80)
        {
            /* ����һЩ��Ϣ����ʾ��Ҫ */
            GUI_CnInfo->Cn = Ch;
            GUI_CnInfo->SumBytes = _paCharInfo.paAsciiInfo.Hight * _paCharInfo.paAsciiInfo.PerLineBytes;
                       
            /* ������ʾ�������Ϣ */
            GUI_CnInfo->paCnInfo = _paCharInfo.paAsciiInfo;
            
            /* ������ʾ������ */
            GUI_CnInfo->tPos.x = xCur;
            GUI_CnInfo->tPos.y = yCur;
            
            /* һ��Ӣ���ַ���һ���ֽڵı��빹�� */
            Cn++;
            
            /* x������תһ���ַ��Ŀ�� */
            xCur += GUI_CnInfo->paCnInfo.Width;
        }
        else
        {
            GUI_CnInfo->Cn = *(uint16_t *)Cn;
            GUI_CnInfo->SumBytes = _paCharInfo.paHanziInfo.Hight * _paCharInfo.paHanziInfo.PerLineBytes;
            GUI_CnInfo->paCnInfo = _paCharInfo.paHanziInfo;
            
            /* ������ʾ������ */
            GUI_CnInfo->tPos.x = xCur;
            GUI_CnInfo->tPos.y = yCur;
            
            /* һ�������������ֽڵı��빹�� */
            Cn += 2;
            
            /* x������תһ���ַ��Ŀ�� */
            xCur += GUI_CnInfo->paCnInfo.Width;
        }

        GUI_CnInfo->Color = _GUI_TextColor;

        /* ��ʾ���� */
        _DispChar(GUI_CnInfo);
    }
}

/**
 * @func    _GuiGetColorBarData
 * @brief   ��ȡ������ɫֵ
 * @param   ColorNum ���ص������
 * @param   FirstColor ��һ����ɫ
 * @param   SecondColor �ڶ�����ɫ
 * @param   ColorData ���ص㻺����
 * @note
 * @retval  ��
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
    
    /* ������� */
    RGB_FirstColor.Color = FirstColor;
    RGB_SecondColor.Color = SecondColor;
    
    /* ����ϵ�� */
    R_Coefficient = abs(RGB_SecondColor.RGB.R - RGB_FirstColor.RGB.R) / (double)(ColorNum - 1);
    G_Coefficient = abs(RGB_SecondColor.RGB.G - RGB_FirstColor.RGB.G) / (double)(ColorNum - 1);
    B_Coefficient = abs(RGB_SecondColor.RGB.B - RGB_FirstColor.RGB.B) / (double)(ColorNum - 1);
    
    /* ѭ���������ص� */
    for (i = 0; i < ColorNum; i++)
    {
        /* �����ɫ */
        if(RGB_SecondColor.RGB.R > RGB_FirstColor.RGB.R)
        {
            R = RGB_FirstColor.RGB.R + i * R_Coefficient;
        }
        else
        {
            R = RGB_FirstColor.RGB.R - i * R_Coefficient;
        }
        
        /* ������ɫ */
        if (RGB_SecondColor.RGB.G > RGB_FirstColor.RGB.G)
        {
            G = RGB_FirstColor.RGB.G + i * G_Coefficient;
        }
        else
        {
            G = RGB_FirstColor.RGB.G - i * G_Coefficient;
        }
        
        /* ������ɫ */
        if (RGB_SecondColor.RGB.B > RGB_FirstColor.RGB.B)
        {
            B = RGB_FirstColor.RGB.B + i * B_Coefficient;
        }
        else
        {
            B = RGB_FirstColor.RGB.B - i * B_Coefficient;
        }
        
        /* �ϳ����ص� */
        ColorData[i] = (uint16_t)(R << 11) | (uint16_t)(G << 5) | B;
    } 
}

/**
 * @func    ColorInverted
 * @brief   �����ص�������е������
 * @param   *pbDest Ŀ���ַ
 * @param   *pbSrc Դ��ַ
 * @param   nLen ����
 * @note
 * @retval  ��
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
 * @brief   ���� Bresenham �㷨����2��仭һ��ֱ�ߡ�
 * @param   xStart ��ʼ��x����
 * @param   yStart ��ʼ��y����
 * @param   xEnd ��ֹ��x����
 * @param   yEnd ��ֹ��y����
 * @param   pColor ������ɫ
 * @note
 * @retval  ��
 */
static inline void _GuiDrawLine(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t pColor)
{
    int32_t dx, dy;
    int32_t tx, ty;
    int32_t inc1, inc2;
    int32_t d, iTag;
    int32_t x, y;
    
    /* ���� Bresenham �㷨����2��仭һ��ֱ�� */
    _GUI_DeviceAPI.PutPixel(xStart, yStart, pColor);
    
    /* ��������غϣ���������Ķ�����*/
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
    
    /*���dyΪ�Ƴ������򽻻��ݺ����ꡣ*/
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
    
    /* ȷ������1���Ǽ�1 */
    tx = xEnd > xStart ? 1 : -1;   
    ty = yEnd > yStart ? 1 : -1;
    x = xStart;
    y = yStart;
    inc1 = 2 * dy;
    inc2 = 2 * (dy - dx);
    d = inc1 - dx;
    
    /* ѭ������ */
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
 * @brief   ����һ���߶�Ϊ1��ˮƽֱ��
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   Lenght ֱ�ߵĳ���
 * @param   pColor ������ɫ
 * @note
 * @retval  ��
 */
static inline void _GuiDrawHorLine(uint16_t xCur, uint16_t yCur, uint16_t Lenght, uint16_t pColor)
{
    _GuiDrawLine(xCur, yCur, xCur + Lenght, yCur, pColor);
}

/**
 * @func    _GuiDrawHorColorLine
 * @brief   ����һ����ɫˮƽ��
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   dWidth ֱ�ߵĿ��
 * @param   *pColor ��ɫ������
 * @note
 * @retval  ��
 */
static inline void _GuiDrawHorColorLine(uint16_t xCur, uint16_t yCur, uint16_t dWidth, const uint16_t *pColor)
{
    uint16_t i;
	
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, dWidth, 1);
    
    /* д�Դ� */
    for (i = 0; i < dWidth; i++)
    {
        _GUI_DeviceAPI.PutPixelNoPos(*(pColor++));
    }
}

/**
 * @func    _GuiDrawVerLine
 * @brief   ����һ�����Ϊ1�Ĵ�ֱֱ��
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   Lenght ֱ�ߵĳ���
 * @param   pColor ������ɫ
 * @note
 * @retval  ��
 */
static inline void _GuiDrawVerLine(uint16_t xCur, uint16_t yCur, uint16_t Lenght, uint16_t pColor)
{
    _GuiDrawLine(xCur, yCur, xCur, yCur + Lenght, pColor);
}

/**
 * @func    _GuiDrawRect
 * @brief   ����ˮƽ���õľ��Ρ�
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   dWidth ���εĿ��
 * @param   dHeight ���εĸ߶�
 * @param   pColor ������ɫ
 * @note    ---------------->---
           |(xCur��yCur)        |
           V                    V  dHeight
           |                    |
           ---------------->---
                dWidth
 * @retval  ��
 */
static inline void _GuiDrawRect(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor)
{
    /* �� */
    _GuiDrawLine(xCur, yCur, xCur + dWidth - 1, yCur, pColor);	
    
    /* �� */
    _GuiDrawLine(xCur, yCur + dHeight - 1, xCur + dWidth - 1, yCur + dHeight - 1, pColor);	
    
    /* �� */
    _GuiDrawLine(xCur, yCur, xCur, yCur + dHeight - 1, pColor);	
    
    /* �� */
    _GuiDrawLine(xCur + dWidth - 1, yCur, xCur + dWidth - 1, yCur + dHeight, pColor);	
}

/**
 * @func    _GuiDrawCircle
 * @brief   ����һ��Բ���ʿ�Ϊ1������
 * @param   xCur Բ��x����
 * @param   yCur Բ��y����
 * @param   rRadius Բ�İ뾶
 * @param   pColor ������ɫ
 * @note
 * @retval  ��
 */
static inline void _GuiDrawCircle(uint16_t xCur, uint16_t yCur, uint16_t rRadius, uint16_t pColor)
{
    int32_t  D;			/* Decision Variable */
    uint32_t  nxCur;		/* ��ǰ X ֵ */
    uint32_t  nyCur;		/* ��ǰ Y ֵ */
    
    D = 3 - (rRadius << 1);
    nxCur = 0;
    nyCur = rRadius;
    
    /* ����Բ���岹��ʽ��Բ */
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
 * @brief   ����ˮƽ���õ�Բ�����Ρ�
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   dWidth ���εĿ��
 * @param   dHeight ���εĸ߶�
 * @param   pColor ������ɫ
 * @param   Round Բ����С
 * @note    
 * @retval  ��
 */
static inline void _GuiDrawRectRound(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor, uint16_t Round)
{
    
    int32_t  D;			/* Decision Variable */
    uint32_t  nxCur;		/* ��ǰ X ֵ */
    uint32_t  nyCur;		/* ��ǰ Y ֵ */
    
    D = 3 - (Round << 1);
    nxCur = 0;
    nyCur = Round;
    
    /* �Ȼ������Σ��ĸ��ǳ��� */
    /* �� */
    _GuiDrawLine(xCur + Round, yCur, xCur + dWidth - Round, yCur, pColor);	
    
    /* �� */
    _GuiDrawLine(xCur + Round, yCur + dHeight - 1, xCur + dWidth - Round, yCur + dHeight - 1, pColor);	
    
    /* �� */
    _GuiDrawLine(xCur, yCur + Round, xCur, yCur + dHeight - Round, pColor);	
    
    /* �� */
    _GuiDrawLine(xCur + dWidth - 1, yCur + Round, xCur + dWidth - 1, yCur + dHeight - Round, pColor);    
        
    /* �����ĸ�Բ�� */
    while (nxCur <= nyCur)
    {
        /* ���Ͻ� */
        _GUI_DeviceAPI.PutPixel(xCur + Round - nxCur, yCur + Round - nyCur, pColor);        
        _GUI_DeviceAPI.PutPixel(xCur + Round - nyCur, yCur + Round - nxCur, pColor);
        /* ���Ͻ� */
        _GUI_DeviceAPI.PutPixel(xCur - Round + dWidth - 1 + nxCur, yCur + Round - nyCur, pColor);
        _GUI_DeviceAPI.PutPixel(xCur - Round + dWidth - 1 + nyCur, yCur + Round - nxCur, pColor);
        
        /* ���½� */
        _GUI_DeviceAPI.PutPixel(xCur + Round - nxCur, yCur - Round + dHeight - 1 + nyCur, pColor);
        _GUI_DeviceAPI.PutPixel(xCur + Round - nyCur, yCur - Round + dHeight - 1 + nxCur, pColor);
        /* ���½� */
        _GUI_DeviceAPI.PutPixel(xCur - Round + dWidth - 1 + nxCur, yCur - Round + dHeight - 1 + nyCur, pColor);
        _GUI_DeviceAPI.PutPixel(xCur - Round + dWidth - 1 + nyCur, yCur - Round + dHeight - 1 + nxCur, pColor);        
        
        /* Բ���岹 */
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
 * @brief   �����Ρ�
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   dWidth ���εĿ��
 * @param   dHeight ���εĸ߶�
 * @param   pColor ������ɫ
 * @note     ---------------->---
            |(xCur��yCur)        |
            V                    V  dHeight
            |                    |
            ---------------->---
                dWidth
 * @retval  ��
 */
static inline void _GuiDrawFillRect(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor)
{
    uint32_t i;
    
    /* ���ô��� */
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, dWidth, dHeight);
    
    /* ѭ����� */
    for (i = 0; i < dWidth * dHeight; i++)
    {
        _GUI_DeviceAPI.PutPixelNoPos(pColor);
    }
}

/**
 * @func    _GuiDrawFillCircle
 * @brief   ���Բ�Ρ�
 * @param   xCur ԭ��x����
 * @param   yCur ԭ��y����
 * @param   rRadius Բ�εİ뾶
 * @param   pColor ������ɫ
 * @note    
 * @retval  ��
 */
static inline void _GuiDrawFillCircle(uint16_t xCur, uint16_t yCur, uint16_t rRadius, uint16_t pColor)
{    
    int32_t  D;			/* Decision Variable */
    uint32_t  nxCur;		/* ��ǰ X ֵ */
    uint32_t  nyCur;		/* ��ǰ Y ֵ */
    
    D = 3 - (rRadius << 1);
    nxCur = 0;
    nyCur = rRadius;
    
    /* Բ���岹�������� */
    while (nxCur <= nyCur)
    {
        /* ����ֱ��������� */
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
 * @brief   ��LCD�����һ��Բ���ľ���
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   dWidth ���εĿ��
 * @param   dHeight ���εĸ߶�
 * @param   pColor ������ɫ
 * @param   Round Բ���Ĵ�С���뾶
 * @note
 * @retval  ��
 */
void _GuiDrawFillRectRound(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor, uint16_t Round)
{    
    int32_t  D;			/* Decision Variable */
    uint32_t  nxCur;		/* ��ǰ X ֵ */
    uint32_t  nyCur;		/* ��ǰ Y ֵ */
    
    D = 3 - (Round << 1);
    nxCur = 0;
    nyCur = Round;
    
    if (dHeight > 2 * Round)
    {
        /* �������һ������ */
        _GuiDrawFillRect(xCur, yCur + Round, dWidth, dHeight - 2 * Round, pColor);
    }
    
    /* �������ಿ�� */
    while (nxCur <= nyCur)
    {
        /* �û������ķ�ʽ����ϰ벿�� */
        _GuiDrawHorLine(xCur + Round - nxCur, yCur + Round - nyCur, dWidth - 2 * Round + 2 * nxCur, pColor);
        _GuiDrawHorLine(xCur + Round - nyCur, yCur + Round - nxCur, dWidth - 2 * Round + 2 * nyCur, pColor);
        
        /* �û������ķ�ʽ����°벿�� */
        _GuiDrawHorLine(xCur + Round - nxCur, yCur - Round + dHeight - 1 + nyCur, dWidth - 2 * Round + 2 * nxCur, pColor);
        _GuiDrawHorLine(xCur + Round - nyCur, yCur - Round + dHeight - 1 + nxCur, dWidth - 2 * Round + 2 * nyCur, pColor);

        /* Բ���岹������Բ����x��y */
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
 * @brief   ��LCD����ʾһ��������
 * @param   xCur �����ε�����x����
 * @param   yCur �����ε�����y����
 * @param   Edges �����α߳�
 * @param   pColor ������ɫ
 * @note
 * @retval  ��
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
 * @brief   ��LCD�������ʾһ��������
 * @param   xCur �����ε�����x����
 * @param   yCur �����ε�����y����
 * @param   Edges �����α߳�
 * @param   pColor ������ɫ
 * @note
 * @retval  ��
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
    
    /* ����������� */
    dxy = (x3 - x1) * 1.0 / Height;  
  
    /* ��ʼ������� */
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
 * @brief   ��LCD����ʾһ��BMPλͼ��λͼ����ɨ����򣺴����ң����ϵ���
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   dWidth ͼƬ���
 * @param   dHeight ͼƬ�߶�
 * @param   *Ptr ͼƬ����ָ��
 * @note
 * @retval  ��
 */
static inline void _GuiDrawBmp(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, const uint16_t *Ptr)
{
    uint32_t Index = 0;
    const uint16_t *p;
  
    /* ����ͼƬ��λ�úʹ�С�� ��������ʾ���� */
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, dWidth, dHeight);
    
    p = Ptr;
    for (Index = 0; Index < dHeight * dWidth; Index++)
    {
        /* ѭ������ */
        _GUI_DeviceAPI.PutPixelNoPos(*(p++));
    }
}























/*****************************************************************************************************
******************************************************************************************************/

/**
 * @func    GuiSetTextFont
 * @brief   ����GUI��ʾ�����ֵĴ�С
 * @param   paCharInfo ������Ϣ�ṹ��
 * @note
 * @retval  ��
 */
void GuiSetTextFont(const paCharsInfo_t* paCharInfo)
{
    /* �ṹ��ֱ�Ӹ��ƣ��˴����������������� */
    _paCharInfo = *paCharInfo;
}

/**
 * @func    GuiSetTextColor
 * @brief   ����GUI��ʾ�����ֵ�ǰ��ɫ�ͱ���ɫ
 * @param   WordColor ����ǰ��ɫ
 * @param   BackColor ���屳��ɫ
 * @note
 * @retval  ��
 */
void GuiSetTextColor(uint16_t WordColor, uint16_t BackColor)
{
    _GUI_TextColor.WordColor = WordColor;
    _GUI_TextColor.BackColor = BackColor;
}

/**
 * @func    GuiSetDeviceAPI
 * @brief   ����GUI��ײ�������API�ӿ�
 * @param   void (* LCDPutPixelNoPos)(uint16_t pColor) ֱ�ӻ��㺯��
 * @param   void (* LCDPutPixel)(uint16_t xCur, uint16_t yCur, uint16_t pColor) ���ݸ������껭�㺯��
 * @param   void (* LCDSetDispCur)(uint16_t xPos, uint16_t yPos) ���ù��
 * @param   void (* LCDSetDispWin)(uint16_t xCur, uint16_t yCur, uint16_t Width, uint16_t Height) ������ʾ����
 * @note
 * @retval  ��
 */
void GuiSetDeviceAPI(void (* PutPixelNoPos)(uint16_t pColor),
                     void (* PutPixel)(uint16_t xCur, uint16_t yCur, uint16_t pColor),
                     void (* SetDispWin)(uint16_t xCur, uint16_t yCur, uint16_t Width, uint16_t Height))
{
    /* ��һ���� */
    _GUI_DeviceAPI.PutPixelNoPos = PutPixelNoPos;
    _GUI_DeviceAPI.PutPixel = PutPixel;
    _GUI_DeviceAPI.SetDispWin = SetDispWin;
}

#if defined USE_ASCII_EXT_LIB || defined USE_CN_EXT_LIB
/**
 * @func    GuiSetFlashReadAPI
 * @brief   ����GUI��ײ�������API�ӿ�
 * @param   void (* ReadData)(uint32_t Address, uint8_t * pDataBuf, uint32_t BufSize) ��Flash�ж�ȡ����
 * @note
 * @retval  ��
 */
void GuiSetFlashReadAPI(void (* ReadData)(uint32_t Address, uint8_t * pDataBuf, uint32_t BufSize))
{
    _GUI_DeviceAPI.ReadData = ReadData;
}
#endif
/**
 * @func    GuiClrScr
 * @brief   ����
 * @param   pColor ��������ɫ
 * @note
 * @retval  ��
 */
void GuiClrScr(uint16_t pColor)
{
    uint32_t n;
    
    /* �������� */
    _GUI_DeviceAPI.SetDispWin(0, 0, LCD_WIDTH, LCD_HEIGHT);
    
    /* �����ܵ����ص� */
    n = LCD_WIDTH * LCD_HEIGHT;
    
    /* ѭ��ˢ�� */
    while (n--)
    {
        _GUI_DeviceAPI.PutPixelNoPos(pColor);
    }
}

/**
 * @func    GuiDrawStringAt
 * @brief   ��ʾһ���ַ������ݣ�������͸��
 * @param   Cn �ַ�������
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @note
 * @retval  ��
 */
void GuiDrawStringAt(const char * Cn, uint16_t xCur, uint16_t yCur)
{
    GUI_CnInfo_t GUI_CnInfo;
    
    /* ������͸����ʶ */
    GUI_CnInfo.TransFlag = CHAR_NO_TRANS;
    
    /* ��ʾ�ַ��� */
    _GuiDrawString(Cn, xCur, yCur, &GUI_CnInfo);
}

/**
 * @func    GuiDrawTranStringAt
 * @brief   ��ʾһ���ַ������ݣ�����͸��
 * @param   Cn �ַ�������
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @note
 * @retval  ��
 */
void GuiDrawTranStringAt(const char * Cn, uint16_t xCur, uint16_t yCur)
{    
    GUI_CnInfo_t GUI_CnInfo;
    
    /* ����͸����ʶ */
    GUI_CnInfo.TransFlag = CHAR_TRANS;
    
    /* ��ʾ�ַ��� */
    _GuiDrawString(Cn, xCur, yCur, &GUI_CnInfo);
}

/**
 * @func    GuiDrawNumberAt
 * @brief   ��ʾһ�����֣�������͸��
 * @param   Number Ҫʵ�ֵ�����
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   PointNum ��������λС����
 * @note
 * @retval  ��
 */
uint8_t GuiDrawNumberAt(double Number, uint16_t xCur, uint16_t yCur, uint8_t PointNum)
{
    uint8_t i = 0;
    char Buf[100] = {'\0'};
    char ConsBuf[10] = {'\0'};
    
    /* PointNum����0��˵����Ҫ��ʾС�������� */
    if (PointNum > 0)
    {
        /* �����ַ��� */
        my_sprintf(ConsBuf, "%%.%df", PointNum);
        my_sprintf(Buf, ConsBuf, Number);
    }
    else
    {
        /* �����ַ��� */
        my_sprintf(Buf, "%d", (int32_t)Number);
    }
    
    /* ��ʾ�ַ��� */
    GuiDrawStringAt(Buf, xCur, yCur);
    
    while (Buf[i++] != '\0');
    return --i;
}

/**
 * @func    GuiDrawTranNumberAt
 * @brief   ��ʾһ�����֣�����͸��
 * @param   Number Ҫʵ�ֵ�����
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   PointNum ��������λС����
 * @note
 * @retval  ��
 */
uint8_t GuiDrawTranNumberAt(double Number, uint16_t xCur, uint16_t yCur, uint8_t PointNum)
{
    uint8_t i;
    char Buf[100] = {'\0'};
    char ConsBuf[10] = {'\0'};
    
    /* PointNum����0��˵����Ҫ��ʾС�������� */
    if (PointNum > 0)
    {
        /* �����ַ��� */
        my_sprintf(ConsBuf, "%%.%df", PointNum);
        my_sprintf(Buf, ConsBuf, Number);
    }
    else
    {
        /* �����ַ��� */
        my_sprintf(Buf, "%d", (int32_t)Number);
    }
    
    /* ��ʾ�ַ��� */
    GuiDrawTranStringAt(Buf, xCur, yCur);
    
    while (Buf[i++] != '\0');
    return --i;
}

/**
 * @func    GuiDrawText
 * @brief   ��ʾ���֣�������͸������Ҫָ�����뷽ʽ
 * @param   Cn ��ʾ���ַ���
 * @param   xStart ��ʼ��x����
 * @param   yStart ��ʼ��y����
 * @param   xEnd �յ��x����
 * @param   yEnd �յ��y����
 * @param   Align ���뷽ʽ
 * @note
 * @retval  ��
 */
void GuiDrawText(const char * Cn, uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t Align)
{
    uint16_t nLen = 0;
    
    /* �����ַ������� */
    while (*(Cn + nLen) != '\0')
    {
        nLen++;
    }
    
    /* û��ָ�����뷽ʽ�����������ʾ */
    if (Align == 0)
    {
        /* ��ʾ�ַ��� */
        GuiDrawStringAt(Cn, xStart, yStart);
    }
    
    if ((Align & LEFT_ALIGN) && (Align & CENTER_ALIGN))
    {
        /* ��ʾ�ַ��� */
        GuiDrawStringAt(Cn, xStart,
                            yStart + ((yEnd - yStart) - _paCharInfo.paAsciiInfo.Hight) / 2);
    }
    else if ((Align & RIGHT_ALIGN) && (Align & CENTER_ALIGN))
    {
        /* ��ʾ�ַ��� */
        GuiDrawStringAt(Cn, xEnd - _paCharInfo.paAsciiInfo.Width * nLen,
                            yStart + ((yEnd - yStart) - _paCharInfo.paAsciiInfo.Hight) / 2);
    }
    else if ((Align & TOP_ALIGN) && (Align & CENTER_ALIGN))
    {
        /* ��ʾ�ַ��� */
        GuiDrawStringAt(Cn, xStart + ((xEnd - xStart) - _paCharInfo.paAsciiInfo.Width * nLen) / 2,
                            yStart);
    }
    else if ((Align & BOTTOM_ALIGN) && (Align & CENTER_ALIGN))
    {
        /* ��ʾ�ַ��� */
        GuiDrawStringAt(Cn, xStart + ((xEnd - xStart) - _paCharInfo.paAsciiInfo.Width * nLen) / 2,
                            yEnd - _paCharInfo.paAsciiInfo.Hight);
    }
    else if (Align & CENTER_ALIGN)
    {        /* ��ʾ�ַ��� */
        GuiDrawStringAt(Cn, xStart + ((xEnd - xStart) - _paCharInfo.paAsciiInfo.Width * nLen) / 2,
                            yStart + ((yEnd - yStart) - _paCharInfo.paAsciiInfo.Hight) / 2);
    }
    else if (Align & LEFT_ALIGN)
    {
        /* ��ʾ�ַ��� */
        GuiDrawStringAt(Cn, xStart, yStart);
    }
    else if (Align & RIGHT_ALIGN)
    {
        /* ��ʾ�ַ��� */
        GuiDrawStringAt(Cn, xEnd - _paCharInfo.paAsciiInfo.Width * nLen, yStart);
    }
    else if (Align & TOP_ALIGN)
    {
        /* ��ʾ�ַ��� */
        GuiDrawStringAt(Cn, xStart, yStart);
    }
    else if (Align & BOTTOM_ALIGN)
    {
        /* ��ʾ�ַ��� */
        GuiDrawStringAt(Cn, xStart, yEnd - _paCharInfo.paAsciiInfo.Hight);
    }
}

/**
 * @func    GuiDrawTranText
 * @brief   ��ʾ���֣�����͸������Ҫָ�����뷽ʽ
 * @param   Cn ��ʾ���ַ���
 * @param   xStart ��ʼ��x����
 * @param   yStart ��ʼ��y����
 * @param   xEnd �յ��x����
 * @param   yEnd �յ��y����
 * @param   Align ���뷽ʽ
 * @note
 * @retval  ��
 */
void GuiDrawTranText(const char * Cn, uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t Align)
{
    uint16_t nLen = 0;
    
    /* �����ַ��ĸ��� */
    while (*(Cn + nLen) != '\0')
    {
        nLen++;
    }
    
    /* û��ָ�����뷽ʽ�����������ʾ */
    if (Align == 0)
    {
        /* ��ʾ�ַ��� */
        GuiDrawStringAt(Cn, xStart, yStart);
    }
    
    if ((Align & LEFT_ALIGN) && (Align & CENTER_ALIGN))
    {
        /* ��ʾ�ַ��� */
        GuiDrawTranStringAt(Cn, xStart,
                                yStart + ((yEnd - yStart) - _paCharInfo.paAsciiInfo.Hight) / 2);
    }
    else if ((Align & RIGHT_ALIGN) && (Align & CENTER_ALIGN))
    {
        /* ��ʾ�ַ��� */
        GuiDrawTranStringAt(Cn, xEnd - _paCharInfo.paAsciiInfo.Width * nLen,
                                yStart + ((yEnd - yStart) - _paCharInfo.paAsciiInfo.Hight) / 2);
    }
    else if ((Align & TOP_ALIGN) && (Align & CENTER_ALIGN))
    {
        /* ��ʾ�ַ��� */
        GuiDrawTranStringAt(Cn, xStart + ((xEnd - xStart) - _paCharInfo.paAsciiInfo.Width * nLen) / 2,
                                yStart);
    }
    else if ((Align & BOTTOM_ALIGN) && (Align & CENTER_ALIGN))
    {
        /* ��ʾ�ַ��� */
        GuiDrawTranStringAt(Cn, xStart + ((xEnd - xStart) - _paCharInfo.paAsciiInfo.Width * nLen) / 2,
                                yEnd - _paCharInfo.paAsciiInfo.Hight);
    }
    else if ((Align & TOP_ALIGN) && (Align & LEFT_ALIGN))
    {
        /* ��ʾ�ַ��� */
        GuiDrawTranStringAt(Cn, xStart,
                                yStart);        
    }
    else if ((Align & TOP_ALIGN) && (Align & RIGHT_ALIGN))
    {
        /* ��ʾ�ַ��� */
        GuiDrawTranStringAt(Cn, xEnd - (_paCharInfo.paAsciiInfo.Width * nLen),
                                yStart);        
    }
    else if ((Align & BOTTOM_ALIGN) && (Align & LEFT_ALIGN))
    {
        /* ��ʾ�ַ��� */
        GuiDrawTranStringAt(Cn, xStart,
                                yEnd - _paCharInfo.paAsciiInfo.Hight);        
    }
    else if ((Align & BOTTOM_ALIGN) && (Align & RIGHT_ALIGN))
    {
        /* ��ʾ�ַ��� */
        GuiDrawTranStringAt(Cn, xEnd - (_paCharInfo.paAsciiInfo.Width * nLen),
                                yEnd - _paCharInfo.paAsciiInfo.Hight);        
    }
    else if (Align & CENTER_ALIGN)
    {        
        /* ��ʾ�ַ��� */
        GuiDrawTranStringAt(Cn, xStart + ((xEnd - xStart) - _paCharInfo.paAsciiInfo.Width * nLen) / 2,
                                yStart + ((yEnd - yStart) - _paCharInfo.paAsciiInfo.Hight) / 2);
    }
    else if (Align & LEFT_ALIGN)
    {
        /* ��ʾ�ַ��� */
        GuiDrawTranStringAt(Cn, xStart, yStart);
    }
    else if (Align & RIGHT_ALIGN)
    {
        /* ��ʾ�ַ��� */
        GuiDrawTranStringAt(Cn, xEnd - _paCharInfo.paAsciiInfo.Width * nLen, yStart);
    }
    else if (Align & TOP_ALIGN)
    {
        /* ��ʾ�ַ��� */
        GuiDrawTranStringAt(Cn, xStart, yStart);
    }
    else if (Align & BOTTOM_ALIGN)
    {
        /* ��ʾ�ַ��� */
        GuiDrawTranStringAt(Cn, xStart, yEnd - _paCharInfo.paAsciiInfo.Hight);
    }
}

/**
 * @func    GuiDrawHorGradientColorBar
 * @brief   ����һ��ˮƽ����Ľ���ɫ��
 * @param   xCur ��ʼx����
 * @param   yCur ��ʼy����
 * @param   Width ɫ�����
 * @param   Height ɫ���߶�
 * @param   FirstColor ��һ����ɫ
 * @param   SecondColor �ڶ�����ɫ
 * @note
 * @retval  ��
 */
void GuiDrawHorGradientColorBar(uint16_t xCur, uint16_t yCur,
                                uint16_t Width, uint16_t Height,
                                uint16_t FirstColor, uint16_t SecondColor)
{
    uint16_t DataBuf[LCD_WIDTH] = {'\0'};
    uint16_t i, Color;
    
    /* ��ȫ��⣬��ֹ����Խ��Ӷ�����Ӳ������ */
    if (Width > LCD_WIDTH)
    {
        Width = LCD_WIDTH;
    }   
    
    /* ��ȡ��ɫ���� */
    _GuiGetColorBarData(Width, FirstColor, SecondColor, DataBuf);
    
    /* ����ɫ�������ݵ���ʱ���� */
    my_memcpy(_GUI_BarColorItems, DataBuf, Width * 2);
    
    /* �������� */
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, Width, Height);								
    
    /* ѭ������ */
    for (i = 0; i < Height * Width; i++)
    {
        Color = DataBuf[i % Width];
        _GUI_DeviceAPI.PutPixelNoPos(Color);
    }	
}

/**
 * @func    GuiDrawHorRadiationColorBar
 * @brief   ����һ��ˮƽ������м����ɫ��
 * @param   xCur ��ʼx����
 * @param   yCur ��ʼy����
 * @param   Width ɫ�����
 * @param   Height ɫ���߶�
 * @param   FirstColor ��һ����ɫ
 * @param   SecondColor �ڶ�����ɫ
 * @note
 * @retval  ��
 */
void GuiDrawHorRadiationColorBar(uint16_t xCur, uint16_t yCur,
                                uint16_t Width, uint16_t Height,
                                uint16_t FirstColor, uint16_t MiddleColor)
{
    uint16_t DataBuf[LCD_WIDTH] = {'\0'};
    uint16_t i, Color;
    
    /* ��ȫ��⣬��ֹ����Խ��Ӷ�����Ӳ������ */
    if (Width > LCD_WIDTH)
    {
        Width = LCD_WIDTH;
    }    
    
    /* �ж��Ƿ�Ϊ2���� */
    if (!(Width % 2))
    {
        /* ��ȡ�������� */
        _GuiGetColorBarData(Width / 2, FirstColor, MiddleColor, DataBuf);
        
        /* �����ص���е��� */
        _ColorInverted(DataBuf + Width / 2, DataBuf, Width / 2);
    }
    else
    {
        /* ��ȡ�������� */
        _GuiGetColorBarData(Width / 2, FirstColor, MiddleColor, DataBuf);
        
        /* ���м�ĵ�����ֶ������ɫ */
        DataBuf[Width / 2]  = MiddleColor;
        
        /* �����ص���е��� */
        _ColorInverted(DataBuf + Width / 2 + 1, DataBuf, Width / 2);
    }
    
    /* ����ɫ�������ݵ���ʱ���� */
    my_memcpy(_GUI_BarColorItems, DataBuf, Width * 2);
    
    /* �������� */
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, Width, Height);								
    
    /* ѭ������ */
    for (i = 0; i < Height * Width; i++)
    {
        Color = DataBuf[i % Width];
        _GUI_DeviceAPI.PutPixelNoPos(Color);
    } 
}

/**
 * @func    GuiDrawHorThreeGradientColorBar
 * @brief   ����һ��ˮƽ�������ɫ�����ɫ��
 * @param   xCur ��ʼx����
 * @param   yCur ��ʼy����
 * @param   Width ɫ�����
 * @param   Height ɫ���߶�
 * @param   FirstColor ��һ����ɫ
 * @param   SecondColor �ڶ�����ɫ
 * @param   ThreeColor ��������ɫ
 * @note
 * @retval  ��
 */
void GuiDrawHorThreeGradientColorBar(uint16_t xCur, uint16_t yCur,
                                    uint16_t Width, uint16_t Height,
                                    uint16_t FirstColor, uint16_t SecondColor,
                                    uint16_t ThreeColor)
{
    uint16_t DataBuf[LCD_WIDTH] = {'\0'};
    uint16_t i, Color;
    
    /* ��ȫ��⣬��ֹ����Խ��Ӷ�����Ӳ������ */
    if (Width > LCD_WIDTH)
    {
        Width = LCD_WIDTH;
    }
    
    if (!(Width % 2))
    {
        /* ��ȡ��һ�ڶ�����ɫ�����ص����� */
        _GuiGetColorBarData(Width / 2, FirstColor, SecondColor, DataBuf);
        
        /* ��ȡ�ڶ���������ɫ�����ص����� */
        _GuiGetColorBarData(Width / 2, SecondColor, ThreeColor, DataBuf + Width / 2);
    }
    else
    {
        /* ��ȡ��һ�ڶ�����ɫ�����ص����� */
        _GuiGetColorBarData(Width / 2, FirstColor, SecondColor, DataBuf);
        
        /* �����м�ĵ�����ֶ���� */
        DataBuf[Width / 2]  = SecondColor;
        
        /* ��ȡ�ڶ���������ɫ�����ص����� */
        _GuiGetColorBarData(Width / 2, SecondColor, ThreeColor, DataBuf + Width / 2 + 1);
    }
    
    /* ����ɫ�������ݵ���ʱ���� */
    my_memcpy(_GUI_BarColorItems, DataBuf, Width * 2);
    
    /* �������� */
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, Width, Height);								
    
    /* ѭ������ */
    for (i = 0; i < Height * Width; i++)
    {
        Color = DataBuf[i % Width];
        _GUI_DeviceAPI.PutPixelNoPos(Color);
    }  
}

/**
 * @func    GuiDrawVerGradientColorBar
 * @brief   ����һ����ֱ����Ľ���ɫ��
 * @param   xCur ��ʼx����
 * @param   yCur ��ʼy����
 * @param   Width ɫ�����
 * @param   Height ɫ���߶�
 * @param   FirstColor ��һ����ɫ
 * @param   SecondColor �ڶ�����ɫ
 * @note
 * @retval  ��
 */
void GuiDrawVerGradientColorBar(uint16_t xCur, uint16_t yCur,
                                uint16_t Width, uint16_t Height,
                                uint16_t FirstColor, uint16_t SecondColor)
{
    uint16_t DataBuf[LCD_HEIGHT] = {'\0'};
    uint16_t i, Color;
    
    /* ��ȫ��� */
    if (Height > LCD_HEIGHT)
    {
        Height = LCD_HEIGHT;
    }
    
    _GuiGetColorBarData(Height, FirstColor, SecondColor, DataBuf);
    
    /* ����ɫ�������ݵ���ʱ���� */
    my_memcpy(_GUI_BarColorItems, DataBuf, Height * 2);
    
    /* �������� */
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, Width, Height);								
    
    for (i = 0; i < Height * Width; i++)
    {
        Color = DataBuf[i / Width];
        _GUI_DeviceAPI.PutPixelNoPos(Color);
    }	
}

/**
 * @func    GuiDrawVerRadiationColorBar
 * @brief   ����һ����ֱ������м����ɫ��
 * @param   xCur ��ʼx����
 * @param   yCur ��ʼy����
 * @param   Width ɫ�����
 * @param   Height ɫ���߶�
 * @param   FirstColor ��һ����ɫ
 * @param   SecondColor �ڶ�����ɫ
 * @note
 * @retval  ��
 */
void GuiDrawVerRadiationColorBar(uint16_t xCur, uint16_t yCur,
                                uint16_t Width, uint16_t Height,
                                uint16_t FirstColor, uint16_t MiddleColor)
{    
    uint16_t DataBuf[LCD_HEIGHT] = {'\0'};
    uint16_t i, Color;
    
    /* ��ȫ��� */
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
    
    /* ����ɫ�������ݵ���ʱ���� */
    my_memcpy(_GUI_BarColorItems, DataBuf, Height * 2);
    
    /* �������� */
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, Width, Height);								
    
    for (i = 0; i < Height * Width; i++)
    {
        Color = DataBuf[i / Width];
        _GUI_DeviceAPI.PutPixelNoPos(Color);
    } 
}

/**
 * @func    GuiDrawHorThreeGradientColorBar
 * @brief   ����һ����ֱ�������ɫ�����ɫ��
 * @param   xCur ��ʼx����
 * @param   yCur ��ʼy����
 * @param   Width ɫ�����
 * @param   Height ɫ���߶�
 * @param   FirstColor ��һ����ɫ
 * @param   SecondColor �ڶ�����ɫ
 * @param   ThreeColor ��������ɫ
 * @note
 * @retval  ��
 */
void GuiDrawVerThreeGradientColorBar(uint16_t xCur, uint16_t yCur,
                                    uint16_t Width, uint16_t Height,
                                    uint16_t FirstColor, uint16_t SecondColor,
                                    uint16_t ThreeColor)
{
    uint16_t DataBuf[LCD_HEIGHT] = {'\0'};
    uint16_t i, Color;
    
    /* ��ȫ��� */
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
    
    /* ����ɫ�������ݵ���ʱ���� */
    my_memcpy(_GUI_BarColorItems, DataBuf, Height * 2);
    
    /* �������� */
    _GUI_DeviceAPI.SetDispWin(xCur, yCur, Width, Height);								
    
    for (i = 0; i < Height * Width; i++)
    {
        Color = DataBuf[i / Width];
        _GUI_DeviceAPI.PutPixelNoPos(Color);
    } 
}

/**
 * @func    GuiGetBarColorItems
 * @brief   ��ȡ������ƵĲ�ɫɫ������ɫ����
 * @param   *Buf ���뻺��
 * @param   nLen ����ĳ���
 * @note
 * @retval  ��
 */
void GuiGetBarColorItems(uint16_t *Buf, uint16_t nLen)
{
#if (LCD_HEIGHT > LCD_WIDTH)
    /* ��ȫ��� */
    if (nLen > LCD_HEIGHT)
    {
        nLen = LCD_HEIGHT;
    }
#elif (LCD_HEIGHT < LCD_WIDTH)
    /* ��ȫ��� */
    if (nLen > LCD_WIDTH)
    {
        nLen = LCD_WIDTH;
    }
#endif
    /* ����ɫ�������ݵ�������� */
    my_memcpy(Buf, _GUI_BarColorItems, nLen * 2);
}

/**
 * @func    GuiDrawLine
 * @brief   ���� Bresenham �㷨����2��仭һ��ֱ�ߡ�
 * @param   xStart ��ʼ��x����
 * @param   yStart ��ʼ��y����
 * @param   xEnd ��ֹ��x����
 * @param   yEnd ��ֹ��y����
 * @param   pColor ������ɫ
 * @note
 * @retval  ��
 */
void GuiDrawLine(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t pColor)
{
    _GuiDrawLine(xStart, yStart, xEnd, yEnd, pColor);
}

/**
 * @func    GuiDrawHorLine
 * @brief   ����һ���߶�Ϊ1��ˮƽֱ��
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   Lenght ֱ�ߵĳ���
 * @param   pColor ������ɫ
 * @note
 * @retval  ��
 */
void GuiDrawHorLine(uint16_t xCur, uint16_t yCur, uint16_t Lenght, uint16_t pColor)
{
    if (Lenght == 0)
        return ;
    _GuiDrawHorLine(xCur, yCur, Lenght, pColor);
}

/**
 * @func    GuiDrawVerLine
 * @brief   ����һ�����Ϊ1�Ĵ�ֱֱ��
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   Lenght ֱ�ߵĳ���
 * @param   pColor ������ɫ
 * @note
 * @retval  ��
 */
void GuiDrawVerLine(uint16_t xCur, uint16_t yCur, uint16_t Lenght, uint16_t pColor)
{
    if (Lenght == 0)
        return ;
    _GuiDrawVerLine(xCur, yCur, Lenght, pColor);
}

/**
 * @func    GuiDrawHorColorLine
 * @brief   ����һ����ɫˮƽ��
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   dWidth ֱ�ߵĿ��
 * @param   *pColor ��ɫ������
 * @note
 * @retval  ��
 */
void GuiDrawHorColorLine(uint16_t xCur, uint16_t yCur, uint16_t dWidth, const uint16_t *pColor)
{
    if (dWidth == 0)
        return ;
    _GuiDrawHorColorLine(xCur, yCur, dWidth, pColor);
}

/**
 * @func    GuiDrawVerLine
 * @brief   ����һ�����Ϊ1������ֱ��
 * @param   *x x��ļ���
 * @param   *y y��ļ���
 * @param   Size ��ļ��ϵĴ�С
 * @param   pColor ������ɫ
 * @note
 * @retval  ��
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
 * @brief   ����һ��Բ���ʿ�Ϊ1������
 * @param   xCur Բ��x����
 * @param   yCur Բ��y����
 * @param   rRadius Բ�İ뾶
 * @param   pColor ������ɫ
 * @note
 * @retval  ��
 */
void GuiDrawCircle(uint16_t xCur, uint16_t yCur, uint16_t rRadius, uint16_t pColor)
{
	_GuiDrawCircle(xCur, yCur, rRadius, pColor);
}

/**
 * @func    _GuiDrawRect
 * @brief   ����ˮƽ���õľ��Ρ�
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   dWidth ���εĿ��
 * @param   dHeight ���εĸ߶�
 * @param   pColor ������ɫ
 * @note    ---------------->---
           |(xCur��yCur)        |
           V                    V  dHeight
           |                    |
           ---------------->---
                dWidth
 * @retval  ��
 */
void GuiDrawRect(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor)
{
    if (dWidth == 0 || dHeight == 0)
        return ;
    _GuiDrawRect(xCur, yCur, dWidth, dHeight, pColor);
}

/**
 * @func    _GuiDrawFillCircle
 * @brief   ���Բ�Ρ�
 * @param   xCur ԭ��x����
 * @param   yCur ԭ��y����
 * @param   rRadius Բ�εİ뾶
 * @param   pColor ������ɫ
 * @note    
 * @retval  ��
 */
void GuiDrawFillCircle(uint16_t xCur, uint16_t yCur, uint16_t rRadius, uint16_t pColor)
{
    _GuiDrawFillCircle(xCur, yCur, rRadius, pColor);
}

/**
 * @func    _GuiDrawRectRound
 * @brief   ����ˮƽ���õ�Բ�����Ρ�
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   dWidth ���εĿ��
 * @param   dHeight ���εĸ߶�
 * @param   pColor ������ɫ
 * @param   Round Բ����С
 * @note    
 * @retval  ��
 */
void GuiDrawRectRound(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor, uint16_t Round)
{
    if (dWidth == 0 || dHeight == 0)
        return ;
    _GuiDrawRectRound(xCur, yCur, dWidth, dHeight, pColor, Round);
}

/**
 * @func    _GuiDrawFillRect
 * @brief   �����Ρ�
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   dWidth ���εĿ��
 * @param   dHeight ���εĸ߶�
 * @param   pColor ������ɫ
 * @note     ---------------->---
            |(xCur��yCur)        |
            V                    V  dHeight
            |                    |
            ---------------->---
                dWidth
 * @retval  ��
 */
void GuiDrawFillRect(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor)
{
    if (dWidth == 0 || dHeight == 0)
        return ;
	_GuiDrawFillRect(xCur, yCur, dWidth, dHeight, pColor);
}

/**
 * @func    GuiDrawFillRectRound
 * @brief   ��LCD�����һ��Բ���ľ���
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   dWidth ���εĿ��
 * @param   dHeight ���εĸ߶�
 * @param   pColor ������ɫ
 * @param   Round Բ���Ĵ�С���뾶
 * @note
 * @retval  ��
 */
void GuiDrawFillRectRound(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, uint16_t pColor, uint16_t Round)
{
    if (dWidth == 0 || dHeight == 0)
        return ;
    _GuiDrawFillRectRound(xCur, yCur, dWidth, dHeight, pColor, Round);
}

/**
 * @func    GuiDrawTopTriangle
 * @brief   ��LCD����ʾһ��������
 * @param   xCur �����ε�����x����
 * @param   yCur �����ε�����y����
 * @param   Edges �����α߳�
 * @param   pColor ������ɫ
 * @note
 * @retval  ��
 */
void GuiDrawTopTriangle(uint16_t xCur, uint16_t yCur, uint16_t Edges, uint16_t pColor)
{
    _GuiDrawTopTriangle(xCur, yCur, Edges, pColor);
}

/**
 * @func    GuiDrawFillTopTriangle
 * @brief   ��LCD�������ʾһ��������
 * @param   xCur �����ε�����x����
 * @param   yCur �����ε�����y����
 * @param   Edges �����α߳�
 * @param   pColor ������ɫ
 * @note
 * @retval  ��
 */
void GuiDrawFillTopTriangle(uint16_t xCur, uint16_t yCur, uint16_t Edges, uint16_t pColor)  
{  
    _GuiDrawFillTopTriangle(xCur, yCur, Edges, pColor);
}

/**
 * @func    GuiDrawBMP
 * @brief   ��LCD����ʾһ��BMPλͼ��λͼ����ɨ����򣺴����ң����ϵ���
 * @param   xCur ��ʼ��x����
 * @param   yCur ��ʼ��y����
 * @param   dWidth ͼƬ���
 * @param   dHeight ͼƬ�߶�
 * @param   *Ptr ͼƬ����ָ��
 * @note
 * @retval  ��
 */
void GuiDrawBMP(uint16_t xCur, uint16_t yCur, uint16_t dWidth, uint16_t dHeight, const uint16_t *Ptr)
{
	_GuiDrawBmp(xCur, yCur, dWidth, dHeight, Ptr);
}

/**
 * @func    GuiGetWarmColdColor
 * @brief   �����������ɫ����ȡ����ɫ����ů��ɫ
 * @param   SrcColor �������ɫ
 * @param   * WarmColor ���ů��ɫ
 * @param   * ColdColor �������ɫ
 * @note    ���������̾Ͳ������ˣ�����ԭ����Ǻ�ɫ����ɫ�ı仯
 * @retval  ��
 */
void GuiGetWarmColdColor(uint16_t SrcColor, uint16_t * WarmColor, uint16_t * ColdColor)
{
    GUI_RGB565Color_t SrcColorData, WarmColorData, ColdColorData;
    
    /* ���ýṹ��͹���������ɫ */
    SrcColorData.Color = SrcColor;
    
    /* ůɫ���� */
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

    /*��ɫ���� */
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
    
    /* �����ɫ */
    *WarmColor = WarmColorData.Color;
    *ColdColor = ColdColorData.Color;
}


