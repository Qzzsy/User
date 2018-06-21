#include "bsp_lcd.h"

/* Private macro Definition --------------------------------------------------*/

/** 娴ｅ潡鏁撻弬銈嗗NOR/SRAM闁跨喐鏋婚幏锟� Bank1.sector1,闁跨喐鏋婚幏宄版絻娴ｅ埠ADDR[27,26]=11 A16闁跨喐鏋婚幏铚傝礋闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻弬銈嗗闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹凤拷
 * @note:闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归弮绂═M32闁跨喕濡拠褎瀚归柨鐔告灮閹风兘鏁撻弬銈嗗闁跨喐鏋婚幏铚傜娴ｅ秹鏁撻弬銈嗗闁跨喐鏋婚幏锟�! 0001 1111 1111 1111 1110=0x0001fffe	
 */    
#define LCD_BASE_ADDRESS        ((uint32_t)(0x6C000000 | 0x0000007E))
#define LCD_OPERATION         	((BspLCD_t *)LCD_BASE_ADDRESS)

/* End private macro Definition ----------------------------------------------*/

/* global variable Declaration -----------------------------------------------*/

/* 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚瑰☉鏌ユ晸閺傘倖瀚归柨鐔告灮閹烽濮搁幀渚€鏁撻弬銈嗗闁跨喓瀚涚紒鎾寸€柨鐔告灮閹凤拷 */
BspLCD_Dev_t BspLCD_Dev;

/* 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚瑰☉鏌ユ晸閺傘倖瀚归柨鐔告灮閹风PI闁跨喐甯撮崠鈩冨闁跨喎褰ㄩ幉瀣瀹勪胶娅㈤幏鐑芥晸閿燂拷 */
BspLCD_Func_t BspLCD;

/* User function Declaration --------------------------------------------------*/
static uint32_t BspLCD_ReadId(void);

void BspLCD_SetDispDir(uint8_t Dir);
void BspLCD_ClrScr(uint16_t pColor);

/* User functions -------------------------------------------------------------*/

/**
 * @func    BspLCD_WriteComm
 * @brief   LCD閸愭瑩鏁撻弬銈嗗闁跨喓鐡旈崚浼存晸娓氥儴鎻幏鐑芥晸閺傘倖瀚�
 * @param   RegVal 閹稿洭鏁撻弬銈嗗闁跨喍鑼庣€靛嫯鎻幏鐑芥晸閺傘倖瀚归柨鐔告灮閹峰嘲娼�
 * @retval  闁跨喐鏋婚幏锟�
 */
static void BspLCD_WriteComm(__IO uint16_t RegVal)
{   
    LCD_OPERATION->REG = RegVal;			/* 閸愭瑩鏁撻弬銈嗗鐟曚礁鍟撻柨鐔惰寧鐎靛嫯鎻幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻敓锟� */
}

/**
 * @func    BspLCD_WriteData
 * @brief   LCD閸愭瑩鏁撻弬銈嗗闁跨喐鏋婚幏锟�
 * @param   Data 鐟曚礁鍟撻柨鐔告灮閹风兘鏁撻弬銈嗗闁跨喐鏋婚幏鐑芥晸閿燂拷
 * @retval  闁跨喐鏋婚幏锟�
 */
static void BspLCD_WriteData(__IO uint16_t  Data)
{	  
    LCD_OPERATION->RAM = Data;		 
}		

/**
 * @func    BspLCD_WR_Reg
 * @brief   闁跨喐鏋婚幏閿嬪瘹闁跨喐鏋婚幏鐑芥晸娓氥儱鐦庢潏鐐闁跨喐鏋婚幏宄板晸闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�
 * @param   Index 闁跨喍鑼庢潏鐐闁跨喐鏋婚幏锟�
 * @param   CongfigTemp 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�
 * @retval  闁跨喐鏋婚幏锟�
 */
static void BspLCD_WR_Reg(uint16_t Index, uint16_t CongfigTemp)
{
    LCD_OPERATION->REG = Index;
    LCD_OPERATION->RAM = CongfigTemp;
}

/**
 * @func    BspLCD_ReadData
 * @brief   LCD闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹凤拷
 * @retval  闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻弬銈嗗闁跨喐鏋婚幏锟�
 */
