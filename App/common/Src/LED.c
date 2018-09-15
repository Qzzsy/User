/**
 ******************************************************************************
 * @file      led.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-07-08
 * @brief     ʵ��LED��˸
 * @History
 * Date           Author    version    		   Notes
 * 2018-06-22      ZSY      V1.0.0          first version.
 * 2018-09-07      ZSY      V1.0.1          ��������LED��������߿�����ֲ��.
 */
/* Includes ------------------------------------------------------------------*/
#include "led.h"

/* ��ʹ��LL�⣬�붨��USER_EFFIΪ1 */
#ifndef USER_EFFI
#define USER_EFFI 0
#endif

/* ����ʹ�õ�LED */
#define USER_LED0
//#define USER_LED1

#ifdef USER_LED0
#define _LED0_GPIO_PORT              LED_GPIO_Port
#define _LED0_PIN                    LED_Pin
#endif
#ifdef USER_LED1
#define _LED1_GPIO_PORT              LED1_GPIO_Port
#define _LED1_PIN                    LED1_Pin
#endif

#ifdef USER_LED0
/**
 * @func    LED0_On
 * @brief   LED0��
 * @note    LED0Ϊ�͵�ƽ�򿪣���ʹ�øߵ�ƽ�򿪣����޸�GPIO_PIN_RESETΪGPIO_PIN_SET
 * @retval  ��
 */
void LED0_On(void)
{
#if USER_EFFI == 1
    LL_GPIO_ResetOutputPin(_LED0_GPIO_PORT, _LED0_PIN);
#else
    HAL_GPIO_WritePin(_LED0_GPIO_PORT, _LED0_PIN, GPIO_PIN_RESET);
#endif
}

/**
 * @func    LED0_Off
 * @brief   LED0�ر�
 * @note    LED0Ϊ�ߵ�ƽ�رգ���ʹ�õ͵�ƽ�رգ����޸�GPIO_PIN_SETΪGPIO_PIN_RESET
 * @retval  ��
 */
void LED0_Off(void)
{
#if USER_EFFI == 1
    LL_GPIO_SetOutputPin(_LED0_GPIO_PORT, _LED0_PIN);
#else
    HAL_GPIO_WritePin(_LED0_GPIO_PORT, _LED0_PIN, GPIO_PIN_SET);
#endif
}
#endif

#ifdef USER_LED1
/**
 * @func    LED1_On
 * @brief   LED1��
 * @note    LED1Ϊ�͵�ƽ�򿪣���ʹ�øߵ�ƽ�򿪣����޸�GPIO_PIN_RESETΪGPIO_PIN_SET
 * @retval  ��
 */
void LED1_On(void)
{
#if USER_EFFI == 1
    LL_GPIO_ResetOutputPin(_LED1_GPIO_PORT, _LED1_PIN);
#else
    HAL_GPIO_WritePin(_LED1_GPIO_PORT, _LED1_PIN, GPIO_PIN_RESET);
#endif
}

/**
 * @func    LED1_Off
 * @brief   LED1�ر�
 * @note    LED1Ϊ�ߵ�ƽ�رգ���ʹ�õ͵�ƽ�رգ����޸�GPIO_PIN_SETΪGPIO_PIN_RESET
 * @retval  ��
 */
void LED1_Off(void)
{
#if USER_EFFI == 1
    LL_GPIO_SetOutputPin(_LED1_GPIO_PORT, _LED1_PIN);
#else
    HAL_GPIO_WritePin(_LED1_GPIO_PORT, _LED1_PIN, GPIO_PIN_SET);
#endif
}
#endif

/**
 * @func    LED_TickInc
 * @brief   ����LED��˸�����жϺ�������
 * @note
 * @retval  ��
 */
void LED_TickInc(void)
{
    static uint16_t LED_Tick = 0;
    static uint8_t Flag = true;
    LED_Tick++;
    if (LED_Tick >= 50)
    {
        LED_Tick = 0;
        if (Flag == true)
        {
            Flag = false;
            LED0_Off();
        }
        else
        {
            Flag = true;
            LED0_On();
        }
    }
}
