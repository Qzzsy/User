/**
 ******************************************************************************
 * @file      bsp_rtp_touch.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-06-21
 * @brief     电容触摸屏触摸面板的驱动程序，集成了数据的读取与处理，校准触摸并保存数据
 *            直接输出对应的坐标
 * @History
 * Date           Author    version    		Notes
 * 2017-10-31       ZSY     V1.0.0      first version.
 */

#include "bsp_rtp_touch.h"
#include "gpio.h"
#include "bsp_lcd.h"
#include "GUIDr.h"
#include "bsp_eeprom_24xx.h"

#if defined USE_FULL_LL_DRIVER
#define T_DIN_WRITE_H LL_GPIO_SetOutputPin(T_MOSI_GPIO_Port, T_MOSI_Pin)
#define T_DIN_WRITE_L LL_GPIO_ResetOutputPin(T_MOSI_GPIO_Port, T_MOSI_Pin)
#define T_CLK_WRITE_H LL_GPIO_SetOutputPin(T_SCK_GPIO_Port, T_SCK_Pin)
#define T_CLK_WRITE_L LL_GPIO_ResetOutputPin(T_SCK_GPIO_Port, T_SCK_Pin)
#define T_CS_WRITE_H LL_GPIO_SetOutputPin(T_CS_GPIO_Port, T_CS_Pin)
#define T_CS_WRITE_L LL_GPIO_ResetOutputPin(T_CS_GPIO_Port, T_CS_Pin)

#define T_MISO_READ LL_GPIO_IsInputPinSet(T_MISO_GPIO_Port, T_MISO_Pin)
#define T_PEN_READ LL_GPIO_IsInputPinSet(T_PEN_GPIO_Port, T_PEN_Pin)

#elif defined USE_HAL_DRIVER
#define T_DIN_WRITE_H HAL_GPIO_WritePin(T_MOSI_GPIO_Port, T_MOSI_Pin, GPIO_PIN_SET)
#define T_DIN_WRITE_L HAL_GPIO_WritePin(T_MOSI_GPIO_Port, T_MOSI_Pin, GPIO_PIN_RESET)
#define T_CLK_WRITE_H HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_SET)
#define T_CLK_WRITE_L HAL_GPIO_WritePin(T_SCK_GPIO_Port, T_SCK_Pin, GPIO_PIN_RESET)
#define T_CS_WRITE_H HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_SET)
#define T_CS_WRITE_L HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_RESET)

#define T_MISO_READ HAL_GPIO_ReadPin(T_MISO_GPIO_Port, T_MISO_Pin)
#define T_PEN_READ HAL_GPIO_ReadPin(T_PEN_GPIO_Port, T_PEN_Pin)
#endif

//默认为touchtype=0的数据.
#define CMD_RDX     0xD0
#define CMD_RDY     0x90
#define READ_TIMES  5       //读取次数
#define LOST_VAL    1       //丢弃值
#define ERR_RANGE   50      //误差范围

#define SAVE_ADJ_DATA_BASE_ADDR         0X20

