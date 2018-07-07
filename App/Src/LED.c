#include "led.h"

#define USER_EFFI           0

void LED0_On(void)
{
#if USER_EFFI == 1
    LL_GPIO_ResetOutputPin(LED0_GPIO_Port, LED0_Pin);
#else
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);
#endif
}

void LED0_Off(void)
{
#if USER_EFFI == 1
    LL_GPIO_SetOutputPin(LED0_GPIO_Port, LED0_Pin);
#else
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
#endif
}

void LED1_On(void)
{
#if USER_EFFI == 1
    LL_GPIO_ResetOutputPin(LED0_GPIO_Port, LED1_Pin);
#else
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
#endif
}

void LED1_Off(void)
{
#if USER_EFFI == 1
    LL_GPIO_SetOutputPin(LED0_GPIO_Port, LED1_Pin);
#else
    HAL_GPIO_WritePin(LED0_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
#endif
}

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
            LED0_On();
            LED1_Off();
        }
        else
        {
            Flag = true;
            LED0_Off();
            LED1_On();
        }
    }
}
