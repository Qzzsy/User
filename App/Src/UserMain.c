#include "STM32F1xx.h"
#include "led.h"
#include "Bsp_Timer.h"
#include "Bsp_key.h"
#include "MyString.h"

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


void UserMainFunc(void)
{
    static uint8_t MotorStartFlag = false;
    static uint16_t Speed = 10000;
    static uint8_t MotorFR_Flag = false;
    Key_t *KeyEvent;
    uint8_t Buf[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    uint8_t status = 0;

    KeyEvent = GetKeyHandle();
    
    TimerStart(&htim3);

    while (true)
    {
        HAL_Delay(10);
    }
}
