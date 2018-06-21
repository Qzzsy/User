#ifndef _FONT_H_
#define _FONT_H_

/* �����Ӧ�ĺ궨����ʾ��Ӧ��С������ */
#define USING_CN_16_CHAR
#define USING_CN_24_CHAR
#define USING_CN_32_CHAR
#define USING_CN_40_CHAR
#define USING_CN_48_CHAR

/* Ӣ���ַ���Ϣ */
typedef struct _paAsciiData
{
    unsigned char Width;
    unsigned char Hight;
    unsigned char PerLinePixels;
    char * ASCII;
}paAsciiDataTypedef;

/* �����ַ�����Ϣ */
typedef struct _paHanziData
{
    unsigned char Width;
    unsigned char Hight;
    unsigned char PerLinePixels;
    char * Hanzi;
}paHanziDataTypedef;

/* ������Ϣ */
typedef struct _paCharInfo
{
    paAsciiDataTypedef paAsciiData;
    paHanziDataTypedef paHanziData;
}paCharInfoTypedef;

/* ������������,һ������ռ�����ֽ� */
typedef struct _CnChar
{
	unsigned char  Index[2];	
}CnCharTypedef;

#ifdef USING_CN_16_CHAR
/* ������ģ���ݽṹ  */
typedef struct  _Cn16Data           
{
	unsigned char  Msk[32];         // ����������(16*16/8) 
}Cn16DataTypeDef;
#endif /* USING_CN_16_CHAR */

#ifdef USING_CN_24_CHAR
/* ������ģ���ݽṹ  */
typedef struct  _Cn24Char           
{
	unsigned char  Msk[72];         // ����������(24*24/8) 
}Cn24DataTypeDef;
#endif /* USING_CN_24_CHAR */

#ifdef USING_CN_32_CHAR
/* ������ģ���ݽṹ  */
typedef struct  _Cn32Char           
{
	unsigned char  Msk[128];        // ����������(32*32/8) 
}Cn32DataTypeDef;
#endif /* USING_CN_32_CHAR */

#ifdef USING_CN_40_CHAR
/* ������ģ���ݽṹ  */
typedef struct  _Cn40Char           
{
	unsigned char  Msk[200];        // ����������(40*40/8) 
}Cn40DataTypeDef;
#endif /* USING_CN_40_CHAR */

#ifdef USING_CN_48_CHAR
/* ������ģ���ݽṹ  */
typedef struct  _Cn48Char           
{
	unsigned char  Msk[288];        // ����������(48*48/8) 
}Cn48DataTypeDef;
#endif /* USING_CN_48_CHAR */

#endif /* _FONT_H_ */


