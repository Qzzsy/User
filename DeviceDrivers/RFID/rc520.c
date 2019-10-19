/******************************************************************************
  * @文件名     ： rc520.c
  * @日期       ： 2016年06月05日
  * @摘要       ： RFID刷卡验证
  ******************************************************************************/
#include "rc520.h"
#include "iso14443b.h"
#include "cpucard.h"
//#include "app.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"

char pcd_config(uint8_t type);
extern uint8_t get_info_form_card_2(uint8_t *sbuf, uint8_t *ebuf);
extern void Card_Open_Clock(void);
extern void Card_Test_Open(void);

uint8_t Card_Sector = 0;      //卡扇区号，需要保存
uint8_t Card_Group = 0;       //卡组号，需要保存
uint8_t Engine_Card_Flag = 0; //工程卡有效标志
uint8_t STORE_ACT_INFO[5];    //存储的房东信息
uint8_t STORE_PWD_INFO[2];    //存储的账户密码
uint8_t Data_Init_Flag = 0;   //清除卡片数据标志
uint8_t exti6_flag = 0;
uint8_t exti6_read_flag = 0;
uint8_t RFID_to_TOUCH_Flag = 0;

static unsigned char MRcvBuffer[72], MSndBuffer[72];
static MfCmdInfo MInfo;
static unsigned char g_cCidNad = 0;

//--------------------------------------控制电源控制----------------------------------------
void open_cv520(void)
{

    //		GPIO_ResetBits(GPIOA, GPIO_Pin_8);  //打开控制电源  PA1
    //		rc520_init();
}

//-----------------------------------IO配置-------------------------------------------
//void rc520_gpio_init(void)
//{

//    GPIO_InitTypeDef GPIO_InitStructure;

//    RCC_AHBPeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;        //CS
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //高速输出
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //输出模式
//    GPIO_Init(GPIOA, &GPIO_InitStructure);

//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5; //SCK-MOSI
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      //高速输出
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       //输出模式
//    GPIO_Init(GPIOB, &GPIO_InitStructure);

//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; //MISO
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//    GPIO_Init(GPIOB, &GPIO_InitStructure);

//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //IRQ
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//    GPIO_Init(GPIOB, &GPIO_InitStructure);
//}
//-----------------------------------RFID初始化-------------------------------------------
void rc520_init(void)
{
    DEBUG("reset rc520");
    PcdReset();

    PcdAntennaOff();
    delay_ms(10);

    DEBUG("config pcd for A mode");
    pcd_config('A'); //配置为A模式
    PcdAntennaOn();  //开启天线

    //		 pcd_lpcd_start();
}

//低功耗模式

/*
	lpcd功能开始函数
*/

#define write_reg WriteRawRC

#define read_reg ReadRawRC

#define clear_bit_mask ClearBitMask

//#define BIT0 0x01
//#define BIT1 0x02
//#define BIT2 0x04
//#define BIT3 0x08
//#define BIT4 0x10
//#define BIT5 0x20
//#define BIT6 0x40
//#define BIT7 0x80

//------------------------------开启LPCD休眠模式------------------------------
void pcd_lpcd_start()
{

    DEBUG("pcd_lpcd_start\n");

    write_reg(0x01, 0x0F); //先复位寄存器再进行lpcd

    write_reg(0x37, 0x00); //恢复版本号
    if (read_reg(0x37) == 0x10)
    {                          //NFC_V10
        write_reg(0x01, 0x00); // idle
    }
    write_reg(0x14, 0x8B); // Tx2CW = 1 ，continue载波发射打开

    write_reg(0x37, 0x5e); // 打开私有寄存器保护开关

    //write_reg(0x3c, 0x30);	//设置Delta[3:0]的值, 开启32k //0 不能使用
    //write_reg(0x3c, 0x31);	//设置Delta[3:0]的值, 开启32k
    //write_reg(0x3c, 0x32);	//设置Delta[3:0]的值, 开启32k
    //write_reg(0x3c, 0x33);	//设置Delta[3:0]的值, 开启32k
    //	write_reg(0x3c, 0x35);	//设置Delta[3:0]的值, 开启32k
    //	write_reg(0x3c, 0x35);	//设置Delta[3:0]的值, 开启32k XU
    //	write_reg(0x3c, 0x37);	//设置Delta[3:0]的值, 开启32k XU
    //write_reg(0x3c, 0x3A);	//设置Delta[3:0]的值, 开启32k XU
    //write_reg(0x3c, 0x3F);	//设置Delta[3:0]的值, 开启32k XU

#if Antenna_Model > 0
    write_reg(0x3c, 0x37); //设置Delta[3:0]的值, 开启32k
#else
    write_reg(0x3c, 0x35); //设置Delta[3:0]的值, 开启32k
#endif

    write_reg(0x3d, 0x0d); //设置休眠时间
    write_reg(0x3e, 0xA5); //设置连续探测次数 3次，开启LPCD_en
    write_reg(0x37, 0x00); // 关闭私有寄存器保护开关
    write_reg(0x03, 0x20); //打开卡探测中断使能
    write_reg(0x01, 0x10); //PCD soft powerdown

    //具体应用相关，配置为高电平为有中断
    clear_bit_mask(0x02, BIT7);
}

//-----------------------------关闭LPCD------------------------------
void pcd_lpcd_end()
{

    DEBUG("pcd_lpcd_end\n");
    write_reg(0x01, 0x0F); //先复位寄存器再进行lpcd
}

uint8_t pcd_lpcd_check()
{
    //if (INT_PIN && (read_reg(DivIrqReg) & BIT5)) //TagDetIrq
    //if ((read_reg(DivIrqReg) & BIT5))
    if (INT_PIN) //TagDetIrq
    {

        write_reg(DivIrqReg, BIT5); //清除卡检测到中断
        pcd_lpcd_end();
        return 1;
    }
    return 0;
}

void clear_nfc_flag(void)
{

    write_reg(DivIrqReg, BIT5); //清除卡检测到中断
}

//**********************************************************

/////////////////////////////////////////////////////////////////////
//功    能：读RC632寄存器
//参数说明：Address[IN]:寄存器地址
//返    回：读出的值
/////////////////////////////////////////////////////////////////////
unsigned char ReadRawRC(unsigned char Address)
{
    unsigned char i, ucAddr;
    unsigned char ucResult = 0;

    MF522_SCK_L;
    MF522_NSS_L;
    ucAddr = ((Address << 1) & 0x7E) | 0x80;

    for (i = 8; i > 0; i--)
    {
        //  MF522_SI = ((ucAddr&0x80)==0x80);

        if ((ucAddr & 0x80) == 0x80)
            MF522_SI_H;
        else
            MF522_SI_L;

        MF522_SCK_H;
        ucAddr <<= 1;
        MF522_SCK_L;
    }

    for (i = 8; i > 0; i--)
    {
        MF522_SCK_H;
        ucResult <<= 1;
        // ucResult|=(bit)MF522_SO;

        ucResult |= MF522_SO;

        MF522_SCK_L;
    }

    MF522_NSS_H;
    MF522_SCK_H;
    return ucResult;
}

/////////////////////////////////////////////////////////////////////
//功    能：写RC632寄存器
//参数说明：Address[IN]:寄存器地址
//          value[IN]:写入的值
/////////////////////////////////////////////////////////////////////
void WriteRawRC(unsigned char Address, unsigned char value)
{
    unsigned char i, ucAddr;

    MF522_SCK_L;
    MF522_NSS_L;
    ucAddr = ((Address << 1) & 0x7E);

    for (i = 8; i > 0; i--)
    {
        //   MF522_SI = ((ucAddr&0x80)==0x80);
        if ((ucAddr & 0x80) == 0x80)
            MF522_SI_H;
        else
            MF522_SI_L;

        MF522_SCK_H;
        ucAddr <<= 1;
        MF522_SCK_L;
    }

    for (i = 8; i > 0; i--)
    {
        //   MF522_SI = ((value&0x80)==0x80);
        if ((value & 0x80) == 0x80)
            MF522_SI_H;
        else
            MF522_SI_L;

        MF522_SCK_H;
        value <<= 1;
        MF522_SCK_L;
    }
    MF522_NSS_H;
    MF522_SCK_H;
}

/////////////////////////////////////////////////////////////////////
//功    能：置RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:置位值
/////////////////////////////////////////////////////////////////////
void SetBitMask(unsigned char reg, unsigned char mask)
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp | mask); // set bit mask
}

void set_bit_mask(unsigned char reg, unsigned char mask)
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp | mask); // set bit mask
}
/////////////////////////////////////////////////////////////////////
//功    能：清RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:清位值
/////////////////////////////////////////////////////////////////////
void ClearBitMask(unsigned char reg, unsigned char mask)
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask); // clear bit mask
}

/////////////////////////////////////////////////////////////////////
//功    能：复位RC522
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdReset(void)
{
    uint32_t i = 0;

    MF522_NSS_H;

    MF522_RST_H;
    delay_ms(1);

    MF522_RST_L;
    delay_ms(1);

    MF522_RST_H;

    WriteRawRC(CommandReg, PCD_RESETPHASE);

    delay_ms(1);

    i = ReadRawRC(0x37);
    DEBUG("version: 0x%02x", i);

    while (ReadRawRC(0x27) != 0x88)
        ; //判断NFC是否启动完成

#if NFC_LPCD < 1
    WriteRawRC(ModeReg, 0x3D); //和Mifare卡通讯，CRC初始值0x6363
    WriteRawRC(TReloadRegL, 30);
    WriteRawRC(TReloadRegH, 0);
    WriteRawRC(TModeReg, 0x8D);
    WriteRawRC(TPrescalerReg, 0x3E);
    WriteRawRC(TxAskReg, 0x40); //TxAutoReg
#endif

    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//开启天线
//每次启动或关闭天险发射之间应至少有1ms的间隔
/////////////////////////////////////////////////////////////////////
void PcdAntennaOn()
{
    unsigned char i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}

/////////////////////////////////////////////////////////////////////
//关闭天线
/////////////////////////////////////////////////////////////////////
void PcdAntennaOff()
{
    ClearBitMask(TxControlReg, 0x03);
}
void pcd_antenna_off()
{
    write_reg(TxControlReg, read_reg(TxControlReg) & (~0x03));
}

//////////////////////////////////////////////////////////////////////
//设置RC522的工作方式
//////////////////////////////////////////////////////////////////////
void M500PcdConfigISOTypeA(void) //ISO14443_A//A:NXP,B:MOTO
{
    ClearBitMask(Status2Reg, 0x08); //清除寄存器
    WriteRawRC(ModeReg, 0x3D);      //3F 选择模式
    WriteRawRC(RxSelReg, 0x86);     //84  内部接收设置
    WriteRawRC(RFCfgReg, 0x7F);     //4F  接收增益

    WriteRawRC(TReloadRegL, 30); //tmoLength);// TReloadVal = 'h6a =tmoLength(dec)
    WriteRawRC(TReloadRegH, 0);
    WriteRawRC(TModeReg, 0x8D);
    WriteRawRC(TPrescalerReg, 0x3E);

    delay_ms(4000);
    PcdAntennaOn();
}

/**
 ****************************************************************
 * @brief pcd_config() 
 *
 * 配置芯片的A/B模式
 *
 * @param: uint8_t type   
 * @return: 
 * @retval: 

    WriteRawRC(ModeReg,0x3D);            //和Mifare卡通讯，CRC初始值0x6363
    WriteRawRC(TReloadRegL,30);           
    WriteRawRC(TReloadRegH,0);
    WriteRawRC(TModeReg,0x8D);
    WriteRawRC(TPrescalerReg,0x3E);
    WriteRawRC(TxAskReg,0x40);

 ****************************************************************
 */

//初始化配置

char pcd_config(uint8_t type)
{
    PcdAntennaOff();

    delay_ms(7);

    if ('A' == type)
    {
        clear_bit_mask(Status2Reg, BIT3);
        clear_bit_mask(ComIEnReg, BIT7); // 高电平
        write_reg(ModeReg, 0x3D);        // 11 // CRC seed:6363
        write_reg(RxSelReg, 0x86);       //RxWait
        write_reg(RFCfgReg, 0x58);       //
        write_reg(TxAskReg, 0x40);       //15  //typeA
        write_reg(TxModeReg, 0x00);      //12 //Tx Framing A
        write_reg(RxModeReg, 0x00);      //13 //Rx framing A
        write_reg(0x0C, 0x10);           //^_^

        //兼容配置
        {
            uint8_t backup, adc;
            backup = read_reg(0x37);
            write_reg(0x37, 0x00);
            adc = read_reg(0x37);

            if (adc == 0x11)
            {
                WriteRawRC(0x26, 0x48);
                WriteRawRC(0x37, 0x5E);
                WriteRawRC(0x17, 0x88);
                WriteRawRC(0x38, 0x6B);
                WriteRawRC(0x3A, 0x23);
                WriteRawRC(0x29, 0x12); //0x0F); //调制指数
                WriteRawRC(0x3B, 0xA5);
            }
            else if (adc == 0x12)
            {
                //以下寄存器需要按顺序配置
                WriteRawRC(0x37, 0x5E);
                WriteRawRC(0x26, 0x48);
                WriteRawRC(0x17, 0x88);
                WriteRawRC(0x29, 0x12); //0x0F); //
                WriteRawRC(0x35, 0xED);
                WriteRawRC(0x3b, 0xA5);
                WriteRawRC(0x37, 0xAE);
                WriteRawRC(0x3b, 0x72);
            }
            else if (adc == 0x15)
            {
                //以下寄存器需要按顺序配置
                WriteRawRC(0x37, 0x5E);
                WriteRawRC(0x26, 0x48);
                WriteRawRC(0x17, 0x88);
                WriteRawRC(0x29, 0x12); //0x0F); //
                WriteRawRC(0x35, 0xAC);
                WriteRawRC(0x3b, 0xA5);
                WriteRawRC(0x37, 0xAE);
                WriteRawRC(0x3b, 0x72);
                WriteRawRC(0x31, 0x54);
            }
            write_reg(0x37, backup);
        }
    }
    else if ('B' == type)
    {
        write_reg(Status2Reg, 0x00);     //清MFCrypto1On
        clear_bit_mask(ComIEnReg, BIT7); // 高电平触发中断
        write_reg(ModeReg, 0x3F);        // CRC seed:FFFF
        write_reg(RxSelReg, 0x85);       //RxWait
        write_reg(RFCfgReg, 0x58);       //
        //Tx
        write_reg(GsNReg, 0xF8);    //调制系数
        write_reg(CWGsPReg, 0x3F);  //
        write_reg(ModGsPReg, 0x17); //调制指数		//0xOE
        write_reg(AutoTestReg, 0x00);
        write_reg(TxAskReg, 0x00); // typeB
        write_reg(RFU1E, 0x13);
        write_reg(TxModeReg, 0x83);     //Tx Framing B
        write_reg(RxModeReg, 0x83);     //Rx framing B
        write_reg(BitFramingReg, 0x00); //TxLastBits=0

        //兼容配置
        {
            uint8_t backup, adc;
            backup = read_reg(0x37);
            write_reg(0x37, 0x00);
            adc = read_reg(0x37);

            //			if (adc == 0x11)
            //			{
            //				write_reg(0x37, 0x5E);
            //				write_reg(0x17, 0x88);
            //				write_reg(0x3A, 0x23);
            //				write_reg(0x38, 0x6B);
            //				write_reg(0x29, 0x12);//0x0F); //调制指数
            //				write_reg(0x3b, 0x25);
            //			}
            //			else if (adc == 0x12)
            {
                //以下寄存器需要按顺序配置
                write_reg(0x37, 0x5E);
                write_reg(0x26, 0x48);
                write_reg(0x17, 0x88);
                write_reg(0x29, 0x17); //0x14
                write_reg(0x35, 0xAC);
                write_reg(0x3b, 0xA5);
                write_reg(0x37, 0xAE);
                write_reg(0x3b, 0x72);
                write_reg(0x31, 0x54);
            }
            write_reg(0x37, backup);
        }
    }
    else
    {
        return USER_ERROR;
    }

    //pcd_antenna_on();
    PcdAntennaOn();
    delay_ms(2);

    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//用MF522计算CRC16函数
/////////////////////////////////////////////////////////////////////
void CalulateCRC(unsigned char *pIndata, unsigned char len, unsigned char *pOutData)
{
    unsigned char i, n;
    ClearBitMask(DivIrqReg, 0x04);
    WriteRawRC(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);
    for (i = 0; i < len; i++)
    {
        WriteRawRC(FIFODataReg, *(pIndata + i));
    }
    WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x04));
    pOutData[0] = ReadRawRC(CRCResultRegL);
    pOutData[1] = ReadRawRC(CRCResultRegM);
}

