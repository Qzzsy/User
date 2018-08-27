#include "STM32F4xx.h"
#include "led.h"
#include "Bsp_W25QXX.h"
#include "BootLoader.h"
#include "usart.h"
#include "MyString.h"

uint32_t buf[2048] = {'\0'};

UART_HandleTypeDef * hUart;

void ConsoleOut(uint8_t * Sendbuf, uint32_t Lenght)
{
    HAL_UART_Transmit(hUart, Sendbuf, Lenght, 10);
}

void UserMainFunc(void)
{
    uint8_t Buf[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    uint8_t status = 0;

    hUart = Uart1_GetHandle();
    SetConsoleDevice(&ConsoleOut);

    HAL_UART_Receive_IT(hUart, (uint8_t *)&status, 1);
//    SCB->VTOR = APPLICATION_ADDRESS;
//    Bootloader_JumpToApplication();

    my_printf("×£ÊÀÒ¶%d\n", 12);
    while(true)
    {
        LED0_On();
        for (uint32_t i = 0; i < 0x7fffff; i++);
        LED0_Off();
        for (uint32_t i = 0; i < 0x7fffff; i++);
    }
}