static uint16_t BspLCD_ReadData(void)
{
    __IO uint16_t Ram;                   //闁跨喐鏋婚幏閿嬵剾闁跨喐鏋婚幏鐑芥晸閼存矮绱幏锟�
    Ram = LCD_OPERATION->RAM;	
    return Ram;	 
}	

/**
 * @func    BspLCD_ReadReg
 * @brief   LCD闁跨喐鏋婚幏鐑芥晸娓氥儴鎻幏鐑芥晸閺傘倖瀚�
 * @param   Reg:闁跨喍鑼庢潏鐐闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归崸鈧�
 * @retval  闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻弬銈嗗闁跨喐鏋婚幏锟�
 */
static uint16_t BspLCD_ReadReg(uint16_t Reg)
{										   
    BspLCD_WriteComm(Reg);              //閸愭瑩鏁撻弬銈嗗鐟曚線鏁撻弬銈嗗闁跨喍鑼庣€靛嫯鎻幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻敓锟�
    return BspLCD_ReadData();           //闁跨喐鏋婚幏鐑芥晸閹搭亣顔愰幏鐑芥晸閺傘倖瀚归柨鐔告灮閹峰嘲鈧拷
}  

/**
 * @func    BspLCD_WR_RamPrepare
 * @brief   lcd闁跨喐鏋婚幏宄邦潗閸愭瑩鏁撻弬銈嗗闁跨喐鏋婚幏鐑芥晸閿燂拷
 * @note    娑撯偓闁跨喐鏋婚幏鐑芥晸閼哄倽顕滈幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻弬銈嗗閺冨爼鏁撻弬銈嗗闁跨喐鏋婚幏锟�
 * @retval  闁跨喐鏋婚幏锟�
 */
static void BspLCD_WR_RamPrepare(void)
{
    LCD_OPERATION->REG = BspLCD_Dev.WramCmd;
}

/**
 * @func    BspLCD_Delay
 * @brief   LCD闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归弮鍫曟晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻懞鍌滎劜閹峰嘲顫愰柨鐔告灮閹风兘鏁撻弬銈嗗闁跨喐鏋婚幏锟�
 * @param   nCount 闁跨喐鏋婚幏閿嬫闁跨喍鑼庢潏鐐鐏忥拷
 * @retval  闁跨喐鏋婚幏锟�
 */
static void BspLCD_Delay(__IO uint32_t nCount)
{	
    volatile int i;
    for (i=0; i < 0XFFFF; i++)
        for (; nCount != 0; nCount--);
    return ;
}  

///**
// * @func    LCDReset
// * @brief   LCD闁跨喐鏋婚幏铚傜秴闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻弬銈嗗闁跨喕濡棃鈺傚娴ｅ硛CD闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔峰建鐎靛嫯鎻幏鐑芥晸閺傘倖瀚�
// * @note    鐟曚線鏁撻弬銈嗗閺冨爼鏁撴慨鎰檮闁跨喐鏋婚幏閿嬫闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撶徊鍍€D闁跨喐鏋婚幏铚傜秴
// * @retval  闁跨喐鏋婚幏锟�
// */
//static void LCDReset(void)
//{		
//    /* write 0 to lcd reset line */
//    LCD_RESET_WRITE_L;
//    LCDDelay(10000);
//    LCDDelay(10000);
//    LCDDelay(10000);
//    LCDDelay(10000);
//    
//    /* write 1 to lcd reset line */	
//    LCD_RESET_WRITE_H;		 	 
//    LCDDelay(10000);	
//    LCDDelay(10000);
//    LCDDelay(10000);
//    LCDDelay(10000);
//} 

/**
 * @func    BspLCD_ConfigLocal
 * @brief   闁跨喐鏋婚幏鐑芥晸閹圭柉顕滈幏宄版倱闁跨喐鏋婚幏绋珻D闁跨喐鏋婚幏鐑芥晸閹搭亣顕滈幏宄版倱闁跨喍鑼庣拠褎瀚归柨鐔告灮閹凤拷
 * @retval  闁跨喐鏋婚幏锟�
 */
