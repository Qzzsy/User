/**
 ******************************************************************************
 * @file      bsp_lcd.c
 * @author    门禁开发小组
 * @version   V1.0.3
 * @date      2018-01-26
 * @brief     这个文件是液晶初始化文件，在此文件内进行液晶的配置还有一些基本的操作
              液晶的驱动芯片为R61529驱动芯片
 * @History
 * Date           Author    version    		Notes
 * 2017-10-31       ZSY     V1.0.0      first version.
 * 2017-11-27       ZSY     V1.0.1      增加LCD的操作方法，完善LCD驱动
 * 2018-01-16       ZSY     V1.0.2      排版格式化，增强可视化
 * 2018-01-26       ZSY     V1.0.3      整理部分定义，添加私有和公有宏定义
 */
	
/* Includes ------------------------------------------------------------------*/
#include "bsp_lcd.h"

/* Private macro Definition --------------------------------------------------*/

/** 使用NOR/SRAM的 Bank1.sector4,地址位HADDR[27,26]=11 A6作为数据命令区分线
 * @note:设置时STM32内部会右移一位对其! 111 1110=0X7E	
 */   
#define LCD_BASE_ADDRESS        ((uint32_t)(0x6C000000 | 0x0000007E))

/* End private macro Definition ----------------------------------------------*/

/* global variable Declaration -----------------------------------------------*/

/* LCD参数对象 */
BspLCD_Dev_t BspLCD_Dev;

/* LCD操作对象 */
BspLCD_Func_t BspLCD;

/* LCD寄存器对象 */
BspLCD_t* BspLCD_RW = ((BspLCD_t *)LCD_BASE_ADDRESS);
/* User function Declaration --------------------------------------------------*/
static uint32_t BspLCD_ReadId(void);

void BspLCD_SetDispDir(uint8_t Dir);
void BspLCD_ClrScr(uint16_t pColor);

/* User functions -------------------------------------------------------------*/

/**
 * @func    BspLCD_WriteComm
 * @brief   lcd写命令到寄存器
 * @param   RegVal 指定的寄存器地址
 * @retval  无
 */
static void BspLCD_WriteComm(__IO uint16_t RegVal)
{   
    BspLCD_RW->REG = RegVal;     /* 写入要写的寄存器序号 */
}

/**
 * @func    BspLCD_WriteData
 * @brief   lcd写数据
 * @param   Data 要写入的数据
 * @retval  无
 */
static void BspLCD_WriteData(__IO uint16_t  Data)
{	  
    BspLCD_RW->RAM = Data;		 
}		

/**
 * @func    BspLCD_WR_Reg
 * @brief   往指定的寄存器写数据
 * @param   Index 寄存器
 * @param   congfig_temp 数据
 * @retval  无
 */
static void BspLCD_WR_Reg(uint16_t Index, uint16_t CongfigTemp)
{
    BspLCD_RW->REG = Index;
    BspLCD_RW->RAM = CongfigTemp;
}

/**
 * @func    BspLCD_ReadData
 * @brief   lcd读数据
 * @retval  读到的数据
 */
static uint16_t BspLCD_ReadData(void)
{
    __IO uint16_t Ram;                   //防止被优化
    Ram = BspLCD_RW->RAM;	
    return Ram;	 
}	

/**
 * @func    BspLCD_ReadReg
 * @brief   lcd读寄存器
 * @param   reg:寄存器地址
 * @retval  读到的数据
 */
static uint16_t BspLCD_ReadReg(uint16_t Reg)
{										   
    BspLCD_WriteComm(Reg);              //写入要读的寄存器序号
    return BspLCD_ReadData();           //返回读到的值
}  

/**
 * @func    BspLCD_WR_RamPrepare
 * @brief   lcd开始写入操作
 * @note    一般在操作结束时调用
 * @retval  无
 */
static void BspLCD_WR_RamPrepare(void)
{
    BspLCD_RW->REG = BspLCD_Dev.WramCmd;
}

/**
 * @func    BspLCD_Delay
 * @brief   lcd启动延时，用于初始化过程
 * @param   nCount 延时的大小
 * @retval  无
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
// * @brief   lcd复位操作，用于复位lcd的所有寄存器
// * @note    要延时足够的时间才能让lcd复位
// * @retval  无
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
 * @brief   根据不同的LCD加载不同的参数
 * @retval  无
 */
