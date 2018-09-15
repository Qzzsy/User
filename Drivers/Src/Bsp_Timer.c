/**
 ******************************************************************************
 * @file      bsp_timer.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-09-14
 * @brief     该文件提供了Timer操作相关的API，使得底层与应用层更加分离
 * @History
 * Date           Author    version    		Notes
 * 2018-09-14       ZSY     V1.0.0      first version.
 */

/* Includes ------------------------------------------------------------------*/
#include "bsp_timer.h"

/**
 * @func    LED_TickInc
 * @brief   LED_TickInc的弱函数
 * @retval  无
 */
__weak void LED_TickInc(void)
{
}

/**
 * @func    KeyScan
 * @brief   KeyScan的弱函数
 * @retval  无
 */
__weak void KeyScan(void)
{
}

/**
 * @func    TimerStart
 * @brief   开启定时器，会触发中断
 * @param   tim_baseHandle 定时器的句柄
 * @retval  无
 */
void TimerStartIT(TIM_HandleTypeDef * tim_baseHandle)
{
    HAL_TIM_Base_Start_IT(tim_baseHandle);
}

/**
 * @func    TimerStart
 * @brief   开启定时器
 * @param   tim_baseHandle 定时器的句柄
 * @retval  无
 */
void TimerStart(TIM_HandleTypeDef * tim_baseHandle)
{
    HAL_TIM_Base_Start(tim_baseHandle);
}

/**
 * @func    TimerStartPWM
 * @brief   开启对应通道的PWM
 * @param   tim_baseHandle 定时器的句柄
 * @param   Channel 通道
 * @retval  无
 */
void TimerStartPWM(TIM_HandleTypeDef * tim_baseHandle, uint16_t Channel)
{
    HAL_TIM_PWM_Start(tim_baseHandle, Channel);
}

/**
 * @func    PwmSetCompare
 * @brief   设置PWM的占空比
 * @param   tim_baseHandle 定时器的句柄
 * @param   Channel 通道
 * @param   IndexWave 占空比
 * @retval  无
 */
void PwmSetCompare(TIM_HandleTypeDef * tim_baseHandle, uint16_t Channel, uint16_t IndexWave)
{
    __HAL_TIM_SET_COMPARE(tim_baseHandle, Channel, IndexWave);
}

/**
 * @func    HAL_TIM_PeriodElapsedCallback
 * @brief   定时器的回调函数
 * @param   tim_baseHandle 定时器的句柄
 * @retval  无
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * tim_baseHandle)
{
    if (tim_baseHandle->Instance == TIM1)
    {
        /* code */
        return ;
    }
    else if (tim_baseHandle->Instance == TIM2)
    {
        /* code */
        return ;
    }
    else if (tim_baseHandle->Instance == TIM3)
    {
        LED_TickInc();
        KeyScan();
        /* code */
        return ;
    }
    else if (tim_baseHandle->Instance == TIM4)
    {
        /* code */
        return ;
    }
    
    /*!< more Timer add in this; */
}