static void BspLCD_ConfigLocal(void)
{
    BspLCD_Dev.pHeight = LCD_HEIGHT;
    BspLCD_Dev.pWidth  = LCD_WDITH;
    BspLCD_Dev.WramCmd  = 0x2C;
    BspLCD_Dev.SetXCmd  = 0x2A;
    BspLCD_Dev.SetYCmd  = 0x2B;
    BspLCD_Dev.MemoryAccContCmd  = 0x36;
    BspLCD_Dev.DirHorNormalData  = 0xa8;
    BspLCD_Dev.DirHorReverseData = 0x68;
    BspLCD_Dev.DirVerNormalData  = 0xc8;
    BspLCD_Dev.DirVerReverseData = 0x08;
    
    BspLCD_Dev.DispOnCmd  = 0x29;
    BspLCD_Dev.DispOffCmd = 0x28;
    
    /* 姒涙﹢鏁撻弬銈嗗娴ｅ潡鏁撻惌顐ゅ皑閹风兘鏁撻弬銈嗗 */
    BspLCD_Dev.Height = BspLCD_Dev.pWidth;
    BspLCD_Dev.Width  = BspLCD_Dev.pHeight;
}

/**
 * @func    BspLCD_Init
 * @brief   闁跨喐鏋婚幏绋珻D闁跨喐鏋婚幏鐑芥晸閸欘偆顒查幏宄邦潗闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹凤拷
 * @retva   闁跨喐鏋婚幏锟�
 */
void BspLCD_Init(void)
{
    BspLCD_ConfigLocal();
    
//    LCDReset();
    
    BspLCD_Delay(50); // delay 50 ms
    BspLCD_WriteComm(0xCB);  
    BspLCD_WriteData(0x39); 
    BspLCD_WriteData(0x2C); 
    BspLCD_WriteData(0x00); 
    BspLCD_WriteData(0x34); 
    BspLCD_WriteData(0x02); 
    
    BspLCD_WriteComm(0xCF);  
    BspLCD_WriteData(0x00); 
    BspLCD_WriteData(0XC1); 
    BspLCD_WriteData(0X30); 
    
    BspLCD_WriteComm(0xE8);  
    BspLCD_WriteData(0x85); 
    BspLCD_WriteData(0x00); 
    BspLCD_WriteData(0x78); 
    
    BspLCD_WriteComm(0xEA);  
    BspLCD_WriteData(0x00); 
    BspLCD_WriteData(0x00); 
    
    BspLCD_WriteComm(0xED);  
    BspLCD_WriteData(0x64); 
    BspLCD_WriteData(0x03); 
    BspLCD_WriteData(0X12); 
    BspLCD_WriteData(0X81); 
    
    BspLCD_WriteComm(0xF7);  
    BspLCD_WriteData(0x20); 
    
    BspLCD_WriteComm(0xC0);    //Power control 
    BspLCD_WriteData(0x23);   //VRH[5:0] 
    
    BspLCD_WriteComm(0xC1);    //Power control 
    BspLCD_WriteData(0x10);   //SAP[2:0];BT[3:0] 
    
    BspLCD_WriteComm(0xC5);    //VCM control 
    BspLCD_WriteData(0x3e); //闁跨喓娈曞В鏂垮绾板瀚归柨鐔告灮閹凤拷
    BspLCD_WriteData(0x28); 
    
    BspLCD_WriteComm(0xC7);    //VCM control2 
    BspLCD_WriteData(0x86);  //--
    
    BspLCD_WriteComm(0x36);    // Memory Access Control 
    BspLCD_WriteData(0xE8); //	   //48 68闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�//28 E8 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�
    
    BspLCD_WriteComm(0x3A);    
    BspLCD_WriteData(0x55); 
    
    BspLCD_WriteComm(0xB1);    
    BspLCD_WriteData(0x00);  
    BspLCD_WriteData(0x18); 
    
    BspLCD_WriteComm(0xB6);    // Display Function Control 
    BspLCD_WriteData(0x08); 
    BspLCD_WriteData(0x82);
    BspLCD_WriteData(0x27);  
    
    BspLCD_WriteComm(0xF2);    // 3Gamma Function Disable 
    BspLCD_WriteData(0x00); 
    
    BspLCD_WriteComm(0x26);    //Gamma curve selected 
    BspLCD_WriteData(0x01); 
    
    BspLCD_WriteComm(0xE0);    //Set Gamma 
    BspLCD_WriteData(0x0F); 
    BspLCD_WriteData(0x31); 
    BspLCD_WriteData(0x2B); 
    BspLCD_WriteData(0x0C); 
    BspLCD_WriteData(0x0E); 
    BspLCD_WriteData(0x08); 
    BspLCD_WriteData(0x4E); 
    BspLCD_WriteData(0xF1); 
    BspLCD_WriteData(0x37); 
    BspLCD_WriteData(0x07); 
    BspLCD_WriteData(0x10); 
    BspLCD_WriteData(0x03); 
    BspLCD_WriteData(0x0E); 
    BspLCD_WriteData(0x09); 
    BspLCD_WriteData(0x00); 
    
    BspLCD_WriteComm(0XE1);    //Set Gamma 
    BspLCD_WriteData(0x00); 
    BspLCD_WriteData(0x0E); 
    BspLCD_WriteData(0x14); 
    BspLCD_WriteData(0x03); 
    BspLCD_WriteData(0x11); 
    BspLCD_WriteData(0x07); 
    BspLCD_WriteData(0x31); 
    BspLCD_WriteData(0xC1); 
    BspLCD_WriteData(0x48); 
    BspLCD_WriteData(0x08); 
    BspLCD_WriteData(0x0F); 
    BspLCD_WriteData(0x0C); 
    BspLCD_WriteData(0x31); 
    BspLCD_WriteData(0x36); 
    BspLCD_WriteData(0x0F); 
    
    BspLCD_WriteComm(0x2A);
    BspLCD_WriteData(0x00);
    BspLCD_WriteData(0x00);
    BspLCD_WriteData(0x00);
    BspLCD_WriteData(0xEF);
    
    BspLCD_WriteComm(0x2B);
    BspLCD_WriteData(0x00);
    BspLCD_WriteData(0x00);
    BspLCD_WriteData(0x01);
    BspLCD_WriteData(0x3F);
    BspLCD_WriteComm(0x11);    //Exit Sleep 
    BspLCD_Delay(100); 
            
    BspLCD_WriteComm(0x29);    //Display on 
    BspLCD_WriteComm(0x2c); 
    
    /* 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚瑰☉鏌ユ晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻弬銈嗗缁€娲晸閺傘倖瀚归柨鐔告灮閹凤拷 */
    BspLCD_SetDispDir(DIR_HORIZONTAL_NORMAL);
    
    /* 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚� */
    BspLCD_ClrScr(0x0000);
}