static void BspLCD_ConfigLocal(void)
{
    BspLCD_Dev.pHeight = LCD_HEIGHT;
    BspLCD_Dev.pWidth  = LCD_WDITH;
    
    if (LCD_ID == 0x9341)
    {
        BspLCD_Dev.WramCmd  = 0x2C;
        BspLCD_Dev.SetXCmd  = 0x2A;
        BspLCD_Dev.SetYCmd  = 0x2B;
        BspLCD_Dev.MemoryAccContCmd  = 0x36;
        BspLCD_Dev.DirHorNormalData  = 0xe8;
        BspLCD_Dev.DirHorReverseData = 0x28;
        BspLCD_Dev.DirVerNormalData  = 0x88;
        BspLCD_Dev.DirVerReverseData = 0x48;
        
        BspLCD_Dev.DispOnCmd  = 0x29;
        BspLCD_Dev.DispOffCmd = 0x28;
    }
    else if (LCD_ID == 0x61529)
    {
        BspLCD_Dev.WramCmd  = 0x2C;
        BspLCD_Dev.SetXCmd  = 0x2A;
        BspLCD_Dev.SetYCmd  = 0x2B;
        BspLCD_Dev.MemoryAccContCmd  = 0x36;
        BspLCD_Dev.DirHorNormalData  = 0xA0;
        BspLCD_Dev.DirHorReverseData = 0x60;
        BspLCD_Dev.DirVerNormalData  = 0x00;
        BspLCD_Dev.DirVerReverseData = 0xC0;
        
        BspLCD_Dev.DispOnCmd  = 0x29;
        BspLCD_Dev.DispOffCmd = 0x28;
    }
    
    /* 默认使用横屏 */
    BspLCD_Dev.Height = BspLCD_Dev.pWidth;
    BspLCD_Dev.Width  = BspLCD_Dev.pHeight;
}

/**
 * @func    BspLCD_Init
 * @brief   对lcd进行初始化配置
 * @retva   无
 */
