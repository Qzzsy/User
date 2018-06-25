/**
 * @file disp.h
 * 
 */

#ifndef DISP_H
#define DISP_H

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include "lv_misc/lv_color.h"
#include "lv_misc/lv_area.h"

/*********************
 *      DEFINES
 *********************/
#define TFT_HOR_RES 1024
#define TFT_VER_RES 600

#define TFT_EXT_FB		1		/*Frame buffer is located into an external SDRAM*/
#define TFT_USE_GPU		0		/*Enable hardware accelerator*/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void LCD_LvglInit(void);

/**********************
 *      MACROS
 **********************/

#endif
