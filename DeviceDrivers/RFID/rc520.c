/******************************************************************************
  * @�ļ���     �� rc520.c
  * @����       �� 2016��06��05��
  * @ժҪ       �� RFIDˢ����֤
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

uint8_t Card_Sector = 0;      //�������ţ���Ҫ����
uint8_t Card_Group = 0;       //����ţ���Ҫ����
uint8_t Engine_Card_Flag = 0; //���̿���Ч��־
uint8_t STORE_ACT_INFO[5];    //�洢�ķ�����Ϣ
uint8_t STORE_PWD_INFO[2];    //�洢���˻�����
uint8_t Data_Init_Flag = 0;   //�����Ƭ���ݱ�־
uint8_t exti6_flag = 0;
uint8_t exti6_read_flag = 0;
uint8_t RFID_to_TOUCH_Flag = 0;

static unsigned char MRcvBuffer[72], MSndBuffer[72];
static MfCmdInfo MInfo;
static unsigned char g_cCidNad = 0;

//--------------------------------------���Ƶ�Դ����----------------------------------------
void open_cv520(void)
{

    //		GPIO_ResetBits(GPIOA, GPIO_Pin_8);  //�򿪿��Ƶ�Դ  PA1
    //		rc520_init();
}

//-----------------------------------IO����-------------------------------------------
//void rc520_gpio_init(void)
//{

//    GPIO_InitTypeDef GPIO_InitStructure;

//    RCC_AHBPeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;        //CS
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //�������
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //���ģʽ
//    GPIO_Init(GPIOA, &GPIO_InitStructure);

//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5; //SCK-MOSI
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      //�������
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       //���ģʽ
//    GPIO_Init(GPIOB, &GPIO_InitStructure);

//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; //MISO
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//    GPIO_Init(GPIOB, &GPIO_InitStructure);

//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //IRQ
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//    GPIO_Init(GPIOB, &GPIO_InitStructure);
//}
//-----------------------------------RFID��ʼ��-------------------------------------------
void rc520_init(void)
{
    DEBUG("reset rc520");
    PcdReset();

    PcdAntennaOff();
    delay_ms(10);

    DEBUG("config pcd for A mode");
    pcd_config('A'); //����ΪAģʽ
    PcdAntennaOn();  //��������

    //		 pcd_lpcd_start();
}

//�͹���ģʽ

/*
	lpcd���ܿ�ʼ����
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

//------------------------------����LPCD����ģʽ------------------------------
void pcd_lpcd_start()
{

    DEBUG("pcd_lpcd_start\n");

    write_reg(0x01, 0x0F); //�ȸ�λ�Ĵ����ٽ���lpcd

    write_reg(0x37, 0x00); //�ָ��汾��
    if (read_reg(0x37) == 0x10)
    {                          //NFC_V10
        write_reg(0x01, 0x00); // idle
    }
    write_reg(0x14, 0x8B); // Tx2CW = 1 ��continue�ز������

    write_reg(0x37, 0x5e); // ��˽�мĴ�����������

    //write_reg(0x3c, 0x30);	//����Delta[3:0]��ֵ, ����32k //0 ����ʹ��
    //write_reg(0x3c, 0x31);	//����Delta[3:0]��ֵ, ����32k
    //write_reg(0x3c, 0x32);	//����Delta[3:0]��ֵ, ����32k
    //write_reg(0x3c, 0x33);	//����Delta[3:0]��ֵ, ����32k
    //	write_reg(0x3c, 0x35);	//����Delta[3:0]��ֵ, ����32k
    //	write_reg(0x3c, 0x35);	//����Delta[3:0]��ֵ, ����32k XU
    //	write_reg(0x3c, 0x37);	//����Delta[3:0]��ֵ, ����32k XU
    //write_reg(0x3c, 0x3A);	//����Delta[3:0]��ֵ, ����32k XU
    //write_reg(0x3c, 0x3F);	//����Delta[3:0]��ֵ, ����32k XU

#if Antenna_Model > 0
    write_reg(0x3c, 0x37); //����Delta[3:0]��ֵ, ����32k
#else
    write_reg(0x3c, 0x35); //����Delta[3:0]��ֵ, ����32k
#endif

    write_reg(0x3d, 0x0d); //��������ʱ��
    write_reg(0x3e, 0xA5); //��������̽����� 3�Σ�����LPCD_en
    write_reg(0x37, 0x00); // �ر�˽�мĴ�����������
    write_reg(0x03, 0x20); //�򿪿�̽���ж�ʹ��
    write_reg(0x01, 0x10); //PCD soft powerdown

    //����Ӧ����أ�����Ϊ�ߵ�ƽΪ���ж�
    clear_bit_mask(0x02, BIT7);
}

//-----------------------------�ر�LPCD------------------------------
void pcd_lpcd_end()
{

    DEBUG("pcd_lpcd_end\n");
    write_reg(0x01, 0x0F); //�ȸ�λ�Ĵ����ٽ���lpcd
}

uint8_t pcd_lpcd_check()
{
    //if (INT_PIN && (read_reg(DivIrqReg) & BIT5)) //TagDetIrq
    //if ((read_reg(DivIrqReg) & BIT5))
    if (INT_PIN) //TagDetIrq
    {

        write_reg(DivIrqReg, BIT5); //�������⵽�ж�
        pcd_lpcd_end();
        return 1;
    }
    return 0;
}

void clear_nfc_flag(void)
{

    write_reg(DivIrqReg, BIT5); //�������⵽�ж�
}

//**********************************************************

/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC632�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//��    �أ�������ֵ
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
//��    �ܣ�дRC632�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//          value[IN]:д���ֵ
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
//��    �ܣ���RC522�Ĵ���λ
//����˵����reg[IN]:�Ĵ�����ַ
//          mask[IN]:��λֵ
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
//��    �ܣ���RC522�Ĵ���λ
//����˵����reg[IN]:�Ĵ�����ַ
//          mask[IN]:��λֵ
/////////////////////////////////////////////////////////////////////
void ClearBitMask(unsigned char reg, unsigned char mask)
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask); // clear bit mask
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���λRC522
//��    ��: �ɹ�����MI_OK
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
        ; //�ж�NFC�Ƿ��������

#if NFC_LPCD < 1
    WriteRawRC(ModeReg, 0x3D); //��Mifare��ͨѶ��CRC��ʼֵ0x6363
    WriteRawRC(TReloadRegL, 30);
    WriteRawRC(TReloadRegH, 0);
    WriteRawRC(TModeReg, 0x8D);
    WriteRawRC(TPrescalerReg, 0x3E);
    WriteRawRC(TxAskReg, 0x40); //TxAutoReg
#endif

    return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//��������
//ÿ��������ر����շ���֮��Ӧ������1ms�ļ��
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
//�ر�����
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
//����RC522�Ĺ�����ʽ
//////////////////////////////////////////////////////////////////////
void M500PcdConfigISOTypeA(void) //ISO14443_A//A:NXP,B:MOTO
{
    ClearBitMask(Status2Reg, 0x08); //����Ĵ���
    WriteRawRC(ModeReg, 0x3D);      //3F ѡ��ģʽ
    WriteRawRC(RxSelReg, 0x86);     //84  �ڲ���������
    WriteRawRC(RFCfgReg, 0x7F);     //4F  ��������

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
 * ����оƬ��A/Bģʽ
 *
 * @param: uint8_t type   
 * @return: 
 * @retval: 

    WriteRawRC(ModeReg,0x3D);            //��Mifare��ͨѶ��CRC��ʼֵ0x6363
    WriteRawRC(TReloadRegL,30);           
    WriteRawRC(TReloadRegH,0);
    WriteRawRC(TModeReg,0x8D);
    WriteRawRC(TPrescalerReg,0x3E);
    WriteRawRC(TxAskReg,0x40);

 ****************************************************************
 */

//��ʼ������

