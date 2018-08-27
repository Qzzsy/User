#include "STM32F4xx.h"

__weak void LED_TickInc(void)
{
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* tim_baseHandle)
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
        /* code */
        return ;
    }
    else if (tim_baseHandle->Instance == TIM4)
    {
        /* code */
        return ;
    }
    else if (tim_baseHandle->Instance == TIM5)
    {
        /* code */
        return ;
    }
    else if (tim_baseHandle->Instance == TIM10)
    {
        LED_TickInc();
        return ;
    }
    
}
