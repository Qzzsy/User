#ifndef _FONT_H_
#define _FONT_H_

/* 定义对应的宏定义显示对应大小的字体 */
#define USING_CN_16_CHAR
#define USING_CN_24_CHAR
#define USING_CN_32_CHAR
#define USING_CN_40_CHAR
#define USING_CN_48_CHAR

/* 英文字符信息 */
typedef struct _paAsciiData
{
    unsigned char Width;
    unsigned char Hight;
    unsigned char PerLinePixels;
    char * ASCII;
}paAsciiDataTypedef;

/* 中文字符的信息 */
typedef struct _paHanziData
{
    unsigned char Width;
    unsigned char Hight;
    unsigned char PerLinePixels;
    char * Hanzi;
}paHanziDataTypedef;

/* 字体信息 */
typedef struct _paCharInfo
{
    paAsciiDataTypedef paAsciiData;
    paHanziDataTypedef paHanziData;
}paCharInfoTypedef;

/* 汉字内码索引,一个汉字占两个字节 */
typedef struct _CnChar
{
	unsigned char  Index[2];	
}CnCharTypedef;

#ifdef USING_CN_16_CHAR
/* 汉字字模数据结构  */
typedef struct  _Cn16Data           
{
	unsigned char  Msk[32];         // 点阵码数据(16*16/8) 
}Cn16DataTypeDef;
#endif /* USING_CN_16_CHAR */

#ifdef USING_CN_24_CHAR
/* 汉字字模数据结构  */
typedef struct  _Cn24Char           
{
	unsigned char  Msk[72];         // 点阵码数据(24*24/8) 
}Cn24DataTypeDef;
#endif /* USING_CN_24_CHAR */

#ifdef USING_CN_32_CHAR
/* 汉字字模数据结构  */
typedef struct  _Cn32Char           
{
	unsigned char  Msk[128];        // 点阵码数据(32*32/8) 
}Cn32DataTypeDef;
#endif /* USING_CN_32_CHAR */

#ifdef USING_CN_40_CHAR
/* 汉字字模数据结构  */
typedef struct  _Cn40Char           
{
	unsigned char  Msk[200];        // 点阵码数据(40*40/8) 
}Cn40DataTypeDef;
#endif /* USING_CN_40_CHAR */

#ifdef USING_CN_48_CHAR
/* 汉字字模数据结构  */
typedef struct  _Cn48Char           
{
	unsigned char  Msk[288];        // 点阵码数据(48*48/8) 
}Cn48DataTypeDef;
#endif /* USING_CN_48_CHAR */

#endif /* _FONT_H_ */


