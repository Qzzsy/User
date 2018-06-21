#include "bsp_rtp_touch.h"
#include "gpio.h"

#define T_DIN_WRITE_H   LL_GPIO_SetOutputPin(T_MOSI_GPIO_Port, T_MOSI_Pin)
#define T_DIN_WRITE_L   LL_GPIO_ResetOutputPin(T_MOSI_GPIO_Port, T_MOSI_Pin)
#define T_CLK_WRITE_H   LL_GPIO_SetOutputPin(T_SCK_GPIO_Port, T_SCK_Pin)
#define T_CLK_WRITE_L   LL_GPIO_ResetOutputPin(T_SCK_GPIO_Port, T_SCK_Pin)
#define T_CS_WRITE_H    LL_GPIO_SetOutputPin(T_CS_GPIO_Port, T_CS_Pin)
#define T_CS_WRITE_L    LL_GPIO_ResetOutputPin(T_CS_GPIO_Port, T_CS_Pin)

#define T_MISO_READ     LL_GPIO_IsInputPinSet(T_MISO_GPIO_Port, T_MISO_Pin)
#define T_PEN_READ      LL_GPIO_IsInputPinSet(T_PEN_GPIO_Port, T_PEN_Pin)

//默认为touchtype=0的数据.
#define CMD_RDX     0xD0
#define CMD_RDY     0x90
#define READ_TIMES  5 	    //读取次数
#define LOST_VAL    1	  	//丢弃值
#define ERR_RANGE   50      //误差范围 

/**
 * @func    BspRTP_Delay
 * @brief   BspRTP延时，用于初始化过程
 * @param   nCount 延时的大小
 * @retval  无
 */
void BspRTP_Delay(__IO uint32_t Number)
{
    uint32_t i=0;

    while (Number--)
    {
        i = 24;
        while (i--);
    }
}
//SPI写数据
//向触摸屏IC写入1byte数据    
//num:要写入的数据
static void RTP_WriteByte(uint8_t num)    
{  
    uint8_t count = 0;   
    for (count = 0; count < 8; count++)  
    { 	  
        if (num & 0x80)
        {
            T_DIN_WRITE_H;  
        }
        else 
        {
            T_DIN_WRITE_L;   
        }
        num <<= 1;    
        T_CLK_WRITE_L; 	   	    
        BspRTP_Delay(1);   
        T_CLK_WRITE_H;		//上升沿有效	        
    }		 			    
} 

//SPI读数据 
//从触摸屏IC读取adc值
//CMD:指令
//返回值:读到的数据	   
static uint16_t RTP_Read_AD(uint8_t CMD)	  
{ 	 
	uint8_t count = 0; 	  
	uint16_t Num = 0; 
	T_CLK_WRITE_L;		//先拉低时钟 	 
	T_DIN_WRITE_L; 	    //拉低数据线
	T_CS_WRITE_L; 		//选中触摸屏IC
	RTP_WriteByte(CMD); //发送命令字
	BspRTP_Delay(8);//ADS7846的转换时间最长为6us
	T_CLK_WRITE_L; 	     	    
	BspRTP_Delay(1);    	   
	T_CLK_WRITE_H;		//给1个时钟，清除BUSY	   	    
	BspRTP_Delay(1);     	    
	T_CLK_WRITE_L; 	     	    
	for (count = 0; count < 16; count++)//读出16位数据,只有高12位有效 
	{ 				
		Num <<= 1; 	  
		T_CLK_WRITE_L;	//下降沿有效    	    
        BspRTP_Delay(1);   	 	   
		T_CLK_WRITE_H;  	   
		if (T_MISO_READ)
        {
            Num++; 		
        }              
	}  	
	Num >>= 4;   	//只有高12位有效
	T_CS_WRITE_H;		//释放片选	 
	return(Num);   
}

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
	for (i = 0; i < READ_TIMES - 1; i++)//排序
	{
		for (j = i + 1; j < READ_TIMES; j++)
		{
			if(buf[i] > buf[j])  //升序排列
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

//读取x,y坐标
//最小值不能少于100.
//x,y:读取到的坐标值
//返回值:0,失败;1,成功。
static uint8_t RTP_Read_XY(uint16_t *x, uint16_t *y)
{
	uint16_t xtemp, ytemp;			 	 		  
	xtemp = RTP_Read_XOY(CMD_RDX);
	ytemp = RTP_Read_XOY(CMD_RDY);	  												   
//	if (xtemp < 100 || ytemp < 100)
//        return 0;//读数失败
	*x = xtemp;
	*y = ytemp;
	return 1;//读数成功
}

uint8_t RTP_Read_XY2(uint16_t *x, uint16_t *y) 
{
	uint16_t x1, y1;
 	uint16_t x2, y2;
 	uint8_t flag;    
    flag = RTP_Read_XY(&x1, &y1);   
    if (flag == 0)
    {
        return(0);
    }
    flag = RTP_Read_XY(&x2, &y2);	
    
    if (flag == 0)
    {
        return(0);   
    }
    if(((x2 <= x1 && x1 < x2 + ERR_RANGE) || (x1 <= x2 && x2 < x1 + ERR_RANGE))//前后两次采样在+-50内
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
	if(T_PEN_READ == 0)//有按键按下
	{
        RTP_Read_XY2(&ta, &tb);//读取物理坐标
    }
    return true;
}  
