#include <stm32f0xx.h>
#include "RC520.h"
#include "cpucard.h"
//#include "memory.h" 
#include  <string.h>

#define DBG_KEY    0

//static CardKey PSAMKey;
uint8_t CardCsn[6];
uint8_t DesKeyFlag;

static uint8_t APPFlag = 0;
CardKey CPUKey;

static uint8_t School_Name[10]={0xBD,0xAD,0xC4,0xCF,0xB4,0xF3,0xD1,0xA7,0x30,0x37};
 
/*
//static LongUnon DevSID;
unsigned char DebitKey[] =  {};
//消费密钥

unsigned char CrebitKey[] = {};
//圈存密钥  

unsigned char MmasterKey[] = {};
//主控密钥

unsigned char MaintainKey_DF01[] = {};
//  3F01维护密钥


unsigned char Name3F01[]= {};
//3F01 名称

unsigned char Pinkey[]= {};
//PIN KEY
*/

unsigned char setcard_password[6]= {0x00,0x00,0x00,0x00,0x00,0x00};	//临时


unsigned char PsamNum[6]= {0x00,0x00,0x00,0x00,0x00,0x01};
//终端机号

unsigned char Name1001[]= {0xA0,0x00,0x00,0x00,0x03,0x86,0x98,0x07,0x01};

unsigned char Name2001[]= {0x41,0x44,0x44,0x2E,0x41,0x50,0x50,0x31};

unsigned char QuerrChas[]= {0x80,0x5c,0x00,0x02,0x04};

static unsigned char GetChall4[] = {0x00,0x84,0x00,0x00,0x04};
//取随机数 4

static unsigned char GetChall8[] = {0x00,0x84,0x00,0x00,0x08};
//取随机数 8
static unsigned char KeyALL[8] = {0xF3,0xBA,0xB5,0x94,0x47,0xD1,0x3A,0xC5};

unsigned char ExterKey[]= {0x28,0x4e,0x73,0x3d,0x3f,0x96,0x6e,0x28,0x80,0xfc,0xdb,0x31,0xa2,0x93,0x89,0x2c};

unsigned char MFExterKey[]= {0x7F,0x02,0x03,0x44,0x55,0xAB,0x5A,0xA5,0xCA,0xCB,0x51,0x52,0x53,0xF8,0x5F,0xFA};


static unsigned char CardCash[2]= {0};

unsigned char Send[256]= {0};
unsigned char Recv[256]= {0};

//////////////////文件标识的定义/////////////////////////////////////////////////////////////////////////////////
//
//为统一方便管理阅读代码，我们在所有的文件名后面加个_Flag,表示就是该对应文件的标识//

unsigned char static MF_File_3F00[2]  = {0x3f,0x00};      //MF文件标识3F00
//unsigned char static KEY_File_0000[2] = {0x00,0x00};      //MF下的密钥文件标识0000
//unsigned char static ExKey_Flag       = {0x00};           //MF下的外部密钥标识00
//unsigned char static LineKey_Flag     = {0x01}; 		    //MF下的线路保护密钥标识01
//unsigned char static RECORD_File[2]   = {0x00,0x01}; 		//MF下的定长记录文件标识0001
//unsigned char static RECORD_Flag      = {0x08}; 		    //MF下的定长记录标识08

//unsigned char static CZKey_Flag={0x01};	  //圈存密钥标识为01
unsigned char static DPKey_Flag={0x00};	  //消费密钥标识为00
unsigned char static DF_ExKey_Flag  = {0x00};      //DF下的外部密钥标识00
unsigned char static DF_Inital_Key_Flag  = {0x00}; //DF下的内部密钥标识00


//unsigned char static FM1208_DF_PIN_Flag={0x00};			//DF下的口令PIN密钥标识00
unsigned char static FM1208_DF_0015_Flag[]={0x00,0x15};	//公共应用基本数据文件标识0015
unsigned char static FM1208_DF_0016_Flag[]={0x00,0x16};	//持卡人基本信息数据的文件标识0016
//unsigned char static FM1208_DF_0018_Flag[]={0x00,0x18};   //交易明细文件标识0018
//unsigned char static FM1208_DF_0001_Flag[]={0x00,0x01};	//电子存折文件标识0001
//unsigned char static FM1208_DF_0002_Flag[]={0x00,0x02};	//电子钱包文件标识0002



unsigned char DErrFlag = 0x00;
static LongUnon OldCash,OldCi;

MeterParameter MeterPara;
//MeterParameter t_MeterPara;
RecordFormat SaveData;//明细存储结构体
/********************************************************************************************
//   CPU 处理部分
//  卡片:复旦FM1208  COS-73
//   建立住建部应用
//   和个人化应用
**************************************************************


*********************************************************************************************/
#define MAKECOMCPU  TypeAComCOS


