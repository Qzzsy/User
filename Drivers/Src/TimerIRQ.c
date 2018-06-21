#include "STM32F4xx.h"

__weak void LED_TickInc(void)
{
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* tim_baseHandle)
{
    if(tim_baseHandle->Instance == TIM10)
    {
        LED_TickInc();
    }
}
