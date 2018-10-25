#include "Iap.h"
#include "stdbool.h"
#include "Bsp_IntFlash.h"
#include "MyString.h"
#include "MD5.h"
#include "myMemory.h"
#include "UartProcess.h"

#define FIRM_INFO_BASE_ADDR 0x08004000
#define FIRM_VER_ADDR 0x08004000
#define SAVE_SITE_BASE_ADDR 0x08005000
#define PER_PACK_SIZE       (4 * 1024)

#define APPLICATION_ADDRESS (uint32_t)0x08008000 /* Start user code address: ADDR_FLASH_PAGE_8 */

/* Notable Flash addresses */
#define USER_FLASH_END_ADDRESS 0x08100000

/* Define the user application size */
#define USER_FLASH_SIZE ((uint32_t)0x00A0000) /* Small default template application */

typedef void (*pFunc)(void);
#pragma pack(1)
typedef struct
{
    uint8_t Flag[8];
    uint8_t Ver[16];
    uint8_t Date[12];
    uint32_t Size;
    uint32_t PackSum;
    uint8_t MD5[16];
} FirmInfo_t;
#pragma pack()

#pragma pack(1)
typedef struct
{
    uint8_t cmd;
    uint16_t PackNum;
    uint32_t Size;
    uint8_t *data;
} FirmPack_t;
#pragma pack()

enum
{
    UPDATING = 1,
    UPDATED = 2,
    NEED_UPDATE = 3
};

static FirmInfo_t FirmInfo;

void (*Send)(const void *Data, uint32_t Size);
void (*GetSize)(uint32_t *Size);
void (*Read)(void *rData);

void SetBootloaderHooks(void (*SendData)(const void *Data, uint32_t Size),
                        void (*GetDataSize)(uint32_t *Size),
                        void (*ReadData)(void *rData))
{
    Send = SendData;
    GetSize = GetDataSize;
    Read = ReadData;
}

void GetFirmInfo(FirmInfo_t * Info)
{
    IntFlashRead(FIRM_INFO_BASE_ADDR, (uint8_t *)Info, sizeof(FirmInfo_t));
}

void GetUpdateStatus(uint8_t * Status)
{
    uint8_t Buf[3] = {'\0'};
    IntFlashRead(FIRM_INFO_BASE_ADDR + 16 * 1024 - 3, Buf, 3);
    
    if (Buf[0] == 0 && Buf[1] == 0xff && Buf[1] == 0xff)
    {
        *Status = UPDATING;
    }
    else if (Buf[0] == 0 && Buf[1] == 0 && Buf[1] == 0xff)
    {
        *Status = UPDATED;
    }
    else if (Buf[0] == 0 && Buf[1] == 0 && Buf[1] == 0)
    {
        *Status = NEED_UPDATE;
    }
}

void SetUpdateStatus(uint8_t Status)
{
    uint8_t Buf = 0;
    switch (Status)
    {
        case 1: IntFlashWrite(FIRM_INFO_BASE_ADDR + 16 * 1024 - 3, (uint8_t *)&Buf, 1); break;
        case 2: IntFlashWrite(FIRM_INFO_BASE_ADDR + 16 * 1024 - 2, (uint8_t *)&Buf, 1); break;
        default : break;
    }
}

