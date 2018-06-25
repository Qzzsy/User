#include "STM32F4xx.h"
#include "led.h"
#include "bsp_lcd.h"
#include "bsp_rtp_touch.h"
#include "bsp_eeprom_24xx.h"
#include "GUIDr.h"

uint8_t bsp_TestExtSRAM(void);

void UserMainFunc(void)
{
    uint8_t Buf[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    uint16_t ta, tb, x, y;
    BspLCD_FuncInit();
    BspLCD.Init();

    GuiSetDeviceAPI(BspLCD_PutPixelNoXY, BspLCD.PutPixel, BspLCD.SetDispWin);

    GuiClrScr(0x0000);
    
    if (Bsp_eeInit() == AT24XX_OK)
    {
        GuiClrScr(0xf800);
    }
    
    GuiSetTextColor(BLACK, WHITE);

    RTP_Adjust(RTP_NEED_ADJ);

    while(true)
    {
        if(RTP_Scan())//读取屏幕坐标
        {
        } 
        HAL_Delay(5);
    }
}

