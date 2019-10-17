#ifndef _CPUCARD_H_
#define _CPUCARD_H_


#ifndef uint8_t
#define uint8_t  	unsigned char
#define uint16_t 	unsigned short
#define uint32_t 	unsigned int
#endif


#define NO_CARD_TIMES 2

typedef union
{
	unsigned char intbuf[2];
	unsigned short i;
} ShortUnon;


typedef union
{
	unsigned char longbuf[4];
	unsigned int  i;
} LongUnon;

typedef struct
{
//////////////////////////// CPU MF ʹ��
    unsigned char MFMasterKey[16];	//������Կ
    unsigned char MFMaintainKey[16]; //ά����Կ
//////////////////////////////CPU ADF ʹ��
    unsigned char MasterKey[16];   //Ӧ��������Կ
    unsigned char MaintainKey[16]; //Ӧ��ά����Կ
    unsigned char MifareKey[16]; //M1��ר����Կ
    unsigned char InsideKey[16]; //�ڲ���֤��Կ
    unsigned char DebitKey[16];  //������Կ
    unsigned char DebitKey1[16];  //������Կ 2
    unsigned char CrebitKey[16]; //Ȧ����Կ
    unsigned char CrebitKey1[16]; //Ȧ����Կ 2
    unsigned char TACKey[16];    //TAC��Կ
    unsigned char DPINKey[16]; //����PIN������Կ
    unsigned char RPINKey[16]; //��װPIN������Կ
    unsigned char PINKey[8]; //PIN������Կ

//////////////////////////// SAM��  process
    /*
    		 unsigned char ProcessKey[16]; //���ɹ�����Կ����Կ
    		 unsigned char UmaintainKey[16]; //�û���ά����Կ(MAMK)
    		 unsigned char MACKey[16]; //MAC��Կ(MMAC)
    		 unsigned char MEKKey[16]; //������Կ(MEK)
    		 unsigned char MMACKey[16]; //���ܡ�MAC��Կ(MMAC)
    */
} CardKey;

extern  CardKey CPUKey;

typedef struct
{
//0--38 ��39�ֽ�
    uint8_t   setcard_pwd[6];//��������
    uint8_t   ParaVersion[2]; //�����汾��
    uint8_t   DeviceType;    //�豸���ͣ� ��ˮ0X01  ��ˮ0X02  ��ˮ0X03   �Ҽ�¼0XEE
    uint8_t   ParaType;      //�������ͣ���׼����0x01   ���߽����շ� 0x02
    uint8_t   sector;       //������
    uint8_t   usermode;    //�û�ģʽ 11������ 00����ʱ
    uint16_t   miniunit;   //��С��λ
    uint8_t   rate[12];  //4�ֵַķ���
    // uint8_t   cmrate[4];  // 4����ķ���
    uint8_t   Ladder[6];  //����
    uint8_t   MinTime[3]; //��������ʱ�䣨0�룩��������ʱ�䣬 ����Ϊ��λ
    uint8_t   MinMoney[3]; //�����ѽ�1000ΪһԪ����λ���壩
    uint16_t   AutoCuttime; //�Զ��Ͽ�ʱ�䣨�룩û�ź����Ͽ�ʱ�䣬��6��Ϊһ����λ
    uint8_t  Max_Money[3];

} MeterParameter;//������


extern MeterParameter MeterPara;//�����ṹ��
//extern MeterParameter t_MeterPara;//�����ṹ��

typedef  struct
{
	unsigned char   CardCSN[4];          // 0- 3    ��оƬ���к�
	unsigned char   OperatorNum[3];      // 4- 6    �û���ţ�������(��ˮ��) 3�ֽ�
	unsigned char	 CardType;           // 7       ����  1 2 3 4 ���ַ���
	unsigned char   DeviceNum[4];         // 8- 11   �ն����û���
	unsigned int    Count;               // 12-13       ��ʹ�ô���
	unsigned char   DeviceMode;          // 14      �豸ģʽ
	unsigned long   WaterStamp;          // 15--18  �豸��ˮ��
	unsigned int    CurrentConsume;      // 19- 20  �������ѵ�Ǯ
	unsigned long   RestMoney;           // 21- 24  ����ʣ����
	unsigned char   Time[6];   	        // 25- 30  ����ʱ�� BCD��
	unsigned char   AccuCRC;             // 31      Ч��λ
} RecordFormat;
extern  RecordFormat SaveData;


extern unsigned char CPUCard_DebitChas(unsigned int Money,unsigned int DevSID,const unsigned char *Key,const unsigned char *TimeC);

extern uint8_t CheckCpu_Online(void);

extern void CpuCard_Loop(void);
extern unsigned char User_CpuCard(void);
extern  unsigned char Cpu_CollectCard(void);



extern void KeySum(const unsigned char *Data,const unsigned char *Key,unsigned char keylen,uint8_t type);
extern void KeyInit(void);
extern unsigned char CPUCard_ReadFileEF(unsigned char APP,unsigned char BinName,unsigned char datastart,unsigned char datalen,const unsigned char *Key,unsigned char *Rdata);
extern unsigned char CPUCard_WriteFileEF(unsigned char APP,unsigned char BinName,unsigned char datastart,unsigned char datalen,const unsigned char *Key,unsigned char *Wdata);
extern unsigned char CPUCard_UserCrebit(unsigned char APP,unsigned char *DEVNUM,unsigned char *Money,const unsigned char *Key,const unsigned char *TimeC,unsigned char *CRecv);
extern unsigned char CPUCard_UserDebit(unsigned char APP,unsigned char *DEVNUM,unsigned char *Money,unsigned char *DEVSID,const unsigned char *Key,const unsigned char *TimeC,unsigned char *CRecv);
extern unsigned char CPUCard_ReadFileAEF(unsigned char BinName,unsigned char datastart,unsigned char datalen,unsigned char *Rdata);
extern unsigned char CPUCard_WriteEFBin(unsigned char type,unsigned char BinName,unsigned char *data,unsigned short datastart,unsigned char datalen);

extern unsigned char CPUCard_GatherData(unsigned char gfiles,unsigned char *data,unsigned short datalen);
extern 	uint8_t GatherData_Test(void);
unsigned char read_card_money(void);
unsigned char CPUCard_CrebitChas(unsigned int Money,const unsigned char *Key,const unsigned char *Vpin,const unsigned char *TimeC);

unsigned char Read_CPU_Card_Value(void);
#endif