#pragma pack(1)
struct _RTP_Param
{
    float xFac;			//触摸屏校准参数		
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

//触摸屏控制器
typedef struct
{
    struct _RTP_Pos CurPos;
    struct _RTP_Pos PrePos;
    uint8_t xCmd;
    uint8_t yCmd;
    uint8_t  Sta;							
    struct _RTP_Param RTP_Param;
}RTP_Dev_t;

RTP_Dev_t RTP_Dev;
/**
 * @func    BspRTP_Delay
 * @brief   BspRTP延时，用于初始化过程
 * @param   nCount 延时的大小
 * @retval  无
 */
void BspRTP_Delay(__IO uint32_t Number)
{
    uint32_t i = 0;

    while (Number--)
    {
        i = 24;
        while (i--);
    }
}

/**
 * @func    RTP_WriteByte
 * @brief   向触摸屏IC写入1byte数据
 * @param   Data 要写入的数据
 * @retval  无
 */
static void RTP_WriteByte(uint8_t Data)
{
    uint8_t count = 0;
    for (count = 0; count < 8; count++)
    {
        if (Data & 0x80)
        {
            T_DIN_WRITE_H;
        }
        else
        {
            T_DIN_WRITE_L;
        }
        Data <<= 1;
        T_CLK_WRITE_L;
        BspRTP_Delay(1);
        T_CLK_WRITE_H; //上升沿有效
    }
}

/**
 * @func    RTP_Read_AD
 * @brief   从触摸屏IC读取adc值
 * @param   Cmd 延时的大小
 * @retval  读到的数据
 */
static uint16_t RTP_ReadAD(uint8_t Cmd)
{
    uint8_t count = 0;
    uint16_t Data = 0;
    T_CLK_WRITE_L;      //先拉低时钟
    T_DIN_WRITE_L;      //拉低数据线
    T_CS_WRITE_L;       //选中触摸屏IC
    RTP_WriteByte(Cmd); //发送命令字
    BspRTP_Delay(8);    //ADS7846的转换时间最长为6us
    T_CLK_WRITE_L;
    BspRTP_Delay(1);
    T_CLK_WRITE_H; //给1个时钟，清除BUSY
    BspRTP_Delay(1);
    T_CLK_WRITE_L;
    for (count = 0; count < 16; count++) //读出16位数据,只有高12位有效
    {
        Data <<= 1;
        T_CLK_WRITE_L; //下降沿有效
        BspRTP_Delay(1);
        T_CLK_WRITE_H;
        if (T_MISO_READ)
        {
            Data++;
        }
    }
    Data >>= 4;   //只有高12位有效
    T_CS_WRITE_H; //释放片选
    return (Data);
}

/**
 * @func    RTP_Read_XOY
 * @brief   读取触摸屏的X或Y值
 * @param   xy 选择读取的值
 * @retval  返回读取到的值
 */
static uint16_t RTP_ReadXOY(uint8_t xy)
{
    uint16_t i, j;
    uint16_t buf[READ_TIMES];
    uint16_t sum = 0;
    uint16_t temp;
    for (i = 0; i < READ_TIMES; i++)
    {
        buf[i] = RTP_ReadAD(xy);
    }
    for (i = 0; i < READ_TIMES - 1; i++) //排序
    {
        for (j = i + 1; j < READ_TIMES; j++)
        {
            if (buf[i] > buf[j]) //升序排列
            {
                temp = buf[i];
                buf[i] = buf[j];
                buf[j] = temp;
            }
        }
    }
    sum = 0;
    for (i = LOST_VAL; i < READ_TIMES - LOST_VAL; i++)
    {
        sum += buf[i];
    }
    temp = sum / (READ_TIMES - 2 * LOST_VAL);

    return temp;
}

/**
 * @func    BspRTP_Delay
 * @brief   读取x,y坐标
 * @param   x,y:读取到的坐标值
 * @retval  0,失败;1,成功
 */
static uint8_t RTP_Read_xyValue(uint16_t *x, uint16_t *y)
{
    uint16_t xtemp, ytemp;
    xtemp = RTP_ReadXOY(RTP_Dev.xCmd);
    ytemp = RTP_ReadXOY(RTP_Dev.yCmd);
    if (xtemp < 100 || ytemp < 100)
        return RTP_FAULT;//读数失败
    *x = xtemp;
    *y = ytemp;
    return RTP_OK; //读数成功
}

/**
 * @func    RTP_Read_XY2
 * @brief   读取到XY值
 * @param   x,y:读取到的坐标值
 * @retval  0,失败;1,成功
 */
uint8_t RTP_ReadXY(uint16_t *x, uint16_t *y)
{
    uint16_t x1, y1;
    uint16_t x2, y2;

    if (RTP_Read_xyValue(&x1, &y1) != RTP_OK)
    {
        return RTP_FAULT;
    }

    if (RTP_Read_xyValue(&x2, &y2) != RTP_OK)
    {
        return RTP_FAULT;
    }
    if (((x2 <= x1 && x1 < x2 + ERR_RANGE) || (x1 <= x2 && x2 < x1 + ERR_RANGE)) //前后两次采样在+-50内
        && ((y2 <= y1 && y1 < y2 + ERR_RANGE) || (y1 <= y2 && y2 < y1 + ERR_RANGE)))
    {
        *x = (x1 + x2) / 2;
        *y = (y1 + y2) / 2;
        return RTP_OK;
    }
    else
    {
        return RTP_FAULT;
    }
}

uint8_t RTP_Scan(void)
{
    uint16_t ta, tb;
    if (T_PEN_READ == 0) //有按键按下
    {
        if (RTP_ReadXY(&ta, &tb) != RTP_OK && (RTP_Dev.Sta & RTP_LIFT_UP)) //读取物理坐标
        {
            return RTP_FAULT;
        }
        RTP_Dev.Sta &= ~RTP_LIFT_UP;
        RTP_Dev.Sta |= RTP_PRESS;
        RTP_Dev.PrePos.x = RTP_Dev.CurPos.x;
        RTP_Dev.PrePos.y = RTP_Dev.CurPos.y;
        RTP_Dev.CurPos.x = RTP_Dev.RTP_Param.xFac * ta + RTP_Dev.RTP_Param.xOff;
        RTP_Dev.CurPos.y = RTP_Dev.RTP_Param.yFac * tb + RTP_Dev.RTP_Param.yOff;
    }
    else if((T_PEN_READ == 1) && (RTP_Dev.Sta & RTP_PRESS))
    {
        RTP_Dev.Sta &= ~RTP_PRESS;
        RTP_Dev.Sta |= RTP_LIFT_UP;
        RTP_Dev.CurPos.x = UINT16_MAX;
        RTP_Dev.CurPos.y = UINT16_MAX;
    }
    return RTP_OK;
}

//////////////////////////////////////////////////////////////////////////////////
//与LCD部分有关的函数
//画一个触摸点
//用来校准用的
//x,y:坐标
//color:颜色
static void RTP_DrawTouchPoint(uint16_t x, uint16_t y, uint16_t Color)
{
    GuiDrawLine(x - 12, y, x + 13, y, Color);  //横线
    GuiDrawLine(x, y - 12, x, y + 13, Color);  //竖线
    // LCD_DrawPoint(x + 1, y + 1);
    // LCD_DrawPoint(x - 1, y + 1);
    // LCD_DrawPoint(x + 1, y - 1);
    // LCD_DrawPoint(x - 1, y - 1);
    GuiDrawCircle(x, y, 6, Color); //画中心圈
}

//保存校准参数
void RTP_SaveAdjdata(void)
{
    RTP_Dev.RTP_Param.AdjFlag = 0x0A;
    Bsp_eeWriteBytes((uint8_t *)&RTP_Dev.RTP_Param, SAVE_ADJ_DATA_BASE_ADDR, sizeof(struct _RTP_Param));
}
//得到保存在EEPROM里面的校准值
//返回值：1，成功获取数据
//        0，获取失败，要重新校准
uint8_t RTP_GetAdjdata(void)
{
    struct _RTP_Param TempParam;

    Bsp_eeReadBytes((uint8_t *)&TempParam, SAVE_ADJ_DATA_BASE_ADDR, sizeof(struct _RTP_Param));

    if (TempParam.AdjFlag == 0x0A)              //触摸屏已经校准过了
    {
        RTP_Dev.RTP_Param = TempParam;

        if (RTP_Dev.RTP_Param.TouchType)        //X,Y方向与屏幕相反
        {
            RTP_Dev.xCmd = CMD_RDY;
            RTP_Dev.yCmd = CMD_RDX;
        }
        else //X,Y方向与屏幕相同
        {
            RTP_Dev.xCmd = CMD_RDX;
            RTP_Dev.yCmd = CMD_RDY;
        }
        return RTP_OK;
    }
    return RTP_FAULT;
}
//触摸屏校准代码
//得到四个校准参数
void RTP_Adjust(void)
{
    uint16_t PosTemp[4][2]; //坐标缓存值
    uint8_t cnt = 0;
    uint16_t d1, d2;
    uint32_t tem1, tem2;
    double fac;
    uint16_t OutTime = 0;
    cnt = 0;

    GuiClrScr(WHITE);

    GuiDrawStringAt("Please calibration screen!", 0, 40);
    
    RTP_DrawTouchPoint(20, 20, RED);     //画点1
    RTP_Dev.RTP_Param.xFac = 0;         //xfac用来标记是否校准过,所以校准之前必须清掉!以免错误
    RTP_Dev.xCmd = CMD_RDX;
    RTP_Dev.yCmd = CMD_RDY;
    RTP_Dev.Sta |= RTP_LIFT_UP;
    RTP_ReadXY(&d1,&d1);//第一次读取初始化
    
    while (1)                           //如果连续10秒钟没有按下,则自动退出
    {
        if (RTP_Scan() != RTP_OK)       //扫描物理坐标
        {
            continue;
        }

        if (RTP_Dev.Sta & RTP_PRESS)    //按键按下了一次(此时按键松开了.)
        {
            OutTime = 0;

            PosTemp[cnt][0] = RTP_Dev.CurPos.x;
            PosTemp[cnt][1] = RTP_Dev.CurPos.y;
            cnt++;
            switch (cnt)
            {
            case 1:
                RTP_DrawTouchPoint(20, 20, WHITE);              //清除点1
                RTP_DrawTouchPoint(BspLCD_Dev.Width - 20, 20, RED); //画点2
                break;
            case 2:
                RTP_DrawTouchPoint(BspLCD_Dev.Width - 20, 20, WHITE); //清除点2
                RTP_DrawTouchPoint(20, BspLCD_Dev.Height - 20, RED);  //画点3
                break;
            case 3:
                RTP_DrawTouchPoint(20, BspLCD_Dev.Height - 20, WHITE);              //清除点3
                RTP_DrawTouchPoint(BspLCD_Dev.Width - 20, BspLCD_Dev.Height - 20, RED); //画点4
                break;
            case 4:                                          //全部四个点已经得到
                                                             //对边相等
                tem1 = abs(PosTemp[0][0] - PosTemp[1][0]); //x1-x2
                tem2 = abs(PosTemp[0][1] - PosTemp[1][1]); //y1-y2
                tem1 *= tem1;
                tem2 *= tem2;
                d1 = sqrt(tem1 + tem2); //得到1,2的距离

                tem1 = abs(PosTemp[2][0] - PosTemp[3][0]); //x3-x4
                tem2 = abs(PosTemp[2][1] - PosTemp[3][1]); //y3-y4
                tem1 *= tem1;
                tem2 *= tem2;
                d2 = sqrt(tem1 + tem2); //得到3,4的距离
                fac = (float)d1 / d2;
                if (fac < 0.95 || fac > 1.05 || d1 == 0 || d2 == 0) //不合格
                {
                    cnt = 0;
                    RTP_DrawTouchPoint(BspLCD_Dev.Width - 20, BspLCD_Dev.Height - 20, WHITE);                                                                                           //清除点4
                    RTP_DrawTouchPoint(20, 20, RED);                                                                                                                            //画点1
                    // TP_Adj_Info_Show(PosTemp[0][0], PosTemp[0][1], PosTemp[1][0], PosTemp[1][1],
                    //                  PosTemp[2][0], PosTemp[2][1], PosTemp[3][0], PosTemp[3][1], fac * 100); //显示数据
                    continue;
                }
                tem1 = abs(PosTemp[0][0] - PosTemp[2][0]); //x1-x3
                tem2 = abs(PosTemp[0][1] - PosTemp[2][1]); //y1-y3
                tem1 *= tem1;
                tem2 *= tem2;
                d1 = sqrt(tem1 + tem2); //得到1,3的距离

                tem1 = abs(PosTemp[1][0] - PosTemp[3][0]); //x2-x4
                tem2 = abs(PosTemp[1][1] - PosTemp[3][1]); //y2-y4
                tem1 *= tem1;
                tem2 *= tem2;
                d2 = sqrt(tem1 + tem2); //得到2,4的距离
                fac = (float)d1 / d2;
                if (fac < 0.95 || fac > 1.05) //不合格
                {
                    cnt = 0;
                    RTP_DrawTouchPoint(BspLCD_Dev.Width - 20, BspLCD_Dev.Height - 20, WHITE);                                                                                           //清除点4
                    RTP_DrawTouchPoint(20, 20, RED);                                                                                                                            //画点1
                    // TP_Adj_Info_Show(PosTemp[0][0], PosTemp[0][1], PosTemp[1][0], PosTemp[1][1],
                    //                  PosTemp[2][0], PosTemp[2][1], PosTemp[3][0], PosTemp[3][1], fac * 100); //显示数据
                    continue;
                } //正确了

                //对角线相等
                tem1 = abs(PosTemp[1][0] - PosTemp[2][0]); //x1-x3
                tem2 = abs(PosTemp[1][1] - PosTemp[2][1]); //y1-y3
                tem1 *= tem1;
                tem2 *= tem2;
                d1 = sqrt(tem1 + tem2); //得到1,4的距离

                tem1 = abs(PosTemp[0][0] - PosTemp[3][0]); //x2-x4
                tem2 = abs(PosTemp[0][1] - PosTemp[3][1]); //y2-y4
                tem1 *= tem1;
                tem2 *= tem2;
                d2 = sqrt(tem1 + tem2); //得到2,3的距离
                fac = (float)d1 / d2;
                if (fac < 0.95 || fac > 1.05) //不合格
                {
                    cnt = 0;
                    RTP_DrawTouchPoint(BspLCD_Dev.Width - 20, BspLCD_Dev.Height - 20, WHITE);                                                                                           //清除点4
                    RTP_DrawTouchPoint(20, 20, RED);                                                                                                                            //画点1
                    // TP_Adj_Info_Show(PosTemp[0][0], PosTemp[0][1], PosTemp[1][0], PosTemp[1][1],
                    //                  PosTemp[2][0], PosTemp[2][1], PosTemp[3][0], PosTemp[3][1], fac * 100); //显示数据
                    continue;
                } //正确了
                //计算结果
                RTP_Dev.RTP_Param.xFac = (float)(BspLCD_Dev.Width - 40) / (PosTemp[1][0] - PosTemp[0][0]);       //得到xfac
                RTP_Dev.RTP_Param.xOff = (BspLCD_Dev.Width - RTP_Dev.RTP_Param.xFac * (PosTemp[1][0] + PosTemp[0][0])) / 2; //得到xoff

                RTP_Dev.RTP_Param.yFac = (float)(BspLCD_Dev.Height - 40) / (PosTemp[2][1] - PosTemp[0][1]);       //得到yfac
                RTP_Dev.RTP_Param.yOff = (BspLCD_Dev.Height - RTP_Dev.RTP_Param.yFac * (PosTemp[2][1] + PosTemp[0][1])) / 2; //得到yoff
                if (abs(RTP_Dev.RTP_Param.yFac) > 2)                                    //触屏和预设的相反了.
                {
                    cnt = 0;
                    RTP_DrawTouchPoint(BspLCD_Dev.Width - 20, BspLCD_Dev.Height - 20, WHITE); //清除点4
                    RTP_DrawTouchPoint(20, 20, RED);                                  //画点1
                    GuiDrawStringAt("TP Need readjust!", 40, 26);
                    RTP_Dev.RTP_Param.TouchType = !RTP_Dev.RTP_Param.TouchType; //修改触屏类型.
                    if (RTP_Dev.RTP_Param.TouchType)                 //X,Y方向与屏幕相反
                    {
                        RTP_Dev.xCmd = CMD_RDY;
                        RTP_Dev.yCmd = CMD_RDX;
                    }
                    else //X,Y方向与屏幕相同
                    {
                        RTP_Dev.xCmd = CMD_RDX;
                        RTP_Dev.yCmd = CMD_RDY;
                    }
                    continue;
                }
                GuiClrScr(WHITE);       
                GuiDrawStringAt("Touch Screen Adjust OK!", 35, 110) ;            
                HAL_Delay(1000);
                RTP_SaveAdjdata();
                GuiClrScr(WHITE); //清屏
                return;           //校正完成
            }
        }
        HAL_Delay(10);
        OutTime++;
        if (OutTime > 1000)
        {
            RTP_GetAdjdata();
            break;
        }
    }
}
