#include "STM32F4xx.h"
#include "led.h"
#include "bsp_lcd.h"
#include "bsp_rtp_touch.h"
#include "bsp_eeprom_24xx.h"

uint8_t bsp_TestExtSRAM(void);

void UserMainFunc(void)
{
    uint8_t Buf[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    uint16_t ta, tb, x, y;
    BspLCD_FuncInit();
    BspLCD.Init();
    BspLCD.ClrScr(0xf800);
    
    if (Bsp_eeCheckOk() == AT24XX_OK)
    {
        BspLCD.ClrScr(0x03e0);
    }

    Bsp_eeWriteBytes(Buf, 0, 10);

    for (int i = 0; i < 10; i++)
    {
        Buf[i] = 0;
    }

    Bsp_eeReadBytes(Buf, 0, 10);

    while(true)
    {
        if(RTP_Scan())//读取屏幕坐标
        {
        } 
        HAL_Delay(5);
    }
}

