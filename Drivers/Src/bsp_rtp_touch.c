/**
 ******************************************************************************
 * @file      bsp_rtp_touch.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-06-21
 * @brief     ���ݴ��������������������򣬼��������ݵĶ�ȡ�봦��У׼��������������
 *            ֱ�������Ӧ������
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

//Ĭ��Ϊtouchtype=0������.
#define CMD_RDX     0xD0
#define CMD_RDY     0x90
#define READ_TIMES  5       //��ȡ����
#define LOST_VAL    1       //����ֵ
#define ERR_RANGE   50      //��Χ

#define SAVE_ADJ_DATA_BASE_ADDR         0X20

#pragma pack(1)
struct _RTP_Param
{
    float xFac;			//������У׼����		
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

//������������
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
 * @brief   BspRTP��ʱ�����ڳ�ʼ������
 * @param   nCount ��ʱ�Ĵ�С
 * @retval  ��
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
 * @brief   ������ICд��1byte����
 * @param   Data Ҫд�������
 * @retval  ��
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
        T_CLK_WRITE_H; //��������Ч
    }
}

/**
 * @func    RTP_Read_AD
 * @brief   �Ӵ�����IC��ȡadcֵ
 * @param   Cmd ��ʱ�Ĵ�С
 * @retval  ����������
 */
static uint16_t RTP_ReadAD(uint8_t Cmd)
{
    uint8_t count = 0;
    uint16_t Data = 0;
    T_CLK_WRITE_L;      //������ʱ��
    T_DIN_WRITE_L;      //����������
    T_CS_WRITE_L;       //ѡ�д�����IC
    RTP_WriteByte(Cmd); //����������
    BspRTP_Delay(8);    //ADS7846��ת��ʱ���Ϊ6us
    T_CLK_WRITE_L;
    BspRTP_Delay(1);
    T_CLK_WRITE_H; //��1��ʱ�ӣ����BUSY
    BspRTP_Delay(1);
    T_CLK_WRITE_L;
    for (count = 0; count < 16; count++) //����16λ����,ֻ�и�12λ��Ч
    {
        Data <<= 1;
        T_CLK_WRITE_L; //�½�����Ч
        BspRTP_Delay(1);
        T_CLK_WRITE_H;
        if (T_MISO_READ)
        {
            Data++;
        }
    }
    Data >>= 4;   //ֻ�и�12λ��Ч
    T_CS_WRITE_H; //�ͷ�Ƭѡ
    return (Data);
}

/**
 * @func    RTP_Read_XOY
 * @brief   ��ȡ��������X��Yֵ
 * @param   xy ѡ���ȡ��ֵ
 * @retval  ���ض�ȡ����ֵ
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
    for (i = 0; i < READ_TIMES - 1; i++) //����
    {
        for (j = i + 1; j < READ_TIMES; j++)
        {
            if (buf[i] > buf[j]) //��������
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
 * @brief   ��ȡx,y����
 * @param   x,y:��ȡ��������ֵ
 * @retval  0,ʧ��;1,�ɹ�
 */
static uint8_t RTP_Read_xyValue(uint16_t *x, uint16_t *y)
{
    uint16_t xtemp, ytemp;
    xtemp = RTP_ReadXOY(RTP_Dev.xCmd);
    ytemp = RTP_ReadXOY(RTP_Dev.yCmd);
    if (xtemp < 100 || ytemp < 100)
        return RTP_FAULT;//����ʧ��
    *x = xtemp;
    *y = ytemp;
    return RTP_OK; //�����ɹ�
}

