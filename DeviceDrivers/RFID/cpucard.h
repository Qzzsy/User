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
//////////////////////////// CPU MF 使用
    unsigned char MFMasterKey[16];	//主控密钥
    unsigned char MFMaintainKey[16]; //维护密钥
//////////////////////////////CPU ADF 使用
    unsigned char MasterKey[16];   //应用主控密钥
    unsigned char MaintainKey[16]; //应用维护密钥
    unsigned char MifareKey[16]; //M1卡专用密钥
    unsigned char InsideKey[16]; //内部认证密钥
    unsigned char DebitKey[16];  //消费密钥
    unsigned char DebitKey1[16];  //消费密钥 2
    unsigned char CrebitKey[16]; //圈存密钥
    unsigned char CrebitKey1[16]; //圈存密钥 2
    unsigned char TACKey[16];    //TAC密钥
    unsigned char DPINKey[16]; //解锁PIN口令密钥
    unsigned char RPINKey[16]; //重装PIN口令密钥
    unsigned char PINKey[8]; //PIN口令密钥

//////////////////////////// SAM用  process
    /*
    		 unsigned char ProcessKey[16]; //生成过程密钥的密钥
    		 unsigned char UmaintainKey[16]; //用户卡维护密钥(MAMK)
    		 unsigned char MACKey[16]; //MAC密钥(MMAC)
    		 unsigned char MEKKey[16]; //加密密钥(MEK)
    		 unsigned char MMACKey[16]; //加密、MAC密钥(MMAC)
    */
} CardKey;

extern  CardKey CPUKey;

typedef struct
{
//0--38 共39字节
    uint8_t   setcard_pwd[6];//操作密码
    uint8_t   ParaVersion[2]; //参数版本号
    uint8_t   DeviceType;    //设备类型： 热水0X01  冷水0X02  开水0X03   灰记录0XEE
    uint8_t   ParaType;      //参数类型：标准无线0x01   无线阶梯收费 0x02
    uint8_t   sector;       //扇区号
    uint8_t   usermode;    //用户模式 11－计量 00－计时
    uint16_t   miniunit;   //最小单位
    uint8_t   rate[12];  //4种分的费率
    // uint8_t   cmrate[4];  // 4种厘的费率
    uint8_t   Ladder[6];  //阶梯
    uint8_t   MinTime[3]; //保底消费时间（0秒）保底消费时间， 以秒为单位
    uint8_t   MinMoney[3]; //底消费金额（1000为一元，单位毫厘）
    uint16_t   AutoCuttime; //自动断开时间（秒）没信号最大断开时间，以6秒为一个单位
    uint8_t  Max_Money[3];

} MeterParameter;//类型名


extern MeterParameter MeterPara;//声明结构体
//extern MeterParameter t_MeterPara;//声明结构体

typedef  struct
{
	unsigned char   CardCSN[4];          // 0- 3    卡芯片序列号
	unsigned char   OperatorNum[3];      // 4- 6    用户编号：发卡号(流水号) 3字节
	unsigned char	 CardType;           // 7       卡类  1 2 3 4 区分费率
	unsigned char   DeviceNum[4];         // 8- 11   终端设置机号
	unsigned int    Count;               // 12-13       卡使用次数
	unsigned char   DeviceMode;          // 14      设备模式
	unsigned long   WaterStamp;          // 15--18  设备流水戳
	unsigned int    CurrentConsume;      // 19- 20  本次消费的钱
	unsigned long   RestMoney;           // 21- 24  卡上剩余金额
	unsigned char   Time[6];   	        // 25- 30  消费时间 BCD码
	unsigned char   AccuCRC;             // 31      效验位
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
