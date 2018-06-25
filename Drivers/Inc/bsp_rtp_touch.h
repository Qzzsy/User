#ifndef _BSP_RTP_TOUCH_H_
#define _BSP_RTP_TOUCH_H_

#ifdef STM32F1
#include "stm32f10cx.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#endif

#define RTP_OK              0
#define RTP_FAULT           1

#define RTP_NEED_ADJ        0
#define RTP_NEEDNT_ADJ      1

#define RTP_PRESS           (1 << 0)
#define RTP_LIFT_UP         (1 << 1)

#pragma pack(1)
//´¥ÃþÆÁÐ£×¼²ÎÊý	
struct _RTP_Param
{
    float xFac;				
    float yFac;
    short xOff;
    short yOff;	   
    uint8_t TouchType;
    uint8_t AdjFlag;
};
#pragma pack()

struct _RTP_Pos
{
    uint16_t x;
    uint16_t y;
};

//´¥ÃþÆÁ¿ØÖÆÆ÷
typedef struct
{
    struct _RTP_Pos CurPos;
    struct _RTP_Pos PrePos;
    struct _RTP_Pos OriPos;
    uint8_t xCmd;
    uint8_t yCmd;
    uint8_t Sta;	
    uint8_t Flags;   
    struct _RTP_Param RTP_Param;
}RTP_Dev_t;

uint8_t RTP_Scan(void);
RTP_Dev_t * RTP_GetXY(void);
void RTP_Adjust(uint8_t Flag);

#endif