/**
 * @func    RTP_Read_XY2
 * @brief   ��ȡ��XYֵ
 * @param   x,y:��ȡ��������ֵ
 * @retval  0,ʧ��;1,�ɹ�
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
    if (((x2 <= x1 && x1 < x2 + ERR_RANGE) || (x1 <= x2 && x2 < x1 + ERR_RANGE)) //ǰ�����β�����+-50��
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
    if (T_PEN_READ == 0) //�а�������
    {
        if (RTP_ReadXY(&ta, &tb) != RTP_OK && (RTP_Dev.Sta & RTP_LIFT_UP)) //��ȡ��������
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
//��LCD�����йصĺ���
//��һ��������
//����У׼�õ�
//x,y:����
//color:��ɫ
static void RTP_DrawTouchPoint(uint16_t x, uint16_t y, uint16_t Color)
{
    GuiDrawLine(x - 12, y, x + 13, y, Color);  //����
    GuiDrawLine(x, y - 12, x, y + 13, Color);  //����
    // LCD_DrawPoint(x + 1, y + 1);
    // LCD_DrawPoint(x - 1, y + 1);
    // LCD_DrawPoint(x + 1, y - 1);
    // LCD_DrawPoint(x - 1, y - 1);
    GuiDrawCircle(x, y, 6, Color); //������Ȧ
}

//����У׼����
void RTP_SaveAdjdata(void)
{
    RTP_Dev.RTP_Param.AdjFlag = 0x0A;
    Bsp_eeWriteBytes((uint8_t *)&RTP_Dev.RTP_Param, SAVE_ADJ_DATA_BASE_ADDR, sizeof(struct _RTP_Param));
}
//�õ�������EEPROM�����У׼ֵ
//����ֵ��1���ɹ���ȡ����
//        0����ȡʧ�ܣ�Ҫ����У׼
uint8_t RTP_GetAdjdata(void)
{
    struct _RTP_Param TempParam;

    Bsp_eeReadBytes((uint8_t *)&TempParam, SAVE_ADJ_DATA_BASE_ADDR, sizeof(struct _RTP_Param));

    if (TempParam.AdjFlag == 0x0A)              //�������Ѿ�У׼����
    {
        RTP_Dev.RTP_Param = TempParam;

        if (RTP_Dev.RTP_Param.TouchType)        //X,Y��������Ļ�෴
        {
            RTP_Dev.xCmd = CMD_RDY;
            RTP_Dev.yCmd = CMD_RDX;
        }
        else //X,Y��������Ļ��ͬ
        {
            RTP_Dev.xCmd = CMD_RDX;
            RTP_Dev.yCmd = CMD_RDY;
        }
        return RTP_OK;
    }
    return RTP_FAULT;
}
//������У׼����
//�õ��ĸ�У׼����
void RTP_Adjust(void)
{
    uint16_t PosTemp[4][2]; //���껺��ֵ
    uint8_t cnt = 0;
    uint16_t d1, d2;
    uint32_t tem1, tem2;
    double fac;
    uint16_t OutTime = 0;
    cnt = 0;

    GuiClrScr(WHITE);

    GuiDrawStringAt("Please calibration screen!", 0, 40);
    
    RTP_DrawTouchPoint(20, 20, RED);     //����1
    RTP_Dev.RTP_Param.xFac = 0;         //xfac��������Ƿ�У׼��,����У׼֮ǰ�������!�������
    RTP_Dev.xCmd = CMD_RDX;
    RTP_Dev.yCmd = CMD_RDY;
    RTP_Dev.Sta |= RTP_LIFT_UP;
    RTP_ReadXY(&d1,&d1);//��һ�ζ�ȡ��ʼ��
    
    while (1)                           //�������10����û�а���,���Զ��˳�
    {
        if (RTP_Scan() != RTP_OK)       //ɨ����������
        {
            continue;
        }

        if (RTP_Dev.Sta & RTP_PRESS)    //����������һ��(��ʱ�����ɿ���.)
        {
            OutTime = 0;

            PosTemp[cnt][0] = RTP_Dev.CurPos.x;
            PosTemp[cnt][1] = RTP_Dev.CurPos.y;
            cnt++;
            switch (cnt)
            {
            case 1:
                RTP_DrawTouchPoint(20, 20, WHITE);              //�����1
                RTP_DrawTouchPoint(BspLCD_Dev.Width - 20, 20, RED); //����2
                break;
            case 2:
                RTP_DrawTouchPoint(BspLCD_Dev.Width - 20, 20, WHITE); //�����2
                RTP_DrawTouchPoint(20, BspLCD_Dev.Height - 20, RED);  //����3
                break;
            case 3:
                RTP_DrawTouchPoint(20, BspLCD_Dev.Height - 20, WHITE);              //�����3
                RTP_DrawTouchPoint(BspLCD_Dev.Width - 20, BspLCD_Dev.Height - 20, RED); //����4
                break;
            case 4:                                          //ȫ���ĸ����Ѿ��õ�
                                                             //�Ա����
                tem1 = abs(PosTemp[0][0] - PosTemp[1][0]); //x1-x2
                tem2 = abs(PosTemp[0][1] - PosTemp[1][1]); //y1-y2
                tem1 *= tem1;
                tem2 *= tem2;
                d1 = sqrt(tem1 + tem2); //�õ�1,2�ľ���

                tem1 = abs(PosTemp[2][0] - PosTemp[3][0]); //x3-x4
                tem2 = abs(PosTemp[2][1] - PosTemp[3][1]); //y3-y4
                tem1 *= tem1;
                tem2 *= tem2;
                d2 = sqrt(tem1 + tem2); //�õ�3,4�ľ���
                fac = (float)d1 / d2;
                if (fac < 0.95 || fac > 1.05 || d1 == 0 || d2 == 0) //���ϸ�
                {
                    cnt = 0;
                    RTP_DrawTouchPoint(BspLCD_Dev.Width - 20, BspLCD_Dev.Height - 20, WHITE);                                                                                           //�����4
                    RTP_DrawTouchPoint(20, 20, RED);                                                                                                                            //����1
                    // TP_Adj_Info_Show(PosTemp[0][0], PosTemp[0][1], PosTemp[1][0], PosTemp[1][1],
                    //                  PosTemp[2][0], PosTemp[2][1], PosTemp[3][0], PosTemp[3][1], fac * 100); //��ʾ����
                    continue;
                }
                tem1 = abs(PosTemp[0][0] - PosTemp[2][0]); //x1-x3
                tem2 = abs(PosTemp[0][1] - PosTemp[2][1]); //y1-y3
                tem1 *= tem1;
                tem2 *= tem2;
                d1 = sqrt(tem1 + tem2); //�õ�1,3�ľ���

                tem1 = abs(PosTemp[1][0] - PosTemp[3][0]); //x2-x4
                tem2 = abs(PosTemp[1][1] - PosTemp[3][1]); //y2-y4
                tem1 *= tem1;
                tem2 *= tem2;
                d2 = sqrt(tem1 + tem2); //�õ�2,4�ľ���
                fac = (float)d1 / d2;
                if (fac < 0.95 || fac > 1.05) //���ϸ�
                {
                    cnt = 0;
                    RTP_DrawTouchPoint(BspLCD_Dev.Width - 20, BspLCD_Dev.Height - 20, WHITE);                                                                                           //�����4
                    RTP_DrawTouchPoint(20, 20, RED);                                                                                                                            //����1
                    // TP_Adj_Info_Show(PosTemp[0][0], PosTemp[0][1], PosTemp[1][0], PosTemp[1][1],
                    //                  PosTemp[2][0], PosTemp[2][1], PosTemp[3][0], PosTemp[3][1], fac * 100); //��ʾ����
                    continue;
                } //��ȷ��

                //�Խ������
                tem1 = abs(PosTemp[1][0] - PosTemp[2][0]); //x1-x3
                tem2 = abs(PosTemp[1][1] - PosTemp[2][1]); //y1-y3
                tem1 *= tem1;
                tem2 *= tem2;
                d1 = sqrt(tem1 + tem2); //�õ�1,4�ľ���

                tem1 = abs(PosTemp[0][0] - PosTemp[3][0]); //x2-x4
                tem2 = abs(PosTemp[0][1] - PosTemp[3][1]); //y2-y4
                tem1 *= tem1;
                tem2 *= tem2;
                d2 = sqrt(tem1 + tem2); //�õ�2,3�ľ���
                fac = (float)d1 / d2;
                if (fac < 0.95 || fac > 1.05) //���ϸ�
                {
                    cnt = 0;
                    RTP_DrawTouchPoint(BspLCD_Dev.Width - 20, BspLCD_Dev.Height - 20, WHITE);                                                                                           //�����4
                    RTP_DrawTouchPoint(20, 20, RED);                                                                                                                            //����1
                    // TP_Adj_Info_Show(PosTemp[0][0], PosTemp[0][1], PosTemp[1][0], PosTemp[1][1],
                    //                  PosTemp[2][0], PosTemp[2][1], PosTemp[3][0], PosTemp[3][1], fac * 100); //��ʾ����
                    continue;
                } //��ȷ��
                //������
                RTP_Dev.RTP_Param.xFac = (float)(BspLCD_Dev.Width - 40) / (PosTemp[1][0] - PosTemp[0][0]);       //�õ�xfac
                RTP_Dev.RTP_Param.xOff = (BspLCD_Dev.Width - RTP_Dev.RTP_Param.xFac * (PosTemp[1][0] + PosTemp[0][0])) / 2; //�õ�xoff

                RTP_Dev.RTP_Param.yFac = (float)(BspLCD_Dev.Height - 40) / (PosTemp[2][1] - PosTemp[0][1]);       //�õ�yfac
                RTP_Dev.RTP_Param.yOff = (BspLCD_Dev.Height - RTP_Dev.RTP_Param.yFac * (PosTemp[2][1] + PosTemp[0][1])) / 2; //�õ�yoff
                if (abs(RTP_Dev.RTP_Param.yFac) > 2)                                    //������Ԥ����෴��.
                {
                    cnt = 0;
                    RTP_DrawTouchPoint(BspLCD_Dev.Width - 20, BspLCD_Dev.Height - 20, WHITE); //�����4
                    RTP_DrawTouchPoint(20, 20, RED);                                  //����1
                    GuiDrawStringAt("TP Need readjust!", 40, 26);
                    RTP_Dev.RTP_Param.TouchType = !RTP_Dev.RTP_Param.TouchType; //�޸Ĵ�������.
                    if (RTP_Dev.RTP_Param.TouchType)                 //X,Y��������Ļ�෴
                    {
                        RTP_Dev.xCmd = CMD_RDY;
                        RTP_Dev.yCmd = CMD_RDX;
                    }
                    else //X,Y��������Ļ��ͬ
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
                GuiClrScr(WHITE); //����
                return;           //У�����
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