/*
*************************************************************************************************************
- 函数名称 : MoneyChange
- 函数说明 :
- 输入参数 :
- 输出参数 :
*************************************************************************************************************
*/
void MoneyChange(unsigned int Monei,unsigned char *Moneo)
{
    LongUnon Lbuf;
    unsigned char i;

    Lbuf.i = Monei;
    for(i=0; i<4; i++)
    {
        Moneo[i] = Lbuf.longbuf[3-i];
    }
}



/*
*************************************************************************************************************
- 函数名称 : uint8_t CL_ErrNo(uint8_t com)
- 函数说明 : 读卡片信息
- 输入参数 : BinName:二进制文件的短文标识
						datastart:起始位置
						datalen：数据长度
						*Rdata：读出的数据

- 输出参数 :
*************************************************************************************************************
*/
unsigned char CPUCard_ReadFileAEF(unsigned char BinName,unsigned char datastart,unsigned char datalen,unsigned char *Rdata)
{
    char status=1;
    unsigned char step = 1;
    unsigned char Loop = 1;
    unsigned char len;
    LongUnon Lbuf;
    APPFlag = 0;



    while(Loop)
    {
        switch(step)
        {
        case 1:
            memset(Send,0,sizeof(Send));
            memset(Recv,0,sizeof(Recv));

            switch(BinName)
            {
            case 0x02:
                len = sizeof(QuerrChas);// 04BYE
                memcpy(Send,QuerrChas,len);
                break;

            case 0x09:
            case 0x15:
            case 0x16:
                Send[0] = 0x00;
                Send[1] = 0xb0;
                Send[2] = 0x80|BinName;  // 00015
                Send[3] = datastart;
                Send[4] = datalen;//00 为总长度
                len = 5;
                break;
						

            case 0x17:
                Send[0] = 0x00;
                Send[1] = 0xb2;
                Send[2] = datastart;  // 00017 记录号
                Send[3] = (BinName<<3)&0xF8;
                Send[4] = datalen;//00 为总长度
                len = 5;
                break;

            case 0x10:
            case 0x18:
            case 0x1a:
                Send[0] = 0x00;
                Send[1] = 0xb2;
                Send[2] = datastart;  // 00 记录号
                Send[3] = (BinName<<3)|0x04;
                Send[4] = datalen;// 00 为总长度
                len = 5;
                break;

            default:
                Loop = 0;
                return 1;

            }

            status = MAKECOMCPU(Send,Recv,&len);
            if(status == MI_OK)
            {
                if(memcmp(Recv,School_Name,10) == 0)	//比较学校名称
                {
//                    memcpy(Rdata,Recv,datalen);
//                    if(BinName == 0x02)
//                    {
//                        memcpy(Lbuf.longbuf,Recv,datalen);
//                        MoneyChange(Lbuf.i,Rdata);
//                    }
                    step++;
                }
                else
                {
                    Loop = 0;
                }

            }
            else Loop = 0;
            break;

        case 2:
            step= 0;
            Loop = 0;
            break;

        default :
            Loop = 0;
            break;

        }
    }

    DEBUG_RFID("CPUCard_ReadFileAEF %d \r\n",step);


    return step;
}


/*
*************************************************************************************************************
- 函数名称 : CPUCard_SelectAppDF
- 函数说明 :	 选择目录
- 输入参数 :
- 输出参数 :
*************************************************************************************************************
*/
unsigned char CPUCard_SelectAppDF(unsigned char *DFname,unsigned char *Recvdata,unsigned  char *plen)
{
    char status=1;
    unsigned char len;
		//选择MF下的根目录命令
    unsigned char selfileDF[]= {0x00,0xa4,0x04,0x00}; //,0x09,0xA0,0x00,0x00,0x00,0x03,0x86,0x98,0x07,0x01,0x00};
    unsigned char selfileDFFci[]= {0x00,0xa4,0x00,0x00}; //,0x09,0xA0,0x00,0x00,0x00,0x03,0x86,0x98,0x07,0x01,0x00};

    //选择应用 DF01  电信应用(3F01 -- 3F06)

    memset(Send,0,sizeof(Send));

    if(*plen == 0x02)
    {
        memcpy(Send,selfileDFFci,sizeof(selfileDFFci));
    }
    else
    {
        memcpy(Send,selfileDF,sizeof(selfileDF));
    }

    Send[4] = *plen;
    len = *plen;
    memcpy(Send+5,DFname,len);
    len = len + 6;

    status = MAKECOMCPU(Send,Recv,&len);	//功能: ISO 14443  TypeA 卡发送COS命令	参数: *send 发送数据  *recv 返回数据 *pLen 发送/接收的长度
    if(status == MI_OK)
    {
        if((len >= 0x02)&&(Recv[len-2]==0x90)&&(Recv[len-1]==0x00))
        {
            memcpy(Recvdata,Recv,len);
            *plen = len;
            status = 0;
        }
        else
        {
            *plen = 0;
            status = 1;
        }

#ifdef DBG_FM1701
        {
            int i;
            DEBUG_RFID("---->SelectAppDF = %d\r\n",len);
            for(i =0 ; i < len; i++)
            {
                DEBUG_RFID("%02X",Recvdata[i]);
            }
            DEBUG_RFID("\r\n");
        }
#endif
    }

    return status;

}



