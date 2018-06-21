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
#define CMD_RDX 0xD0
#define CMD_RDY 0x90
#define READ_TIMES 5 //读取次数
#define LOST_VAL 1   //丢弃值
#define ERR_RANGE 50 //误差范围

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
static uint16_t RTP_Read_AD(uint8_t Cmd)
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
static uint16_t RTP_Read_XOY(uint8_t xy)
{
    uint16_t i, j;
    uint16_t buf[READ_TIMES];
    uint16_t sum = 0;
    uint16_t temp;
    for (i = 0; i < READ_TIMES; i++)
    {
        buf[i] = RTP_Read_AD(xy);
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
static uint8_t RTP_Read_XY(uint16_t *x, uint16_t *y)
{
    uint16_t xtemp, ytemp;
    xtemp = RTP_Read_XOY(CMD_RDX);
    ytemp = RTP_Read_XOY(CMD_RDY);
    //	if (xtemp < 100 || ytemp < 100)
    //        return 0;//读数失败
    *x = xtemp;
    *y = ytemp;
    return 1; //读数成功
}

/**
 * @func    RTP_Read_XY2
 * @brief   读取到XY值
 * @param   x,y:读取到的坐标值
 * @retval  0,失败;1,成功
 */
uint8_t RTP_Read_XY2(uint16_t *x, uint16_t *y)
{
    uint16_t x1, y1;
    uint16_t x2, y2;
    uint8_t flag;
    flag = RTP_Read_XY(&x1, &y1);
    if (flag == 0)
    {
        return (0);
    }
    flag = RTP_Read_XY(&x2, &y2);

    if (flag == 0)
    {
        return (0);
    }
    if (((x2 <= x1 && x1 < x2 + ERR_RANGE) || (x1 <= x2 && x2 < x1 + ERR_RANGE)) //前后两次采样在+-50内
        && ((y2 <= y1 && y1 < y2 + ERR_RANGE) || (y1 <= y2 && y2 < y1 + ERR_RANGE)))
    {
        *x = (x1 + x2) / 2;
        *y = (y1 + y2) / 2;
        return 1;
    }
    else
    {
        return 0;
    }
}

uint16_t ta, tb;
uint8_t RTP_Scan(void)
{
    if (T_PEN_READ == 0) //有按键按下
    {
        RTP_Read_XY2(&ta, &tb); //读取物理坐标
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////
//与LCD部分有关的函数
//画一个触摸点
//用来校准用的
//x,y:坐标
//color:颜色
static void RTP_DrowTouchPoint(uint16_t x, uint16_t y, uint16_t Color)
{
    POINT_COLOR = color;
    LCD_DrawLine(x - 12, y, x + 13, y); //横线
    LCD_DrawLine(x, y - 12, x, y + 13); //竖线
    LCD_DrawPoint(x + 1, y + 1);
    LCD_DrawPoint(x - 1, y + 1);
    LCD_DrawPoint(x + 1, y - 1);
    LCD_DrawPoint(x - 1, y - 1);
    LCD_Draw_Circle(x, y, 6); //画中心圈
}

//保存校准参数
void RTP_SaveAdjdata(void)
{
    uint32_t temp;
    //保存校正结果!
    temp = tp_dev.xfac * 100000000; //保存x校正因素
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE, temp, 4);
    temp = tp_dev.yfac * 100000000; //保存y校正因素
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE + 4, temp, 4);
    //保存x偏移量
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE + 8, tp_dev.xoff, 2);
    //保存y偏移量
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE + 10, tp_dev.yoff, 2);
    //保存触屏类型
    AT24CXX_WriteOneByte(SAVE_ADDR_BASE + 12, tp_dev.touchtype);
    temp = 0X0A; //标记校准过了
    AT24CXX_WriteOneByte(SAVE_ADDR_BASE + 13, temp);
}
//得到保存在EEPROM里面的校准值
//返回值：1，成功获取数据
//        0，获取失败，要重新校准
uint8_t RTP_GetAdjdata(void)
{
    uint32_t tempfac;
    tempfac = AT24CXX_ReadOneByte(SAVE_ADDR_BASE + 13); //读取标记字,看是否校准过！
    if (tempfac == 0X0A)                                //触摸屏已经校准过了
    {
        tempfac = AT24CXX_ReadLenByte(SAVE_ADDR_BASE, 4);
        tp_dev.xfac = (float)tempfac / 100000000; //得到x校准参数
        tempfac = AT24CXX_ReadLenByte(SAVE_ADDR_BASE + 4, 4);
        tp_dev.yfac = (float)tempfac / 100000000; //得到y校准参数
                                                  //得到x偏移量
        tp_dev.xoff = AT24CXX_ReadLenByte(SAVE_ADDR_BASE + 8, 2);
        //得到y偏移量
        tp_dev.yoff = AT24CXX_ReadLenByte(SAVE_ADDR_BASE + 10, 2);
        tp_dev.touchtype = AT24CXX_ReadOneByte(SAVE_ADDR_BASE + 12); //读取触屏类型标记
        if (tp_dev.touchtype)                                        //X,Y方向与屏幕相反
        {
            CMD_RDX = 0X90;
            CMD_RDY = 0XD0;
        }
        else //X,Y方向与屏幕相同
        {
            CMD_RDX = 0XD0;
            CMD_RDY = 0X90;
        }
        return 1;
    }
    return 0;
}
//触摸屏校准代码
//得到四个校准参数
void RTP_Adjust(void)
{
    uint16_t posTemp[4][2]; //坐标缓存值
    uint8_t cnt = 0;
    uint16_t d1, d2;
    uint32_t tem1, tem2;
    double fac;
    uint16_t OutTime = 0;
    cnt = 0;
    POINT_COLOR = BLUE;
    BACK_COLOR = WHITE;
    BspLCD.ClrScr(WHITE); //清屏
    POINT_COLOR = RED;    //红色
    LCD_Clear(WHITE);     //清屏
    POINT_COLOR = BLACK;
    LCD_ShowString(40, 40, 160, 100, 16, (u8 *)TP_REMIND_MSG_TBL); //显示提示信息
    TP_Drow_Touch_Point(20, 20, RED);                              //画点1
    tp_dev.sta = 0;                                                //消除触发信号
    tp_dev.xfac = 0;                                               //xfac用来标记是否校准过,所以校准之前必须清掉!以免错误
    while (1)                                                      //如果连续10秒钟没有按下,则自动退出
    {
        tp_dev.scan(1);                          //扫描物理坐标
        if ((tp_dev.sta & 0xc0) == TP_CATH_PRES) //按键按下了一次(此时按键松开了.)
        {
            outtime = 0;
            tp_dev.sta &= ~(1 << 6); //标记按键已经被处理过了.

            pos_temp[cnt][0] = tp_dev.x[0];
            pos_temp[cnt][1] = tp_dev.y[0];
            cnt++;
            switch (cnt)
            {
            case 1:
                TP_Drow_Touch_Point(20, 20, WHITE);              //清除点1
                TP_Drow_Touch_Point(lcddev.width - 20, 20, RED); //画点2
                break;
            case 2:
                TP_Drow_Touch_Point(lcddev.width - 20, 20, WHITE); //清除点2
                TP_Drow_Touch_Point(20, lcddev.height - 20, RED);  //画点3
                break;
            case 3:
                TP_Drow_Touch_Point(20, lcddev.height - 20, WHITE);              //清除点3
                TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, RED); //画点4
                break;
            case 4:                                          //全部四个点已经得到
                                                             //对边相等
                tem1 = abs(pos_temp[0][0] - pos_temp[1][0]); //x1-x2
                tem2 = abs(pos_temp[0][1] - pos_temp[1][1]); //y1-y2
                tem1 *= tem1;
                tem2 *= tem2;
                d1 = sqrt(tem1 + tem2); //得到1,2的距离

                tem1 = abs(pos_temp[2][0] - pos_temp[3][0]); //x3-x4
                tem2 = abs(pos_temp[2][1] - pos_temp[3][1]); //y3-y4
                tem1 *= tem1;
                tem2 *= tem2;
                d2 = sqrt(tem1 + tem2); //得到3,4的距离
                fac = (float)d1 / d2;
                if (fac < 0.95 || fac > 1.05 || d1 == 0 || d2 == 0) //不合格
                {
                    cnt = 0;
                    TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE);                                                                                           //清除点4
                    TP_Drow_Touch_Point(20, 20, RED);                                                                                                                            //画点1
                    TP_Adj_Info_Show(pos_temp[0][0], pos_temp[0][1], pos_temp[1][0], pos_temp[1][1],
                                     pos_temp[2][0], pos_temp[2][1], pos_temp[3][0], pos_temp[3][1], fac * 100); //显示数据
                    continue;
                }
                tem1 = abs(pos_temp[0][0] - pos_temp[2][0]); //x1-x3
                tem2 = abs(pos_temp[0][1] - pos_temp[2][1]); //y1-y3
                tem1 *= tem1;
                tem2 *= tem2;
                d1 = sqrt(tem1 + tem2); //得到1,3的距离

                tem1 = abs(pos_temp[1][0] - pos_temp[3][0]); //x2-x4
                tem2 = abs(pos_temp[1][1] - pos_temp[3][1]); //y2-y4
                tem1 *= tem1;
                tem2 *= tem2;
                d2 = sqrt(tem1 + tem2); //得到2,4的距离
                fac = (float)d1 / d2;
                if (fac < 0.95 || fac > 1.05) //不合格
                {
                    cnt = 0;
                    TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE);                                                                                           //清除点4
                    TP_Drow_Touch_Point(20, 20, RED);                                                                                                                            //画点1
                    TP_Adj_Info_Show(pos_temp[0][0], pos_temp[0][1], pos_temp[1][0], pos_temp[1][1],
                                     pos_temp[2][0], pos_temp[2][1], pos_temp[3][0], pos_temp[3][1], fac * 100); //显示数据
                    continue;
                } //正确了

                //对角线相等
                tem1 = abs(pos_temp[1][0] - pos_temp[2][0]); //x1-x3
                tem2 = abs(pos_temp[1][1] - pos_temp[2][1]); //y1-y3
                tem1 *= tem1;
                tem2 *= tem2;
                d1 = sqrt(tem1 + tem2); //得到1,4的距离

                tem1 = abs(pos_temp[0][0] - pos_temp[3][0]); //x2-x4
                tem2 = abs(pos_temp[0][1] - pos_temp[3][1]); //y2-y4
                tem1 *= tem1;
                tem2 *= tem2;
                d2 = sqrt(tem1 + tem2); //得到2,3的距离
                fac = (float)d1 / d2;
                if (fac < 0.95 || fac > 1.05) //不合格
                {
                    cnt = 0;
                    TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE);                                                                                           //清除点4
                    TP_Drow_Touch_Point(20, 20, RED);                                                                                                                            //画点1
                    TP_Adj_Info_Show(pos_temp[0][0], pos_temp[0][1], pos_temp[1][0], pos_temp[1][1],
                                     pos_temp[2][0], pos_temp[2][1], pos_temp[3][0], pos_temp[3][1], fac * 100); //显示数据
                    continue;
                } //正确了
                //计算结果
                tp_dev.xfac = (float)(lcddev.width - 40) / (pos_temp[1][0] - pos_temp[0][0]);       //得到xfac
                tp_dev.xoff = (lcddev.width - tp_dev.xfac * (pos_temp[1][0] + pos_temp[0][0])) / 2; //得到xoff

                tp_dev.yfac = (float)(lcddev.height - 40) / (pos_temp[2][1] - pos_temp[0][1]);       //得到yfac
                tp_dev.yoff = (lcddev.height - tp_dev.yfac * (pos_temp[2][1] + pos_temp[0][1])) / 2; //得到yoff
                if (abs(tp_dev.xfac) > 2 || abs(tp_dev.yfac) > 2)                                    //触屏和预设的相反了.
                {
                    cnt = 0;
                    TP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE); //清除点4
                    TP_Drow_Touch_Point(20, 20, RED);                                  //画点1
                    LCD_ShowString(40, 26, lcddev.width, lcddev.height, 16, "TP Need readjust!");
                    tp_dev.touchtype = !tp_dev.touchtype; //修改触屏类型.
                    if (tp_dev.touchtype)                 //X,Y方向与屏幕相反
                    {
                        CMD_RDX = 0X90;
                        CMD_RDY = 0XD0;
                    }
                    else //X,Y方向与屏幕相同
                    {
                        CMD_RDX = 0XD0;
                        CMD_RDY = 0X90;
                    }
                    continue;
                }
                POINT_COLOR = BLUE;
                LCD_Clear(WHITE);                                                                    //清屏
                LCD_ShowString(35, 110, lcddev.width, lcddev.height, 16, "Touch Screen Adjust OK!"); //校正完成
                delay_ms(1000);
                TP_Save_Adjdata();
                LCD_Clear(WHITE); //清屏
                return;           //校正完成
            }
        }
        delay_ms(10);
        outtime++;
        if (outtime > 1000)
        {
            TP_Get_Adjdata();
            break;
        }
    }
}
