#include "led.h"

static uint16_t LED_Tick = 0;


void LED0_On(void)
{
	
    LL_GPIO_ResetOutputPin(GPIOF, LED0_Pin);
}

void LED0_Off(void)
{
    LL_GPIO_SetOutputPin(GPIOF, LED0_Pin);
}

void LED1_On(void)
{
    LL_GPIO_ResetOutputPin(GPIOF, LED1_Pin);
}

void LED1_Off(void)
{
    LL_GPIO_SetOutputPin(GPIOF, LED1_Pin);
}

void LED_TickInc(void)
{
	
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
