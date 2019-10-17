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
//������Կ

unsigned char CrebitKey[] = {};
//Ȧ����Կ  

unsigned char MmasterKey[] = {};
//������Կ

unsigned char MaintainKey_DF01[] = {};
//  3F01ά����Կ


unsigned char Name3F01[]= {};
//3F01 ����

unsigned char Pinkey[]= {};
//PIN KEY
*/

unsigned char setcard_password[6]= {0x00,0x00,0x00,0x00,0x00,0x00};	//��ʱ


unsigned char PsamNum[6]= {0x00,0x00,0x00,0x00,0x00,0x01};
//�ն˻���

unsigned char Name1001[]= {0xA0,0x00,0x00,0x00,0x03,0x86,0x98,0x07,0x01};

unsigned char Name2001[]= {0x41,0x44,0x44,0x2E,0x41,0x50,0x50,0x31};

unsigned char QuerrChas[]= {0x80,0x5c,0x00,0x02,0x04};

static unsigned char GetChall4[] = {0x00,0x84,0x00,0x00,0x04};
//ȡ����� 4

static unsigned char GetChall8[] = {0x00,0x84,0x00,0x00,0x08};
//ȡ����� 8
static unsigned char KeyALL[8] = {0xF3,0xBA,0xB5,0x94,0x47,0xD1,0x3A,0xC5};

unsigned char ExterKey[]= {0x28,0x4e,0x73,0x3d,0x3f,0x96,0x6e,0x28,0x80,0xfc,0xdb,0x31,0xa2,0x93,0x89,0x2c};

unsigned char MFExterKey[]= {0x7F,0x02,0x03,0x44,0x55,0xAB,0x5A,0xA5,0xCA,0xCB,0x51,0x52,0x53,0xF8,0x5F,0xFA};


static unsigned char CardCash[2]= {0};

unsigned char Send[256]= {0};
unsigned char Recv[256]= {0};

//////////////////�ļ���ʶ�Ķ���/////////////////////////////////////////////////////////////////////////////////
//
//Ϊͳһ��������Ķ����룬���������е��ļ�������Ӹ�_Flag,��ʾ���Ǹö�Ӧ�ļ��ı�ʶ//

unsigned char static MF_File_3F00[2]  = {0x3f,0x00};      //MF�ļ���ʶ3F00
//unsigned char static KEY_File_0000[2] = {0x00,0x00};      //MF�µ���Կ�ļ���ʶ0000
//unsigned char static ExKey_Flag       = {0x00};           //MF�µ��ⲿ��Կ��ʶ00
//unsigned char static LineKey_Flag     = {0x01}; 		    //MF�µ���·������Կ��ʶ01
//unsigned char static RECORD_File[2]   = {0x00,0x01}; 		//MF�µĶ�����¼�ļ���ʶ0001
//unsigned char static RECORD_Flag      = {0x08}; 		    //MF�µĶ�����¼��ʶ08

//unsigned char static CZKey_Flag={0x01};	  //Ȧ����Կ��ʶΪ01
unsigned char static DPKey_Flag={0x00};	  //������Կ��ʶΪ00
unsigned char static DF_ExKey_Flag  = {0x00};      //DF�µ��ⲿ��Կ��ʶ00
unsigned char static DF_Inital_Key_Flag  = {0x00}; //DF�µ��ڲ���Կ��ʶ00


//unsigned char static FM1208_DF_PIN_Flag={0x00};			//DF�µĿ���PIN��Կ��ʶ00
unsigned char static FM1208_DF_0015_Flag[]={0x00,0x15};	//����Ӧ�û��������ļ���ʶ0015
unsigned char static FM1208_DF_0016_Flag[]={0x00,0x16};	//�ֿ��˻�����Ϣ���ݵ��ļ���ʶ0016
//unsigned char static FM1208_DF_0018_Flag[]={0x00,0x18};   //������ϸ�ļ���ʶ0018
//unsigned char static FM1208_DF_0001_Flag[]={0x00,0x01};	//���Ӵ����ļ���ʶ0001
//unsigned char static FM1208_DF_0002_Flag[]={0x00,0x02};	//����Ǯ���ļ���ʶ0002



unsigned char DErrFlag = 0x00;
static LongUnon OldCash,OldCi;