//===================================CPU卡处理流程=====================================
unsigned char User_CpuCard(void)
{
    unsigned char status;
    unsigned char Loop = 1;
    unsigned char step = 1;
    unsigned char len;
    unsigned char type[2];	
    unsigned char csn[6];
    uint8_t password[8];
    unsigned char Recvbuf[256] = {0};
    uint8_t buf[8];
    uint8_t fen[3];

    uint8_t second=0;
    uint8_t i=0;
    uint16_t max_money=0;
    uint8_t   rateuintcm;//厘单位
		
		
#if 1
    while(Loop)
    {

        switch(step)
        {
				//-------------寻卡---------------
//        case 1:				
//            status = Read_M1_or_CPU_Card(csn,type);
//            if(status == 1)
//            {						
//                step++;
//							
//                break;
//            }
//            else Loop = 0;


//        break;			
				//-------------CPU卡复位---------------
        case 1:
						DEBUG_RFID("---->1<----\r\n");					
            status = TypeARest(Recvbuf,&len); //CPU卡复位
            if(status == MI_OK)
            {
                step++;

#ifdef  DBG_FM1701
                {
                    int i;
                    DEBUG_RFID("TypeARest len %02d Data:",len);
                    for(i=0; i<len; i++)
                    {
                        DEBUG_RFID("%02X",Recvbuf[i]);
                    }
                    DEBUG_RFID("\r\n");
                }
#endif

            }
            else
            {
                Loop = 0;
            }

            break;
				//---------------选择目录-----------------
        case 2:
						DEBUG_RFID("---->2<----\r\n");					
            len = 2;
            memset(Recvbuf,0,sizeof(Recvbuf));
				
            status = CPUCard_SelectAppDF("\x3F\x00",Recvbuf,&len); //打开CPU卡中MF主目录的3F01文件//		         				
            if(status == MI_OK)
            {
                step++;

            }
            else
            {
                Loop = 0;
            }

            break;
				//---------------读文件-----------------
        case 3:
						DEBUG_RFID("---->3<----\r\n");
				
            memset(Recvbuf,0,sizeof(Recvbuf));				
            status = CPUCard_SelectAppDF("\xEC\x01",Recvbuf,&len); //打开CPU卡中3F01的EC01文件//		         				
            if(status == MI_OK)
            {
                step++;

            }
            else
            {
                Loop = 0;
            }		
						
            break;
						
        case 4:
						DEBUG_RFID("---->4<----\r\n");
				
            memset(Recvbuf,0,sizeof(Recvbuf));				
            status = CPUCard_SelectAppDF("\x00\x16",Recvbuf,&len); //打开CPU卡中EC01的0016文件//		         				
            if(status == MI_OK)
            {
                step++;
							
            }
            else
            {
                Loop = 0;
            }		
						
            break;						
						
						

        case 5:						
						DEBUG_RFID("---->5<----\r\n");	
				
            memset(Recvbuf ,0,sizeof(Recvbuf));
            status = CPUCard_ReadFileAEF(0x16,47,10,Recvbuf); //读卡片信息				
            if(status == MI_OK)
            {
							step++;	
							
            }
            else
            {
                Loop = 0;
            }
						
						
            break;
							
        case 6:
            step=0;
            Loop = 0;
            break;        
        default :

            Loop = 0;
            break;
        }
    }
    //  Mf500PiccHalt();

    if(step != 1)
    {
        DEBUG_RFID("ReadCardCPU step= %d status=%02x \r\n",step,status);
    }
#endif
		
    return status;

}




void CpuCard_Loop(void)
{

    uint8_t status=1;
    uint8_t Loop,step;
    uint8_t stopflag;

    Loop = 1;
    step = 1;


    while(Loop)
    {


#ifdef  WDION
        FreeDog();
#endif


        switch(step)
        {



        case 1://用户
//            TMR1_Init();
//            test_time=0;

             status=User_CpuCard();//CPU卡只有用户卡
            if(status!=0) step++;
            else Loop = 0;


            break;
        case 2://采集卡
            step++;
//            status = Cpu_CollectCard();
//            if(status!=0) step++;
//            else Loop = 0;

            break;


        default :


            stopflag=0;

            Loop = 0;

            break;



        }


    }


}



