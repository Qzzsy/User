/**
 ******************************************************************************
 * @file      bsp_key.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-08-31
 * @brief     ���ļ�ʵ���˰����Ĵ�������֧�ְ����ĳ��̰�����˫����������ֵ������һ��
 *            ���ڣ�����Ҫ��Ϊ������ϵͳ��ʹ�õ�ʱ����Ҫע��ɨ��ʱ����ж�ʱ��ĳ�ͻ
 * @History
 * Date           Author    version    		Notes
 * 2018-08-31       ZSY     V1.0.0      first version.
 * 2018-09-07       ZSY     V1.0.1      ��Ӱ����ͷż�⣬��Ӷ�F1��֧��
 */

/* Includes ------------------------------------------------------------------*/
#include "bsp_key.h"

#if defined STM32F1
#include "STM32F1xx.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#endif

/* ���尴����������� */
#define KEY_ERROR (-1)

//typedef unsigned char uint8_t;
//typedef unsigned short uint16_t;
#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

//typedef char int8_t;
//typedef short int16_t;
//typedef int int32_t;

/* ��ʼ���������¼� */
Key_t KeyEvent = {
    0,
    0,
#ifdef KEY_USE_LONGPRESS
    0,
#endif
#ifdef KEY_USE_DOUBLE_CLICK
    0,
#endif
};

/**
 * @func    GetKeyValue
 * @brief   get the key value, implement the interface according to the hardware.
 * @retval  none
 */
uint32_t GetKeyValue(void)
{
    __IO uint32_t Value = 0;
    uint32_t KeyValue = 0;

    Value = 0xfffffff8;

    Value |= HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin);
    Value |= HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) << 1;
    Value |= HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin) << 2;

    return ~Value;
}

/**
 * @func    KeyScan
 * @brief   scan the key, the function needs to be scanned periodically.
 * @retval  none
 */
void KeyScan(void)
{
    __IO uint8_t i = 0;
    static Key_t KeyCount = {
        0,
        0,
#ifdef KEY_USE_LONGPRESS
        0,
#endif
#ifdef KEY_USE_DOUBLE_CLICK
        0,
#endif
    };

#ifdef KEY_USE_LONGPRESS
    static uint16_t KeyLongPressTime[KEY_NUM] = {0};
#endif
#ifdef KEY_USE_DOUBLE_CLICK
    static uint16_t KeyDoubleClickTime[KEY_NUM] = {0};
#endif

    uint32_t KeyValue;

    KeyValue = GetKeyValue();

    if (KeyValue == (uint32_t)KEY_ERROR)
    {
        return;
    }

    KeyEvent.pShort.v = KeyValue & (KeyValue ^ KeyCount.pShort.v);
    KeyCount.pShort.v = KeyValue;
    KeyEvent.rShort.v = KeyCount.rShort.v & (KeyValue ^ KeyCount.rShort.v);
    KeyCount.rShort.v = KeyCount.pShort.v;

#ifdef KEY_USE_LONGPRESS
    for (i = 0; i < KEY_NUM; i++)
    {
        KeyLongPressTime[i]++;
        if (KeyEvent.pShort.v & (1 << i))
        {
            KeyLongPressTime[i] = 0;

            continue;
        }

        if (KeyLongPressTime[i] > KEY_LONGPRESS_TIME)
        {
            KeyEvent.pLong.v &= ~(1 << i);
            KeyEvent.pLong.v |= (1 << i) & ((1 << i) ^ KeyCount.pLong.v);
            KeyCount.pLong.v &= ~(1 << i);
            KeyCount.pLong.v |= 1 << i;
        }

        if (!(KeyCount.pShort.v & (1 << i)))
        {
            KeyCount.pLong.v &= ~(1 << i);

            KeyLongPressTime[i] = 0;
        }
    }
#endif
#ifdef KEY_USE_DOUBLE_CLICK
    for (i = 0; i < KEY_NUM; i++)
    {
        KeyDoubleClickTime[i]++;

        if ((KeyEvent.DoubleClick.v & (1 << i)) && (KeyCount.DoubleClick.v & (1 << i)))
        {
            KeyEvent.DoubleClick.v &= ~(1 << i);
            KeyCount.DoubleClick.v &= ~(1 << i);
        }

        if (KeyEvent.pShort.v & (1 << i))
        {
            if (!(KeyCount.DoubleClick.v & (1 << i)))
            {
                KeyDoubleClickTime[i] = 0;

                KeyCount.DoubleClick.v &= ~(1 << i);
                KeyCount.DoubleClick.v |= 1 << i;
            }
            else
            {
                if (KeyDoubleClickTime[i] < KEY_DOUBLECLICK_TIME)
                {
                    KeyEvent.DoubleClick.v &= ~(1 << i);
                    KeyEvent.DoubleClick.v |= 1 << i;
                }
                else
                {
                    KeyCount.DoubleClick.v &= ~(1 << i);
                }
            }
            continue;
        }

        if (KeyDoubleClickTime[i] >= KEY_DOUBLECLICK_TIME && (KeyCount.DoubleClick.v & (1 << i)))
        {
            KeyEvent.DoubleClick.v &= ~(1 << i);
            KeyCount.DoubleClick.v &= ~(1 << i);

            KeyDoubleClickTime[i] = 0;
        }
    }
#endif
}

/**
 * @func    GetKeyHandle
 * @brief   it will return the key handle.
 * @retval  KeyEvent
 */
Key_t *GetKeyHandle(void)
{
    return &KeyEvent;
}
