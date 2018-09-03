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
    if(tim_baseHandle->Instance == TIM10)
    {
        LED_TickInc();
        KeyScan();
    }
}
