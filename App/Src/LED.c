/**
 ******************************************************************************
 * @file      led.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-07-08
 * @brief     实现LED闪烁
 * @History
 * Date           Author    version    		   Notes
 * 2018-06-22      ZSY      V1.0.0          first version.
 */
/* Includes ------------------------------------------------------------------*/
#include "led.h"

/* 若使用LL库，请定义USER_EFFI为1 */
#ifndef USER_EFFI
#define USER_EFFI 0
#endif

/* 定义使用的LED */
#define USER_LED0
//#define USER_LED1

#ifdef USER_LED0
/**
 * @func    LED0_On
 * @brief   LED0打开
 * @note    LED0为低电平打开，若使用高电平打开，请修改GPIO_PIN_RESET为GPIO_PIN_SET
 * @retval  无
 */
void LED0_On(void)
{
#if USER_EFFI == 1
    LL_GPIO_ResetOutputPin(LED0_GPIO_Port, LED0_Pin);
#else
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);
#endif
}

/**
 * @func    LED0_Off
 * @brief   LED0关闭
 * @note    LED0为高电平关闭，若使用低电平关闭，请修改GPIO_PIN_SET为GPIO_PIN_RESET
 * @retval  无
 */
void LED0_Off(void)
{
#if USER_EFFI == 1
    LL_GPIO_SetOutputPin(LED0_GPIO_Port, LED0_Pin);
#else
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
#endif
}
#endif

#ifdef USER_LED1
/**
 * @func    LED1_On
 * @brief   LED1打开
 * @note    LED0为低电平打开，若使用高电平打开，请修改GPIO_PIN_RESET为GPIO_PIN_SET
 * @retval  无
 */
void LED1_On(void)
{
#if USER_EFFI == 1
    LL_GPIO_ResetOutputPin(LED0_GPIO_Port, LED1_Pin);
#else
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
#endif
}

/**
 * @func    LED1_Off
 * @brief   LED1关闭
 * @note    LED1为高电平关闭，若使用低电平关闭，请修改GPIO_PIN_SET为GPIO_PIN_RESET
 * @retval  无
 */
void LED1_Off(void)
{
#if USER_EFFI == 1
    LL_GPIO_SetOutputPin(LED0_GPIO_Port, LED1_Pin);
#else
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
#endif
}
#endif

/**
 * @func    LED_TickInc
 * @brief   控制LED闪烁，由中断函数调用
 * @note
 * @retval  无
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