/////////////////////////////////////////////////////////////////////
//设置PCD定时器
//input:fwi=0~15
/////////////////////////////////////////////////////////////////////
void pcd_set_tmo(uint8_t fwi)

{
    write_reg(TPrescalerReg, (TP_FWT_302us)&0xFF);
    write_reg(TModeReg, BIT7 | (((TP_FWT_302us) >> 8) & 0xFF));

    write_reg(TReloadRegL, (1 << fwi) & 0xFF);
    write_reg(TReloadRegH, ((1 << fwi) & 0xFF00) >> 8);
}

//-----------------------------------------------------------------------
unsigned char PcdFastSearchCard(void)
{
    unsigned char irqEn = 0x77;
    unsigned char errmask = 0x30;
    unsigned char errflag;
    unsigned int times;
#if 1
    //RC522_SPI_TRIS;

    MF522_RST_H;
    delay_ms(50);
    MF522_RST_L; //载波消失
    delay_ms(20);
    MF522_RST_H;

    delay_ms(20);
// GPIO_Init(GPIOC, GPIO_PIN_7, GPIO_MODE_IN_FL_NO_IT);
//while(ReadRawRC(0x27) != 0x88);
#endif
    delay_ms(4000);
    ; //delay 500us 请客户确认delayus函数延时准确。

    WriteRawRC(ModeReg, 0x3D);       //和Mifare卡通讯，CRC初始值0x6363
    WriteRawRC(TxAskReg, 0x40);      //100%ASK调制
    WriteRawRC(BitFramingReg, 0x07); //

    // WriteRawRC(TxControlReg,0x83); //T1，T2发送经过连续调制的载波。出现载波0X83

    WriteRawRC(ComIEnReg, irqEn | 0x80); //
    WriteRawRC(ComIrqReg, 0x14);         //
    WriteRawRC(CommandReg, PCD_IDLE);    //
    WriteRawRC(FIFOLevelReg, 0x80);      //

    // WriteRawRC(FIFODataReg,PICC_REQIDL); //先将数据写入fifo  0X26
    WriteRawRC(0X09, 0X26); //先将数据写入fifo  0X26
    //WriteRawRC(CommandReg, PCD_TRANSCEIVE);//执行操作命令写入卡中
    WriteRawRC(TxControlReg, 0x83); //T1，T2发送经过连续调制的载波。出现载波0X83
    WriteRawRC(0x01, 0x0C);         //执行操作命令写入卡中

    //WriteRawRC(BitFramingReg,0x87);//启动数据发送
    WriteRawRC(0x0D, 0x87); //启动数据发送

    //GPIOC->ODR |= 0x10;
    times = 18;
    do
    {
        errflag = ReadRawRC(ComIrqReg); //判断是否有中断发生
        times--;
    } while ((times != 0) && !(errflag & errmask));
    //  GPIOC->ODR &= 0xef;

    if (times == 0)
    {
#if 1
        // CLR_RC522RST;
#else
                           //  WriteRawRC(TxControlReg,0x80); //
#endif
        return MI_ERR;
    }

    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//设置PCD速率
//
/////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void pcd_set_rate(uint8_t rate)
{
    uint8_t val, rxwait;
    switch (rate)
    {
    case '1':
        clear_bit_mask(TxModeReg, BIT4 | BIT5 | BIT6);
        clear_bit_mask(RxModeReg, BIT4 | BIT5 | BIT6);
        write_reg(ModWidthReg, 0x26); //Miller Pulse Length

        write_reg(RxSelReg, 0x88);

        break;

    case '2':
        clear_bit_mask(TxModeReg, BIT4 | BIT5 | BIT6);
        set_bit_mask(TxModeReg, BIT4);
        clear_bit_mask(RxModeReg, BIT4 | BIT5 | BIT6);
        set_bit_mask(RxModeReg, BIT4);
        write_reg(ModWidthReg, 0x12); //Miller Pulse Length
        //rxwait相对于106基本速率需增加相应倍数
        val = read_reg(RxSelReg);
        rxwait = ((val & 0x3F) * 2);
        if (rxwait > 0x3F)
        {
            rxwait = 0x3F;
        }
        write_reg(RxSelReg, (rxwait | (val & 0xC0)));

        break;

    case '4':
        clear_bit_mask(TxModeReg, BIT4 | BIT5 | BIT6);
        set_bit_mask(TxModeReg, BIT5);
        clear_bit_mask(RxModeReg, BIT4 | BIT5 | BIT6);
        set_bit_mask(RxModeReg, BIT5);
        write_reg(ModWidthReg, 0x0A); //Miller Pulse Length
        //rxwait相对于106基本速率需增加相应倍数
        val = read_reg(RxSelReg);
        rxwait = ((val & 0x3F) * 4);
        if (rxwait > 0x3F)
        {
            rxwait = 0x3F;
        }
        write_reg(RxSelReg, (rxwait | (val & 0xC0)));

        break;

    default:
        clear_bit_mask(TxModeReg, BIT4 | BIT5 | BIT6);
        clear_bit_mask(RxModeReg, BIT4 | BIT5 | BIT6);
        write_reg(ModWidthReg, 0x26); //Miller Pulse Length

        break;
    }
}

//-------------------------------------询卡-----------------------------------------
int g_statistic_refreshed;
statistics_t g_statistics;

//char com_reqa(uint8_t *pcmd)
//{
//	int status;
//	uint8_t sak,i,j;
//	uint8_t  buf[10];
//
//	g_statistics.reqa_cnt++;
//	g_statistic_refreshed=TRUE;

//	pcd_default_info();

//	status = pcd_request(pcmd[1], g_tag_info.tag_type_bytes);
//	//一次防冲突及选卡
//	if (status == MI_OK)
//	{
//		g_tag_info.uid_length = UID_4;
//		//make_packet(COM_PKT_CMD_CARD_TYPE, g_tag_info.tag_type_bytes, sizeof(g_tag_info.tag_type_bytes));
//
//		status = pcd_cascaded_anticoll(PICC_ANTICOLL1, 0, &g_tag_info.serial_num[0]);
//		if (status == MI_OK)
//		{
//			status = pcd_cascaded_select(PICC_ANTICOLL1, &g_tag_info.serial_num[0], &sak);
//		}
//	}
//	//二次防冲突及选卡
//	if(status == MI_OK && (sak & BIT2))
//	{
//		g_tag_info.uid_length = UID_7;
//		status = pcd_cascaded_anticoll(PICC_ANTICOLL2, 0, &g_tag_info.serial_num[4]);
//		if(status == MI_OK)
//		{
//			status = pcd_cascaded_select(PICC_ANTICOLL2, &g_tag_info.serial_num[4], &sak);
//		}
//	}
//	//回复uid
//	if (status == MI_OK)
//	{
//		buf[0] = g_tag_info.uid_length;
//		memcpy(buf+1, (g_tag_info.uid_length == UID_4 ? &g_tag_info.serial_num[0]:&g_tag_info.serial_num[1]), g_tag_info.uid_length);
//
//		//make_packet(COM_PKT_CMD_REQA, buf, g_tag_info.uid_length + 1);
//	}

//	if(status == MI_OK)
//	{        j=g_tag_info.uid_length;
//
//             LED_GREEN_ON;
//	       for(i=0;i<j+1;i++)
//         	{
//             UART1_send_data(buf[i]);
//         	}
//               delay_ms(100);
//               LED_OFF;
//	}
//	else
//	{
//		g_statistics.reqa_fail++;
//		#if(NFC_DEBUG)
//		DEBUG("reqa_fail\n");
//		#endif
//	}
//
//	return status;
//}

//---------------------------------身份证读卡处理------------------------------------

char Typeb_Get(uint8_t *pcmd, uint8_t *id)
{
    int status;
    uint8_t i;
    uint8_t cnt;
    uint8_t ATQB[16];
    uint8_t req_code;

    g_statistics.reqb_cnt++;
    g_statistic_refreshed = 1;

    req_code = pcmd[1];

    cnt = 5; //应用中 可以使用轮询N次
    while (cnt--)
    {
        status = pcd_request_b(req_code, 0, 0, ATQB);

        if (status == MI_COLLERR) // 有冲突，超过一张卡
        {
            if ((status = pcd_request_b(req_code, 0, 2, ATQB)) != MI_OK)
            {
                for (i = 1; i < 4; i++)
                {
                    if ((status = pcd_slot_marker(i, ATQB)) == MI_OK)
                    {
                        break;
                    }
                }
                if (status == MI_OK)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        else if (status == MI_OK)
        {
            //			Beep_on_pwd_ok();

            DEBUG("pcd_request_b ok \n");
            break;
        }
    }

    if (status == MI_OK)
    {
        //typeB 106默认速率
        status = pcd_attri_b(&ATQB[1], 0, ATQB[10] & 0x0f, PICC_CID, ATQB);

        if (status == MI_OK)
        {
            ATQB[0] = 0x50; //恢复默认值
                            //make_packet(COM_PKT_CMD_REQB, ATQB, 12);
        }
    }

    if (status == MI_OK)
    {
        com_get_idcard_num(id); //获取卡号
        MF522_RST_L;            //522复位
        delay_ms(10);           //PJW	20190315	减少延时

        DEBUG("reqb_success\n");
    }
    else
    {
        g_statistics.reqb_fail++;
#if (NFC_DEBUG)
        DEBUG("reqb_fail\n");
#endif
    }

    return status;
}

//-------------------------------身份证读卡测试-----------------------------
char com_reqb(uint8_t *pcmd)
{
    int status;
    uint8_t i;
    uint8_t cnt;
    uint8_t ATQB[16];
    uint8_t req_code;

    g_statistics.reqb_cnt++;
    g_statistic_refreshed = 1;

    req_code = pcmd[1];

    cnt = 5; //应用中 可以使用轮询N次
    while (cnt--)
    {
        status = pcd_request_b(req_code, 0, 0, ATQB);

        if (status == MI_COLLERR) // 有冲突，超过一张卡
        {
            if ((status = pcd_request_b(req_code, 0, 2, ATQB)) != MI_OK)
            {
                for (i = 1; i < 4; i++)
                {
                    if ((status = pcd_slot_marker(i, ATQB)) == MI_OK)
                    {
                        break;
                    }
                }
                if (status == MI_OK)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        else if (status == MI_OK)
        {
            //			Beep_on_close_clock(2);
            //Beep_on_pwd_ok();

            DEBUG("pcd_request_b ok \n");

            break;
        }
    }

    if (status == MI_OK)
    {
        //typeB 106默认速率
        status = pcd_attri_b(&ATQB[1], 0, ATQB[10] & 0x0f, PICC_CID, ATQB);

        if (status == MI_OK)
        {
            ATQB[0] = 0x50; //恢复默认值
                            //make_packet(COM_PKT_CMD_REQB, ATQB, 12);
        }
    }

    if (status == MI_OK)
    {
        //GPIO_WriteHigh(GPIOA, GPIO_PIN_1);//
        com_get_idcard_num(); //获取卡号
        MF522_RST_L;          //522复位
        delay_ms(300);
        //GPIO_WriteLow(GPIOA, GPIO_PIN_1);
        delay_ms(300);

        DEBUG("reqb_success\n");
    }
    else
    {
        g_statistics.reqb_fail++;
#if (NFC_DEBUG)
        DEBUG("reqb_fail\n");
#endif
    }

    return status;
}

/**
 ****************************************************************
 * @brief pcd_com_transceive() 
 *
 * 通过芯片和ISO14443卡通讯
 *
 * @param: pi->mf_command = 芯片命令字
 * @param: pi->mf_length  = 发送的数据长度
 * @param: pi->mf_data[]  = 发送数据
 * @return: status 值为MI_OK:成功
 * @retval: pi->mf_length  = 接收的数据BIT长度
 * @retval: pi->mf_data[]  = 接收数据
 ****************************************************************
 */
char pcd_com_transceive(struct transceive_buffer *pi)
{
    uint8_t recebyte;
    int status;
    uint8_t irq_en;
    uint8_t wait_for;
    uint8_t last_bits;
    uint8_t j;
    uint8_t val;
    uint8_t err;

    uint8_t irq_inv;
    uint16_t len_rest;
    uint8_t len;

    len = 0;
    len_rest = 0;
    err = 0;
    recebyte = 0;
    irq_en = 0;
    wait_for = 0;

    switch (pi->mf_command)
    {
    case PCD_IDLE:
        irq_en = 0x00;
        wait_for = 0x00;
        break;
    case PCD_AUTHENT:
        irq_en = IdleIEn | TimerIEn;
        wait_for = IdleIRq;
        break;
    case PCD_RECEIVE:
        irq_en = RxIEn | IdleIEn;
        wait_for = RxIRq;
        recebyte = 1;
        break;
    case PCD_TRANSMIT:
        irq_en = TxIEn | IdleIEn;
        wait_for = TxIRq;
        break;
    case PCD_TRANSCEIVE:
        irq_en = RxIEn | IdleIEn | TimerIEn | TxIEn;
        wait_for = RxIRq;
        recebyte = 1;
        break;
    default:
        pi->mf_command = MI_UNKNOWN_COMMAND;
        break;
    }

    if (pi->mf_command != MI_UNKNOWN_COMMAND && (((pi->mf_command == PCD_TRANSCEIVE || pi->mf_command == PCD_TRANSMIT) && pi->mf_length > 0) || (pi->mf_command != PCD_TRANSCEIVE && pi->mf_command != PCD_TRANSMIT)))
    {
        write_reg(CommandReg, PCD_IDLE);

        irq_inv = read_reg(ComIEnReg) & BIT7;
        write_reg(ComIEnReg, irq_inv | irq_en | BIT0); //使能Timer 定时器中断
        write_reg(ComIrqReg, 0x7F);                    //Clear INT
        write_reg(DivIrqReg, 0x7F);                    //Clear INT
        //Flush Fifo
        set_bit_mask(FIFOLevelReg, BIT7);
        if (pi->mf_command == PCD_TRANSCEIVE || pi->mf_command == PCD_TRANSMIT || pi->mf_command == PCD_AUTHENT)
        {
#if (NFC_DEBUG)
            DEBUG(" PCD_tx:");
#endif
            for (j = 0; j < pi->mf_length; j++)
            {

#if (NFC_DEBUG)
                DEBUG("%02x ", (uint16_t)pi->mf_data[j]);
#endif
            }
#if (NFC_DEBUG)
            DEBUG("\n");
#endif

            len_rest = pi->mf_length;
            if (len_rest >= FIFO_SIZE)
            {
                len = FIFO_SIZE;
            }
            else
            {
                len = len_rest;
            }

            for (j = 0; j < len; j++)
            {
                write_reg(FIFODataReg, pi->mf_data[j]);
            }
            len_rest -= len; //Rest bytes
            if (len_rest != 0)
            {
                write_reg(ComIrqReg, BIT2);    // clear LoAlertIRq
                set_bit_mask(ComIEnReg, BIT2); // enable LoAlertIRq
            }

            write_reg(CommandReg, pi->mf_command);
            if (pi->mf_command == PCD_TRANSCEIVE)
            {
                set_bit_mask(BitFramingReg, 0x80);
            }

            while (len_rest != 0)
            {
                delay_ms(10);
                //while(INT_PIN == 0);//Wait LoAlertIRq
                if (len_rest > (FIFO_SIZE - WATER_LEVEL))
                {
                    len = FIFO_SIZE - WATER_LEVEL;
                }
                else
                {
                    len = len_rest;
                }
                for (j = 0; j < len; j++)
                {
                    write_reg(FIFODataReg, pi->mf_data[pi->mf_length - len_rest + j]);
                }

                write_reg(ComIrqReg, BIT2); //在write fifo之后，再清除中断标记才可以

                //DEBUG("\n8 comirq=%02bx,ien=%02bx,INT= %bd \n", read_reg(ComIrqReg), read_reg(ComIEnReg), (uint8_t)INT_PIN);
                len_rest -= len; //Rest bytes
                if (len_rest == 0)
                {
                    clear_bit_mask(ComIEnReg, BIT2); // disable LoAlertIRq
                                                     //DEBUG("\n9 comirq=%02bx,ien=%02bx,INT= %bd \n", read_reg(ComIrqReg), read_reg(ComIEnReg), (uint8_t)INT_PIN);
                }
            }
            //Wait TxIRq
            delay_ms(10);
            //while (INT_PIN == 0);
            val = read_reg(ComIrqReg);
            if (val & TxIRq)
            {
                write_reg(ComIrqReg, TxIRq);
            }
        }
        if (PCD_RECEIVE == pi->mf_command)
        {
            set_bit_mask(ControlReg, BIT6); // TStartNow
        }

        len_rest = 0;                  // bytes received
        write_reg(ComIrqReg, BIT3);    // clear HoAlertIRq
        set_bit_mask(ComIEnReg, BIT3); // enable HoAlertIRq

        //等待命令执行完成
        //while(INT_PIN == 0);
        delay_ms(10);

        while (1)
        {
            //while(0 == INT_PIN);
            delay_ms(10);
            val = read_reg(ComIrqReg);
            if ((val & BIT3) && !(val & BIT5))
            {
                if (len_rest + FIFO_SIZE - WATER_LEVEL > 255)
                {
#if (NFC_DEBUG)
                    DEBUG("AF RX_LEN > 255B\n");
#endif
                    break;
                }
                for (j = 0; j < FIFO_SIZE - WATER_LEVEL; j++)
                {
                    pi->mf_data[len_rest + j] = read_reg(FIFODataReg);
                }
                write_reg(ComIrqReg, BIT3); //在read fifo之后，再清除中断标记才可以
                len_rest += FIFO_SIZE - WATER_LEVEL;
            }
            else
            {
                clear_bit_mask(ComIEnReg, BIT3); //disable HoAlertIRq
                break;
            }
        }

        val = read_reg(ComIrqReg);
#if (NFC_DEBUG)
        DEBUG(" INT:fflvl=%d,rxlst=%02bx ,ien=%02bx,cirq=%02bx\n", (uint16_t)read_reg(FIFOLevelReg), read_reg(ControlReg) & 0x07, read_reg(ComIEnReg), val); //XU
#endif
        write_reg(ComIrqReg, val); // 清中断

        if (val & BIT0)
        { //发生超时
            status = MI_NOTAGERR;
        }
        else
        {
            err = read_reg(ErrorReg);

            status = MI_COM_ERR;
            if ((val & wait_for) && (val & irq_en))
            {
                if (!(val & ErrIRq))
                { //指令执行正确
                    status = MI_OK;

                    if (recebyte)
                    {
                        val = 0x7F & read_reg(FIFOLevelReg);
                        last_bits = read_reg(ControlReg) & 0x07;
                        if (len_rest + val > MAX_TRX_BUF_SIZE)
                        { //长度过长超出缓存
                            status = MI_COM_ERR;
#if (NFC_DEBUG)
                            DEBUG("RX_LEN > 255B\n");
#endif
                        }
                        else
                        {
                            if (last_bits && val) //防止spi读错后 val-1成为负值
                            {
                                pi->mf_length = (val - 1) * 8 + last_bits;
                            }
                            else
                            {
                                pi->mf_length = val * 8;
                            }
                            pi->mf_length += len_rest * 8;

#if (NFC_DEBUG)
                            DEBUG(" RX:len=%02x,dat:", (uint16_t)pi->mf_length);
#endif
                            if (val == 0)
                            {
                                val = 1;
                            }
                            for (j = 0; j < val; j++)
                            {
                                pi->mf_data[len_rest + j] = read_reg(FIFODataReg);
                            }

#if (NFC_DEBUG)
                            for (j = 0; j < pi->mf_length / 8 + !!(pi->mf_length % 8); j++)
                            {
                                //	if (j > 4)
                                //	{
                                //		DEBUG("..");
                                //		break;
                                //	}
                                //	else
                                //	{
                                DEBUG("%02X ", (uint16_t)pi->mf_data[j]);
                                //	}
                                //DEBUG("%02X ", (uint16_t)pi->mf_data[j]);
                            }
                            //DEBUG("l=%d", pi->mf_length/8 + !!(pi->mf_length%8));
                            DEBUG("\n");
#endif
                        }
                    }
                }
                else if ((err & CollErr) && (!(read_reg(CollReg) & BIT5)))
                { //a bit-collision is detected
                    status = MI_COLLERR;
                    if (recebyte)
                    {
                        val = 0x7F & read_reg(FIFOLevelReg);
                        last_bits = read_reg(ControlReg) & 0x07;
                        if (len_rest + val > MAX_TRX_BUF_SIZE)
                        { //长度过长超出缓存
#if (NFC_DEBUG)
                            DEBUG("COLL RX_LEN > 255B\n");
#endif
                        }
                        else
                        {
                            if (last_bits && val) //防止spi读错后 val-1成为负值
                            {
                                pi->mf_length = (val - 1) * 8 + last_bits;
                            }
                            else
                            {
                                pi->mf_length = val * 8;
                            }
                            pi->mf_length += len_rest * 8;
#if (NFC_DEBUG)
                            DEBUG(" RX: pi_cmd=%02x,pi_len=%02x,pi_dat:", (uint16_t)pi->mf_command, (uint16_t)pi->mf_length);
#endif
                            if (val == 0)
                            {
                                val = 1;
                            }
                            for (j = 0; j < val; j++)
                            {
                                pi->mf_data[len_rest + j + 1] = read_reg(FIFODataReg);
                            }
#if (NFC_DEBUG)
                            for (j = 0; j < pi->mf_length / 8 + !!(pi->mf_length % 8); j++)
                            {
                                DEBUG("%02X ", (uint16_t)pi->mf_data[j + 1]);
                            }
                            DEBUG("\n");
#endif
                        }
                    }
                    pi->mf_data[0] = (read_reg(CollReg) & CollPos);
                    if (pi->mf_data[0] == 0)
                    {
                        pi->mf_data[0] = 32;
                    }
#if (NFC_DEBUG)
                    DEBUG("\n COLL_DET pos=%02x\n", (uint16_t)pi->mf_data[0]);
#endif
                    pi->mf_data[0]--; // 与之前版本有点映射区别，为了不改变上层代码，这里直接减一；
                }
                else if ((err & CollErr) && (read_reg(CollReg) & BIT5))
                {
                    //DEBUG("COLL_DET,but CollPosNotValid=1\n");
                }
                //else if (err & (CrcErr | ParityErr | ProtocolErr))
                else if (err & (ProtocolErr))
                {
#if (NFC_DEBUG)
                    DEBUG("protocol err=%b02x\n", err);
#endif
                    status = MI_FRAMINGERR;
                }
                else if ((err & (CrcErr | ParityErr)) && !(err & ProtocolErr))
                {
                    //EMV  parity err EMV 307.2.3.4
                    val = 0x7F & read_reg(FIFOLevelReg);
                    last_bits = read_reg(ControlReg) & 0x07;
                    if (len_rest + val > MAX_TRX_BUF_SIZE)
                    { //长度过长超出缓存
                        status = MI_COM_ERR;
#if (NFC_DEBUG)
                        DEBUG("RX_LEN > 255B\n");
#endif
                    }
                    else
                    {
                        if (last_bits && val)
                        {
                            pi->mf_length = (val - 1) * 8 + last_bits;
                        }
                        else
                        {
                            pi->mf_length = val * 8;
                        }
                        pi->mf_length += len_rest * 8;
                    }
#if (NFC_DEBUG)
                    DEBUG("crc-parity err=%b02x\n", err);
                    DEBUG("l=%d\n", pi->mf_length);
#endif

                    status = MI_INTEGRITY_ERR;
                }
                else
                {
#if (NFC_DEBUG)
                    DEBUG("unknown ErrorReg=%02bx\n", err);
#endif
                    status = MI_INTEGRITY_ERR;
                }
            }
            else
            {
                status = MI_COM_ERR;
#if (NFC_DEBUG)
                DEBUG(" MI_COM_ERR\n");
#endif
            }
        }

        set_bit_mask(ControlReg, BIT7);  // TStopNow =1,必要的；
        write_reg(ComIrqReg, 0x7F);      // 清中断0
        write_reg(DivIrqReg, 0x7F);      // 清中断1
        clear_bit_mask(ComIEnReg, 0x7F); //清中断使能,最高位是控制位
        clear_bit_mask(DivIEnReg, 0x7F); //清中断使能,最高位是控制位
        write_reg(CommandReg, PCD_IDLE);
    }
    else
    {
        status = USER_ERROR;
#if (NFC_DEBUG)
        DEBUG("USER_ERROR\n");
#endif
    }
#if (NFC_DEBUG)
    DEBUG(" pcd_com: sta=%bd,err=%02bx\n", status, err);
#endif
    return status;
}

/////////////////////////////////////////////////////////////////////
//获取B卡的ID号
//
/////////////////////////////////////////////////////////////////////
void com_get_idcard_num(uint8_t *uid)
{
    uint8_t id[10];
    char status, i;
    //char COM_PKT_CMD_TYPEB_UID=0X37;

    status = get_idcard_num(id); //获取ID号
    if (status == MI_OK)
    {
        //		Beep_on_close_clock(2);

        DEBUG("\r\n  ID_card=%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \r\n ", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7], id[8], id[9]);
    }
    memcpy(uid, id, 4);
}

/////////////////////////////////////////////////////////////////////
//功    能：通过RC522和ISO14443卡通讯
//参数说明：Command[IN]:RC522命令字
//          pInData[IN]:通过RC522发送到卡片的数据
//          InLenByte[IN]:发送数据的字节长度
//          pOutData[OUT]:接收到的卡片返回数据
//          *pOutLenBit[OUT]:返回数据的位长度
/////////////////////////////////////////////////////////////////////
char PcdComMF522(unsigned char Command,
                 unsigned char *pInData,
                 unsigned char InLenByte,
                 unsigned char *pOutData,
                 unsigned int *pOutLenBit)
{
    char status = MI_ERR;
    unsigned char irqEn = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    switch (Command)
    {
    case PCD_AUTHENT:
        irqEn = 0x12;
        waitFor = 0x10;
        break;
    case PCD_TRANSCEIVE:
        irqEn = 0x77;
        waitFor = 0x30;
        break;
    default:
        break;
    }

    WriteRawRC(ComIEnReg, irqEn | 0x80);
    ClearBitMask(ComIrqReg, 0x80);
    WriteRawRC(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);

    for (i = 0; i < InLenByte; i++)
    {
        WriteRawRC(FIFODataReg, pInData[i]);
    }
    WriteRawRC(CommandReg, Command);

    if (Command == PCD_TRANSCEIVE)
    {
        SetBitMask(BitFramingReg, 0x80);
    }

    i = 600; //根据时钟频率调整，操作M1卡最大等待时间25ms
    do
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitFor));
    ClearBitMask(BitFramingReg, 0x80);

    if (i != 0)
    {
        if (!(ReadRawRC(ErrorReg) & 0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {
                status = MI_NOTAGERR;
            }
            if (Command == PCD_TRANSCEIVE)
            {
                n = ReadRawRC(FIFOLevelReg);
                lastBits = ReadRawRC(ControlReg) & 0x07;
                if (lastBits)
                {
                    *pOutLenBit = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *pOutLenBit = n * 8;
                }
                if (n == 0)
                {
                    n = 1;
                }
                if (n > MAXRLEN)
                {
                    n = MAXRLEN;
                }
                for (i = 0; i < n; i++)
                {
                    pOutData[i] = ReadRawRC(FIFODataReg);
                }
            }
        }
        else
        {
            status = MI_ERR;
        }
    }

    SetBitMask(ControlReg, 0x80); // stop timer now
    WriteRawRC(CommandReg, PCD_IDLE);
    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：读取M1卡一块数据
//参数说明: addr[IN]：块地址
//          pData[OUT]：读出的数据，16字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdRead(unsigned char addr, unsigned char *pData)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
    if ((status == MI_OK) && (unLen == 0x90))
    //   {   memcpy(pData, ucComMF522Buf, 16);   }
    {
        for (i = 0; i < 16; i++)
        {
            *(pData + i) = ucComMF522Buf[i];
        }
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：写数据到M1卡一块
//参数说明: addr[IN]：块地址
//          pData[IN]：写入的数据，16字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdWrite(unsigned char addr, unsigned char *pData)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        //memcpy(ucComMF522Buf, pData, 16);
        for (i = 0; i < 16; i++)
        {
            ucComMF522Buf[i] = *(pData + i);
        }
        CalulateCRC(ucComMF522Buf, 16, &ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, &unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：命令卡片进入休眠状态
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdHalt(void)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//功    能：寻卡
//参数说明: req_code[IN]:寻卡方式
//                0x52 = 寻感应区内所有符合14443A标准的卡
//                0x26 = 寻未进入休眠状态的卡
//          pTagType[OUT]：卡片类型代码
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdRequest(unsigned char req_code, unsigned char *pTagType)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ClearBitMask(Status2Reg, 0x08);
    WriteRawRC(BitFramingReg, 0x07);
    SetBitMask(TxControlReg, 0x03);

    ucComMF522Buf[0] = req_code;

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);

    if ((status == MI_OK) && (unLen == 0x10))
    {
        *pTagType = ucComMF522Buf[0];
        *(pTagType + 1) = ucComMF522Buf[1];
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：防冲撞
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdAnticoll(unsigned char *pSnr)
{
    char status;
    unsigned char i, snr_check = 0;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ClearBitMask(Status2Reg, 0x08);  //清RC522寄存器位
    WriteRawRC(BitFramingReg, 0x00); //写RC632寄存器
    ClearBitMask(CollReg, 0x80);     //清RC522寄存器位

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);

    if (status == MI_OK)
    {
        for (i = 0; i < 4; i++)
        {
            *(pSnr + i) = ucComMF522Buf[i];
            snr_check ^= ucComMF522Buf[i];
        }
        if (snr_check != ucComMF522Buf[i])
        {
            status = MI_ERR;
        }
    }

    SetBitMask(CollReg, 0x80); //置RC522寄存器位
    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：选定卡片
//参数说明: pSnr[IN]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdSelect(unsigned char *pSnr)
{
    char status;
    unsigned char i;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i = 0; i < 4; i++)
    {
        ucComMF522Buf[i + 2] = *(pSnr + i);
        ucComMF522Buf[6] ^= *(pSnr + i);
    }
    CalulateCRC(ucComMF522Buf, 7, &ucComMF522Buf[7]);

    ClearBitMask(Status2Reg, 0x08);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen);

    if ((status == MI_OK) && (unLen == 0x18))
    {
        status = MI_OK;
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：验证卡片密码
//参数说明: auth_mode[IN]: 密码验证模式
//                 0x60 = 验证A密钥
//                 0x61 = 验证B密钥
//          addr[IN]：块地址
//          pKey[IN]：密码
//          pSnr[IN]：卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdAuthState(unsigned char auth_mode, unsigned char addr, unsigned char *pKey, unsigned char *pSnr)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i = 0; i < 6; i++)
    {
        ucComMF522Buf[i + 2] = *(pKey + i);
    }
    for (i = 0; i < 6; i++)
    {
        ucComMF522Buf[i + 8] = *(pSnr + i);
    }
    //   memcpy(&ucComMF522Buf[2], pKey, 6);
    //   memcpy(&ucComMF522Buf[8], pSnr, 4);

    status = PcdComMF522(PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, &unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {
        status = MI_ERR;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：扣款和充值
//参数说明: dd_mode[IN]：命令字
//               0xC0 = 扣款
//               0xC1 = 充值
//          addr[IN]：钱包地址
//          pValue[IN]：4字节增(减)值，低位在前
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdValue(unsigned char dd_mode, unsigned char addr, unsigned char *pValue)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = dd_mode;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        // memcpy(ucComMF522Buf, pValue, 4);
        for (i = 0; i < 16; i++)
        {
            ucComMF522Buf[i] = *(pValue + i);
        }
        CalulateCRC(ucComMF522Buf, 4, &ucComMF522Buf[4]);
        unLen = 0;
        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 6, ucComMF522Buf, &unLen);
        if (status != MI_ERR)
        {
            status = MI_OK;
        }
    }

    if (status == MI_OK)
    {
        ucComMF522Buf[0] = PICC_TRANSFER;
        ucComMF522Buf[1] = addr;
        CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }
    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：备份钱包
//参数说明: sourceaddr[IN]：源地址
//          goaladdr[IN]：目标地址
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdBakValue(unsigned char sourceaddr, unsigned char goaladdr)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_RESTORE;
    ucComMF522Buf[1] = sourceaddr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        ucComMF522Buf[0] = 0;
        ucComMF522Buf[1] = 0;
        ucComMF522Buf[2] = 0;
        ucComMF522Buf[3] = 0;
        CalulateCRC(ucComMF522Buf, 4, &ucComMF522Buf[4]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 6, ucComMF522Buf, &unLen);
        if (status != MI_ERR)
        {
            status = MI_OK;
        }
    }

    if (status != MI_OK)
    {
        return MI_ERR;
    }

    ucComMF522Buf[0] = PICC_TRANSFER;
    ucComMF522Buf[1] = goaladdr;

    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    return status;
}

void pcd_lpcd_config_start(uint8_t delta, uint32_t t_inactivity_ms, uint8_t skip_times, uint8_t t_detect_us)
{
    uint8_t WUPeriod;
    uint8_t SwingsCnt;

#if (NFC_DEBUG)
    DEBUG("pcd_lpcd_config_start\n");
#endif
    WUPeriod = t_inactivity_ms * 32.768 / 256 + 0.5;
    SwingsCnt = t_detect_us * 27.12 / 2 / 16 + 0.5;

    write_reg(0x01, 0x0F); //先复位寄存器再进行lpcd

    write_reg(0x14, 0x8B);                                                   // Tx2CW = 1 ，continue载波发射打开
    write_reg(0x37, 0x00);                                                   //恢复版本号
    write_reg(0x37, 0x5e);                                                   // 打开私有寄存器保护开关
    write_reg(0x3c, 0x30 | delta);                                           //设置Delta[3:0]的值, 开启32k
    write_reg(0x3d, WUPeriod);                                               //设置休眠时间
    write_reg(0x3e, 0x80 | ((skip_times & 0x07) << 4) | (SwingsCnt & 0x0F)); //开启LPCD_en设置,跳过探测次数，探测时间
    write_reg(0x37, 0x00);                                                   // 关闭私有寄存器保护开关
    write_reg(0x03, 0x20);                                                   //打开卡探测中断使能
    write_reg(0x01, 0x10);                                                   //PCD soft powerdown

    //具体应用相关，本示例工程配置为高电平为有中断
    clear_bit_mask(0x02, BIT7);
}

//=======================================================================================================
//
//																					CPU卡处理函数
//
//=======================================================================================================

char SetTimeOut(unsigned int uiMicroSeconds)
{
    unsigned int RegVal;
    unsigned char TmpVal;
    RegVal = uiMicroSeconds / 100;
    /*
    NOTE: The supported hardware range is bigger, since the prescaler here
          is always set to 100 us.
          定时器的基础时间为100us
    */
    if (RegVal >= 0xfff)
    {
        return 0x01;
    }
    SetBitMask(TModeReg, 0x80);

    WriteRawRC(TPrescalerReg, 0xa6);

    TmpVal = ReadRawRC(TModeReg);
    TmpVal &= 0xf0;
    TmpVal |= 0x02;
    WriteRawRC(TModeReg, TmpVal);

    WriteRawRC(TReloadRegL, ((unsigned char)(RegVal & 0xff)));
    WriteRawRC(TReloadRegH, ((unsigned char)((RegVal >> 8) & 0xff)));
    return 0x00;
}

/////////////////////////////////////////////////////////////////////
//功    能：通过RC522和ISO14443卡通讯		CPU卡使用
//参数说明：Command[IN]:RC522命令字
//          pInData[IN]:通过RC522发送到卡片的数据
//          InLenByte[IN]:发送数据的字节长度
//          pOutData[OUT]:接收到的卡片返回数据
//          *pOutLenBit[OUT]:返回数据的位长度
/////////////////////////////////////////////////////////////////////
char PcdComMF522_CPU(unsigned char Command,
                     unsigned char *pInData,
                     unsigned char InLenByte,
                     unsigned char *pOutData,
                     unsigned char *pOutLenByte,
                     unsigned int *pOutLenBit)
{
    char status = MI_ERR;
    unsigned char irqEn = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    switch (Command)
    {
    case PCD_AUTHENT: //验证密钥
        irqEn = 0x12;
        waitFor = 0x10;
        break;
    case PCD_TRANSCEIVE: //发送并接收数据
        irqEn = 0x77;
        waitFor = 0x30;
        break;
    default:
        break;
    }

    WriteRawRC(ComIEnReg, irqEn | 0x80); //ComIEnReg:中断请求的使能位。验证密钥：（10010010）位7：管脚IRQ与Status1Reg的IRq反相。位4：允许空闲中断请求（IdleIRq位）传递到IRQ管脚上。位1：允许错误中断请求（ErrIRq位）传递到IRQ管脚上。                                        //                             发送并接收数据：（11110111）除高位中断请求外，其他都能传到IRQ管脚上
    ClearBitMask(ComIrqReg, 0x80);       //ComIrqReg的屏蔽位清零
    WriteRawRC(CommandReg, PCD_IDLE);    //CommandReg低4位写0000B，处于空闲模式
    SetBitMask(FIFOLevelReg, 0x80);      //FIFOLevelReg中FlushBuffer位置1，表示缓冲区读和写指针清除，即缓冲区无数据，用来存放一下批数据，ErrReg的BufferOvfl清除

    for (i = 0; i < InLenByte; i++)
    {
        WriteRawRC(FIFODataReg, pInData[i]); //将pInData数组的数据写进FIFO缓冲区
    }
    WriteRawRC(CommandReg, Command); //验证密钥 or 发送并接收数据

    if (Command == PCD_TRANSCEIVE)
    {
        SetBitMask(BitFramingReg, 0x80); //启动数据的发送
    }

    i = 600; //根据时钟频率调整，操作M1卡最大等待时间25ms
    do
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitFor)); //如果i不为0，并且定时器没减到0，并且没有未知命令和自身终止命令和接收器没有检测到有效数据流，就继续循环，退出循环则表示接收完成
    ClearBitMask(BitFramingReg, 0x80);                   //StartSend位清零

    if (i != 0)
    {
        status = ReadRawRC(ErrorReg);
        WriteRawRC(ErrorReg, 0X00);
        if (!(status & 0x1b))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {
                status = MI_NOTAGERR;
            }
        }
        else if (status & 0x08)
        {
            status = MI_COLLERR;
        }
        else
            status = MI_ERR;
        if (Command == PCD_TRANSCEIVE)
        {
            n = ReadRawRC(FIFOLevelReg);
            lastBits = ReadRawRC(ControlReg) & 0x07;
            if (lastBits)
            {
                *pOutLenBit = (n - 1) * 8 + lastBits;
            }
            else
            {
                *pOutLenBit = n * 8;
            }
            if (n == 0)
            {
                n = 1;
            }
            if (n > MAXRLEN)
            {
                n = MAXRLEN;
            }
            *pOutLenByte = n;
            for (i = 0; i < n; i++)
            {
                pOutData[i] = ReadRawRC(FIFODataReg);
            }
        }
    }

    SetBitMask(ControlReg, 0x80);     // stop timer now
    WriteRawRC(CommandReg, PCD_IDLE); //空闲
    WriteRawRC(CommandReg, PCD_IDLE);
    return status;
}

void ResetInfo(void)
{
    MInfo.cmd = 0;
    MInfo.status = MI_OK;
    MInfo.irqSource = 0;
    MInfo.nBytesSent = 0;
    MInfo.nBytesToSend = 0;
    MInfo.nBytesReceived = 0;
    MInfo.nBitsReceived = 0;
    MInfo.collPos = 0;
}

/****************************************************************************
Request 指令将通知MCM在天线有效的工作范围（距离）内寻找MIFARE 1卡片。如果有
MIFARE 1卡片存在，这一指令将分别与MIFARE 1进行通信，读取MIFARE 1卡片上的卡片
类型号TAGTYPE（2个字节），由MCM传递给MCU，进行识别处理。
程序员可以根据TAGTYPE来区别卡片的不同类型。
对于MIFARE 1卡片来说，返回卡片的TAGTYPE（2个字节）可能为0004h。
* Function:     Mf500PiccRequest                                              *
* Input:        req_code                                                      *
* Output:       TagType                                                     *
* req_code			寻卡方式
* atq						卡号 4字节
****************************************************************************/

/*************************************************
Function:
		Deselect
Description:
		CPU卡停活
Parameter:
		atd: 		卡片复位响应
		RcByte:	返回的数据长度
Return:
		status
**************************************************/
char Deselect(unsigned char *atd, unsigned char *RcByte)
{
    char status;
    unsigned int unLen;
    unsigned char rcByte;
    unsigned char ucComMF522Buf[MAXRLEN];
    unsigned char i = 0;

    SetBitMask(TxModeReg, 0x80);
    SetBitMask(RxModeReg, 0x80);
    ClearBitMask(Status2Reg, 0x08);

    ucComMF522Buf[0] = 0xCA;
    ucComMF522Buf[1] = 0x01;

    //	SetTimeOut(1000);
    status = PcdComMF522_CPU(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &rcByte, &unLen);
    if (status == 0x00)
    {
        *RcByte = rcByte;
        for (i = 0; i < rcByte; i++)
        {
            *(atd + i) = ucComMF522Buf[i];
        }
    }
    return status;
}

/////////////////////////////////////////////////////////////////////
//Mifare_Pro复位
//input: parameter = PCD BUFERR SIZE
//output:pLen      = 复位信息长度
//       pData     = 复位信息
/////////////////////////////////////////////////////////////////////
char MifareProRst(unsigned char parameter, unsigned char *pData, unsigned char *pLen)
{
    char status;
    unsigned int unLen;
    unsigned char rcByte;
    unsigned char ucComMF522Buf[MAXRLEN];

    //    PcdSetTmo(4);     // long timeout
    //    WriteRC(RegChannelRedundancy,0x0f); // RxCRC, TxCRC, Parity enable
    WriteRawRC(TxModeReg, 0x80); //使能在数据发送过程中产生 CRC
    WriteRawRC(RxModeReg, 0x80);
    WriteRawRC(Status2Reg, 0x08);
    ucComMF522Buf[0] = PICC_RESET;
    ucComMF522Buf[1] = parameter;
    //ucComMF522Buf[1] = 0x51;
#ifdef DBG_FM1701
    {
        DEBUG_RFID("\r\nparameter:%d,Send:", parameter);
        DEBUG_RFID("\r\n");
    }
#endif
    //CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);//计算校验位
    //status = PcdSingleResponseCmd(PCD_TRANSCEIVE);
    //status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    SetTimeOut(408000); //408000
    status = PcdComMF522_CPU(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &rcByte, &unLen);
    if (status != MI_OK)
    {
        DEBUG_RFID("PcdComMF522_CPU ERR \r\n");
        status = MI_ERR;
    }
    else // Response Processing
    {
        g_cCidNad = 0;
        memcpy(pData, ucComMF522Buf, unLen);
        *pLen = rcByte;
    }

    return status;
}

char TypeARest(unsigned char *pData, unsigned char *pLen)
{
    unsigned char bb;

    bb = (FSDI << 4) & 0xF0;
    return MifareProRst(bb, pData, pLen);
}

/////////////////////////////////////////////////////////////////////
//向ISO14443-4卡发送COS命令
//input:CID_NAD  = 是否包含CID或NAD
//      timeout  = FWI
//      pLen     = 命令长度
//      pCommand = COS命令
//ouput:pLen     = 返回数据长度
//      pCommand = 返回数据
/////////////////////////////////////////////////////////////////////
char MifareProCom(unsigned char CID_NAD, const unsigned char *Send, unsigned char *Recv, unsigned char *pLen)
{
    unsigned char status;
    unsigned char i, CidNadLg;
    unsigned char PCB_I = 0;
    unsigned char PCB_R = 0;
    unsigned char PCB_S = 0;
    unsigned char sendlgok = 0;
    unsigned char sendlgnow = 0;
    unsigned char recelgnow = 0;
    unsigned char recelgsum = 0;
    unsigned char sendlgsum = *pLen;
    unsigned int unLen;
    unsigned char rcByte;
    switch (CID_NAD)
    {
    case 4:
    case 8:
        CidNadLg = 1;
        break;
    case 0x0C:
        CidNadLg = 2;
        break;
    default:
        CidNadLg = 0;
        break;
    }

    if (sendlgsum > FSD - 1 - CidNadLg)
    {
        sendlgnow = FSD - 1 - CidNadLg;
        sendlgsum -= sendlgnow;
        PCB_I = 0x12;
    }
    else
    {
        sendlgnow = sendlgsum;
        PCB_I = 0x02;
    }

    memset(MSndBuffer, 0, sizeof(MSndBuffer));
    MSndBuffer[0] = 0x02 | CID_NAD | PCB_I;
    for (i = 0; i < CidNadLg; i++)
    {
        MSndBuffer[i + 1] = 1;
    }
    memcpy(&MSndBuffer[CidNadLg + 1], Send, sendlgnow);

    MInfo.nBytesToSend = sendlgnow + CidNadLg + 1;
    sendlgok += sendlgnow;
    //memset(MRcvBuffer,0,sizeof(MRcvBuffer));
    //WriteRC(RegChannelRedundancy,0x0f); // RxCRC, TxCRC, Parity enable
    WriteRawRC(TxModeReg, 0x80); //使能在数据发送过程中产生 CRC
    WriteRawRC(RxModeReg, 0x80);
    //status = PcdSingleResponseCmd(PCD_TRANSCEIVE);
#ifdef DBG_FM1701
    {
        unsigned char i;
        DEBUG_RFID("\r\n%d,Send:", MInfo.nBytesToSend);
        for (i = 0; i < MInfo.nBytesToSend; i++)
        {
            DEBUG_RFID("%02X", MSndBuffer[i]);
        }
        DEBUG_RFID("\r\n");
    }
#endif
    SetTimeOut(408000); //408000
    status = PcdComMF522_CPU(PCD_TRANSCEIVE, MSndBuffer, MInfo.nBytesToSend, MRcvBuffer, &rcByte, &unLen);
    //////////////////////////////////////////////
    while (status == MI_OK)
    {
        MInfo.nBytesReceived = rcByte;
        if ((MInfo.nBytesReceived - 1 - CidNadLg > FSD) || (MInfo.nBytesReceived == 0))
        {
            status = MI_ERR;

            break;
        }

        if ((MRcvBuffer[0] & 0xF0) == 0x00) //命令通讯结束
        {
            if (MRcvBuffer[0] & 0x01)
            {
                g_cCidNad = 0;
            }
            else
            {
                g_cCidNad = 1;
            }

            recelgnow = MInfo.nBytesReceived - 1 - CidNadLg;
            memcpy(Recv + recelgsum, &MRcvBuffer[CidNadLg + 1], recelgnow);
            recelgsum += recelgnow;
            *pLen = recelgsum;
            break;
        }

        if ((MRcvBuffer[0] & 0xF0) == 0xA0) //发送后续数据  I块
        {
            PCB_I ^= 1;
            if (sendlgsum > FSD - 1 - CidNadLg)
            {
                sendlgnow = FSD - 1 - CidNadLg;
                sendlgsum -= sendlgnow;
                PCB_I |= 0x10;
            }
            else
            {
                sendlgnow = sendlgsum;
                PCB_I &= 0xEF;
            }

            MSndBuffer[0] = 0x02 | CID_NAD | PCB_I;
            for (i = 0; i < CidNadLg; i++)
            {
                MSndBuffer[i + 1] = 0;
            }
            memcpy(&MSndBuffer[CidNadLg + 1], Send + sendlgok, sendlgnow);
            ResetInfo();
            MInfo.nBytesToSend = sendlgnow + CidNadLg + 1;
            sendlgok += sendlgnow;
            memset(MRcvBuffer, 0, sizeof(MRcvBuffer));
            //PcdSetTmo(11);
            //WriteRC(RegChannelRedundancy,0x0f); // RxCRC, TxCRC, Parity enable
            WriteRawRC(TxModeReg, 0x80); //使能在数据发送过程中产生 CRC
            //status = PcdSingleResponseCmd(PCD_TRANSCEIVE);
            status = PcdComMF522_CPU(PCD_TRANSCEIVE, MSndBuffer, MInfo.nBytesToSend, MRcvBuffer, &rcByte, &unLen);

            continue;
        }

        if ((MRcvBuffer[0] & 0xF0) == 0x10) //接收后续数据   R块
        {

#if 0 //DBG_RFCard
            {
                unsigned char i;
                DEBUG_RFID("\nMifareProCom   Recv other  %2X  len %d ==\n",MInfo.nBytesReceived,*pLen);
                for(i=0; i<MInfo.nBytesReceived; i++)
                {
                    DEBUG_RFID("%02X",MRcvBuffer[i]);
                }
                DEBUG_RFID("\n");

            }
#endif

            if ((MInfo.nBytesReceived - 1 - CidNadLg > FSD) || (MInfo.nBytesReceived == 0))
            {
                status = MI_ERR;
                break;
            }

            recelgnow = MInfo.nBytesReceived - 1 - CidNadLg;
            memcpy(Recv + recelgsum, &MRcvBuffer[CidNadLg + 1], recelgnow);
            recelgsum += recelgnow;
            PCB_R = 0xa2;
            if (MRcvBuffer[0] & 0x01)
            {
                PCB_R &= 0xFE;
            }
            else
            {
                PCB_R |= 0x01;
            }

            MSndBuffer[0] = PCB_R;
            for (i = 0; i < CidNadLg; i++)
            {
                MSndBuffer[i + 1] = 0;
            }
            ResetInfo();
            MInfo.nBytesToSend = CidNadLg + 1;
            memset(MRcvBuffer, 0, sizeof(MRcvBuffer));
            //PcdSetTmo(11);
            //WriteRC(RegChannelRedundancy,0x0f); // RxCRC, TxCRC, Parity enable
            WriteRawRC(TxModeReg, 0x80); //使能在数据发送过程中产生 CRC
            //status = PcdSingleResponseCmd(PCD_TRANSCEIVE);
            status = PcdComMF522_CPU(PCD_TRANSCEIVE, MSndBuffer, MInfo.nBytesToSend, MRcvBuffer, &rcByte, &unLen);
            continue;
        }

        if ((MRcvBuffer[0] & 0xC0) == 0xC0) //S 块
        {

            PCB_S = 0xC0;
            MSndBuffer[0] = PCB_S | MRcvBuffer[0];
            for (i = 0; i < CidNadLg; i++)
            {
                MSndBuffer[i + 1] = 0;
            }

            ResetInfo();
            MInfo.nBytesToSend = CidNadLg + 1;
            memset(MRcvBuffer, 0, sizeof(MRcvBuffer));
            //PcdSetTmo(11);
            //WriteRC(RegChannelRedundancy,0x0f); // RxCRC, TxCRC, Parity enable
            WriteRawRC(TxModeReg, 0x80); //使能在数据发送过程中产生 CRC
            //status = PcdSingleResponseCmd(PCD_TRANSCEIVE);
            status = PcdComMF522_CPU(PCD_TRANSCEIVE, MSndBuffer, MInfo.nBytesToSend, MRcvBuffer, &rcByte, &unLen);
            continue;
        }

        status = MI_ERR;
        break;
    }

#if 0 //DBG_RFCard
    {
        DEBUG_RFID("\nMifareProCom  stats = %d Rcv %d==\n",status,*pLen);
    }
#endif

    return status;
}

/*
*************************************************
uint8_t TypeAComRest(uint8_t *gchar)
功能: ISO 14443  TypeA 卡发送COS命令
参数: *send 发送数据  *recv 返回数据 *pLen 发送/接收的长度
返回:  0成功    其它失败
*************************************************
*/

char TypeAComCOS(const unsigned char *send, unsigned char *recv, unsigned char *pLen)
{
    char status = 1;
    unsigned char Sendbuf[6];

#ifdef DBG_FM1701
    {
        unsigned char i;
        DEBUG_RFID("\r\nSend:");
        for (i = 0; i < *pLen; i++)
        {
            DEBUG_RFID("%02X", send[i]);
        }
        DEBUG_RFID("\r\n");
    }
#endif

    // while(getchar() != '\n');

    status = MifareProCom(g_cCidNad, send, recv, pLen);
    if (status == MI_OK)
    {
        if ((*pLen == 2) && (recv[0] == 0x61)) //00 C0 00 00 17
        {
            Sendbuf[0] = 0x00;
            Sendbuf[1] = 0xc0;
            Sendbuf[2] = 0x00;
            Sendbuf[3] = 0x00;
            Sendbuf[4] = recv[1];
            *pLen = 5;
            status = MifareProCom(g_cCidNad, Sendbuf, recv, pLen);
        }
    }

    if (status != MI_OK)
        *pLen = 0;

#ifdef DBG_FM1701
    {
        unsigned char i;
        DEBUG_RFID("Recv:");
        for (i = 0; i < *pLen; i++)
        {
            DEBUG_RFID("%02X", recv[i]);
        }
        DEBUG_RFID("\r\n");
    }
#endif

    return status;
}

//--------------------------------读CPU卡---------------------------------------
uint8_t Read_M1_or_CPU_Card(uint8_t *id, uint8_t *type)
{

    unsigned char status;
    unsigned char uid[4];
    unsigned char g_ucTempbuf[20];

    status = PcdRequest(PICC_REQIDL, g_ucTempbuf); //第一次寻卡
    if (status != MI_OK)
    {
        //					 status = PcdRequest(PICC_REQALL, g_ucTempbuf);  //第二次寻卡
        //					 if (status != MI_OK)
        //					 {
        //							 status = PcdRequest(PICC_REQALL, g_ucTempbuf);  //第三次次寻卡
        //							 if (status != MI_OK)
        //							 {

        return (0); //	continue;
                    //							 }
                    //					 }
    }

    *type++ = g_ucTempbuf[0];
    *type = g_ucTempbuf[1];

    status = PcdAnticoll(uid); //防冲撞
    if (status != MI_OK)
    {
        return (0); //continue;
    }

    DEBUG("\r\n PcdAnticoll_ok ");

    status = PcdSelect(uid); //选定卡片
    if (status != MI_OK)
    {
        return (0); //continue;
    }

    memcpy(id, uid, 4);

    //			 *id++ = uid[0];
    //			 *id++ = uid[1];
    //			 *id++ = uid[2];
    //			 *id =   uid[3];

    return (1);
}

//------------------累加和校验---------------------

uint8_t Num_Check(uint8_t *buf)
{
    uint8_t Buf_Num = 0, i = 0;
    for (i = 0; i < 15; i++)
    {
        Buf_Num += buf[i];
    }

    if (Buf_Num != buf[15])
    {
        return (0);
    }
    else
    {
        return (1);
    }
}

#if 0

//==================================下发卡片   初始化卡==========================================

uint8_t Init_Card(uint8_t *Card_UID)  //局部变量不要定义过大，会有栈溢出问题
{
		 unsigned char i=0,j=0;	
		 unsigned char status;
     unsigned char  DefaultKey[6] = {0xAB, 0x02, 0x03, 0x44, 0x55, 0xAB}; 	//扇区密码  AB 02 03 44 55 AB
		 unsigned char  g_ucTempbuf[16];    
		 static unsigned char  card_type[2];		 
 	  							 
/*-----------读第3扇区0块数据----------（判断卡片类型)
				 
	判断是否是 初始化卡 				 
---------------------------------------
*/			
			 
				DEBUG("\r\n Card_UID= %02x %02x %02x %02x ",Card_UID[0],Card_UID[1],Card_UID[2],Card_UID[3]);			

			 //--------------验证卡片A密码----------
			 status = PcdAuthState(PICC_AUTHENT1A, 12, DefaultKey, Card_UID);	// 写入UID
			 if (status != MI_OK)
			 {   

					DEBUG("\r\n One PICC_AUTHENT1A_err ");
					
					 return(0);

			 }
				
				DEBUG("\r\n One PICC_AUTHENT1A_ok ");
			
			 status = PcdRead(12, g_ucTempbuf);					 //读块（固定第三扇区）	
			 if (status != MI_OK)
			 {   
						return(0);
			 }				
			 else
			 {
						
					if(g_ucTempbuf[8] == 0x01)	//不判断钥匙打开状态	PJW 2018.11.23
					{
						//-------初始化卡---------
								Data_Init_Flag=1;	//开启灯光
								//保存扇区号	
								Card_Sector=g_ucTempbuf[5];
								at24cxx_WriteOneByte(CARD_SECTOR_ADDR,Card_Sector);
								
									DEBUG("\r\n Card_Sector=%d ",Card_Sector);						
						
								//将密码的数据--全部清空	PJW 2018.11.21									
								USE_PWD_CLEAR_PWD();
								//将卡片的数据--全部清空	PJW 2018.11.21
								USE_CARD_ALL_CLEAR();																		
								//将开门次数数据清空
								USE_LOCK_CNT_CLEAR();
								//保存的门锁记录全部清空
								LOCK_RECORD_CLEAR();
								//喂狗
								IWDG_ReloadCounter();				
								//上发清空数据	
								rcv_clear_pwd_cmd_to_link();						
							
								Data_Init_Flag=0;	//关闭灯光
								
								//程序重启
								NVIC_SystemReset();	

					}
					else if(g_ucTempbuf[8] == 0x08)		
					{
						//---------信道卡----------
						
								//保存信道	
								dev_info.ch=g_ucTempbuf[9];
								at24cxx_WriteOneByte(LORA_CH_ADDR,dev_info.ch);
								
									DEBUG("\r\n dev_info.ch=%d ",dev_info.ch);	
						
								TIMDelay_Nms(100);

								//程序重启
								NVIC_SystemReset();						
						
					}						
					else
					{
						 
								DEBUG("\r\n Again Verify OK ");							
							return(0);	
					
					}
						
			}
			 
}


//--------------------------------下发卡片的扇区密码验证---------------------------------------

uint8_t M1_Card_Verify(uint8_t *Card_UID)  //局部变量不要定义过大，会有栈溢出问题
{
		 unsigned char i=0,j=0;	
		 unsigned char status;
     unsigned char  DefaultKey[6] = {0xAB, 0x02, 0x03, 0x44, 0x55, 0xAB}; 	//扇区密码  AB 02 03 44 55 AB
		 unsigned char  g_ucTempbuf[16];    
		 static unsigned char  card_type[2];		 
 	  							 
/*-----------读第3扇区0块数据----------（判断卡片类型)
				 
	判断是否是 初始化卡 				 
---------------------------------------
*/			
			 
			DEBUG("\r\n Card_UID= %02x %02x %02x %02x ",Card_UID[0],Card_UID[1],Card_UID[2],Card_UID[3]);
				
		if(Card_Sector==0)	//没有使用过初始化卡
		{	
			 //--------------验证卡片A密码----------
			 status = PcdAuthState(PICC_AUTHENT1A, 12, DefaultKey, Card_UID);	// 写入UID
			 if (status != MI_OK)
			 {   
		 
					DEBUG("\r\n One PICC_AUTHENT1A_err ");
					
					 return(0);

			 }
				
				DEBUG("\r\n One PICC_AUTHENT1A_ok ");
			
			 status = PcdRead(12, g_ucTempbuf);					 //读块（固定第三扇区）	
			 if (status != MI_OK)
			 {   
						return(0);
			 }				
			 else
			 {
						
					if(g_ucTempbuf[8] == 0x01)	//不判断钥匙打开状态	PJW 2018.11.23
					{
						//-------初始化卡---------
								Data_Init_Flag=1;	//开启灯光
												
								//保存扇区号	
								Card_Sector=g_ucTempbuf[5];
								at24cxx_WriteOneByte(CARD_SECTOR_ADDR,Card_Sector);
								
									DEBUG("\r\n Card_Sector=%d ",Card_Sector);						
								
																	
								//将卡片的数据--全部清空	PJW 2018.11.21
								USE_CARD_ALL_CLEAR();																		
								//将开门次数数据清空
								USE_LOCK_CNT_CLEAR();
								//保存的门锁记录全部清空
								LOCK_RECORD_CLEAR();
								//喂狗
								IWDG_ReloadCounter();				
								//上发清空数据	
								rcv_clear_pwd_cmd_to_link();						
							
								Data_Init_Flag=0;	//关闭灯光
								
								//程序重启
								NVIC_SystemReset();	

					}
					else
					{
						 
								DEBUG("\r\n Again Verify OK ");
						
							return(1);	
					
					}
						
				}
			 
		}
		else	//使用过初始化卡、有扇区号
		{
			
/*=============================================================================================
			
-----------使用过初始化卡、有扇区号--------------					
			
判断 扇区密码、房东信息、账户密码（初始化卡不验证）
			
=============================================================================================*/	
			 
				DEBUG("\r\n Card_UID= %02x %02x %02x %02x ",Card_UID[0],Card_UID[1],Card_UID[2],Card_UID[3]);
			
			 //--------------验证卡片A密码------------(初始化卡，第3扇区)
			 status = PcdAuthState(PICC_AUTHENT1A, 12, DefaultKey, Card_UID);	// 写入UID
			 if (status != MI_OK)
			 {   

					 
					DEBUG("\r\n Again PICC_AUTHENT1A_12_err ");
				 
					if(Read_M1_or_CPU_Card(Card_UID,card_type) == 1)	//再次询卡，否则不能再次验证密码		PJW		20181127
					{
						 //--------------验证卡片A密码------------(用户卡，Card_Sector扇区)	
						 status = PcdAuthState(PICC_AUTHENT1A, Card_Sector*4, DefaultKey, Card_UID);	// 写入UID
						 if (status != MI_OK)
						 {   

								 
								DEBUG("\r\n Again PICC_AUTHENT1A_Sector_err Card_Sector=%d ",Card_Sector);

								 return(0);
						 }
						 else		//用户卡验证成功
						 {
								 
									DEBUG("\r\n Again Verify OK ");
								
									return(1);							 
						 }
						 
					}else;

			 }
				
				DEBUG("\r\n Again PICC_AUTHENT1A_ok ");				
			

			 status = PcdRead(12, g_ucTempbuf);			//读第3扇区0块（0-15扇区，每扇区4个块）			初始化卡				
			 if (status != MI_OK)
			 { 
						
						DEBUG("\r\n Again Read_0_Sector_err ");
						DEBUG("\r\n Card_Sector=%d ",Card_Sector);
				 
						return(0);
			 }				
			 else
			 {
		 
					if(g_ucTempbuf[8] == 0x01)	//不判断钥匙打开状态	PJW 2018.11.23
					{
						//-------初始化卡---------
								Data_Init_Flag=1;	//开启灯光
												
								//保存扇区号	
								Card_Sector=g_ucTempbuf[5];
								at24cxx_WriteOneByte(CARD_SECTOR_ADDR,Card_Sector);
								
									DEBUG("\r\n Card_Sector=%d ",Card_Sector);						
								
																	
								//将卡片的数据--全部清空	PJW 2018.11.21
								USE_CARD_ALL_CLEAR();																		
								//将开门次数数据清空
								USE_LOCK_CNT_CLEAR();
								//保存的门锁记录全部清空
								LOCK_RECORD_CLEAR();
								//喂狗
								IWDG_ReloadCounter();				
								//上发清空数据	
								rcv_clear_pwd_cmd_to_link();						
							
								Data_Init_Flag=0;	//关闭灯光
								
								//程序重启
								NVIC_SystemReset();	

					}
					else	//在初始化卡密码正确的情况下也验证其他扇区
					{
						 //--------------验证卡片A密码------------(用户卡，Card_Sector扇区)	
						 status = PcdAuthState(PICC_AUTHENT1A, Card_Sector*4, DefaultKey, Card_UID);	// 写入UID
						 if (status != MI_OK)
						 {   

									DEBUG("\r\n Again PICC_AUTHENT1A_err ");

								 return(0);
						 }
						 else		//用户卡验证成功
						 {
								 
									DEBUG("\r\n Again Verify OK ");
								
									return(1);							 
						 }						
												
					}


			}						
				
		}
				
}



/************************************************
函数名称 ： Bl_Card_Check
功    能 ： 离线卡片黑名单比较
参    数 ： buf：读出的卡号	
返 回 值 ： 0：黑名单中存在
作    者 ： PJW
*************************************************/
uint8_t Bl_Card_Check(uint8_t *buf)
{
	 static unsigned char i=0,j=0;	
	 static unsigned char BL_UID[4];   		 	//读出的卡片UID		
	
		 
		DEBUG("\r\n Card_UID buf= %02x %02x %02x %02x ",buf[0],buf[1],buf[2],buf[3]);

		for(i=0;i<CARD_BL_NUM;i++) 	//遍历黑名单块
		{
			for(j=0;j<4;j++)	//4位UID
			{
					BL_UID[j]=at24cxx_ReadOneByte(BL_CARD_ADDR+i*BL_CARD_len+j);
			}
 
			DEBUG("\r\n i=%d BL_UID= %02x %02x %02x %02x ",i,BL_UID[0],BL_UID[1],BL_UID[2],BL_UID[3]);
			
			if( memcmp(BL_UID,buf,4) == 0 )	//比较相同 0
			{
				 
					DEBUG("\r\n Card_Bl Exist ");
							
					return(0);
			}else;
		}
		
		return(1);
		
}
				

//--------------------------------读 M1 卡块数据---------------------------------------
uint8_t Read_M1_Data(uint8_t *Card_UID)  //局部变量不要定义过大，会有栈溢出问题
{
		 unsigned char i=0,j=0;	
		 unsigned char idex_bl=0;		//卡片黑名单索引
		 unsigned char status;
     unsigned char  DefaultKey[6] = {0xAB, 0x02, 0x03, 0x44, 0x55, 0xAB}; 	//扇区密码  AB 02 03 44 55 AB
		 unsigned char  g_ucTempbuf[16];    
		 unsigned char  ACT_INFO[5];   	  	//卡片中的房东信息
		 unsigned char  Week_Buf[7];   	  	//周期循环中的星期标志
		 
		 unsigned char  UID_Temp[4];   	  	//
//		 unsigned char  Block0_Buf[16],Block1_Buf[16],Block2_Buf[16]; 		 
		 unsigned char	Staff_Card_Type=0;		//员工卡类型		 
		 unsigned char	Group_Sta=0;					//组对比标志
		 unsigned char	Read_Group[6];				//读出的 组号	 
		 static unsigned char  card_type[2];		 	
	
	 
/*-----------判断卡片是否在黑名单----------				 
1、读出卡片UID
2、读出黑名单中的卡号
3、逐一比对				 
------------------------------------------*/
				 			 
				 if( Bl_Card_Check(Card_UID) == 0)
				 {
					 	return(0);
				 }

				
				 
					DEBUG("\r\n Card_Bl No Exist ");					
				 
/*-----------读第3扇区0块数据----------（判断卡片类型)
				 
	判断是否是 初始化卡、组号卡、工程卡			 
				 
				 
				  初始化卡：0x01
					组号卡：  0x02
					工程卡：  0x03
					客人卡：  0x04
					员工卡：  0x05
					应急卡：  0x06
					挂失卡：  0x07
---------------------------------------
*/			
				   
				DEBUG("\r\n Card_UID= %02x %02x %02x %02x ",Card_UID[0],Card_UID[1],Card_UID[2],Card_UID[3]);
			
				if(Card_Sector==0)	//没有使用过初始化卡
				{	
					 //--------------验证卡片A密码----------
					 status = PcdAuthState(PICC_AUTHENT1A, 12, DefaultKey, Card_UID);	// 写入UID
					 if (status != MI_OK)
					 {   
 
							DEBUG("\r\n One PICC_AUTHENT1A_err ");
							
							 return(0);//continue; 
		
					 }
					  
						DEBUG("\r\n One PICC_AUTHENT1A_ok ");
					
					 status = PcdRead(12, g_ucTempbuf);					 //读块（固定第三扇区）	
					 if (status != MI_OK)
					 {   
								return(0);
					 }				
					 else
					 {
								//和校验
								if( Num_Check(g_ucTempbuf) == 0)
								{
									 
										DEBUG("\r\n One Num_Check_err ");
								
										return(0);
								}
								
								/*----------------还未刷初始化卡不判断房东信息------------------	*/			 
											
								switch(g_ucTempbuf[8])
								{
									//-------初始化卡---------
									case 0x01:
														Data_Init_Flag=1;	//开启灯光
												  	//保存房东信息	
													  memcpy(STORE_ACT_INFO,g_ucTempbuf,5);									
														for(i=0;i<5;i++)
														{
															at24cxx_WriteOneByte(ACCOUNTS_INFO_ADDR+i,STORE_ACT_INFO[i]);	
														}		
												  	//保存账户密码	
													  memcpy(STORE_PWD_INFO,&g_ucTempbuf[6],2);									
														for(i=0;i<2;i++)
														{
															at24cxx_WriteOneByte(ACCOUNTS_PWD_ADDR+i,STORE_PWD_INFO[i]);	
														}																		
													  //保存扇区号	
													  Card_Sector=g_ucTempbuf[5];
													  at24cxx_WriteOneByte(CARD_SECTOR_ADDR,Card_Sector);
														
															DEBUG("\r\n Card_Sector=%d ",Card_Sector);

														//工程卡变为有效
														Engine_Card_Flag=0;
													  at24cxx_WriteOneByte(ENGINE_CARD_FLAG_ADDR,Engine_Card_Flag);									
														
																							
														//清空卡片黑名单
														CARD_BL_CLEAR();
														//将开门次数数据清空
														USE_LOCK_CNT_CLEAR();
														//保存的门锁记录全部清空
														LOCK_RECORD_CLEAR();
														//喂狗
														IWDG_ReloadCounter();
														Data_Init_Flag=0;	//关闭灯光
																												
														//程序重启
														NVIC_SystemReset();	
								
									break;
									
									//-------组号卡---------（未初始化卡不允许使用）
//									case 0x02:									
//														Card_Group=g_ucTempbuf[5];
//														at24cxx_WriteOneByte(CARD_GROUP_ADDR,Card_Group);//保存组号			 
//									break;
									
									//-------工程卡---------
									case 0x03:
																		
														if(Engine_Card_Flag==0)//有效
														{
															 return(1);	//正确，开门
																														
														}			
									break;
														
														
									default:
										
									break;
									
								}
						 
					 }
					 
				}
				else	//使用过初始化卡、有扇区号
				{
					
/*=============================================================================================
					
-----------使用过初始化卡、有扇区号--------------					
					
1、先判断记录的扇区内有无正确的数据
					
					
2、若无再判断第3扇区有无正确的数据
					
					
=============================================================================================*/	
				   
				 				 DEBUG("\r\n Card_UID= %02x %02x %02x %02x ",Card_UID[0],Card_UID[1],Card_UID[2],Card_UID[3]);
			

							
							 //--------------验证卡片A密码------------(用户卡，Card_Sector扇区)	
							 status = PcdAuthState(PICC_AUTHENT1A, Card_Sector*4, DefaultKey, Card_UID);	// 写入UID
							 if (status != MI_OK)
							 {   

									 
											DEBUG("\r\n Again PICC_AUTHENT1A_Sector_err Card_Sector=%d ",Card_Sector);

									if(Read_M1_or_CPU_Card(Card_UID,card_type) == 1)	//再次询卡，否则不能再次验证密码		PJW		20181127
									{
										 //--------------验证卡片A密码----------(初始化卡，第3扇区)
										 status = PcdAuthState(PICC_AUTHENT1A, 12, DefaultKey, Card_UID);	// 写入UID
										 if (status != MI_OK)
										 {   

												 
													DEBUG("\r\n Again PICC_AUTHENT1A_12_err ");
		
													return(0);
										 }
										 else		//3扇区密码验证正确
										 {
											 
												 status = PcdRead(12, g_ucTempbuf);//读3扇区0块（0-15扇区，每扇区4个块）					
												 if (status != MI_OK)
												 { 
														
															DEBUG("\r\n Again Read_0_Sector_err ");
															DEBUG("\r\n Card_Sector=%d ",Card_Sector);
						 
															return(0);
												 }				
												 else
												 {
															//和校验
															if( Num_Check(g_ucTempbuf) == 0)
															{
															 
																	DEBUG("\r\n Again Num_Check_err ");
							
																	return(0);
															}
															
															/*----------------判断房东信息------------------	*/			 
															if((g_ucTempbuf[8] != 0x01)&&(g_ucTempbuf[8] != 0x03))	//不是初始化卡和工程卡，需要判断房东信息
															{
															
																	DEBUG("\r\n STORE_ACT_INFO=%02x %02x %02x %02x %02x ",STORE_ACT_INFO[0],STORE_ACT_INFO[1],STORE_ACT_INFO[2],STORE_ACT_INFO[3],STORE_ACT_INFO[4]);
																	DEBUG("\r\n g_ucTempbuf=%02x %02x %02x %02x %02x ",g_ucTempbuf[0],g_ucTempbuf[1],g_ucTempbuf[2],g_ucTempbuf[3],g_ucTempbuf[4]);											
																														
																 if(memcmp(STORE_ACT_INFO,&g_ucTempbuf[0],5) != 0) 	//不完全相同
																 {   
																			
																			DEBUG("\r\n Again ACT_INFO err ");	
																	 
																			return(0);
																 }
																 
																 /*----------------判断账户密码------------------	*/
																 if(memcmp(STORE_PWD_INFO,&g_ucTempbuf[6],2) != 0) 	//不完全相同
																 {   
																			
																			DEBUG("\r\n Again PWD_INFO err ");
		 
																			return(0);
																 }	

															}						 
											
																										 
															switch(g_ucTempbuf[8])
															{
																//-------初始化卡---------
																case 0x01:
																					Data_Init_Flag=1;	//开启灯光
																					//保存房东信息	
																					memcpy(STORE_ACT_INFO,g_ucTempbuf,5);									
																					for(i=0;i<5;i++)
																					{
																						at24cxx_WriteOneByte(ACCOUNTS_INFO_ADDR+i,STORE_ACT_INFO[i]);	
																					}		
																					//保存账户密码	
																					memcpy(STORE_PWD_INFO,&g_ucTempbuf[6],2);									
																					for(i=0;i<2;i++)
																					{
																						at24cxx_WriteOneByte(ACCOUNTS_PWD_ADDR+i,STORE_PWD_INFO[i]);	
																					}																		
																					//保存扇区号	
																					Card_Sector=g_ucTempbuf[5];
																					at24cxx_WriteOneByte(CARD_SECTOR_ADDR,Card_Sector);
																					//工程卡变为有效
																					Engine_Card_Flag=0;
																					at24cxx_WriteOneByte(ENGINE_CARD_FLAG_ADDR,Engine_Card_Flag);									
																					
																					
																					//清空卡片黑名单
																					CARD_BL_CLEAR();
																					//将开门次数数据清空
																					USE_LOCK_CNT_CLEAR();
																					//保存的门锁记录全部清空
																					LOCK_RECORD_CLEAR();
																					//喂狗
																					IWDG_ReloadCounter();
																					Data_Init_Flag=0;	//关闭灯光
																																			
																					//程序重启
																					NVIC_SystemReset();								
																break;
																
																//-------组号卡---------
																case 0x02:									
																					Card_Group=g_ucTempbuf[5];
																					at24cxx_WriteOneByte(CARD_GROUP_ADDR,Card_Group);//保存组号	

																					
																					DEBUG("\r\n Card_Group=%02x ",Card_Group);
											
																break;
																
																//-------工程卡---------
																case 0x03:
																									
																					if(Engine_Card_Flag==0)//有效
																					{
																						return(1);	//正确，开门																						
																					}			
																break;

														}
																		 
												}
														
										}
									 
									}else;
									
							 }
							 else		//用户卡验证成功
							 {
									 
									 DEBUG("\r\n Again Verify OK ");
								
									
									 status = PcdRead(Card_Sector*4, g_ucTempbuf);//读Card_Sector扇区0块（0-15扇区，每扇区4个块）					
									 if (status != MI_OK)
									 { 
												
												DEBUG("\r\n Again Read_0_Sector_err ");
												DEBUG("\r\n Card_Sector=%d ",Card_Sector);
						 
												return(0);
									 }				
									 else
									 {
												//和校验
												if( Num_Check(g_ucTempbuf) == 0)
												{
													 
														DEBUG("\r\n Again Num_Check_err ");
								
														return(0);
												}
												
												/*----------------判断房东信息------------------	*/			 
												if((g_ucTempbuf[8] != 0x01)&&(g_ucTempbuf[8] != 0x03))	//不是初始化卡和工程卡，需要判断房东信息
												{
													
												
														DEBUG("\r\n STORE_ACT_INFO=%02x %02x %02x %02x %02x ",STORE_ACT_INFO[0],STORE_ACT_INFO[1],STORE_ACT_INFO[2],STORE_ACT_INFO[3],STORE_ACT_INFO[4]);
														DEBUG("\r\n g_ucTempbuf=%02x %02x %02x %02x %02x ",g_ucTempbuf[0],g_ucTempbuf[1],g_ucTempbuf[2],g_ucTempbuf[3],g_ucTempbuf[4]);										
											
													
													 if(memcmp(STORE_ACT_INFO,&g_ucTempbuf[0],5) != 0) 	//不完全相同
													 {   
																
																DEBUG("\r\n Again ACT_INFO err ");
														 
																return(0);
													 }
													 
													 /*----------------判断账户密码------------------	*/
													 if(memcmp(STORE_PWD_INFO,&g_ucTempbuf[6],2) != 0) 	//不完全相同
													 {   
														
																DEBUG("\r\n Again PWD_INFO err ");	
														 
																return(0);
													 }	

												}						 
								
																							 
												switch(g_ucTempbuf[8])
												{
													//-------初始化卡---------
													case 0x01:
																		Data_Init_Flag=1;	//开启灯光
																		//保存房东信息	
																		memcpy(STORE_ACT_INFO,g_ucTempbuf,5);									
																		for(i=0;i<5;i++)
																		{
																			at24cxx_WriteOneByte(ACCOUNTS_INFO_ADDR+i,STORE_ACT_INFO[i]);	
																		}		
																		//保存账户密码	
																		memcpy(STORE_PWD_INFO,&g_ucTempbuf[6],2);									
																		for(i=0;i<2;i++)
																		{
																			at24cxx_WriteOneByte(ACCOUNTS_PWD_ADDR+i,STORE_PWD_INFO[i]);	
																		}																		
																		//保存扇区号	
																		Card_Sector=g_ucTempbuf[5];
																		at24cxx_WriteOneByte(CARD_SECTOR_ADDR,Card_Sector);
																		//工程卡变为有效
																		Engine_Card_Flag=0;
																		at24cxx_WriteOneByte(ENGINE_CARD_FLAG_ADDR,Engine_Card_Flag);									
																		
																		
																		//清空卡片黑名单
																		CARD_BL_CLEAR();
																		//将开门次数数据清空
																		USE_LOCK_CNT_CLEAR();
																		//保存的门锁记录全部清空
																		LOCK_RECORD_CLEAR();
																		//喂狗
																		IWDG_ReloadCounter();
																		Data_Init_Flag=0;	//关闭灯光
																																
																		//程序重启
																		NVIC_SystemReset();								
													break;
													
													//-------组号卡---------
													case 0x02:									
																		Card_Group=g_ucTempbuf[5];
																		at24cxx_WriteOneByte(CARD_GROUP_ADDR,Card_Group);//保存组号	

																		
																		DEBUG("\r\n Card_Group=%02x ",Card_Group);
								
													break;
													
													//-------工程卡---------
													case 0x03:
																						
																		if(Engine_Card_Flag==0)//有效
																		{
																			return(1);	//正确，开门
																						
																		}			
													break;
																		
													//-------客人卡---------
													case 0x04:
																	status = PcdRead(Card_Sector*4+1, g_ucTempbuf);//读1块（0-15扇区，每扇区4个块）					
																	 if (status != MI_OK)
																	 {   
																				return(0);
																	 }		

																		//-----和校验-----
																		if( Num_Check(g_ucTempbuf) == 0)
																		{
																			 
																					DEBUG("\r\n Guest B1 Num_Check_err ");
										
																				return(0);
																		}
																	 
																	 //-----比较SN----
																	 if(memcmp(dev_info.sn,&g_ucTempbuf[0],6) != 0) 	//PJW	20181112 SN不同时
																	 {   
																				
																				DEBUG("\r\n Guest SN err ");
								
																				return(0);
																	 }																		 
																	 else
																	 {
																			 status = PcdRead(Card_Sector*4+2, g_ucTempbuf);//读2块（0-15扇区，每扇区4个块）					
																			 if (status != MI_OK)
																			 {   
																						return(0);
																			 }								

																				//-----和校验-----
																				if( Num_Check(g_ucTempbuf) == 0)
																				{
																					 
																							DEBUG("\r\n Guest B2 Num_Check_err ");
								
																						return(0);
																				}
																			 
																			 //-----比较有效期----
																			 RTC_Get();
																			 if( Validity_Compare(&g_ucTempbuf[0],&g_ucTempbuf[6]) == 1 ) //不在有效期
																			 {   
																						
																							DEBUG("\r\n Guest Card Time err ");
																						
																						return(0);
																			 }
																			 else
																			 {																    		
																						return(1);	//正确，开门
																			 }

																	 }
																			
													break;														
																		
													//-------员工卡---------
													case 0x05:							
																	Staff_Card_Type=g_ucTempbuf[9];		//员工卡类型
													
																	status = PcdRead(Card_Sector*4+1, g_ucTempbuf);//读1块（0-15扇区，每扇区4个块）					
																	 if (status != MI_OK)
																	 {   
																				return(0);
																	 }	

																		//-----和校验-----
																		if( Num_Check(g_ucTempbuf) == 0)
																		{
																			 
																					DEBUG("\r\n Staff B1 Num_Check_err ");
																					
																				return(0);
																		}
																	 
																		memcpy(Read_Group,&g_ucTempbuf[0],6);	//读出的组号信息
																		
																		memcpy(Week_Buf,&g_ucTempbuf[6],7);	//星期标志
																		
																		
																				DEBUG("\r\n g_ucTempbuf=%02x %02x %02x %02x %02x %02x",g_ucTempbuf[0],g_ucTempbuf[1],g_ucTempbuf[2],g_ucTempbuf[3],g_ucTempbuf[4],g_ucTempbuf[5]);
																				DEBUG("\r\n Card_Group=%02x ",Card_Group);														

					 
																	 //-----比较周期（星期）-------													 													
																	 status = PcdRead(Card_Sector*4+2, g_ucTempbuf);//读2块（0-15扇区，每扇区4个块）					
																	 if (status != MI_OK)
																	 {   
																				
																				DEBUG("\r\n Staff B2 err ");
																															 
																				return(0);
																	 }								

																		//-----和校验-----
																		if( Num_Check(g_ucTempbuf) == 0)
																		{
																			 
																					DEBUG("\r\n Staff B2 Num_Check_err ");
																								
																				return(0);
																		}
																		
																		switch(Staff_Card_Type)
																		{
																			//限时卡
																			case 0x01:																		
																							 //-----比较组-------
																								if(Read_Group[0] == 0)
																								{
																										
																										DEBUG("\r\n No Group ");
																																		  
																										return(0);															
																								}
																								else
																								{
																									 for(i=1;i<Read_Group[0]+1;i++)
																									 {
																											if( Card_Group == Read_Group[i] )
																											{
																												 Group_Sta=1;	//找到相同的组
																												 break;
																											}else;
																									 }
																								}	 
																								
																							 if(Group_Sta == 0)
																							 {
																									
																										DEBUG("\r\n Group err ");
																																			  
																									return(0);
																							 }																		
																			
																							 //-----比较有效期----
																							 RTC_Get();
																							 if( Validity_Compare(&g_ucTempbuf[0],&g_ucTempbuf[6]) == 1 ) //不在有效期
																							 {   
																										
																											DEBUG("\r\n Staff Card Time err ");
																										
																										return(0);
																							 }
																							 else
																							 {
																										return(1);	//正确，开门
																							 }
																			break;
																							 
																			//周期循环卡
																			case 0x02:						
																							 //-----比较组-------
																								if(Read_Group[0] == 0)
																								{
																										
																											DEBUG("\r\n No Group ");
																																							  
																										return(0);															
																								}
																								else
																								{
																									 for(i=1;i<Read_Group[0]+1;i++)
																									 {
																											if( Card_Group == Read_Group[i] )
																											{
																												 Group_Sta=1;	//找到相同的组
																												 break;
																											}else;
																									 }
																								}	 
																								
																							 if(Group_Sta == 0)
																							 {
																									
																										DEBUG("\r\n Group err ");
																																				  
																									return(0);
																							 }	

																							//-----比较周期（星期）-------
																								RTC_Get();
																								
																								DEBUG("\r\n Week_Buf=%02x %02x %02x %02x %02x %02x %02x",Week_Buf[0],Week_Buf[1],Week_Buf[2],Week_Buf[3],Week_Buf[4],Week_Buf[5],Week_Buf[6]);
																								DEBUG("\r\n calendar.week=%02x ",calendar.week);
																																																				
																								if(Week_Buf[calendar.week-1] == 0)	//本星期无效
																								{
																										
																											DEBUG("\r\n Staff Week err ");
																							
																										return(0);																
																								}
																								else
																								{
																										return(1);	//正确，开门
																								}																						
																							 
																			break;
																			
																			//总卡 --- 无限制  不比较组号
																			case 0x03:
																									 return(1);	//正确，开门
																							 
																			break;
																				
																			//巡检卡 --- “滴”不开门  不比较组号
																			case 0x04:

				//																					 Beep_on_key();
																			break;
																			

																			default:
																				
																			break;
																			
																		}

													
													break;														
																		
																		
													//-------应急卡---------
													case 0x06:			
														
																	 status = PcdRead(Card_Sector*4+2, g_ucTempbuf);//读2块（0-15扇区，每扇区4个块）					
																	 if (status != MI_OK)
																	 {   
																				return(0);
																	 }								

																		//-----和校验-----
																		if( Num_Check(g_ucTempbuf) == 0)
																		{
																			 
																				DEBUG("\r\n Emergency B2 Num_Check_err ");
																						
																				return(0);
																		}
																	 
																	 //-----比较有效期----
																	 if( Validity_Compare(&g_ucTempbuf[0],&g_ucTempbuf[6]) == 1 ) //不在有效期
																	 {   
																				
																				DEBUG("\r\n Emergency Card Time err ");
															
																				return(0);
																	 }
																	 else
																	 {
																				return(1);	//正确，开门
																		 
																	 }	

													break;														
																		
													//-------挂失卡-------（卡片黑名单）
													case 0x07:									
																	status = PcdRead(Card_Sector*4+1, g_ucTempbuf);//读1块（0-15扇区，每扇区4个块）					
																	 if (status != MI_OK)
																	 {   
																				return(0);
																	 }								
																	 
																		//-----和校验-----
																		if( Num_Check(g_ucTempbuf) == 0)
																		{
																			 
																				DEBUG("\r\n Loss B1 Num_Check_err ");
																					
																				return(0);
																		}													 
																	 
																	 //-----比较SN-----
																	 if(memcmp(dev_info.sn,&g_ucTempbuf[0],6) != 0) 	//PJW	20181112 SN不同时
																	 {   
																				
																				DEBUG("\r\n Loss SN err ");														
																		 
																				return(0);
																	 }	
																	 else
																	 {
																		 
																			if( Bl_Card_Check(&g_ucTempbuf[6]) == 0)	//黑名单中存在														 
																			{
																				
																				DEBUG("\r\n Bl_Card Exsit ");																
																				
																				return(0);
																			}
																			else	//不存在
																			{
																				
																				idex_bl=at24cxx_ReadOneByte(CARD_BL_IDEX);	//读出索引
																		 
																				if(idex_bl>=CARD_BL_NUM)
																				{
																						idex_bl = 0;
																				}
																																		 
																					DEBUG(" idex_bl=%02d \n ",idex_bl);										 													 
																		 

																				for(j=0;j<BL_CARD_len;j++)	//4位UID
																				{
																						at24cxx_WriteOneByte(BL_CARD_ADDR+(idex_bl*BL_CARD_len)+j,g_ucTempbuf[j+6]);
																				}														 

																				idex_bl++;
																				
																				
																						for(j=0;j<BL_CARD_len;j++)	//4位UID
																						{
																								UID_Temp[j]=at24cxx_ReadOneByte(BL_CARD_ADDR+(idex_bl*BL_CARD_len)+j);
																						}																						
																				
																						DEBUG("\r\n Read_BlUID=%02x %02x %02x %02x ",UID_Temp[0],UID_Temp[1],UID_Temp[2],UID_Temp[3]);
																				
																				at24cxx_WriteOneByte(CARD_BL_IDEX,idex_bl);	//写入索引
																			}
																		 
																	 }

																		//挂失成功后 亮灯“双闪”，蜂鸣器“滴滴”
													break;														
																			
																		
																		
													default:
														
													break;
													
												}


											}
									 
								 
							 }
							

					 }
					  
						DEBUG("\r\n Again PICC_AUTHENT1A_ok ");						
					
					 //验证3扇区密码正确
						 status = PcdRead(12, g_ucTempbuf);//读3扇区0块（0-15扇区，每扇区4个块）					
						 if (status != MI_OK)
						 { 
									
									DEBUG("\r\n Again Read_0_Sector_err ");
									DEBUG("\r\n Card_Sector=%d ",Card_Sector);
						 
									return(0);
						 }				
						 else
						 {


					
				}

 
				 //---进入休眠模式---
//				 PcdHalt();
			 
					
						DEBUG("\r\n card_test_ok ");
		
				 return(1);
				
}

#endif

//--------------------------------读卡测试---------------------------------------
uint8_t rc520_test(void)
{
    unsigned char status;
    unsigned char data1[16] = {0x12, 0x34, 0x56, 0x78, 0xED, 0xCB, 0xA9, 0x87, 0x12, 0x34, 0x56, 0x78, 0x01, 0xFE, 0x01, 0xFE};
    //M1卡的某一块写为如下格式，则该块为钱包，可接收扣款和充值命令
    //4字节金额（低字节在前）＋4字节金额取反＋4字节金额＋1字节块地址＋1字节块地址取反＋1字节块地址＋1字节块地址取反
    unsigned char data2[4] = {0x12, 0, 0, 0};
    unsigned char DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //扇区密码
    unsigned char g_ucTempbuf[20];

    status = PcdRequest(PICC_REQALL, g_ucTempbuf); //第一次寻卡
    if (status != MI_OK)
    {
        status = PcdRequest(PICC_REQALL, g_ucTempbuf); //第二次寻卡
        if (status != MI_OK)
        {
            status = PcdRequest(PICC_REQALL, g_ucTempbuf); //第三次次寻卡
            if (status != MI_OK)
            {
                return (0); //	continue;
            }
        }
    }

    DEBUG("\r\n  m1 card ");

    status = PcdAnticoll(g_ucTempbuf); //防冲撞
    if (status != MI_OK)
    {
        return (0); //continue;
    }

    DEBUG("\r\n PcdAnticoll_ok ");

    status = PcdSelect(g_ucTempbuf); //选定卡片
    if (status != MI_OK)
    {
        return (0); //continue;
    }

    DEBUG("\r\n PcdSelect_ok ");

    //         status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);//验证卡片密码
    //         if (status != MI_OK)
    //         {
    //
    //							DEBUG("\r\n PICC_AUTHENT1A_err ");
    //
    //					   return(0);//continue;
    //
    //				 }
    //
    //						DEBUG("\r\n PICC_AUTHENT1A_ok ");
    //
    //
    //         status = PcdWrite(1, data1);//写块
    //         if (status != MI_OK)
    //         {
    //						return(0);//continue;
    //					}
    //
    //					 DEBUG("\r\n card_write_data_ok ");
    //
    //         status = PcdValue(PICC_DECREMENT,1,data2);//扣款和充值
    //         if (status != MI_OK)
    //         {
    //
    //					 DEBUG("\r\n PcdValue_err ");
    //
    //					 return(0);//continue;

    //				 }
    //
    //
    //					DEBUG("\r\n card_write_data_ok ");
    //
    //         status = PcdBakValue(1, 2);//块备份
    //         if (status != MI_OK)
    //         {
    //							return(0);//continue;
    //					}
    //
    //         status = PcdRead(2, g_ucTempbuf);//读块
    //         if (status != MI_OK)
    //         {
    //							return(0);//continue;
    //				 }

    //---进入休眠模式---
    PcdHalt();

    DEBUG("\r\n card_test_ok ");

    //					while(1)
    //					{
    //						status = PcdRequest(PICC_REQALL, g_ucTempbuf);
    //						 if (status != MI_OK)
    //						 {
    //
    //								break;
    //						 }
    //					}
    return (1);
}