MeterParameter MeterPara;
//MeterParameter t_MeterPara;
RecordFormat SaveData;//��ϸ�洢�ṹ��
/********************************************************************************************
//   CPU ������
//  ��Ƭ:����FM1208  COS-73
//   ����ס����Ӧ��
//   �͸��˻�Ӧ��
**************************************************************


*********************************************************************************************/
#define MAKECOMCPU  TypeAComCOS


/*
*************************************************************************************************************
- �������� : MoneyChange
- ����˵�� :
- ������� :
- ������� :
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
- �������� : uint8_t CL_ErrNo(uint8_t com)
- ����˵�� : ����Ƭ��Ϣ
- ������� : BinName:�������ļ��Ķ��ı�ʶ
						datastart:��ʼλ��
						datalen�����ݳ���
						*Rdata������������

- ������� :
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
                Send[4] = datalen;//00 Ϊ�ܳ���
                len = 5;
                break;
						

            case 0x17:
                Send[0] = 0x00;
                Send[1] = 0xb2;
                Send[2] = datastart;  // 00017 ��¼��
                Send[3] = (BinName<<3)&0xF8;
                Send[4] = datalen;//00 Ϊ�ܳ���
                len = 5;
                break;

            case 0x10:
            case 0x18:
            case 0x1a:
                Send[0] = 0x00;
                Send[1] = 0xb2;
                Send[2] = datastart;  // 00 ��¼��
                Send[3] = (BinName<<3)|0x04;
                Send[4] = datalen;// 00 Ϊ�ܳ���
                len = 5;
                break;

            default:
                Loop = 0;
                return 1;

            }

            status = MAKECOMCPU(Send,Recv,&len);
            if(status == MI_OK)
            {
                if(memcmp(Recv,School_Name,10) == 0)	//�Ƚ�ѧУ����
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
- �������� : CPUCard_SelectAppDF
- ����˵�� :	 ѡ��Ŀ¼
- ������� :
- ������� :
*************************************************************************************************************
*/
unsigned char CPUCard_SelectAppDF(unsigned char *DFname,unsigned char *Recvdata,unsigned  char *plen)
{
    char status=1;
    unsigned char len;
		//ѡ��MF�µĸ�Ŀ¼����
    unsigned char selfileDF[]= {0x00,0xa4,0x04,0x00}; //,0x09,0xA0,0x00,0x00,0x00,0x03,0x86,0x98,0x07,0x01,0x00};
    unsigned char selfileDFFci[]= {0x00,0xa4,0x00,0x00}; //,0x09,0xA0,0x00,0x00,0x00,0x03,0x86,0x98,0x07,0x01,0x00};

    //ѡ��Ӧ�� DF01  ����Ӧ��(3F01 -- 3F06)

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

    status = MAKECOMCPU(Send,Recv,&len);	//����: ISO 14443  TypeA ������COS����	����: *send ��������  *recv �������� *pLen ����/���յĳ���
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



//===================================CPU����������=====================================
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
    uint8_t   rateuintcm;//�嵥λ
		
		
#if 1
    while(Loop)
    {

        switch(step)
        {
				//-------------Ѱ��---------------
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
				//-------------CPU����λ---------------
        case 1:
						DEBUG_RFID("---->1<----\r\n");					
            status = TypeARest(Recvbuf,&len); //CPU����λ
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
				//---------------ѡ��Ŀ¼-----------------
        case 2:
						DEBUG_RFID("---->2<----\r\n");					
            len = 2;
            memset(Recvbuf,0,sizeof(Recvbuf));
				
            status = CPUCard_SelectAppDF("\x3F\x00",Recvbuf,&len); //��CPU����MF��Ŀ¼��3F01�ļ�//		         				
            if(status == MI_OK)
            {
                step++;

            }
            else
            {
                Loop = 0;
            }

            break;
				//---------------���ļ�-----------------
        case 3:
						DEBUG_RFID("---->3<----\r\n");
				
            memset(Recvbuf,0,sizeof(Recvbuf));				
            status = CPUCard_SelectAppDF("\xEC\x01",Recvbuf,&len); //��CPU����3F01��EC01�ļ�//		         				
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
            status = CPUCard_SelectAppDF("\x00\x16",Recvbuf,&len); //��CPU����EC01��0016�ļ�//		         				
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
            status = CPUCard_ReadFileAEF(0x16,47,10,Recvbuf); //����Ƭ��Ϣ				
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



        case 1://�û�
//            TMR1_Init();
//            test_time=0;

             status=User_CpuCard();//CPU��ֻ���û���
            if(status!=0) step++;
            else Loop = 0;


            break;
        case 2://�ɼ���
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