bool GetUpdateFlag(void)
{
    uint8_t Buf[3] = {'\0'};
    IntFlashRead(FIRM_INFO_BASE_ADDR + 16 * 1024 - 3, Buf, 3);

    if (Buf[0] == 0 && Buf[1] == 0 && Buf[2] == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void ClearFirmInfo(void)
{
    IntFlashErase(FIRM_INFO_BASE_ADDR, FIRM_INFO_BASE_ADDR + 16 * 1024);
}

void PackBuild(uint8_t cmd, uint16_t packNum, void *Buf, uint16_t Len)
{
    uint8_t Err[8] = {'\0'};
    FirmPack_t Pack, *packTmp;
    uint8_t *DataBuf = NULL;
    Pack.cmd = cmd;
    Pack.PackNum = packNum;
    Pack.data = Buf;
    Pack.Size = Len;

    DataBuf = MemMalloc(Pack.Size + 7);
    if (DataBuf == NULL)
    {
        packTmp = (FirmPack_t *)Err;
        packTmp->cmd = IAP_CMD_ERR;    
        packTmp->PackNum = 0;
        packTmp->Size = 1;
        Err[7] = IAP_ERR_MEM_IS_FULL;
        Send(Err, 8);
        /* err code */
        return ;
    }

    my_memcpy(DataBuf, &Pack, 7);
    if (Len > 0)
    {
        my_memcpy(DataBuf + 7, Pack.data, 7);
    }
    Send(DataBuf, Pack.Size + 7);

    MemFree(DataBuf);
}

void SaveSite(uint16_t PackNum, uint32_t Addr)
{
    uint8_t Buf = 0;
    IntFlashWrite(SAVE_SITE_BASE_ADDR + (PackNum - 1) * 5, (uint8_t *)&Buf, 1);
    IntFlashWrite(SAVE_SITE_BASE_ADDR + (PackNum - 1) * 5 + 1, (uint8_t *)&Addr, 4);
}

void RecoverySite(uint16_t *PackNum, uint32_t *wAddr)
{
    uint8_t Buf;
    uint32_t Addr, SectorAddr;
    for (int i = 0; i < FirmInfo.PackSum; i++)
    {
        IntFlashRead(SAVE_SITE_BASE_ADDR + i * 5, &Buf, 1);
        
        if (Buf != 0)
        {
            IntFlashRead(SAVE_SITE_BASE_ADDR + i * 5 + 1, (uint8_t *)&Addr, 4);
            SectorAddr = GetSectorStartAddr(Addr);
            *wAddr = SectorAddr;
#if defined STM32F1
#elif defined STM32F4
            IntFlashErase(SectorAddr, Addr);
            
            *PackNum = i - (Addr - SectorAddr) / PER_PACK_SIZE + 1;
#endif
        }
    }
    
    ClearFirmInfo();
    IntFlashWrite(FIRM_INFO_BASE_ADDR, (uint8_t *)&FirmInfo, sizeof(FirmInfo_t));
    for (int i = 0; i < *PackNum; i++)
    {
        SaveSite(i + 1, SAVE_SITE_BASE_ADDR + i * PER_PACK_SIZE);
    }
}

void UpdateFirm(uint8_t * JumpUserAppFlag)
{
    static uint8_t Status = 0;
    FirmPack_t Pack;
    static uint32_t Timeout, Size = 0, Offiset = 0;
    static uint16_t PackNum;
    void *p;
    
    /* 获取服务器上的固件信息 */
    if (Status == 0)
    {
        static uint8_t _Status = 0;
        if (_Status == 0)
        {
            PackBuild(IAP_CMD_GET_FIRM_INFO, 0, NULL, 0);
            _Status = 1;
            Timeout = 0;
        }
        else if (_Status == 1)
        {
            Timeout++;
            GetSize(&Size);
            if (Size != 0)
            {
                _Status = 2;
            }
            if (Timeout > 1000)
            {
                uint8_t Tmp = IAP_ERR_TIMEOUT;
                PackBuild(IAP_CMD_ERR, 0, &Tmp, 1);
                _Status = 0;
            }
        }
        else if (_Status == 2)
        {
            p = MemMalloc(Size);
            Read(p);

            Pack = *(FirmPack_t *)p;

            if (Pack.cmd == IAP_CMD_GET_FIRM_INFO)
            {
                FirmInfo_t recvFirmInfo;
                my_memcpy(((uint8_t *)&recvFirmInfo) + 8, (uint8_t *)p + 7, Pack.Size);
//                if (my_memcmp(FirmInfo.Ver, recvFirmInfo.Ver, 16) == 0)
//                {
//                    uint8_t UpdateStatus;
//                    /* 固件版本已存在，但不排除固件升级失败的可能 */
//                    GetUpdateStatus(&UpdateStatus);
//                    if (UpdateStatus == UPDATING)
//                    {
//                        RecoverySite(&PackNum, &Offiset);
//                        Offiset -= APPLICATION_ADDRESS;
//                        _Status = 0;
//                        Status = 1;
//                        Timeout = 0;
//                        MemFree(p);
//                        return;
//                    }
//                    else if (UpdateStatus == UPDATED)
//                    {
//                        uint8_t Tmp = IAP_ERR_FIRM_ALREADY;
//                        PackBuild(IAP_CMD_ERR, 0, &Tmp, 1);
//                    }
//                }
                
                ClearFirmInfo();
                
                my_memcpy(recvFirmInfo.Flag, "Start!", 6);
                
                IntFlashWrite(FIRM_INFO_BASE_ADDR, (uint8_t *)&recvFirmInfo, sizeof(FirmInfo_t));
                
                FirmInfo = recvFirmInfo;
                
                /* 设置标志，显示正在进行升级操作 */
                SetUpdateStatus(UPDATING);
                
                _Status = 0;
                Status = 1;
                Timeout = 0;
                PackNum = 1;
            }
            else
            {
                uint8_t Tmp = IAP_ERR_CMD_ERROR;
                PackBuild(IAP_CMD_ERR, 0, &Tmp, 1);
            }

            MemFree(p);
        }
    }
    /* 开始升级固件 */
    else if (Status == 1)
    {
        static uint8_t _Status = 0;
        if (_Status == 0)
        {
            PackBuild(IAP_CMD_GET_FIRM_DATA, PackNum, NULL, 0);
            _Status = 1;
            Timeout = 0;
        }
        else if (_Status == 1)
        {
            Timeout++;
            GetSize(&Size);
            if (Size != 0)
            {
                _Status = 2;
            }
            if (Timeout > 1000)
            {
                uint8_t Tmp = IAP_ERR_TIMEOUT;
                PackBuild(IAP_CMD_ERR, 0, &Tmp, 1);
                _Status = 0;
            }
        }
        else if (_Status == 2)
        {
            p = MemMalloc(Size);
            Read(p);

            Pack = *(FirmPack_t *)p;

            if (Pack.cmd == IAP_CMD_GET_FIRM_DATA)
            {
                IntFlashWrite(APPLICATION_ADDRESS + Offiset, (uint8_t *)p + 7, Pack.Size);
                
                /* 保护现场操作 */
                SaveSite(PackNum, APPLICATION_ADDRESS + Offiset);
                
                _Status = 0;
                Timeout = 0;
                PackNum++;
                Offiset += Pack.Size;
                
                if (Offiset > USER_FLASH_SIZE)
                {
                    uint8_t Tmp = IAP_ERR_FLASH_IS_FULL;
                    PackBuild(IAP_CMD_ERR, 0, &Tmp, 1);
                }
            }
            else
            {
                uint8_t Tmp = IAP_ERR_CMD_ERROR;
                PackBuild(IAP_CMD_ERR, 0, &Tmp, 1);
            }

            if (PackNum == (FirmInfo.PackSum + 1))
            {
                _Status = 0;
                Status = 2;
            }

            MemFree(p);
        }
        else if (Status == 2)
        {
            uint8_t DataMD5[16];
            /* 计算MD5码 */
            mbedtls_md5((uint8_t *)APPLICATION_ADDRESS, FirmInfo.Size, DataMD5);

            if (my_memcmp(FirmInfo.MD5, DataMD5, 16) != 0)
            {
                uint8_t Tmp = IAP_ERR_FIRM_ERROR;
                PackBuild(IAP_CMD_ERR, 0, &Tmp, 1);
                /* err code */
                return;
            }
            /* 设置标志，表示已经升级并校验完成 */
            SetUpdateStatus(UPDATED);
            *JumpUserAppFlag = true;
        }
    }
}

/*** Jump to application ******************************************************/
void Bootloader(void)
{
    static uint8_t FirstFlag = true, JumpUserAppFlag = false;
    uint8_t DataMD5[16];
    if (FirstFlag == true)
    {
        FirstFlag = false;
        
        GetFirmInfo(&FirmInfo);
        
        if (GetUpdateFlag() != true && my_memcmp(FirmInfo.Flag, "Start!", 5) == 0)
        {
            /* 计算MD5码 */
            mbedtls_md5((uint8_t *)APPLICATION_ADDRESS, FirmInfo.Size, DataMD5);

            if (my_memcmp(FirmInfo.MD5, DataMD5, 16) != 0)
            {
                uint8_t Tmp = IAP_ERR_FIRM_ERROR;
                PackBuild(IAP_CMD_ERR, 0, &Tmp, 1);
                /* err code */
                return;
            }
            JumpUserAppFlag = true;
        }
    }
    else
    {
        UpdateFirm(&JumpUserAppFlag);
    }

    if (JumpUserAppFlag == true)
    {
        /* 判断堆栈的有效性 */
//        if (((*(uint32_t *)APPLICATION_ADDRESS) & 0x2FFE0000) == 0x20000000)
//        {
//            /* 获取复位地址 */
//            uint32_t JumpAddress = *(__IO uint32_t *)(APPLICATION_ADDRESS + 4);

//            pFunc JumpToApplication = (pFunc)JumpAddress;

//            /* 设置中断向量表 */
//            SCB->VTOR = APPLICATION_ADDRESS;

//            /* 设置堆栈地址 */
//            __set_MSP(*(__IO uint32_t *)APPLICATION_ADDRESS);

//            /* 执行跳转 */
//            JumpToApplication();
//        }
    }
}
