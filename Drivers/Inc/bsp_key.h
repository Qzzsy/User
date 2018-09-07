/**
 ******************************************************************************
 * @file      bsp_key.h
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-08-31
 * @brief     ���ļ�ʵ���˰����Ĵ�������֧�ְ����ĳ��̰�����˫����������ֵ������һ��
 *            ���ڣ�����Ҫ��Ϊ������ϵͳ��ʹ�õ�ʱ����Ҫע��ɨ��ʱ����ж�ʱ��ĳ�ͻ
 * @History
 * Date           Author    version    		Notes
 * 2018-08-31       ZSY     V1.0.0      first version.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_KEY_H_
#define __BSP_KEY_H_

#define KEY_USE_LONGPRESS					/*!< ����ʹ�ܳ��� */
#define KEY_USE_DOUBLE_CLICK				/*!< ����ʹ��˫�� */

#define KEY_NUM						(6)		/*!< �������� */
#define KEY_LONGPRESS_TIME			100		/*!< ɨ������ */
#define KEY_DOUBLECLICK_TIME		100		/*!< ˫�������ʱ�� */

/* ������ */
typedef union
{
    unsigned int v;

    struct
    {
        unsigned int Mode : 1;
        unsigned int Stup : 1;
        unsigned int M1	  : 1;
        unsigned int M2   : 1;
        unsigned int UP   : 1;
        unsigned int Down : 1;
    }Key;
}KeyEvent_t;

/* �ṹ�� */
typedef struct
{
    KeyEvent_t pShort;
    KeyEvent_t rShort;
#ifdef KEY_USE_LONGPRESS
    KeyEvent_t pLong;
#endif
#ifdef KEY_USE_DOUBLE_CLICK
    KeyEvent_t DoubleClick;
#endif
}Key_t;

Key_t * GetKeyHandle(void);
void KeyScan(void);

#endif