char pcd_config(uint8_t type)
{
    PcdAntennaOff();

    delay_ms(7);

    if ('A' == type)
    {
        clear_bit_mask(Status2Reg, BIT3);
        clear_bit_mask(ComIEnReg, BIT7); // �ߵ�ƽ
        write_reg(ModeReg, 0x3D);        // 11 // CRC seed:6363
        write_reg(RxSelReg, 0x86);       //RxWait
        write_reg(RFCfgReg, 0x58);       //
        write_reg(TxAskReg, 0x40);       //15  //typeA
        write_reg(TxModeReg, 0x00);      //12 //Tx Framing A
        write_reg(RxModeReg, 0x00);      //13 //Rx framing A
        write_reg(0x0C, 0x10);           //^_^

        //��������
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
                WriteRawRC(0x29, 0x12); //0x0F); //����ָ��
                WriteRawRC(0x3B, 0xA5);
            }
            else if (adc == 0x12)
            {
                //���¼Ĵ�����Ҫ��˳������
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
                //���¼Ĵ�����Ҫ��˳������
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
        write_reg(Status2Reg, 0x00);     //��MFCrypto1On
        clear_bit_mask(ComIEnReg, BIT7); // �ߵ�ƽ�����ж�
        write_reg(ModeReg, 0x3F);        // CRC seed:FFFF
        write_reg(RxSelReg, 0x85);       //RxWait
        write_reg(RFCfgReg, 0x58);       //
        //Tx
        write_reg(GsNReg, 0xF8);    //����ϵ��
        write_reg(CWGsPReg, 0x3F);  //
        write_reg(ModGsPReg, 0x17); //����ָ��		//0xOE
        write_reg(AutoTestReg, 0x00);
        write_reg(TxAskReg, 0x00); // typeB
        write_reg(RFU1E, 0x13);
        write_reg(TxModeReg, 0x83);     //Tx Framing B
        write_reg(RxModeReg, 0x83);     //Rx framing B
        write_reg(BitFramingReg, 0x00); //TxLastBits=0

        //��������
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
            //				write_reg(0x29, 0x12);//0x0F); //����ָ��
            //				write_reg(0x3b, 0x25);
            //			}
            //			else if (adc == 0x12)
            {
                //���¼Ĵ�����Ҫ��˳������
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
//��MF522����CRC16����
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
//����PCD��ʱ��
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
    MF522_RST_L; //�ز���ʧ
    delay_ms(20);
    MF522_RST_H;

    delay_ms(20);
// GPIO_Init(GPIOC, GPIO_PIN_7, GPIO_MODE_IN_FL_NO_IT);
//while(ReadRawRC(0x27) != 0x88);
#endif
    delay_ms(4000);
    ; //delay 500us ��ͻ�ȷ��delayus������ʱ׼ȷ��

    WriteRawRC(ModeReg, 0x3D);       //��Mifare��ͨѶ��CRC��ʼֵ0x6363
    WriteRawRC(TxAskReg, 0x40);      //100%ASK����
    WriteRawRC(BitFramingReg, 0x07); //

    // WriteRawRC(TxControlReg,0x83); //T1��T2���;����������Ƶ��ز��������ز�0X83

    WriteRawRC(ComIEnReg, irqEn | 0x80); //
    WriteRawRC(ComIrqReg, 0x14);         //
    WriteRawRC(CommandReg, PCD_IDLE);    //
    WriteRawRC(FIFOLevelReg, 0x80);      //

    // WriteRawRC(FIFODataReg,PICC_REQIDL); //�Ƚ�����д��fifo  0X26
    WriteRawRC(0X09, 0X26); //�Ƚ�����д��fifo  0X26
    //WriteRawRC(CommandReg, PCD_TRANSCEIVE);//ִ�в�������д�뿨��
    WriteRawRC(TxControlReg, 0x83); //T1��T2���;����������Ƶ��ز��������ز�0X83
    WriteRawRC(0x01, 0x0C);         //ִ�в�������д�뿨��

    //WriteRawRC(BitFramingReg,0x87);//�������ݷ���
    WriteRawRC(0x0D, 0x87); //�������ݷ���

    //GPIOC->ODR |= 0x10;
    times = 18;
    do
    {
        errflag = ReadRawRC(ComIrqReg); //�ж��Ƿ����жϷ���
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
//����PCD����
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
        //rxwait�����106����������������Ӧ����
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
        //rxwait�����106����������������Ӧ����
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

//-------------------------------------ѯ��-----------------------------------------
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
//	//һ�η���ͻ��ѡ��
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
//	//���η���ͻ��ѡ��
//	if(status == MI_OK && (sak & BIT2))
//	{
//		g_tag_info.uid_length = UID_7;
//		status = pcd_cascaded_anticoll(PICC_ANTICOLL2, 0, &g_tag_info.serial_num[4]);
//		if(status == MI_OK)
//		{
//			status = pcd_cascaded_select(PICC_ANTICOLL2, &g_tag_info.serial_num[4], &sak);
//		}
//	}
//	//�ظ�uid
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

//---------------------------------���֤��������------------------------------------

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

    cnt = 5; //Ӧ���� ����ʹ����ѯN��
    while (cnt--)
    {
        status = pcd_request_b(req_code, 0, 0, ATQB);

        if (status == MI_COLLERR) // �г�ͻ������һ�ſ�
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
        //typeB 106Ĭ������
        status = pcd_attri_b(&ATQB[1], 0, ATQB[10] & 0x0f, PICC_CID, ATQB);

        if (status == MI_OK)
        {
            ATQB[0] = 0x50; //�ָ�Ĭ��ֵ
                            //make_packet(COM_PKT_CMD_REQB, ATQB, 12);
        }
    }

    if (status == MI_OK)
    {
        com_get_idcard_num(id); //��ȡ����
        MF522_RST_L;            //522��λ
        delay_ms(10);           //PJW	20190315	������ʱ

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

//-------------------------------���֤��������-----------------------------
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

    cnt = 5; //Ӧ���� ����ʹ����ѯN��
    while (cnt--)
    {
        status = pcd_request_b(req_code, 0, 0, ATQB);

        if (status == MI_COLLERR) // �г�ͻ������һ�ſ�
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
        //typeB 106Ĭ������
        status = pcd_attri_b(&ATQB[1], 0, ATQB[10] & 0x0f, PICC_CID, ATQB);

        if (status == MI_OK)
        {
            ATQB[0] = 0x50; //�ָ�Ĭ��ֵ
                            //make_packet(COM_PKT_CMD_REQB, ATQB, 12);
        }
    }

    if (status == MI_OK)
    {
        //GPIO_WriteHigh(GPIOA, GPIO_PIN_1);//
        com_get_idcard_num(); //��ȡ����
        MF522_RST_L;          //522��λ
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
 * ͨ��оƬ��ISO14443��ͨѶ
 *
 * @param: pi->mf_command = оƬ������
 * @param: pi->mf_length  = ���͵����ݳ���
 * @param: pi->mf_data[]  = ��������
 * @return: status ֵΪMI_OK:�ɹ�
 * @retval: pi->mf_length  = ���յ�����BIT����
 * @retval: pi->mf_data[]  = ��������
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
        write_reg(ComIEnReg, irq_inv | irq_en | BIT0); //ʹ��Timer ��ʱ���ж�
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

                write_reg(ComIrqReg, BIT2); //��write fifo֮��������жϱ�ǲſ���

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

        //�ȴ�����ִ�����
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
                write_reg(ComIrqReg, BIT3); //��read fifo֮��������жϱ�ǲſ���
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
        write_reg(ComIrqReg, val); // ���ж�

        if (val & BIT0)
        { //������ʱ
            status = MI_NOTAGERR;
        }
        else
        {
            err = read_reg(ErrorReg);

            status = MI_COM_ERR;
            if ((val & wait_for) && (val & irq_en))
            {
                if (!(val & ErrIRq))
                { //ָ��ִ����ȷ
                    status = MI_OK;

                    if (recebyte)
                    {
                        val = 0x7F & read_reg(FIFOLevelReg);
                        last_bits = read_reg(ControlReg) & 0x07;
                        if (len_rest + val > MAX_TRX_BUF_SIZE)
                        { //���ȹ�����������
                            status = MI_COM_ERR;
#if (NFC_DEBUG)
                            DEBUG("RX_LEN > 255B\n");
#endif
                        }
                        else
                        {
                            if (last_bits && val) //��ֹspi����� val-1��Ϊ��ֵ
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
                        { //���ȹ�����������
#if (NFC_DEBUG)
                            DEBUG("COLL RX_LEN > 255B\n");
#endif
                        }
                        else
                        {
                            if (last_bits && val) //��ֹspi����� val-1��Ϊ��ֵ
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
                    pi->mf_data[0]--; // ��֮ǰ�汾�е�ӳ������Ϊ�˲��ı��ϲ���룬����ֱ�Ӽ�һ��
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
                    { //���ȹ�����������
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

        set_bit_mask(ControlReg, BIT7);  // TStopNow =1,��Ҫ�ģ�
        write_reg(ComIrqReg, 0x7F);      // ���ж�0
        write_reg(DivIrqReg, 0x7F);      // ���ж�1
        clear_bit_mask(ComIEnReg, 0x7F); //���ж�ʹ��,���λ�ǿ���λ
        clear_bit_mask(DivIEnReg, 0x7F); //���ж�ʹ��,���λ�ǿ���λ
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
//��ȡB����ID��
//
/////////////////////////////////////////////////////////////////////
void com_get_idcard_num(uint8_t *uid)
{
    uint8_t id[10];
    char status, i;
    //char COM_PKT_CMD_TYPEB_UID=0X37;

    status = get_idcard_num(id); //��ȡID��
    if (status == MI_OK)
    {
        //		Beep_on_close_clock(2);

        DEBUG("\r\n  ID_card=%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \r\n ", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7], id[8], id[9]);
    }
    memcpy(uid, id, 4);
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�ͨ��RC522��ISO14443��ͨѶ
//����˵����Command[IN]:RC522������
//          pInData[IN]:ͨ��RC522���͵���Ƭ������
//          InLenByte[IN]:�������ݵ��ֽڳ���
//          pOutData[OUT]:���յ��Ŀ�Ƭ��������
//          *pOutLenBit[OUT]:�������ݵ�λ����
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

    i = 600; //����ʱ��Ƶ�ʵ���������M1�����ȴ�ʱ��25ms
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
//��    �ܣ���ȡM1��һ������
//����˵��: addr[IN]�����ַ
//          pData[OUT]�����������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ�д���ݵ�M1��һ��
//����˵��: addr[IN]�����ַ
//          pData[IN]��д������ݣ�16�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ����Ƭ��������״̬
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ�Ѱ��
//����˵��: req_code[IN]:Ѱ����ʽ
//                0x52 = Ѱ��Ӧ�������з���14443A��׼�Ŀ�
//                0x26 = Ѱδ��������״̬�Ŀ�
//          pTagType[OUT]����Ƭ���ʹ���
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ�����ײ
//����˵��: pSnr[OUT]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
/////////////////////////////////////////////////////////////////////
char PcdAnticoll(unsigned char *pSnr)
{
    char status;
    unsigned char i, snr_check = 0;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ClearBitMask(Status2Reg, 0x08);  //��RC522�Ĵ���λ
    WriteRawRC(BitFramingReg, 0x00); //дRC632�Ĵ���
    ClearBitMask(CollReg, 0x80);     //��RC522�Ĵ���λ

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

    SetBitMask(CollReg, 0x80); //��RC522�Ĵ���λ
    return status;
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�ѡ����Ƭ
//����˵��: pSnr[IN]:��Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ���֤��Ƭ����
//����˵��: auth_mode[IN]: ������֤ģʽ
//                 0x60 = ��֤A��Կ
//                 0x61 = ��֤B��Կ
//          addr[IN]�����ַ
//          pKey[IN]������
//          pSnr[IN]����Ƭ���кţ�4�ֽ�
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ��ۿ�ͳ�ֵ
//����˵��: dd_mode[IN]��������
//               0xC0 = �ۿ�
//               0xC1 = ��ֵ
//          addr[IN]��Ǯ����ַ
//          pValue[IN]��4�ֽ���(��)ֵ����λ��ǰ
//��    ��: �ɹ�����MI_OK
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
//��    �ܣ�����Ǯ��
//����˵��: sourceaddr[IN]��Դ��ַ
//          goaladdr[IN]��Ŀ���ַ
//��    ��: �ɹ�����MI_OK
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

    write_reg(0x01, 0x0F); //�ȸ�λ�Ĵ����ٽ���lpcd

    write_reg(0x14, 0x8B);                                                   // Tx2CW = 1 ��continue�ز������
    write_reg(0x37, 0x00);                                                   //�ָ��汾��
    write_reg(0x37, 0x5e);                                                   // ��˽�мĴ�����������
    write_reg(0x3c, 0x30 | delta);                                           //����Delta[3:0]��ֵ, ����32k
    write_reg(0x3d, WUPeriod);                                               //��������ʱ��
    write_reg(0x3e, 0x80 | ((skip_times & 0x07) << 4) | (SwingsCnt & 0x0F)); //����LPCD_en����,����̽�������̽��ʱ��
    write_reg(0x37, 0x00);                                                   // �ر�˽�мĴ�����������
    write_reg(0x03, 0x20);                                                   //�򿪿�̽���ж�ʹ��
    write_reg(0x01, 0x10);                                                   //PCD soft powerdown

    //����Ӧ����أ���ʾ����������Ϊ�ߵ�ƽΪ���ж�
    clear_bit_mask(0x02, BIT7);
}

//=======================================================================================================
//
//																					CPU��������
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
          ��ʱ���Ļ���ʱ��Ϊ100us
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
//��    �ܣ�ͨ��RC522��ISO14443��ͨѶ		CPU��ʹ��
//����˵����Command[IN]:RC522������
//          pInData[IN]:ͨ��RC522���͵���Ƭ������
//          InLenByte[IN]:�������ݵ��ֽڳ���
//          pOutData[OUT]:���յ��Ŀ�Ƭ��������
//          *pOutLenBit[OUT]:�������ݵ�λ����
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
    case PCD_AUTHENT: //��֤��Կ
        irqEn = 0x12;
        waitFor = 0x10;
        break;
    case PCD_TRANSCEIVE: //���Ͳ���������
        irqEn = 0x77;
        waitFor = 0x30;
        break;
    default:
        break;
    }

    WriteRawRC(ComIEnReg, irqEn | 0x80); //ComIEnReg:�ж������ʹ��λ����֤��Կ����10010010��λ7���ܽ�IRQ��Status1Reg��IRq���ࡣλ4����������ж�����IdleIRqλ�����ݵ�IRQ�ܽ��ϡ�λ1����������ж�����ErrIRqλ�����ݵ�IRQ�ܽ��ϡ�                                        //                             ���Ͳ��������ݣ���11110111������λ�ж������⣬�������ܴ���IRQ�ܽ���
    ClearBitMask(ComIrqReg, 0x80);       //ComIrqReg������λ����
    WriteRawRC(CommandReg, PCD_IDLE);    //CommandReg��4λд0000B�����ڿ���ģʽ
    SetBitMask(FIFOLevelReg, 0x80);      //FIFOLevelReg��FlushBufferλ��1����ʾ����������дָ��������������������ݣ��������һ�������ݣ�ErrReg��BufferOvfl���

    for (i = 0; i < InLenByte; i++)
    {
        WriteRawRC(FIFODataReg, pInData[i]); //��pInData���������д��FIFO������
    }
    WriteRawRC(CommandReg, Command); //��֤��Կ or ���Ͳ���������

    if (Command == PCD_TRANSCEIVE)
    {
        SetBitMask(BitFramingReg, 0x80); //�������ݵķ���
    }

    i = 600; //����ʱ��Ƶ�ʵ���������M1�����ȴ�ʱ��25ms
    do
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitFor)); //���i��Ϊ0�����Ҷ�ʱ��û����0������û��δ֪�����������ֹ����ͽ�����û�м�⵽��Ч���������ͼ���ѭ�����˳�ѭ�����ʾ�������
    ClearBitMask(BitFramingReg, 0x80);                   //StartSendλ����

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
    WriteRawRC(CommandReg, PCD_IDLE); //����
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
Request ָ�֪ͨMCM��������Ч�Ĺ�����Χ�����룩��Ѱ��MIFARE 1��Ƭ�������
MIFARE 1��Ƭ���ڣ���һָ��ֱ���MIFARE 1����ͨ�ţ���ȡMIFARE 1��Ƭ�ϵĿ�Ƭ
���ͺ�TAGTYPE��2���ֽڣ�����MCM���ݸ�MCU������ʶ����
����Ա���Ը���TAGTYPE������Ƭ�Ĳ�ͬ���͡�
����MIFARE 1��Ƭ��˵�����ؿ�Ƭ��TAGTYPE��2���ֽڣ�����Ϊ0004h��
* Function:     Mf500PiccRequest                                              *
* Input:        req_code                                                      *
* Output:       TagType                                                     *
* req_code			Ѱ����ʽ
* atq						���� 4�ֽ�
****************************************************************************/

/*************************************************
Function:
		Deselect
Description:
		CPU��ͣ��
Parameter:
		atd: 		��Ƭ��λ��Ӧ
		RcByte:	���ص����ݳ���
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
//Mifare_Pro��λ
//input: parameter = PCD BUFERR SIZE
//output:pLen      = ��λ��Ϣ����
//       pData     = ��λ��Ϣ
/////////////////////////////////////////////////////////////////////
char MifareProRst(unsigned char parameter, unsigned char *pData, unsigned char *pLen)
{
    char status;
    unsigned int unLen;
    unsigned char rcByte;
    unsigned char ucComMF522Buf[MAXRLEN];

    //    PcdSetTmo(4);     // long timeout
    //    WriteRC(RegChannelRedundancy,0x0f); // RxCRC, TxCRC, Parity enable
    WriteRawRC(TxModeReg, 0x80); //ʹ�������ݷ��͹����в��� CRC
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
    //CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);//����У��λ
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
//��ISO14443-4������COS����
//input:CID_NAD  = �Ƿ����CID��NAD
//      timeout  = FWI
//      pLen     = �����
//      pCommand = COS����
//ouput:pLen     = �������ݳ���
//      pCommand = ��������
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
    WriteRawRC(TxModeReg, 0x80); //ʹ�������ݷ��͹����в��� CRC
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

        if ((MRcvBuffer[0] & 0xF0) == 0x00) //����ͨѶ����
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

        if ((MRcvBuffer[0] & 0xF0) == 0xA0) //���ͺ�������  I��
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
            WriteRawRC(TxModeReg, 0x80); //ʹ�������ݷ��͹����в��� CRC
            //status = PcdSingleResponseCmd(PCD_TRANSCEIVE);
            status = PcdComMF522_CPU(PCD_TRANSCEIVE, MSndBuffer, MInfo.nBytesToSend, MRcvBuffer, &rcByte, &unLen);

            continue;
        }

        if ((MRcvBuffer[0] & 0xF0) == 0x10) //���պ�������   R��
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
            WriteRawRC(TxModeReg, 0x80); //ʹ�������ݷ��͹����в��� CRC
            //status = PcdSingleResponseCmd(PCD_TRANSCEIVE);
            status = PcdComMF522_CPU(PCD_TRANSCEIVE, MSndBuffer, MInfo.nBytesToSend, MRcvBuffer, &rcByte, &unLen);
            continue;
        }

        if ((MRcvBuffer[0] & 0xC0) == 0xC0) //S ��
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
            WriteRawRC(TxModeReg, 0x80); //ʹ�������ݷ��͹����в��� CRC
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
����: ISO 14443  TypeA ������COS����
����: *send ��������  *recv �������� *pLen ����/���յĳ���
����:  0�ɹ�    ����ʧ��
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

//--------------------------------��CPU��---------------------------------------
uint8_t Read_M1_or_CPU_Card(uint8_t *id, uint8_t *type)
{

    unsigned char status;
    unsigned char uid[4];
    unsigned char g_ucTempbuf[20];

    status = PcdRequest(PICC_REQIDL, g_ucTempbuf); //��һ��Ѱ��
    if (status != MI_OK)
    {
        //					 status = PcdRequest(PICC_REQALL, g_ucTempbuf);  //�ڶ���Ѱ��
        //					 if (status != MI_OK)
        //					 {
        //							 status = PcdRequest(PICC_REQALL, g_ucTempbuf);  //�����δ�Ѱ��
        //							 if (status != MI_OK)
        //							 {

        return (0); //	continue;
                    //							 }
                    //					 }
    }

    *type++ = g_ucTempbuf[0];
    *type = g_ucTempbuf[1];

    status = PcdAnticoll(uid); //����ײ
    if (status != MI_OK)
    {
        return (0); //continue;
    }

    DEBUG("\r\n PcdAnticoll_ok ");

    status = PcdSelect(uid); //ѡ����Ƭ
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

//------------------�ۼӺ�У��---------------------

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

//==================================�·���Ƭ   ��ʼ����==========================================

uint8_t Init_Card(uint8_t *Card_UID)  //�ֲ�������Ҫ������󣬻���ջ�������
{
		 unsigned char i=0,j=0;	
		 unsigned char status;
     unsigned char  DefaultKey[6] = {0xAB, 0x02, 0x03, 0x44, 0x55, 0xAB}; 	//��������  AB 02 03 44 55 AB
		 unsigned char  g_ucTempbuf[16];    
		 static unsigned char  card_type[2];		 
 	  							 
/*-----------����3����0������----------���жϿ�Ƭ����)
				 
	�ж��Ƿ��� ��ʼ���� 				 
---------------------------------------
*/			
			 
				DEBUG("\r\n Card_UID= %02x %02x %02x %02x ",Card_UID[0],Card_UID[1],Card_UID[2],Card_UID[3]);			

			 //--------------��֤��ƬA����----------
			 status = PcdAuthState(PICC_AUTHENT1A, 12, DefaultKey, Card_UID);	// д��UID
			 if (status != MI_OK)
			 {   

					DEBUG("\r\n One PICC_AUTHENT1A_err ");
					
					 return(0);

			 }
				
				DEBUG("\r\n One PICC_AUTHENT1A_ok ");
			
			 status = PcdRead(12, g_ucTempbuf);					 //���飨�̶�����������	
			 if (status != MI_OK)
			 {   
						return(0);
			 }				
			 else
			 {
						
					if(g_ucTempbuf[8] == 0x01)	//���ж�Կ�״�״̬	PJW 2018.11.23
					{
						//-------��ʼ����---------
								Data_Init_Flag=1;	//�����ƹ�
								//����������	
								Card_Sector=g_ucTempbuf[5];
								at24cxx_WriteOneByte(CARD_SECTOR_ADDR,Card_Sector);
								
									DEBUG("\r\n Card_Sector=%d ",Card_Sector);						
						
								//�����������--ȫ�����	PJW 2018.11.21									
								USE_PWD_CLEAR_PWD();
								//����Ƭ������--ȫ�����	PJW 2018.11.21
								USE_CARD_ALL_CLEAR();																		
								//�����Ŵ����������
								USE_LOCK_CNT_CLEAR();
								//�����������¼ȫ�����
								LOCK_RECORD_CLEAR();
								//ι��
								IWDG_ReloadCounter();				
								//�Ϸ��������	
								rcv_clear_pwd_cmd_to_link();						
							
								Data_Init_Flag=0;	//�رյƹ�
								
								//��������
								NVIC_SystemReset();	

					}
					else if(g_ucTempbuf[8] == 0x08)		
					{
						//---------�ŵ���----------
						
								//�����ŵ�	
								dev_info.ch=g_ucTempbuf[9];
								at24cxx_WriteOneByte(LORA_CH_ADDR,dev_info.ch);
								
									DEBUG("\r\n dev_info.ch=%d ",dev_info.ch);	
						
								TIMDelay_Nms(100);

								//��������
								NVIC_SystemReset();						
						
					}						
					else
					{
						 
								DEBUG("\r\n Again Verify OK ");							
							return(0);	
					
					}
						
			}
			 
}


//--------------------------------�·���Ƭ������������֤---------------------------------------

uint8_t M1_Card_Verify(uint8_t *Card_UID)  //�ֲ�������Ҫ������󣬻���ջ�������
{
		 unsigned char i=0,j=0;	
		 unsigned char status;
     unsigned char  DefaultKey[6] = {0xAB, 0x02, 0x03, 0x44, 0x55, 0xAB}; 	//��������  AB 02 03 44 55 AB
		 unsigned char  g_ucTempbuf[16];    
		 static unsigned char  card_type[2];		 
 	  							 
/*-----------����3����0������----------���жϿ�Ƭ����)
				 
	�ж��Ƿ��� ��ʼ���� 				 
---------------------------------------
*/			
			 
			DEBUG("\r\n Card_UID= %02x %02x %02x %02x ",Card_UID[0],Card_UID[1],Card_UID[2],Card_UID[3]);
				
		if(Card_Sector==0)	//û��ʹ�ù���ʼ����
		{	
			 //--------------��֤��ƬA����----------
			 status = PcdAuthState(PICC_AUTHENT1A, 12, DefaultKey, Card_UID);	// д��UID
			 if (status != MI_OK)
			 {   
		 
					DEBUG("\r\n One PICC_AUTHENT1A_err ");
					
					 return(0);

			 }
				
				DEBUG("\r\n One PICC_AUTHENT1A_ok ");
			
			 status = PcdRead(12, g_ucTempbuf);					 //���飨�̶�����������	
			 if (status != MI_OK)
			 {   
						return(0);
			 }				
			 else
			 {
						
					if(g_ucTempbuf[8] == 0x01)	//���ж�Կ�״�״̬	PJW 2018.11.23
					{
						//-------��ʼ����---------
								Data_Init_Flag=1;	//�����ƹ�
												
								//����������	
								Card_Sector=g_ucTempbuf[5];
								at24cxx_WriteOneByte(CARD_SECTOR_ADDR,Card_Sector);
								
									DEBUG("\r\n Card_Sector=%d ",Card_Sector);						
								
																	
								//����Ƭ������--ȫ�����	PJW 2018.11.21
								USE_CARD_ALL_CLEAR();																		
								//�����Ŵ����������
								USE_LOCK_CNT_CLEAR();
								//�����������¼ȫ�����
								LOCK_RECORD_CLEAR();
								//ι��
								IWDG_ReloadCounter();				
								//�Ϸ��������	
								rcv_clear_pwd_cmd_to_link();						
							
								Data_Init_Flag=0;	//�رյƹ�
								
								//��������
								NVIC_SystemReset();	

					}
					else
					{
						 
								DEBUG("\r\n Again Verify OK ");
						
							return(1);	
					
					}
						
				}
			 
		}
		else	//ʹ�ù���ʼ��������������
		{
			
/*=============================================================================================
			
-----------ʹ�ù���ʼ��������������--------------					
			
�ж� �������롢������Ϣ���˻����루��ʼ��������֤��
			
=============================================================================================*/	
			 
				DEBUG("\r\n Card_UID= %02x %02x %02x %02x ",Card_UID[0],Card_UID[1],Card_UID[2],Card_UID[3]);
			
			 //--------------��֤��ƬA����------------(��ʼ��������3����)
			 status = PcdAuthState(PICC_AUTHENT1A, 12, DefaultKey, Card_UID);	// д��UID
			 if (status != MI_OK)
			 {   

					 
					DEBUG("\r\n Again PICC_AUTHENT1A_12_err ");
				 
					if(Read_M1_or_CPU_Card(Card_UID,card_type) == 1)	//�ٴ�ѯ�����������ٴ���֤����		PJW		20181127
					{
						 //--------------��֤��ƬA����------------(�û�����Card_Sector����)	
						 status = PcdAuthState(PICC_AUTHENT1A, Card_Sector*4, DefaultKey, Card_UID);	// д��UID
						 if (status != MI_OK)
						 {   

								 
								DEBUG("\r\n Again PICC_AUTHENT1A_Sector_err Card_Sector=%d ",Card_Sector);

								 return(0);
						 }
						 else		//�û�����֤�ɹ�
						 {
								 
									DEBUG("\r\n Again Verify OK ");
								
									return(1);							 
						 }
						 
					}else;

			 }
				
				DEBUG("\r\n Again PICC_AUTHENT1A_ok ");				
			

			 status = PcdRead(12, g_ucTempbuf);			//����3����0�飨0-15������ÿ����4���飩			��ʼ����				
			 if (status != MI_OK)
			 { 
						
						DEBUG("\r\n Again Read_0_Sector_err ");
						DEBUG("\r\n Card_Sector=%d ",Card_Sector);
				 
						return(0);
			 }				
			 else
			 {
		 
					if(g_ucTempbuf[8] == 0x01)	//���ж�Կ�״�״̬	PJW 2018.11.23
					{
						//-------��ʼ����---------
								Data_Init_Flag=1;	//�����ƹ�
												
								//����������	
								Card_Sector=g_ucTempbuf[5];
								at24cxx_WriteOneByte(CARD_SECTOR_ADDR,Card_Sector);
								
									DEBUG("\r\n Card_Sector=%d ",Card_Sector);						
								
																	
								//����Ƭ������--ȫ�����	PJW 2018.11.21
								USE_CARD_ALL_CLEAR();																		
								//�����Ŵ����������
								USE_LOCK_CNT_CLEAR();
								//�����������¼ȫ�����
								LOCK_RECORD_CLEAR();
								//ι��
								IWDG_ReloadCounter();				
								//�Ϸ��������	
								rcv_clear_pwd_cmd_to_link();						
							
								Data_Init_Flag=0;	//�رյƹ�
								
								//��������
								NVIC_SystemReset();	

					}
					else	//�ڳ�ʼ����������ȷ�������Ҳ��֤��������
					{
						 //--------------��֤��ƬA����------------(�û�����Card_Sector����)	
						 status = PcdAuthState(PICC_AUTHENT1A, Card_Sector*4, DefaultKey, Card_UID);	// д��UID
						 if (status != MI_OK)
						 {   

									DEBUG("\r\n Again PICC_AUTHENT1A_err ");

								 return(0);
						 }
						 else		//�û�����֤�ɹ�
						 {
								 
									DEBUG("\r\n Again Verify OK ");
								
									return(1);							 
						 }						
												
					}


			}						
				
		}
				
}



/************************************************
�������� �� Bl_Card_Check
��    �� �� ���߿�Ƭ�������Ƚ�
��    �� �� buf�������Ŀ���	
�� �� ֵ �� 0���������д���
��    �� �� PJW
*************************************************/
uint8_t Bl_Card_Check(uint8_t *buf)
{
	 static unsigned char i=0,j=0;	
	 static unsigned char BL_UID[4];   		 	//�����Ŀ�ƬUID		
	
		 
		DEBUG("\r\n Card_UID buf= %02x %02x %02x %02x ",buf[0],buf[1],buf[2],buf[3]);

		for(i=0;i<CARD_BL_NUM;i++) 	//������������
		{
			for(j=0;j<4;j++)	//4λUID
			{
					BL_UID[j]=at24cxx_ReadOneByte(BL_CARD_ADDR+i*BL_CARD_len+j);
			}
 
			DEBUG("\r\n i=%d BL_UID= %02x %02x %02x %02x ",i,BL_UID[0],BL_UID[1],BL_UID[2],BL_UID[3]);
			
			if( memcmp(BL_UID,buf,4) == 0 )	//�Ƚ���ͬ 0
			{
				 
					DEBUG("\r\n Card_Bl Exist ");
							
					return(0);
			}else;
		}
		
		return(1);
		
}
				

//--------------------------------�� M1 ��������---------------------------------------
uint8_t Read_M1_Data(uint8_t *Card_UID)  //�ֲ�������Ҫ������󣬻���ջ�������
{
		 unsigned char i=0,j=0;	
		 unsigned char idex_bl=0;		//��Ƭ����������
		 unsigned char status;
     unsigned char  DefaultKey[6] = {0xAB, 0x02, 0x03, 0x44, 0x55, 0xAB}; 	//��������  AB 02 03 44 55 AB
		 unsigned char  g_ucTempbuf[16];    
		 unsigned char  ACT_INFO[5];   	  	//��Ƭ�еķ�����Ϣ
		 unsigned char  Week_Buf[7];   	  	//����ѭ���е����ڱ�־
		 
		 unsigned char  UID_Temp[4];   	  	//
//		 unsigned char  Block0_Buf[16],Block1_Buf[16],Block2_Buf[16]; 		 
		 unsigned char	Staff_Card_Type=0;		//Ա��������		 
		 unsigned char	Group_Sta=0;					//��Աȱ�־
		 unsigned char	Read_Group[6];				//������ ���	 
		 static unsigned char  card_type[2];		 	
	
	 
/*-----------�жϿ�Ƭ�Ƿ��ں�����----------				 
1��������ƬUID
2�������������еĿ���
3����һ�ȶ�				 
------------------------------------------*/
				 			 
				 if( Bl_Card_Check(Card_UID) == 0)
				 {
					 	return(0);
				 }

				
				 
					DEBUG("\r\n Card_Bl No Exist ");					
				 
/*-----------����3����0������----------���жϿ�Ƭ����)
				 
	�ж��Ƿ��� ��ʼ��������ſ������̿�			 
				 
				 
				  ��ʼ������0x01
					��ſ���  0x02
					���̿���  0x03
					���˿���  0x04
					Ա������  0x05
					Ӧ������  0x06
					��ʧ����  0x07
---------------------------------------
*/			
				   
				DEBUG("\r\n Card_UID= %02x %02x %02x %02x ",Card_UID[0],Card_UID[1],Card_UID[2],Card_UID[3]);
			
				if(Card_Sector==0)	//û��ʹ�ù���ʼ����
				{	
					 //--------------��֤��ƬA����----------
					 status = PcdAuthState(PICC_AUTHENT1A, 12, DefaultKey, Card_UID);	// д��UID
					 if (status != MI_OK)
					 {   
 
							DEBUG("\r\n One PICC_AUTHENT1A_err ");
							
							 return(0);//continue; 
		
					 }
					  
						DEBUG("\r\n One PICC_AUTHENT1A_ok ");
					
					 status = PcdRead(12, g_ucTempbuf);					 //���飨�̶�����������	
					 if (status != MI_OK)
					 {   
								return(0);
					 }				
					 else
					 {
								//��У��
								if( Num_Check(g_ucTempbuf) == 0)
								{
									 
										DEBUG("\r\n One Num_Check_err ");
								
										return(0);
								}
								
								/*----------------��δˢ��ʼ�������жϷ�����Ϣ------------------	*/			 
											
								switch(g_ucTempbuf[8])
								{
									//-------��ʼ����---------
									case 0x01:
														Data_Init_Flag=1;	//�����ƹ�
												  	//���淿����Ϣ	
													  memcpy(STORE_ACT_INFO,g_ucTempbuf,5);									
														for(i=0;i<5;i++)
														{
															at24cxx_WriteOneByte(ACCOUNTS_INFO_ADDR+i,STORE_ACT_INFO[i]);	
														}		
												  	//�����˻�����	
													  memcpy(STORE_PWD_INFO,&g_ucTempbuf[6],2);									
														for(i=0;i<2;i++)
														{
															at24cxx_WriteOneByte(ACCOUNTS_PWD_ADDR+i,STORE_PWD_INFO[i]);	
														}																		
													  //����������	
													  Card_Sector=g_ucTempbuf[5];
													  at24cxx_WriteOneByte(CARD_SECTOR_ADDR,Card_Sector);
														
															DEBUG("\r\n Card_Sector=%d ",Card_Sector);

														//���̿���Ϊ��Ч
														Engine_Card_Flag=0;
													  at24cxx_WriteOneByte(ENGINE_CARD_FLAG_ADDR,Engine_Card_Flag);									
														
																							
														//��տ�Ƭ������
														CARD_BL_CLEAR();
														//�����Ŵ����������
														USE_LOCK_CNT_CLEAR();
														//�����������¼ȫ�����
														LOCK_RECORD_CLEAR();
														//ι��
														IWDG_ReloadCounter();
														Data_Init_Flag=0;	//�رյƹ�
																												
														//��������
														NVIC_SystemReset();	
								
									break;
									
									//-------��ſ�---------��δ��ʼ����������ʹ�ã�
//									case 0x02:									
//														Card_Group=g_ucTempbuf[5];
//														at24cxx_WriteOneByte(CARD_GROUP_ADDR,Card_Group);//�������			 
//									break;
									
									//-------���̿�---------
									case 0x03:
																		
														if(Engine_Card_Flag==0)//��Ч
														{
															 return(1);	//��ȷ������
																														
														}			
									break;
														
														
									default:
										
									break;
									
								}
						 
					 }
					 
				}
				else	//ʹ�ù���ʼ��������������
				{
					
/*=============================================================================================
					
-----------ʹ�ù���ʼ��������������--------------					
					
1�����жϼ�¼��������������ȷ������
					
					
2���������жϵ�3����������ȷ������
					
					
=============================================================================================*/	
				   
				 				 DEBUG("\r\n Card_UID= %02x %02x %02x %02x ",Card_UID[0],Card_UID[1],Card_UID[2],Card_UID[3]);
			

							
							 //--------------��֤��ƬA����------------(�û�����Card_Sector����)	
							 status = PcdAuthState(PICC_AUTHENT1A, Card_Sector*4, DefaultKey, Card_UID);	// д��UID
							 if (status != MI_OK)
							 {   

									 
											DEBUG("\r\n Again PICC_AUTHENT1A_Sector_err Card_Sector=%d ",Card_Sector);

									if(Read_M1_or_CPU_Card(Card_UID,card_type) == 1)	//�ٴ�ѯ�����������ٴ���֤����		PJW		20181127
									{
										 //--------------��֤��ƬA����----------(��ʼ��������3����)
										 status = PcdAuthState(PICC_AUTHENT1A, 12, DefaultKey, Card_UID);	// д��UID
										 if (status != MI_OK)
										 {   

												 
													DEBUG("\r\n Again PICC_AUTHENT1A_12_err ");
		
													return(0);
										 }
										 else		//3����������֤��ȷ
										 {
											 
												 status = PcdRead(12, g_ucTempbuf);//��3����0�飨0-15������ÿ����4���飩					
												 if (status != MI_OK)
												 { 
														
															DEBUG("\r\n Again Read_0_Sector_err ");
															DEBUG("\r\n Card_Sector=%d ",Card_Sector);
						 
															return(0);
												 }				
												 else
												 {
															//��У��
															if( Num_Check(g_ucTempbuf) == 0)
															{
															 
																	DEBUG("\r\n Again Num_Check_err ");
							
																	return(0);
															}
															
															/*----------------�жϷ�����Ϣ------------------	*/			 
															if((g_ucTempbuf[8] != 0x01)&&(g_ucTempbuf[8] != 0x03))	//���ǳ�ʼ�����͹��̿�����Ҫ�жϷ�����Ϣ
															{
															
																	DEBUG("\r\n STORE_ACT_INFO=%02x %02x %02x %02x %02x ",STORE_ACT_INFO[0],STORE_ACT_INFO[1],STORE_ACT_INFO[2],STORE_ACT_INFO[3],STORE_ACT_INFO[4]);
																	DEBUG("\r\n g_ucTempbuf=%02x %02x %02x %02x %02x ",g_ucTempbuf[0],g_ucTempbuf[1],g_ucTempbuf[2],g_ucTempbuf[3],g_ucTempbuf[4]);											
																														
																 if(memcmp(STORE_ACT_INFO,&g_ucTempbuf[0],5) != 0) 	//����ȫ��ͬ
																 {   
																			
																			DEBUG("\r\n Again ACT_INFO err ");	
																	 
																			return(0);
																 }
																 
																 /*----------------�ж��˻�����------------------	*/
																 if(memcmp(STORE_PWD_INFO,&g_ucTempbuf[6],2) != 0) 	//����ȫ��ͬ
																 {   
																			
																			DEBUG("\r\n Again PWD_INFO err ");
		 
																			return(0);
																 }	

															}						 
											
																										 
															switch(g_ucTempbuf[8])
															{
																//-------��ʼ����---------
																case 0x01:
																					Data_Init_Flag=1;	//�����ƹ�
																					//���淿����Ϣ	
																					memcpy(STORE_ACT_INFO,g_ucTempbuf,5);									
																					for(i=0;i<5;i++)
																					{
																						at24cxx_WriteOneByte(ACCOUNTS_INFO_ADDR+i,STORE_ACT_INFO[i]);	
																					}		
																					//�����˻�����	
																					memcpy(STORE_PWD_INFO,&g_ucTempbuf[6],2);									
																					for(i=0;i<2;i++)
																					{
																						at24cxx_WriteOneByte(ACCOUNTS_PWD_ADDR+i,STORE_PWD_INFO[i]);	
																					}																		
																					//����������	
																					Card_Sector=g_ucTempbuf[5];
																					at24cxx_WriteOneByte(CARD_SECTOR_ADDR,Card_Sector);
																					//���̿���Ϊ��Ч
																					Engine_Card_Flag=0;
																					at24cxx_WriteOneByte(ENGINE_CARD_FLAG_ADDR,Engine_Card_Flag);									
																					
																					
																					//��տ�Ƭ������
																					CARD_BL_CLEAR();
																					//�����Ŵ����������
																					USE_LOCK_CNT_CLEAR();
																					//�����������¼ȫ�����
																					LOCK_RECORD_CLEAR();
																					//ι��
																					IWDG_ReloadCounter();
																					Data_Init_Flag=0;	//�رյƹ�
																																			
																					//��������
																					NVIC_SystemReset();								
																break;
																
																//-------��ſ�---------
																case 0x02:									
																					Card_Group=g_ucTempbuf[5];
																					at24cxx_WriteOneByte(CARD_GROUP_ADDR,Card_Group);//�������	

																					
																					DEBUG("\r\n Card_Group=%02x ",Card_Group);
											
																break;
																
																//-------���̿�---------
																case 0x03:
																									
																					if(Engine_Card_Flag==0)//��Ч
																					{
																						return(1);	//��ȷ������																						
																					}			
																break;

														}
																		 
												}
														
										}
									 
									}else;
									
							 }
							 else		//�û�����֤�ɹ�
							 {
									 
									 DEBUG("\r\n Again Verify OK ");
								
									
									 status = PcdRead(Card_Sector*4, g_ucTempbuf);//��Card_Sector����0�飨0-15������ÿ����4���飩					
									 if (status != MI_OK)
									 { 
												
												DEBUG("\r\n Again Read_0_Sector_err ");
												DEBUG("\r\n Card_Sector=%d ",Card_Sector);
						 
												return(0);
									 }				
									 else
									 {
												//��У��
												if( Num_Check(g_ucTempbuf) == 0)
												{
													 
														DEBUG("\r\n Again Num_Check_err ");
								
														return(0);
												}
												
												/*----------------�жϷ�����Ϣ------------------	*/			 
												if((g_ucTempbuf[8] != 0x01)&&(g_ucTempbuf[8] != 0x03))	//���ǳ�ʼ�����͹��̿�����Ҫ�жϷ�����Ϣ
												{
													
												
														DEBUG("\r\n STORE_ACT_INFO=%02x %02x %02x %02x %02x ",STORE_ACT_INFO[0],STORE_ACT_INFO[1],STORE_ACT_INFO[2],STORE_ACT_INFO[3],STORE_ACT_INFO[4]);
														DEBUG("\r\n g_ucTempbuf=%02x %02x %02x %02x %02x ",g_ucTempbuf[0],g_ucTempbuf[1],g_ucTempbuf[2],g_ucTempbuf[3],g_ucTempbuf[4]);										
											
													
													 if(memcmp(STORE_ACT_INFO,&g_ucTempbuf[0],5) != 0) 	//����ȫ��ͬ
													 {   
																
																DEBUG("\r\n Again ACT_INFO err ");
														 
																return(0);
													 }
													 
													 /*----------------�ж��˻�����------------------	*/
													 if(memcmp(STORE_PWD_INFO,&g_ucTempbuf[6],2) != 0) 	//����ȫ��ͬ
													 {   
														
																DEBUG("\r\n Again PWD_INFO err ");	
														 
																return(0);
													 }	

												}						 
								
																							 
												switch(g_ucTempbuf[8])
												{
													//-------��ʼ����---------
													case 0x01:
																		Data_Init_Flag=1;	//�����ƹ�
																		//���淿����Ϣ	
																		memcpy(STORE_ACT_INFO,g_ucTempbuf,5);									
																		for(i=0;i<5;i++)
																		{
																			at24cxx_WriteOneByte(ACCOUNTS_INFO_ADDR+i,STORE_ACT_INFO[i]);	
																		}		
																		//�����˻�����	
																		memcpy(STORE_PWD_INFO,&g_ucTempbuf[6],2);									
																		for(i=0;i<2;i++)
																		{
																			at24cxx_WriteOneByte(ACCOUNTS_PWD_ADDR+i,STORE_PWD_INFO[i]);	
																		}																		
																		//����������	
																		Card_Sector=g_ucTempbuf[5];
																		at24cxx_WriteOneByte(CARD_SECTOR_ADDR,Card_Sector);
																		//���̿���Ϊ��Ч
																		Engine_Card_Flag=0;
																		at24cxx_WriteOneByte(ENGINE_CARD_FLAG_ADDR,Engine_Card_Flag);									
																		
																		
																		//��տ�Ƭ������
																		CARD_BL_CLEAR();
																		//�����Ŵ����������
																		USE_LOCK_CNT_CLEAR();
																		//�����������¼ȫ�����
																		LOCK_RECORD_CLEAR();
																		//ι��
																		IWDG_ReloadCounter();
																		Data_Init_Flag=0;	//�رյƹ�
																																
																		//��������
																		NVIC_SystemReset();								
													break;
													
													//-------��ſ�---------
													case 0x02:									
																		Card_Group=g_ucTempbuf[5];
																		at24cxx_WriteOneByte(CARD_GROUP_ADDR,Card_Group);//�������	

																		
																		DEBUG("\r\n Card_Group=%02x ",Card_Group);
								
													break;
													
													//-------���̿�---------
													case 0x03:
																						
																		if(Engine_Card_Flag==0)//��Ч
																		{
																			return(1);	//��ȷ������
																						
																		}			
													break;
																		
													//-------���˿�---------
													case 0x04:
																	status = PcdRead(Card_Sector*4+1, g_ucTempbuf);//��1�飨0-15������ÿ����4���飩					
																	 if (status != MI_OK)
																	 {   
																				return(0);
																	 }		

																		//-----��У��-----
																		if( Num_Check(g_ucTempbuf) == 0)
																		{
																			 
																					DEBUG("\r\n Guest B1 Num_Check_err ");
										
																				return(0);
																		}
																	 
																	 //-----�Ƚ�SN----
																	 if(memcmp(dev_info.sn,&g_ucTempbuf[0],6) != 0) 	//PJW	20181112 SN��ͬʱ
																	 {   
																				
																				DEBUG("\r\n Guest SN err ");
								
																				return(0);
																	 }																		 
																	 else
																	 {
																			 status = PcdRead(Card_Sector*4+2, g_ucTempbuf);//��2�飨0-15������ÿ����4���飩					
																			 if (status != MI_OK)
																			 {   
																						return(0);
																			 }								

																				//-----��У��-----
																				if( Num_Check(g_ucTempbuf) == 0)
																				{
																					 
																							DEBUG("\r\n Guest B2 Num_Check_err ");
								
																						return(0);
																				}
																			 
																			 //-----�Ƚ���Ч��----
																			 RTC_Get();
																			 if( Validity_Compare(&g_ucTempbuf[0],&g_ucTempbuf[6]) == 1 ) //������Ч��
																			 {   
																						
																							DEBUG("\r\n Guest Card Time err ");
																						
																						return(0);
																			 }
																			 else
																			 {																    		
																						return(1);	//��ȷ������
																			 }

																	 }
																			
													break;														
																		
													//-------Ա����---------
													case 0x05:							
																	Staff_Card_Type=g_ucTempbuf[9];		//Ա��������
													
																	status = PcdRead(Card_Sector*4+1, g_ucTempbuf);//��1�飨0-15������ÿ����4���飩					
																	 if (status != MI_OK)
																	 {   
																				return(0);
																	 }	

																		//-----��У��-----
																		if( Num_Check(g_ucTempbuf) == 0)
																		{
																			 
																					DEBUG("\r\n Staff B1 Num_Check_err ");
																					
																				return(0);
																		}
																	 
																		memcpy(Read_Group,&g_ucTempbuf[0],6);	//�����������Ϣ
																		
																		memcpy(Week_Buf,&g_ucTempbuf[6],7);	//���ڱ�־
																		
																		
																				DEBUG("\r\n g_ucTempbuf=%02x %02x %02x %02x %02x %02x",g_ucTempbuf[0],g_ucTempbuf[1],g_ucTempbuf[2],g_ucTempbuf[3],g_ucTempbuf[4],g_ucTempbuf[5]);
																				DEBUG("\r\n Card_Group=%02x ",Card_Group);														

					 
																	 //-----�Ƚ����ڣ����ڣ�-------													 													
																	 status = PcdRead(Card_Sector*4+2, g_ucTempbuf);//��2�飨0-15������ÿ����4���飩					
																	 if (status != MI_OK)
																	 {   
																				
																				DEBUG("\r\n Staff B2 err ");
																															 
																				return(0);
																	 }								

																		//-----��У��-----
																		if( Num_Check(g_ucTempbuf) == 0)
																		{
																			 
																					DEBUG("\r\n Staff B2 Num_Check_err ");
																								
																				return(0);
																		}
																		
																		switch(Staff_Card_Type)
																		{
																			//��ʱ��
																			case 0x01:																		
																							 //-----�Ƚ���-------
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
																												 Group_Sta=1;	//�ҵ���ͬ����
																												 break;
																											}else;
																									 }
																								}	 
																								
																							 if(Group_Sta == 0)
																							 {
																									
																										DEBUG("\r\n Group err ");
																																			  
																									return(0);
																							 }																		
																			
																							 //-----�Ƚ���Ч��----
																							 RTC_Get();
																							 if( Validity_Compare(&g_ucTempbuf[0],&g_ucTempbuf[6]) == 1 ) //������Ч��
																							 {   
																										
																											DEBUG("\r\n Staff Card Time err ");
																										
																										return(0);
																							 }
																							 else
																							 {
																										return(1);	//��ȷ������
																							 }
																			break;
																							 
																			//����ѭ����
																			case 0x02:						
																							 //-----�Ƚ���-------
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
																												 Group_Sta=1;	//�ҵ���ͬ����
																												 break;
																											}else;
																									 }
																								}	 
																								
																							 if(Group_Sta == 0)
																							 {
																									
																										DEBUG("\r\n Group err ");
																																				  
																									return(0);
																							 }	

																							//-----�Ƚ����ڣ����ڣ�-------
																								RTC_Get();
																								
																								DEBUG("\r\n Week_Buf=%02x %02x %02x %02x %02x %02x %02x",Week_Buf[0],Week_Buf[1],Week_Buf[2],Week_Buf[3],Week_Buf[4],Week_Buf[5],Week_Buf[6]);
																								DEBUG("\r\n calendar.week=%02x ",calendar.week);
																																																				
																								if(Week_Buf[calendar.week-1] == 0)	//��������Ч
																								{
																										
																											DEBUG("\r\n Staff Week err ");
																							
																										return(0);																
																								}
																								else
																								{
																										return(1);	//��ȷ������
																								}																						
																							 
																			break;
																			
																			//�ܿ� --- ������  ���Ƚ����
																			case 0x03:
																									 return(1);	//��ȷ������
																							 
																			break;
																				
																			//Ѳ�쿨 --- ���Ρ�������  ���Ƚ����
																			case 0x04:

				//																					 Beep_on_key();
																			break;
																			

																			default:
																				
																			break;
																			
																		}

													
													break;														
																		
																		
													//-------Ӧ����---------
													case 0x06:			
														
																	 status = PcdRead(Card_Sector*4+2, g_ucTempbuf);//��2�飨0-15������ÿ����4���飩					
																	 if (status != MI_OK)
																	 {   
																				return(0);
																	 }								

																		//-----��У��-----
																		if( Num_Check(g_ucTempbuf) == 0)
																		{
																			 
																				DEBUG("\r\n Emergency B2 Num_Check_err ");
																						
																				return(0);
																		}
																	 
																	 //-----�Ƚ���Ч��----
																	 if( Validity_Compare(&g_ucTempbuf[0],&g_ucTempbuf[6]) == 1 ) //������Ч��
																	 {   
																				
																				DEBUG("\r\n Emergency Card Time err ");
															
																				return(0);
																	 }
																	 else
																	 {
																				return(1);	//��ȷ������
																		 
																	 }	

													break;														
																		
													//-------��ʧ��-------����Ƭ��������
													case 0x07:									
																	status = PcdRead(Card_Sector*4+1, g_ucTempbuf);//��1�飨0-15������ÿ����4���飩					
																	 if (status != MI_OK)
																	 {   
																				return(0);
																	 }								
																	 
																		//-----��У��-----
																		if( Num_Check(g_ucTempbuf) == 0)
																		{
																			 
																				DEBUG("\r\n Loss B1 Num_Check_err ");
																					
																				return(0);
																		}													 
																	 
																	 //-----�Ƚ�SN-----
																	 if(memcmp(dev_info.sn,&g_ucTempbuf[0],6) != 0) 	//PJW	20181112 SN��ͬʱ
																	 {   
																				
																				DEBUG("\r\n Loss SN err ");														
																		 
																				return(0);
																	 }	
																	 else
																	 {
																		 
																			if( Bl_Card_Check(&g_ucTempbuf[6]) == 0)	//�������д���														 
																			{
																				
																				DEBUG("\r\n Bl_Card Exsit ");																
																				
																				return(0);
																			}
																			else	//������
																			{
																				
																				idex_bl=at24cxx_ReadOneByte(CARD_BL_IDEX);	//��������
																		 
																				if(idex_bl>=CARD_BL_NUM)
																				{
																						idex_bl = 0;
																				}
																																		 
																					DEBUG(" idex_bl=%02d \n ",idex_bl);										 													 
																		 

																				for(j=0;j<BL_CARD_len;j++)	//4λUID
																				{
																						at24cxx_WriteOneByte(BL_CARD_ADDR+(idex_bl*BL_CARD_len)+j,g_ucTempbuf[j+6]);
																				}														 

																				idex_bl++;
																				
																				
																						for(j=0;j<BL_CARD_len;j++)	//4λUID
																						{
																								UID_Temp[j]=at24cxx_ReadOneByte(BL_CARD_ADDR+(idex_bl*BL_CARD_len)+j);
																						}																						
																				
																						DEBUG("\r\n Read_BlUID=%02x %02x %02x %02x ",UID_Temp[0],UID_Temp[1],UID_Temp[2],UID_Temp[3]);
																				
																				at24cxx_WriteOneByte(CARD_BL_IDEX,idex_bl);	//д������
																			}
																		 
																	 }

																		//��ʧ�ɹ��� ���ơ�˫���������������εΡ�
													break;														
																			
																		
																		
													default:
														
													break;
													
												}


											}
									 
								 
							 }
							

					 }
					  
						DEBUG("\r\n Again PICC_AUTHENT1A_ok ");						
					
					 //��֤3����������ȷ
						 status = PcdRead(12, g_ucTempbuf);//��3����0�飨0-15������ÿ����4���飩					
						 if (status != MI_OK)
						 { 
									
									DEBUG("\r\n Again Read_0_Sector_err ");
									DEBUG("\r\n Card_Sector=%d ",Card_Sector);
						 
									return(0);
						 }				
						 else
						 {


					
				}

 
				 //---��������ģʽ---
//				 PcdHalt();
			 
					
						DEBUG("\r\n card_test_ok ");
		
				 return(1);
				
}

#endif

//--------------------------------��������---------------------------------------
uint8_t rc520_test(void)
{
    unsigned char status;
    unsigned char data1[16] = {0x12, 0x34, 0x56, 0x78, 0xED, 0xCB, 0xA9, 0x87, 0x12, 0x34, 0x56, 0x78, 0x01, 0xFE, 0x01, 0xFE};
    //M1����ĳһ��дΪ���¸�ʽ����ÿ�ΪǮ�����ɽ��տۿ�ͳ�ֵ����
    //4�ֽڽ����ֽ���ǰ����4�ֽڽ��ȡ����4�ֽڽ�1�ֽڿ��ַ��1�ֽڿ��ַȡ����1�ֽڿ��ַ��1�ֽڿ��ַȡ��
    unsigned char data2[4] = {0x12, 0, 0, 0};
    unsigned char DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //��������
    unsigned char g_ucTempbuf[20];

    status = PcdRequest(PICC_REQALL, g_ucTempbuf); //��һ��Ѱ��
    if (status != MI_OK)
    {
        status = PcdRequest(PICC_REQALL, g_ucTempbuf); //�ڶ���Ѱ��
        if (status != MI_OK)
        {
            status = PcdRequest(PICC_REQALL, g_ucTempbuf); //�����δ�Ѱ��
            if (status != MI_OK)
            {
                return (0); //	continue;
            }
        }
    }

    DEBUG("\r\n  m1 card ");

    status = PcdAnticoll(g_ucTempbuf); //����ײ
    if (status != MI_OK)
    {
        return (0); //continue;
    }

    DEBUG("\r\n PcdAnticoll_ok ");

    status = PcdSelect(g_ucTempbuf); //ѡ����Ƭ
    if (status != MI_OK)
    {
        return (0); //continue;
    }

    DEBUG("\r\n PcdSelect_ok ");

    //         status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);//��֤��Ƭ����
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
    //         status = PcdWrite(1, data1);//д��
    //         if (status != MI_OK)
    //         {
    //						return(0);//continue;
    //					}
    //
    //					 DEBUG("\r\n card_write_data_ok ");
    //
    //         status = PcdValue(PICC_DECREMENT,1,data2);//�ۿ�ͳ�ֵ
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
    //         status = PcdBakValue(1, 2);//�鱸��
    //         if (status != MI_OK)
    //         {
    //							return(0);//continue;
    //					}
    //
    //         status = PcdRead(2, g_ucTempbuf);//����
    //         if (status != MI_OK)
    //         {
    //							return(0);//continue;
    //				 }

    //---��������ģʽ---
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