void BspLCD_Init(void)
{
    BspLCD_ConfigLocal();
    
    BspLCD_Delay(10000);
    BspLCD_Delay(10000);
//    LCDReset();
    if (LCD_ID == 0x9341)
    {
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
        
        BspLCD_WriteComm(0xC0);     //Power control 
        BspLCD_WriteData(0x23);     //VRH[5:0] 
        
        BspLCD_WriteComm(0xC1);     //Power control 
        BspLCD_WriteData(0x10);     //SAP[2:0];BT[3:0] 
        
        BspLCD_WriteComm(0xC5);     //VCM control 
        BspLCD_WriteData(0x3e);     //对比度调节
        BspLCD_WriteData(0x28); 
        
        BspLCD_WriteComm(0xC7);     //VCM control2 
        BspLCD_WriteData(0x86);     //--
        
        BspLCD_WriteComm(0x36);     // Memory Access Control 
        BspLCD_WriteData(0xE8);     //48 68竖屏//28 E8 横屏
        
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
    }
    else if (LCD_ID == 0x61529)
    {
        /* config tft regval	*/
        BspLCD_WriteComm(0xB0);
        BspLCD_WriteData(0x04);
        
        BspLCD_WriteComm(0x36);    
        BspLCD_WriteData(0x00);    
        
        BspLCD_WriteComm(0x3A);    
        BspLCD_WriteData(0x55);   
        
        BspLCD_WriteComm(0xB4);    
        BspLCD_WriteData(0x00); 
        
        BspLCD_WriteComm(0xC0);
        BspLCD_WriteData(0x03);//0013
        BspLCD_WriteData(0xDF);//480
        BspLCD_WriteData(0x40);
        BspLCD_WriteData(0x12);
        BspLCD_WriteData(0x00);
        BspLCD_WriteData(0x01);
        BspLCD_WriteData(0x00);
        BspLCD_WriteData(0x43);
        
        
        BspLCD_WriteComm(0xC1);//frame frequency
        BspLCD_WriteData(0x05);//BCn,DIVn[1:0
        BspLCD_WriteData(0x2f);//RTNn[4:0] 
        BspLCD_WriteData(0x08);// BPn[7:0]
        BspLCD_WriteData(0x08);// FPn[7:0]
        BspLCD_WriteData(0x00);
        
        
        
        BspLCD_WriteComm(0xC4);
        BspLCD_WriteData(0x63);
        BspLCD_WriteData(0x00);
        BspLCD_WriteData(0x08);
        BspLCD_WriteData(0x08);
        
        BspLCD_WriteComm(0xC8);//Gamma
        BspLCD_WriteData(0x06);
        BspLCD_WriteData(0x0c);
        BspLCD_WriteData(0x16);
        BspLCD_WriteData(0x24);//26
        BspLCD_WriteData(0x30);//32 
        BspLCD_WriteData(0x48);
        BspLCD_WriteData(0x3d);
        BspLCD_WriteData(0x28);
        BspLCD_WriteData(0x20);
        BspLCD_WriteData(0x14);
        BspLCD_WriteData(0x0c);
        BspLCD_WriteData(0x04);
        
        BspLCD_WriteData(0x06);
        BspLCD_WriteData(0x0c);
        BspLCD_WriteData(0x16);
        BspLCD_WriteData(0x24);
        BspLCD_WriteData(0x30);
        BspLCD_WriteData(0x48);
        BspLCD_WriteData(0x3d);
        BspLCD_WriteData(0x28);
        BspLCD_WriteData(0x20);
        BspLCD_WriteData(0x14);
        BspLCD_WriteData(0x0c);
        BspLCD_WriteData(0x04);
        
        
        BspLCD_WriteComm(0xC9);//Gamma
        BspLCD_WriteData(0x06);
        BspLCD_WriteData(0x0c);
        BspLCD_WriteData(0x16);
        BspLCD_WriteData(0x24);//26
        BspLCD_WriteData(0x30);//32 
        BspLCD_WriteData(0x48);
        BspLCD_WriteData(0x3d);
        BspLCD_WriteData(0x28);
        BspLCD_WriteData(0x20);
        BspLCD_WriteData(0x14);
        BspLCD_WriteData(0x0c);
        BspLCD_WriteData(0x04);
        
        BspLCD_WriteData(0x06);
        BspLCD_WriteData(0x0c);
        BspLCD_WriteData(0x16);
        BspLCD_WriteData(0x24);
        BspLCD_WriteData(0x30);
        BspLCD_WriteData(0x48);
        BspLCD_WriteData(0x3d);
        BspLCD_WriteData(0x28);
        BspLCD_WriteData(0x20);
        BspLCD_WriteData(0x14);
        BspLCD_WriteData(0x0c);
        BspLCD_WriteData(0x04);
        
        
        BspLCD_WriteComm(0xCA);//Gamma
        BspLCD_WriteData(0x06);
        BspLCD_WriteData(0x0c);
        BspLCD_WriteData(0x16);
        BspLCD_WriteData(0x24);//26
        BspLCD_WriteData(0x30);//32 
        BspLCD_WriteData(0x48);
        BspLCD_WriteData(0x3d);
        BspLCD_WriteData(0x28);
        BspLCD_WriteData(0x20);
        BspLCD_WriteData(0x14);
        BspLCD_WriteData(0x0c);
        BspLCD_WriteData(0x04);
        
        BspLCD_WriteData(0x06);
        BspLCD_WriteData(0x0c);
        BspLCD_WriteData(0x16);
        BspLCD_WriteData(0x24);
        BspLCD_WriteData(0x30);
        BspLCD_WriteData(0x48);
        BspLCD_WriteData(0x3d);
        BspLCD_WriteData(0x28);
        BspLCD_WriteData(0x20);
        BspLCD_WriteData(0x14);
        BspLCD_WriteData(0x0c);
        BspLCD_WriteData(0x04);
        
        
        BspLCD_WriteComm(0xD0);
        BspLCD_WriteData(0x95);
        BspLCD_WriteData(0x06);
        BspLCD_WriteData(0x08);
        BspLCD_WriteData(0x10);
        BspLCD_WriteData(0x3f);
        
        
        BspLCD_WriteComm(0xD1);
        BspLCD_WriteData(0x02);
        BspLCD_WriteData(0x28);
        BspLCD_WriteData(0x28);
        BspLCD_WriteData(0x40);
        
        BspLCD_WriteComm(0xE1);    
        BspLCD_WriteData(0x00);    
        BspLCD_WriteData(0x00);    
        BspLCD_WriteData(0x00);    
        BspLCD_WriteData(0x00);    
        BspLCD_WriteData(0x00);   
        BspLCD_WriteData(0x00);   
        
        BspLCD_WriteComm(0xE2);    
        BspLCD_WriteData(0x80);    
        
        BspLCD_WriteComm(0x2A);    
        BspLCD_WriteData(0x00);    
        BspLCD_WriteData(0x00);    
        BspLCD_WriteData(0x01);    
        BspLCD_WriteData(0x3F);    
        
        BspLCD_WriteComm(0x2B);    
        BspLCD_WriteData(0x00);    
        BspLCD_WriteData(0x00);    
        BspLCD_WriteData(0x01);    
        BspLCD_WriteData(0xDF);    
        
        BspLCD_WriteComm(0x11);
        
        BspLCD_Delay(120);
        
        BspLCD_WriteComm(0x29);
        
        BspLCD_WriteComm(0xC1);//frame frequency
        BspLCD_WriteData(0x05);//BCn,DIVn[1:0]
        BspLCD_WriteData(0x2f);//RTNn[4:0] 
        BspLCD_WriteData(0x08);// BPn[7:0]
        BspLCD_WriteData(0x08);// FPn[7:0]
        BspLCD_WriteData(0x00);
      
        BspLCD_WriteComm(0x20);
        
        /**	横竖屏以及显示方向设置
         * bit D7-page address order	0的话输入和输出方向相同，1的话输入和输出上下颠倒
         * bit D6-column address order 0的话输出和输出方向相同，1的话输出和输出左右颠倒
         * bit D5-page/column addressing order 0的话正常，1的话行列调换
         * bit D4-display device line refresh order 	预留，默认设置为0
         * bit D3-RGB/BGR order 0为RGB，1为BGR
         * bit D2-display data latch order 预留，默认设置为0
         * bit D1-flip horizontal 预留 默认设置为0
         * bit D0-flip vertic 0正常，1上下对调
         */
        BspLCD_WriteComm(0x36);    
        BspLCD_WriteData(0xA0);
    }
    /* 默认情况下为横屏，设置横屏 */
    BspLCD_SetDispDir(DIR_HORIZONTAL_NORMAL);
    
    /* 清屏 */
    BspLCD_ClrScr(0x0000);
}

