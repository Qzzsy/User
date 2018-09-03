#include "STM32F4xx.h"
#include "led.h"
#include "Bsp_uart.h"
#include "Bsp_Timer.h"
#include "Bsp_key.h"
#include "MyString.h"
#include "Bsp_TM1639.h"

uint8_t CalCrc8(void *Data, uint16_t Len)
{
    uint8_t *tmp = (uint8_t *)Data;

    __IO uint8_t i;
    __IO uint8_t _CRC8 = 0;

    while (Len--)
    {
        _CRC8 ^= *tmp++;

        for (i = 0; i < 8; i++)
        {
            if (_CRC8 & 0x01)
            {
                _CRC8 = (_CRC8 >> 1) ^ 0x8c;
            }
            else
            {
                _CRC8 = _CRC8 >> 1;
            }
        }
    }

    return _CRC8;
}

void PackgeData(UART_HandleTypeDef *hUart, uint16_t Speed, uint8_t Start_Stop, uint8_t F_R)
{
    uint8_t SendData[4] = {'\0'};

    SendData[0] = Speed / 1000;
    SendData[1] = ((Speed % 1000) / 100) << 4;
    SendData[1] |= (Start_Stop & 0x01) << 1;
    SendData[1] |= F_R & 0x01;

    SendData[3] = CalCrc8(SendData, 3);

    HAL_UART_Transmit(hUart, SendData, 4, 10);
}

void UserMainFunc(void)
{
    static uint8_t MotorStartFlag = false;
    static uint16_t Speed = 10000;
    static uint8_t MotorFR_Flag = false;
    Key_t *KeyEvent;
    UART_HandleTypeDef *hUart;
    uint8_t Buf[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    uint8_t status = 0;

    KeyEvent = GetKeyHandle();
    hUart = Uart1_GetHandle();
    SetConsoleDevice(hUart);
    TimerStart(&htim10);

    HAL_UART_Receive_IT(hUart, (uint8_t *)&status, 1);

    hUart = Uart2_GetHandle();
    HAL_UART_Receive_IT(hUart, (uint8_t *)&status, 1);

    PackgeData(hUart, Speed, MotorStartFlag, MotorFR_Flag);

    TM1639_DispOn();

    TM1639_Disp(Buf, TM_MODE_DISP_DIG);

    while (true)
    {
        if (KeyEvent->DoubleClick.Key.OK == true)
        {
            if (MotorStartFlag == false)
            {
                MotorFR_Flag = false;
                my_printf("Now is F!\n");
                PackgeData(hUart, Speed, MotorStartFlag, MotorFR_Flag);
            }
        }
        if (KeyEvent->pShort.Key.OK == true)
        {
            if (MotorStartFlag == true)
            {
                if (Speed <= 39000)
                {
                    Speed += 1000;
                    my_printf("Now speed is %d\n", Speed);
                }
                else
                    my_printf("The speed is max speed!\n");
                PackgeData(hUart, Speed, MotorStartFlag, MotorFR_Flag);
            }
        }
        if (KeyEvent->pLong.Key.OK == true)
        {
            my_printf("Motor Start!\n");
            MotorStartFlag = true;
            PackgeData(hUart, Speed, MotorStartFlag, MotorFR_Flag);
        }
        if (KeyEvent->DoubleClick.Key.Back == true)
        {
            if (MotorStartFlag == false)
            {
                MotorFR_Flag = true;
                my_printf("Now is R!\n");
                PackgeData(hUart, Speed, MotorStartFlag, MotorFR_Flag);
            }
        }
        if (KeyEvent->pShort.Key.Back == true)
        {
            if (MotorStartFlag == true)
            {
                if (Speed > 1000)
                {
                    Speed -= 1000;
                    my_printf("Now speed is %d\n", Speed);
                }
                else
                    my_printf("The speed is min speed!\n");
                PackgeData(hUart, Speed, MotorStartFlag, MotorFR_Flag);
            }
        }
        if (KeyEvent->pLong.Key.Back == true)
        {
            my_printf("Motor Stop!\n");
            MotorStartFlag = false;
            PackgeData(hUart, Speed, MotorStartFlag, MotorFR_Flag);
        }
        HAL_Delay(10);
    }
}
