/**
 ******************************************************************************
 * @Copyright     (C) 2017 - 2018 guet-sctc-hardwarepart Team
 * @filename      font.h
 * @author        ZSY
 * @version       V1.0.1
 * @date          2018-09-15
 * @Description   �ֿ����ݵ�һЩ�ṹ�嶨�壬ʵ�������ֵı�������ʾ
 * @Others
 * @History
 * Date           Author    version    		Notes
 * 2018-06-22      ZSY      V1.0.0      first version.
 * 2018-09-15      ZSY      V1.0.1          ���Ĳ��ֽṹ��������.
 */
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _FONT_H_
#define _FONT_H_

/* �����Ӧ�ĺ궨����ʾ��Ӧ��С������ */
#define USING_CN_16_CHAR
#define USING_CN_24_CHAR
//#define USING_CN_32_CHAR
//#define USING_CN_40_CHAR
//#define USING_CN_48_CHAR

/* ѡ��ʹ���ڲ����ֿ⻹���ⲿ�ֿ� */
#define USE_CN_INT_LIB
#define USE_ASCII_INT_LIB

// #define USE_CN_EXT_LIB
// #define USE_ASCII_EXT_LIB

/* �ַ�����Ϣ */
typedef struct
{
    unsigned char Width;
    unsigned char Hight;
    unsigned char PerLineBytes;
    char * Char;
}paCnInfo_t;

/* ������Ϣ */
typedef struct
{
    paCnInfo_t paAsciiInfo;
    paCnInfo_t paHanziInfo;
}paCharsInfo_t;

/* ������������,һ������ռ�����ֽ� */
typedef struct
{
	unsigned char  Index[2];	
}CnChar_t;

#ifdef USING_CN_16_CHAR
/* ������ģ���ݽṹ  */
typedef struct  _Cn16Data           
{
	unsigned char  Msk[32];         // ����������(16*16/8) 
}Cn16Data_t;
#endif /* USING_CN_16_CHAR */

#ifdef USING_CN_24_CHAR
/* ������ģ���ݽṹ  */
typedef struct  _Cn24Char           
{
	unsigned char  Msk[72];         // ����������(24*24/8) 
}Cn24Data_t;
#endif /* USING_CN_24_CHAR */

#ifdef USING_CN_32_CHAR
/* ������ģ���ݽṹ  */
typedef struct  _Cn32Char           
{
	unsigned char  Msk[128];        // ����������(32*32/8) 
}Cn32Data_t;
#endif /* USING_CN_32_CHAR */

#ifdef USING_CN_40_CHAR
/* ������ģ���ݽṹ  */
typedef struct  _Cn40Char           
{
	unsigned char  Msk[200];        // ����������(40*40/8) 
}Cn40Data_t;
#endif /* USING_CN_40_CHAR */

#ifdef USING_CN_48_CHAR
/* ������ģ���ݽṹ  */
typedef struct  _Cn48Char           
{
	unsigned char  Msk[288];        // ����������(48*48/8) 
}Cn48Data_t;
#endif /* USING_CN_48_CHAR */

#endif /* _FONT_H_ */