/**
 * @func    BspLCD_ReadId
 * @brief   读取LCD的ID
 * @retval  LCD的ID
 */
__attribute__((unused)) static uint32_t BspLCD_ReadId(void)
{
    uint16_t Buf[4];
    
    BspLCD_RW->REG = 0x04;
    Buf[0] = BspLCD_RW->RAM;
    Buf[1] = BspLCD_RW->RAM;
    Buf[2] = BspLCD_RW->RAM;
    Buf[3] = BspLCD_RW->RAM;
    
    return (Buf[1] << 16) + (Buf[2] << 8) + Buf[3];
}

/**
 * @func    BspLCD_SetDispDir
 * @brief   设置LCD的扫描方向
 * @param   Dir 需要设置的方向
 * @retval  无
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
 * @brief   选定LCD上指定的矩形区域，即设置一个窗口
 * @param   xCur x方向的起始点
 * @param   yCur y方向的起始点
 * @param   Width  窗口的宽度
 * @param   Height 窗口的高度
 * @retval  无
 */
void BspLCD_SetDispWin(uint16_t xCur, uint16_t yCur, uint16_t Width, uint16_t Height)
{
    __IO BspLCD_Pos_t StartPos, EndPos;

    StartPos.xyPos = xCur;
    EndPos.xyPos = xCur + Width - 1;
    /* 设定X坐标 */
    BspLCD_WriteComm(BspLCD_Dev.SetXCmd);
    BspLCD_WriteData(StartPos.Pos.hBit);
    BspLCD_WriteData(StartPos.Pos.lBit);                  
    BspLCD_WriteData(EndPos.Pos.hBit);
    BspLCD_WriteData(EndPos.Pos.lBit);
    
    StartPos.xyPos = yCur;
    EndPos.xyPos = yCur + Height - 1;
    /* 设定Y坐标 */
    BspLCD_WriteComm(BspLCD_Dev.SetYCmd);
    BspLCD_WriteData(StartPos.Pos.hBit);
    BspLCD_WriteData(StartPos.Pos.lBit);
    BspLCD_WriteData(EndPos.Pos.hBit);
    BspLCD_WriteData(EndPos.Pos.lBit);
    
    /* 开显存 */
    BspLCD_WR_RamPrepare();
}