/**
 * @func    BspLCD_ReadId
 * @brief   闁跨喐鏋婚幏宄板絿LCD闁跨喐鏋婚幏绋〥
 * @retval  LCD闁跨喐鏋婚幏绋〥
 */
__attribute__((unused)) static uint32_t BspLCD_ReadId(void)
{
    uint16_t Buf[4];
    
    LCD_OPERATION->REG = 0x04;
    Buf[0] = LCD_OPERATION->RAM;
    Buf[1] = LCD_OPERATION->RAM;
    Buf[2] = LCD_OPERATION->RAM;
    Buf[3] = LCD_OPERATION->RAM;
    
    return (Buf[1] << 16) + (Buf[2] << 8) + Buf[3];
}

/**
 * @func    BspLCD_SetDispDir
 * @brief   闁跨喐鏋婚幏鐑芥晸閺傘倖瀚筁CD闁跨喐鏋婚幏閿嬪闁跨喎鈧喐鏌熼柨鐔告灮閹凤拷
 * @param   Dir 闁跨喐鏋婚幏鐤洣闁跨喐鏋婚幏鐑芥晸閻偆娈戦崙銈嗗闁跨喐鏋婚幏锟�
 * @retval  闁跨喐鏋婚幏锟�
 */
void BspLCD_SetDispDir(uint8_t Dir)
{
    BspLCD_Dev.Dir = Dir;
    BspLCD_WriteComm(BspLCD_Dev.MemoryAccContCmd);
    
    if (Dir == DIR_HORIZONTAL_NORMAL)
    {
        BspLCD_WriteData(BspLCD_Dev.DirHorNormalData);	
        BspLCD_Dev.Height = BspLCD_Dev.pWidth;
        BspLCD_Dev.Width  = BspLCD_Dev.pHeight;
    }
    else if (Dir == DIR_HORIZONTAL_REVERSE)
    {
        BspLCD_WriteData(BspLCD_Dev.DirHorReverseData);
        BspLCD_Dev.Height = BspLCD_Dev.pWidth;
        BspLCD_Dev.Width  = BspLCD_Dev.pHeight;
    } 
    else if (Dir == DIR_VERTICAL_NORMAL)
    {
        BspLCD_WriteData(BspLCD_Dev.DirVerNormalData);
        BspLCD_Dev.Height = BspLCD_Dev.pHeight;
        BspLCD_Dev.Width  = BspLCD_Dev.pWidth;
    }
    else if (Dir == DIR_VERTICAL_REVERSE)
    {
        BspLCD_WriteData(BspLCD_Dev.DirVerReverseData);
        BspLCD_Dev.Height = BspLCD_Dev.pHeight;
        BspLCD_Dev.Width  = BspLCD_Dev.pWidth;
    }
}

