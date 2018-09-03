/**
 * @file indev.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_hal/lv_hal.h"
#include "bsp_rtp_touch.h"
#include "lv_touchpad.h"
#ifdef STM32F1
#include "stm32f10x.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#elif defined STM32F7
#include "stm32f7xx.h"
#endif

static bool touchpad_read(lv_indev_data_t *data);

/**
 * Initialize your input devices here
 */
void TouchpadInit(void)
{
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.read = touchpad_read;
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  lv_indev_drv_register(&indev_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Read an input device
 * @param indev_id id of the input device to read
 * @param x put the x coordinate here
 * @param y put the y coordinate here
 * @return true: the device is pressed, false: released
 */
static bool touchpad_read(lv_indev_data_t *data)
{
	static int16_t last_x = 0;
	static int16_t last_y = 0;
    RTP_Dev_t * RTP_pDev;
	RTP_pDev = RTP_GetXY();
	if(RTP_pDev->Flags == RTP_PRESS)
    {
		data->point.x = RTP_pDev->CurPos.x;
		data->point.y = RTP_pDev->CurPos.y;
		last_x = data->point.x;
		last_y = data->point.y;
		data->state = LV_INDEV_STATE_PR;
	}
    else
    {
		data->point.x = last_x;
		data->point.y = last_y;
		data->state = LV_INDEV_STATE_REL;
	}

	return false;
}
