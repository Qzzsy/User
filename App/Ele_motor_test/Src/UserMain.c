#include "STM32F4xx.h"
#include "led.h"
#include "Bsp_uart.h"
#include "Bsp_Timer.h"
#include "Bsp_key.h"
#include "MyString.h"
#include "Bsp_TM1639.h"

#define Mode1_1 0x01
#define Mode1_5 0x02
#define Mode16_1 0x03

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

void DIG_Disp(uint32_t Speed, uint16_t DIG_DispCoe)
{
    uint8_t DispBuf[2] = {'\0'};
    DispBuf[0] = Speed / DIG_DispCoe % 10;
    DispBuf[1] = Speed / (DIG_DispCoe * 10);
    TM1639_Disp(DispBuf, TM_MODE_DISP_DIG);
}

void UserMainFunc(void)
{
    static uint8_t MotorStartFlag = false;
    static uint8_t Mode = Mode1_1;
    static uint8_t DispLED = 0x00, StupLock = false;
    static uint16_t MotorSpeed = 0;
    static uint32_t Speed = 0, LastMode1Speed, LastMode2Speed, LastMode3Speed;
    static float Ratio = 1;
    static uint16_t DIG_DispCoe = 0;
    static uint8_t MotorFR_Flag = false;
    Key_t *KeyEvent;
    UART_HandleTypeDef *hUart;
    uint8_t DispBuf[2] = {1, 2};
    uint8_t status = 0;

    KeyEvent = GetKeyHandle();
    hUart = Uart1_GetHandle();
    SetConsoleDevice(hUart);
    TimerStart(&htim10);

    HAL_UART_Receive_IT(hUart, (uint8_t *)&status, 1);

    hUart = Uart2_GetHandle();
    HAL_UART_Receive_IT(hUart, (uint8_t *)&status, 1);

    PackgeData(hUart, Speed, MotorStartFlag, MotorFR_Flag);

    HAL_Delay(10);

    TM1639_DispOn();

    LastMode1Speed = 100;
    LastMode2Speed = 2000;
    LastMode3Speed = 10000;
    Ratio = 1;
    Speed = 2000;
    DIG_DispCoe = 1000;
    MotorSpeed = Speed * Ratio;
    DIG_Disp(Speed, DIG_DispCoe);

    /*!< ³õÊ¼»¯ÅäÖÃ³É1£º1£» */
    DispLED = 0x12;
    TM1639_Disp(&DispLED, TM_MODE_DISP_LED);

    while (true)
    {
        if (KeyEvent->pShort.Key.Mode == true)
        {
            Mode++;
            if (Mode > Mode16_1)
            {
                Mode = Mode1_1;
            }

            DispLED &= 0xF8;

            switch (Mode)
            {
            case Mode16_1:
                DispLED |= 0x01;
                Ratio = 16;
                LastMode3Speed = Speed;
                Speed = LastMode1Speed;
                DIG_DispCoe = 100;
                MotorSpeed = Speed * Ratio;

                DIG_Disp(Speed, DIG_DispCoe);
                my_printf("Motor Setting Mode 16:1!\n");
                break;
            case Mode1_1:
                DispLED |= 0x02;
                Ratio = 1;
                LastMode1Speed = Speed;
                Speed = LastMode2Speed;
                DIG_DispCoe = 1000;
                MotorSpeed = Speed * Ratio;

                DIG_Disp(Speed, DIG_DispCoe);
                my_printf("Motor Setting Mode 1:1!\n");
                break;
            case Mode1_5:
                DispLED |= 0x04;
                Ratio = 1.0 / 5.0;
                LastMode2Speed = Speed;
                Speed = LastMode3Speed;
                DIG_DispCoe = 10000;
                MotorSpeed = Speed * Ratio;

                DIG_Disp(Speed, DIG_DispCoe);
                my_printf("Motor Setting Mode 1:5!\n");
                break;
            default:
                break;
            }

            TM1639_Disp(&DispLED, TM_MODE_DISP_LED);
            PackgeData(hUart, MotorSpeed, MotorStartFlag, MotorFR_Flag);
        }
        if (KeyEvent->pLong.Key.Stup == true)
        {
            StupLock = true;
            if (MotorStartFlag == true)
            {
                MotorStartFlag = false;
                my_printf("Motor Stop!\n");
            }
            else if (MotorStartFlag == false)
            {
                MotorStartFlag = true;
                my_printf("Motor Start!\n");
            }
            PackgeData(hUart, MotorSpeed, MotorStartFlag, MotorFR_Flag);
        }
        else if (KeyEvent->rShort.Key.Stup == true)
        {
            if (StupLock == true)
            {
                StupLock = false;
            }
            else if (StupLock == false && MotorStartFlag == false)
            {
                DispLED &= 0xE7;

                if (MotorFR_Flag == true)
                {
                    MotorFR_Flag = false;
                    DispLED |= 0x10;

                    my_printf("Motor Setting F!\n");
                }
                else if (MotorFR_Flag == false)
                {
                    MotorFR_Flag = true;
                    DispLED |= 0x08;

                    my_printf("Motor Setting R!\n");
                }

                TM1639_Disp(&DispLED, TM_MODE_DISP_LED);
            }
            PackgeData(hUart, MotorSpeed, MotorStartFlag, MotorFR_Flag);
        }
        if (KeyEvent->pShort.Key.UP == true)
        {
            Speed += DIG_DispCoe;
            switch (Mode)
            {
                case Mode16_1:
                    if (Speed > 2500)
                        Speed = 2500;
                    break;
                case Mode1_1:
                    if (Speed > 40000)
                        Speed = 40000;
                    break;
                case Mode1_5:
                    if (Speed > 200000)
                        Speed = 200000;
                    break;
                default:break;
            }
            MotorSpeed = Speed * Ratio;

            DIG_Disp(Speed, DIG_DispCoe);
            my_printf("The speed is:%d \n", Speed);
            my_printf("The motor speed is:%d\n", MotorSpeed);
            PackgeData(hUart, MotorSpeed, MotorStartFlag, MotorFR_Flag);
        }
        else if (KeyEvent->pShort.Key.Down == true)
        {
            Speed -= DIG_DispCoe;
            switch (Mode)
            {
                case Mode16_1:
                    if (Speed < 100)
                        Speed = 100;
                    break;
                case Mode1_1:
                    if (Speed < 2000)
                        Speed = 2000;
                    break;
                case Mode1_5:
                    if (Speed < 10000)
                        Speed = 10000;
                    break;
                default:break;
            }
            MotorSpeed = Speed * Ratio;

            DIG_Disp(Speed, DIG_DispCoe);
            my_printf("The speed is:%d \n", Speed);
            my_printf("The motor speed is:%d\n", MotorSpeed);
            PackgeData(hUart, MotorSpeed, MotorStartFlag, MotorFR_Flag);
        }
        if (KeyEvent->pShort.Key.M1 == true)
        {
        }
        else if (KeyEvent->pLong.Key.M2 == true)
        {
        }
        HAL_Delay(10);
    }
}
