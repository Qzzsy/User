#include "STM32F4xx.h"
#include "led.h"
#include "bsp_lcd.h"
#include "bsp_rtp_touch.h"
#include "bsp_eeprom_24xx.h"
#include "GUIDr.h"
#include "lvgl.h"
#include "lv_tft.h"
#include "lv_touchpad.h"
#include "lv_demo.h"
#include "arm_math.h"
#include "Bsp_W25QXX.h"
#include "lv_test_theme.h"

uint8_t bsp_TestExtSRAM(void);

void UserMainFunc(void)
{
    uint8_t Buf[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    uint16_t ta, tb, x, y;
    BspLCD_FuncInit();
    BspLCD.Init();
    BspW25QXX_Init();
    bsp_TestExtSRAM();
    GuiSetDeviceAPI(BspLCD_PutPixelNoXY, BspLCD.PutPixel, BspLCD.SetDispWin);
    
    GuiClrScr(0x0000);
    
    if (Bsp_eeInit() == AT24XX_OK)
    {
        GuiClrScr(0xf800);
    }
    
    RTP_Adjust(RTP_NEEDNT_ADJ);
    
    GuiSetTextColor(BLACK, WHITE);

    lv_init();
    LCD_LvglInit();
    TouchpadInit();

//    demo_create();
    lv_theme_t *th = lv_theme_templ_init(21, &lv_font_dejavu_20);
    lv_test_theme_1(th);
    while(true)
    {
        lv_tick_inc(5);
		lv_task_handler();
        HAL_Delay(5);
    }
}

