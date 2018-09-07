#include "bsp_timer.h"

__weak void LED_TickInc(void)
{
}

__weak void KeyScan(void)
{
}

void TimerStart(TIM_HandleTypeDef * tim_baseHandle)
{
    HAL_TIM_Base_Start_IT(tim_baseHandle);
}

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
