/**
 ******************************************************************************
 * @file      bsp_key.h
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-08-31
 * @brief     该文件实现了按键的处理方法，支持按键的长短按，单双吉，按键的值仅存在一个
 *            周期，不需要人为清理，在系统上使用的时候需要注意扫描时间跟判断时间的冲突
 * @History
 * Date           Author    version    		Notes
 * 2018-08-31       ZSY     V1.0.0      first version.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_KEY_H_
#define __BSP_KEY_H_

#define KEY_USE_LONGPRESS					/*!< 定义使能长按 */
#define KEY_USE_DOUBLE_CLICK				/*!< 定义使能双击 */

#define KEY_NUM						(6)		/*!< 按键数量 */
#define KEY_LONGPRESS_TIME			100		/*!< 扫面周期 */
#define KEY_DOUBLECLICK_TIME		100		/*!< 双击最大间隔时间 */

/* 共用体 */
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

/* 结构体 */
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