/**
 * @func    BspLCD_SetDispWin
 * @brief   闁鏁撻弬銈嗗LCD闁跨喐鏋婚幏閿嬪瘹闁跨喐鏋婚幏鐑芥晸娓氥儲鎷濋幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撳ú銉礉绾攱瀚归柨鐔告灮閹风兘鏁撻弬銈嗗娑撯偓闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹凤拷
 * @param   xCur x闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撶紒鐐点€嬮幏鐑芥晸閿燂拷
 * @param   yCur y闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撶紒鐐点€嬮幏鐑芥晸閿燂拷
 * @param   Width  闁跨喐鏋婚幏鐑芥晸閼哄倻娈戦崠鈩冨闁跨噦鎷�
 * @param   Height 闁跨喐鏋婚幏鐑芥晸閼哄倻娈戞妯款啇閹凤拷
 * @retval  闁跨喐鏋婚幏锟�
 */
void BspLCD_SetDispWin(uint16_t xCur, uint16_t yCur, uint16_t Width, uint16_t Height)
{
    /* 闁跨喎鈧喎鐣綳闁跨喐鏋婚幏鐑芥晸閺傘倖瀚� */
    BspLCD_WriteComm(BspLCD_Dev.SetXCmd);
    BspLCD_WriteData(xCur >> 8);
    BspLCD_WriteData(0xFF & xCur);                  
    BspLCD_WriteData((xCur + Width - 1) >> 8);
    BspLCD_WriteData(0xFF & (xCur + Width - 1));
    
    /* 闁跨喎鈧喎鐣綴闁跨喐鏋婚幏鐑芥晸閺傘倖瀚� */
    BspLCD_WriteComm(BspLCD_Dev.SetYCmd);
    BspLCD_WriteData(yCur >> 8);
    BspLCD_WriteData(0xFF & yCur);
    BspLCD_WriteData((yCur + Height - 1) >> 8);
    BspLCD_WriteData(0xFF & (yCur + Height - 1));
    
    /* 闁跨喐鏋婚幏鐑芥晸閻ㄥ棜鎻幏锟� */
    BspLCD_WR_RamPrepare();
}

/**
 * @func    BspLCD_GetXSize
 * @brief   闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹峰嘲绠烽柨鐔惰寧閸栤剝瀚归柨鐕傛嫹
 * @retval  闁跨喐鏋婚幏宄扮闁跨喍鑼庨崠鈩冨闁跨噦鎷�
 */
uint16_t BspLCD_GetXSize(void)
{
    return BspLCD_Dev.Width;
}

/**
 * @func    BspLCD_GetYSize
 * @brief   闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹峰嘲绠烽柨鐔惰寧妤傛ǹ顔愰幏锟�
 * @retval  闁跨喐鏋婚幏宄扮闁跨喍鑼庢妯款啇閹凤拷
 */
uint16_t BspLCD_GetYSize(void)
{
    return BspLCD_Dev.Height;
}
/**
 * @func    BspLCD_SetDispCur
 * @brief   闁跨喐鏋婚幏绋珻D闁跨喐鏋婚幏閿嬪瘹闁跨喐鏋婚幏鐑芥晸閺傘倖瀚规担宥夋晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻惌顐ゆ閹风兘鏁撻敓锟�
 * @param   xPos x闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撶紒鐐点€嬮幏鐑芥晸閿燂拷
 * @param   yPos y闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撶紒鐐点€嬮幏鐑芥晸閿燂拷
 * @retval  闁跨喐鏋婚幏锟�
 */