/**
 * @func    BspLCD_GetXSize
 * @brief   获取屏幕的宽度
 * @retval  返回屏幕的宽度
 */
uint16_t BspLCD_GetXSize(void)
{
    return BspLCD_Dev.Width;
}

/**
 * @func    BspLCD_GetYSize
 * @brief   获取屏幕的高度
 * @retval  返回屏幕的高度
 */
uint16_t BspLCD_GetYSize(void)
{
    return BspLCD_Dev.Height;
}
/**
 * @func    BspLCD_SetDispCur
 * @brief   在LCD上指定的位置设置光标
 * @param   xPos x方向的起始点
 * @param   yPos y方向的起始点
 * @retval  无
 */
void BspLCD_SetDispCur(uint16_t xPos, uint16_t yPos)
{
    BspLCD_Pos_t xyInput;
    
    xyInput.xyPos = xPos;
    
    /* 设定X坐标 */
    BspLCD_RW->REG = BspLCD_Dev.SetXCmd;
    BspLCD_RW->RAM = xyInput.Pos.hBit;
    BspLCD_RW->RAM = xyInput.Pos.lBit;
    BspLCD_RW->RAM = 0x01;
    BspLCD_RW->RAM = 0xDF;
    
    xyInput.xyPos = yPos;
    
    /* 设定Y坐标 */
    BspLCD_RW->REG = BspLCD_Dev.SetYCmd;
    BspLCD_RW->RAM = xyInput.Pos.hBit;
    BspLCD_RW->RAM = xyInput.Pos.lBit;
    BspLCD_RW->RAM = 0x01;
    BspLCD_RW->RAM = 0x3F;
    
    /* 开显存 */
    BspLCD_RW->REG = BspLCD_Dev.WramCmd;
}

/**
 * @func    BspLCD_BGR_ToRGB
 * @brief   将像素格式进行转换
 * @param   Color 要执行转换的像素
 * @retval  RGB 转换完成的像素
 */
__attribute__((unused)) uint16_t BspLCD_BGR2RGB(uint16_t Color)
{
    uint16_t  r,g,b,RGB;  
    RGB_t RGB_Data, BGR_Data; 
    
    BGR_Data.Value = Color;
    RGB_Data.RGB.R = BGR_Data.RGB.B;
    RGB_Data.RGB.G = BGR_Data.RGB.G;
    RGB_Data.RGB.B = BGR_Data.RGB.R;	 
    
    return RGB_Data.Value;
} 

/**
 * @func    BspLCD_QuitWinMode
 * @brief   退出窗口显示模式，变为全屏显示模式
 * @retval  无
 */
__attribute__((unused)) static void BspLCD_QuitWinMode(void)
{
    BspLCD_SetDispWin(0, 0, BspLCD_Dev.Width, BspLCD_Dev.Height);
}

/**
 * @func    BspLCD_DispOn
 * @brief   打开显示
 * @retval  无
 */
void BspLCD_DispOn(void)
{
    BspLCD_WriteComm(BspLCD_Dev.DispOnCmd);
}

/**
 * @func    BspLCD_DispOff
 * @brief   关闭显示
 * @retval  无
 */
void BspLCD_DispOff(void)
{
    BspLCD_WriteComm(BspLCD_Dev.DispOffCmd);
}

/**
 * @func    BspLCD_ClrScr
 * @brief   根据输入的颜色值清屏
 * @param   pColor 背景色
 * @retval  无
 */