void BspLCD_SetDispCur(uint16_t xPos, uint16_t yPos)
{
    BspLCD_Pos_t xyInput;
    
    xyInput.xyPos = xPos;
    
    /* 闁跨喎鈧喎鐣綳闁跨喐鏋婚幏鐑芥晸閺傘倖瀚� */
    LCD_OPERATION->REG = BspLCD_Dev.SetXCmd;
    LCD_OPERATION->RAM = xyInput.Pos.hBit;
    LCD_OPERATION->RAM = xyInput.Pos.lBit;
    LCD_OPERATION->RAM = 0x01;
    LCD_OPERATION->RAM = 0xDF;
    
    xyInput.xyPos = yPos;
    
    /* 闁跨喎鈧喎鐣綴闁跨喐鏋婚幏鐑芥晸閺傘倖瀚� */
    LCD_OPERATION->REG = BspLCD_Dev.SetYCmd;
    LCD_OPERATION->RAM = xyInput.Pos.hBit;
    LCD_OPERATION->RAM = xyInput.Pos.lBit;
    LCD_OPERATION->RAM = 0x01;
    LCD_OPERATION->RAM = 0x3F;
    
    /* 闁跨喐鏋婚幏鐑芥晸閻ㄥ棜鎻幏锟� */
    LCD_OPERATION->REG = BspLCD_Dev.WramCmd;
}

/**
 * @func    BspLCD_BGR_ToRGB
 * @brief   闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔稿焻闂堚晜瀚瑰蹇涙晸閺傘倖瀚归柨鐔告灮閹风柉娴嗛柨鐔告灮閹凤拷
 * @param   Color 鐟曚焦澧介柨鐔告灮閹风柉娴嗛柨鐔告灮閹风兘鏁撻弬銈嗗闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�
 * @retval  RGB 鏉烆剟鏁撻弬銈嗗闁跨喐鏋婚幏宄板仸闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐕傛嫹
 */
__attribute__((unused)) uint16_t BspLCD_BGR_ToRGB(uint16_t Color)
{
    uint16_t  r,g,b,RGB;   
    
    b = (Color >> 0) & 0x1f;
    g = (Color >> 5) & 0x3f;
    r = (Color >> 11) & 0x1f;
    
    RGB = (b << 11) + (g << 5) + (r << 0);		 
    
    return(RGB);
} 

/**
 * @func    BspLCD_QuitWinMode
 * @brief   闁跨喎澹欑粵瑙勫闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹烽銇氬Ο鈥崇础闁跨喐鏋婚幏鐑芥晸閺傘倖瀚规稉鍝勫弿闁跨喐鏋婚幏鐑芥晸閺傘倖瀚圭粈鐑樐佸锟�
 * @retval  闁跨喐鏋婚幏锟�
 */
__attribute__((unused)) static void BspLCD_QuitWinMode(void)
{
    BspLCD_SetDispWin(0, 0, BspLCD_Dev.Width, BspLCD_Dev.Height);
}

/**
 * @func    BspLCD_DispOn
 * @brief   闁跨喐鏋婚幏鐑芥晸閺傘倖瀚圭粈锟�
 * @retval  闁跨喐鏋婚幏锟�
 */
void BspLCD_DispOn(void)
{
    BspLCD_WriteComm(BspLCD_Dev.DispOnCmd);
}

/**
 * @func    BspLCD_DispOff
 * @brief   闁跨喐鍩呴幉瀣闁跨喐鏋婚幏椋庛仛
 * @retval  闁跨喐鏋婚幏锟�
 */
void BspLCD_DispOff(void)
{
    BspLCD_WriteComm(BspLCD_Dev.DispOffCmd);
}

/**
 * @func    BspLCD_ClrScr
 * @brief   闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻弬銈嗗闁跨喐鏋婚幏鐑芥晸缂傛惌鍊辩喊澶嬪闁跨喐鏋婚幏鐑芥晸閿燂拷
 * @param   pColor 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归懝锟�
 * @retval  闁跨喐鏋婚幏锟�
 */
void BspLCD_ClrScr(uint16_t pColor)
{
    uint32_t i;
    uint32_t n;
    
    BspLCD_SetDispWin(0, 0, BspLCD_Dev.Width, BspLCD_Dev.Height);
    
#if 1		/* 闁跨喕鍓兼导娆愬闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归幍褔鏁撻弬銈嗗闁跨喎濮拋瑙勫 */
    n = (BspLCD_Dev.Width * BspLCD_Dev.Height) / 8;
    for (i = 0; i < n; i++)
    {
        LCD_OPERATION->RAM = pColor;
        LCD_OPERATION->RAM = pColor;
        LCD_OPERATION->RAM = pColor;
        LCD_OPERATION->RAM = pColor;
      
        LCD_OPERATION->RAM = pColor;
        LCD_OPERATION->RAM = pColor;
        LCD_OPERATION->RAM = pColor;
        LCD_OPERATION->RAM = pColor;
    }
#else
    n = LCDDev.Width * LCDDev.Height;
    while (n--)
        LCD_OPERATION->RAM = pColor;
#endif
}

/**
 * @func	BspLCD_PutPixel
 * @brief 	闁跨喐鏋婚幏锟�1闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹凤拷
 * @param	xCur 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚箈闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�
 * @param	yCur 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚箉闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�
 * @param	pColor 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风柉澹�
 * @retval	闁跨喐鏋婚幏锟�
 */
void BspLCD_PutPixel(uint16_t xCur, uint16_t yCur, uint16_t pColor)
{
    BspLCD_SetDispCur(xCur, yCur);/* 闁跨喐鏋婚幏鐑芥晸閻偆娅㈤幏鐑芥晸鏉炲じ绱幏鐑芥晸閿燂拷 */
	
	LCD_OPERATION->RAM = pColor;
}

/**
 * @func	BspLCD_PutPixelNoPos
 * @brief 	闁跨喐鏋婚幏锟�1闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹凤拷
 * @param	pColor 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风柉澹�
 * @retval	闁跨喐鏋婚幏锟�
 */
void BspLCD_PutPixelNoPos(uint16_t pColor)
{
	LCD_OPERATION->RAM = pColor;
}

/**
 * @func    BspLCD_GetPixel
 * @brief   闁跨喐鏋婚幏宄板絿1闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹凤拷
 * @param   xCur 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚箈闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�
 * @param   yCur 闁跨喐鏋婚幏鐑芥晸閺傘倖瀚箉闁跨喐鏋婚幏鐑芥晸閺傘倖瀚�
 * @retval  闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鏁撻弬銈嗗闁跨喐鍩呯喊澶嬪闁跨喐鏋婚幏鐑芥晸缂傝揪鎷�
 */
uint16_t BspLCD_GetPixel(uint16_t xCur, uint16_t yCur)
{
    uint16_t R = 0, G = 0, B = 0 ;
    
    BspLCD_SetDispCur(xCur, yCur);	/* 闁跨喐鏋婚幏鐑芥晸閻偆娅㈤幏鐑芥晸鏉炲じ绱幏鐑芥晸閿燂拷 */
    
    LCD_OPERATION->REG = 0x2E;
    R = LCD_OPERATION->RAM; 	/* 闁跨喐鏋婚幏锟�1闁跨喐鏋婚幏鐑芥晸閻櫬ゎ啇閹风兘鏁撻弬銈嗗闁跨喐鏋婚幏鐑芥晸閺傘倖瀚� */
    R = LCD_OPERATION->RAM;
    B = LCD_OPERATION->RAM;
    G = LCD_OPERATION->RAM;
    
    return (((R >> 11) << 11) | ((G >> 10 ) << 5) | (B >> 11));
}

/**
 * @func    BspLCD_FuncInit
 * @brief   LCD闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹峰嘲鎲抽柨鐔告灮閹峰嘲顫愰柨鐔告灮閹凤拷
 * @retval  闁跨喐鏋婚幏锟�
 */
void BspLCD_FuncInit(void)
{
    BspLCD.ClrScr = &BspLCD_ClrScr;
    BspLCD.DispOff = &BspLCD_DispOff;
    BspLCD.DispOn = &BspLCD_DispOn;
    BspLCD.GetXSize = &BspLCD_GetXSize;
    BspLCD.GetYSize = &BspLCD_GetYSize;
    BspLCD.BGRToRGB = &BspLCD_BGR_ToRGB;
    BspLCD.GetPixel = &BspLCD_GetPixel;
    BspLCD.Init = &BspLCD_Init;
    BspLCD.PutPixel = &BspLCD_PutPixel;
    BspLCD.PutPixelNoPos = &BspLCD_PutPixelNoPos;
    BspLCD.SetDispDir = &BspLCD_SetDispDir;
    BspLCD.SetDispCur = &BspLCD_SetDispCur;
    BspLCD.SetDispWin = &BspLCD_SetDispWin;
}