void BspLCD_ClrScr(uint16_t pColor)
{
    uint32_t i;
    uint32_t n;
    
    BspLCD_SetDispWin(0, 0, BspLCD_Dev.Width, BspLCD_Dev.Height);
    
#if 1		/* 优化代码执行速度 */
    n = (BspLCD_Dev.Width * BspLCD_Dev.Height) / 8;
    for (i = 0; i < n; i++)
    {
        BspLCD_RW->RAM = pColor;
        BspLCD_RW->RAM = pColor;
        BspLCD_RW->RAM = pColor;
        BspLCD_RW->RAM = pColor;
      
        BspLCD_RW->RAM = pColor;
        BspLCD_RW->RAM = pColor;
        BspLCD_RW->RAM = pColor;
        BspLCD_RW->RAM = pColor;
    }
#else
    n = LCDDev.Width * LCDDev.Height;
    while (n--)
        BspLCD_RW->RAM = pColor;
#endif
}

/**
 * @func	BspLCD_PutPixel
 * @brief 	画1个像素
 * @param	xCur 像素x坐标
 * @param	yCur 像素y坐标
 * @param	pColor 像素颜色
 * @retval	无
 */
void BspLCD_PutPixel(uint16_t xCur, uint16_t yCur, uint16_t pColor)
{
    BspLCD_SetDispCur(xCur, yCur);/* 设置光标位置 */
	
	BspLCD_RW->RAM = pColor;
}

/**
 * @func	BspLCD_PutPixelNoPos
 * @brief 	画1个像素
 * @param	pColor 像素颜色
 * @retval	无
 */
void BspLCD_PutPixelNoPos(uint16_t pColor)
{
	BspLCD_RW->RAM = pColor;
}

/**
 * @func    BspLCD_GetPixel
 * @brief   读取1个像素
 * @param   xCur 像素x坐标
 * @param   yCur 像素y坐标
 * @retval  读到的像素点的颜色
 */
uint16_t BspLCD_GetPixel(uint16_t xCur, uint16_t yCur)
{
    RGB_t ReadData;
    
    BspLCD_SetDispCur(xCur, yCur);	/* 设置光标位置 */
    
    BspLCD_RW->REG = 0x2E;
    ReadData.RGB.R = BspLCD_RW->RAM; 	/* 第1个哑读，丢弃 */
    ReadData.RGB.R = BspLCD_RW->RAM;
    ReadData.RGB.G = BspLCD_RW->RAM;
    ReadData.RGB.B = BspLCD_RW->RAM;
    
    return ReadData.Value;
}

void lv_flush_ready(void);
void BspLCD_Fill(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t pColor)
{
    uint32_t i;
    
    /* ??????? */
    BspLCD_SetDispWin(xStart, yStart, xEnd - xStart + 1, yEnd - yStart + 1);
    
    i = (xEnd - xStart + 1) * (yEnd - yStart + 1);

    /* ?????? */
    while(i--)
    {
        BspLCD_RW->RAM = pColor;
    }
    lv_flush_ready();
}

void BspLCD_FillColor(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t * pColor)
{
    uint32_t i;
    
    /* ??????? */
    BspLCD_SetDispWin(xStart, yStart, xEnd - xStart + 1, yEnd - yStart + 1);
    
    i = (xEnd - xStart + 1) * (yEnd - yStart + 1);

    /* ?????? */
    while(i--)
    {
        BspLCD_RW->RAM = *pColor++;
    }
    lv_flush_ready();
}
/**
 * @func    BspLCD_FuncInit
 * @brief   LCD方法成员初始化
 * @retval  无
 */
void BspLCD_FuncInit(void)
{
    BspLCD.ClrScr = &BspLCD_ClrScr;
    BspLCD.DispOff = &BspLCD_DispOff;
    BspLCD.DispOn = &BspLCD_DispOn;
    BspLCD.GetXSize = &BspLCD_GetXSize;
    BspLCD.GetYSize = &BspLCD_GetYSize;
    BspLCD.BGR2RGB = &BspLCD_BGR2RGB;
    BspLCD.GetPixel = &BspLCD_GetPixel;
    BspLCD.Init = &BspLCD_Init;
    BspLCD.PutPixel = &BspLCD_PutPixel;
    BspLCD.PutPixelNoPos = &BspLCD_PutPixelNoPos;
    BspLCD.SetDispDir = &BspLCD_SetDispDir;
    BspLCD.SetDispCur = &BspLCD_SetDispCur;
    BspLCD.SetDispWin = &BspLCD_SetDispWin;
    BspLCD.Fill = &BspLCD_Fill;
    BspLCD.FillColor = &BspLCD_FillColor;
}






