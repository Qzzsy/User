/**
 ******************************************************************************
 * @file      Bsp_SFLASH.c
 * @author    ZSY
 * @version   V1.0.1
 * @date      2018-10-08
 * @brief     Flash的驱动程序，实现对SPI Flash的读写数据
 * @note      4Kbytes为一个Sector
 * @History
 * Date           Author    version    		Notes
 * 2018-06-27       ZSY     V1.0.0      first version.
 * 2018-10-08       ZSY     V1.0.1      完善驱动框架.
 */
/* Includes ------------------------------------------------------------------*/
#include "Bsp_SFLASH.h"
#include "bsp_spi_bus.h"
#include "spi.h"
#include "string.h"

/* 若使用LL库，请定义USER_EFFI为1 */
#ifndef USE_EFFI
#define USE_EFFI 0
#endif

/* 定义使用的是否为W25Q256 */
//#define USING_W25Q256

#define SF_SPIX SPI1
#define SF_CS_PORT FLASH_CS_GPIO_Port
#define SF_CS_PIN FLASH_CS_Pin

#if USE_EFFI == 1
#define SF_CS_RELEASE LL_GPIO_SetOutputPin(SF_CS_PORT, SF_CS_PIN)
#define SF_CS_TAKE LL_GPIO_ResetOutputPin(SF_CS_PORT, SF_CS_PIN)
#else
#define SF_CS_RELEASE HAL_GPIO_WritePin(SF_CS_PORT, SF_CS_PIN, GPIO_PIN_SET)
#define SF_CS_TAKE HAL_GPIO_WritePin(SF_CS_PORT, SF_CS_PIN, GPIO_PIN_RESET)
#endif

#define PAGE_SIZE 4096

/*!< 指令表 */
#define SF_CMD_WRSR 0x01  /* 写状态寄存器命令 */
#define SF_CMD_PPG 0x02   /* 页编程指令 */
#define SF_CMD_READ 0x03  /* 读数据区命令 */
#define SF_CMD_DISWR 0x04 /* 禁止写, 退出AAI状态 */
#define SF_CMD_RDSR 0x05  /* 读状态寄存器命令 */
#define SF_CMD_WREN 0x06  /* 写使能命令 */
#define SF_CMD_FRDATA 0x0B
#define SF_CMD_ERASE_4K 0x20 /* 擦除4K扇区命令 */
#define SF_CMD_FRDUAL 0x3B
#define SF_CMD_EWRSR 0x50      /* 允许写状态寄存器的命令 */
#define SF_CMD_ERASE_32K 0x52  /* 擦除扇区命令 */
#define SF_CMD_RDID 0x9F       /* 读器件ID命令 */
#define SF_CMD_DUMMY_BYTE 0xA5 /* 哑命令，可以为任意值，用于读操作 */
#define SF_CMD_RPOWRDON 0xAB   /* 唤醒电源 */
#define SF_CMD_AAI 0xAD        /* AAI 连续编程指令(FOR SST25VF016B) */
#define SF_CMD_POWRDON 0xB9    /* 关闭电源 */
#define SF_CMD_ERASE_CHIP 0xC7 /* 芯片擦除命令 */
#define SF_CMD_ERASE_64K 0xD8  /* 64K块擦除命令 */

#define SF_WIP_FLAG 0x01 /* 状态寄存器中的正在编程标志（WIP) */

#ifdef USING_W25Q256
#define SF_CMD_ENTER_4_BYTE_MODE (0XB7) /* 进入4字节地址模式 */
#define SF_CMD_EXIT_4_BYTE_MODE (0XE9)  /* 退出4字节地址模式 */
#define SF_CMD_READ_32B_ADDR (0X13)     /* Read Data 32bit address */
static void sfWnter4ByteMode(void);
#endif /* USING_W25Q256 */

/* 定义地址结构体 */
typedef union {
    uint32_t AddrValue;
    struct
    {
        uint8_t LL;
        uint8_t L;
        uint8_t H;
        uint8_t HH;
    } Addr;
} sfAddr_t;

uint8_t sfReadInfo(void);
uint8_t sfReadSR(void);     /* 读取状态寄存器 */
void sfWriteSR(uint8_t sr); /* 写状态寄存器 */
void sfWriteEnable(void);   /* 写使能 */
void sfWriteDisable(void);  /* 写保护 */
void sfWaitBusy(void);      /* 等待空闲 */

static spiDeviceHandle_t sfHandle;
static uint8_t sfBuf[4096];
sfInfo_t _sfInfo;

void sfCsTake(void);
void sfCsRelease(void);

spiOps_t Ops =
    {
        sfCsTake,
        sfCsRelease};

void sfCsTake(void)
{
    SF_CS_TAKE;
}

void sfCsRelease(void)
{
    SF_CS_RELEASE;
}
/**
 * @func    sfDelay
 * @brief   FLASH延时函数
 * @param   us 要延时的us数	
 * @retval  无
 */
void sfDelay(__IO uint32_t us)
{
    __IO uint32_t i = 0;

    while (us--)
    {
        i = 24;
        while (i--)
            ;
    }
}

/**
 * @func    sfInit
 * @brief   初始化SPI FLASH的IO口
 * @retval  SF_OK 初始化成功，SF_ERR 初始化失败
 */
uint8_t sfInit(void)
{
    static spiDevice_t sfDevice;
    static spiBus_t spiBus;
    static spiConfiguration_t sfConfiguration;

    spiBus.ID = 1;
    spiBus.Ops = &Ops;

    sfConfiguration.DataWidth = 8;
    sfConfiguration.Mode = SPI_MODE_1;
    sfConfiguration.MaxFreq = 80 * 1000000;

    sfDevice.Bus = &spiBus;
    sfDevice.Config = &sfConfiguration;
    sfDevice.Device = &hspi1;

    sfHandle = &sfDevice;

    /* 读取FLASH ID. */
    if (sfReadInfo() != SF_OK)
    {
        return (uint8_t)SF_ERR;
    }

#ifdef USING_W25Q256
    sfWnter4ByteMode();
#endif
    return SF_OK;
}

#ifdef USING_W25Q256
/**
 * @func    sfWnter4ByteMode
 * @brief   FLASH设置为4-bit地址模式
 * @retval  无
 */
static void sfWnter4ByteMode(void)
{
    uint8_t cmd = 0;

    /* 写使能 */
    sfWriteEnable();

    /* wait operation done. */
    sfWaitBusy();

    cmd = SF_CMD_ENTER_4_BYTE_MODE;
    /* 发送进入4byte-address模式命令 */
    spiTransfer(sfHandle, &cmd, NULL, 1);

    /* wait operation done. */
    sfWaitBusy();
}
#endif

/**
 * @func    sfReadSR
 * @brief   FLASH读取状态寄存器
 * @note    BIT7  6   5   4   3   2   1   0
 *          SPR   RV  TB BP2 BP1 BP0 WEL BUSY
 *          SPR:默认0,状态寄存器保护位,配合WP使用
 *          TB,BP2,BP1,BP0:FLASH区域写保护设置
 *          WEL:写使能锁定
 *          BUSY:忙标记位(1,忙;0,空闲)
 *          默认:0x00
 * @retval  寄存器的状态
 */
uint8_t sfReadSR(void)
{
    uint8_t byte = 0;
    byte = SF_CMD_RDSR;
    spiSendThenRecv(sfHandle, &byte, 1, &byte, 1); /* 发送读取状态寄存器命令 */
    return byte;
}

/**
 * @func    sfWriteSR
 * @brief   FLASH写状态寄存器
 * @param   SR 写入的状态
 * @note    只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
 * @retval  无
 */
void sfWriteSR(uint8_t SR)
{
    uint8_t cmd = 0;
    if (_sfInfo.ChipID == SF_SST25VF016B_ID)
    {
        cmd = SF_CMD_EWRSR;
        spiTransfer(sfHandle, &cmd, NULL, 1);
        cmd = SF_CMD_WRSR;
        spiSendThenSend(sfHandle, &cmd, 1, &SR, 1); /* 发送写取状态寄存器命令 */
    }
    else
    {
        cmd = SF_CMD_WRSR;
        spiSendThenSend(sfHandle, &cmd, 1, &SR, 1); /* 发送写取状态寄存器命令 */
    }
}

/**
 * @func    sfWriteEnable
 * @brief   FLASH写使能
 * @retval  无
 */
void sfWriteEnable(void)
{
    uint8_t cmd = 0;
    cmd = SF_CMD_WREN;
    spiTransfer(sfHandle, &cmd, NULL, 1);
}

/**
 * @func    sfWriteDisable
 * @brief   FLASH写禁止
 * @retval  无
 */
void sfWriteDisable(void)
{
    uint8_t cmd = 0;
    cmd = SF_CMD_DISWR;
    spiTransfer(sfHandle, &cmd, NULL, 1);
}

/**
 * @func    sfReadID
 * @brief   FLASH读取芯片ID
 * @retval  芯片的ID代号
 */
uint32_t sfReadID(void)
{
    uint8_t Buf[3] = {'\0'}, cmd;

    cmd = SF_CMD_RDID;

    spiSendThenRecv(sfHandle, &cmd, 1, Buf, 3);

    return (Buf[0] << 16) | (Buf[1] << 8) | Buf[2];
}

/**
 * @func    sfReadInfo
 * @brief   FLASH获取芯片信息
 * @note    
 * @retval  SF_OK 获取到芯片信号及信息，SF_ERR 获取不到信息信息（查看是否支持）
 */
uint8_t sfReadInfo(void)
{
    _sfInfo.ChipID = sfReadID();

    switch (_sfInfo.ChipID)
    {
    case SF_W25Q80_BV:
        strcpy(_sfInfo.ChipName, "W25Q80_BV");
        _sfInfo.TotalSize = 1 * 1024 * 1024;
        _sfInfo.PageSize = 4 * 1024;
        break;
    case SF_W25Q16_BV_CL_CV:
        strcpy(_sfInfo.ChipName, "W25Q16_BV_CL_CV");
        _sfInfo.TotalSize = 2 * 1024 * 1024;
        _sfInfo.PageSize = 4 * 1024;
        break;
    case SF_W25Q16_DW:
        strcpy(_sfInfo.ChipName, "W25Q16_DW");
        _sfInfo.TotalSize = 2 * 1024 * 1024;
        _sfInfo.PageSize = 4 * 1024;
        break;
    case SF_W25Q32_BV:
        strcpy(_sfInfo.ChipName, "W25Q32_BV");
        _sfInfo.TotalSize = 4 * 1024 * 1024;
        _sfInfo.PageSize = 4 * 1024;
        break;
    case SF_W25Q32_DW:
        strcpy(_sfInfo.ChipName, "W25Q32_DW");
        _sfInfo.TotalSize = 4 * 1024 * 1024;
        _sfInfo.PageSize = 4 * 1024;
        break;
    case SF_W25Q64_DW_BV_CV:
        strcpy(_sfInfo.ChipName, "W25Q64_DW_BV_CV");
        _sfInfo.TotalSize = 8 * 1024 * 1024;
        _sfInfo.PageSize = 4 * 1024;
        break;
    case SF_W25Q128_BV:
        strcpy(_sfInfo.ChipName, "W25Q128_BV");
        _sfInfo.TotalSize = 16 * 1024 * 1024;
        _sfInfo.PageSize = 4 * 1024;
        break;
    case SF_W25Q256_FV:
        strcpy(_sfInfo.ChipName, "W25Q256_FV");
        _sfInfo.TotalSize = 32 * 1024 * 1024;
        _sfInfo.PageSize = 4 * 1024;
        break;
    case SF_SST25VF016B_ID:
        strcpy(_sfInfo.ChipName, "SST25VF016B_ID");
        _sfInfo.TotalSize = 2 * 1024 * 1024;
        _sfInfo.PageSize = 4 * 1024;
        break;
    case SF_MX25L1606E_ID:
        strcpy(_sfInfo.ChipName, "MX25L1606E_ID");
        _sfInfo.TotalSize = 2 * 1024 * 1024;
        _sfInfo.PageSize = 4 * 1024;
        break;
    default:
        return (uint8_t)SF_ERR;
    }
    return SF_OK;
}

/**
 * @func    sfRead
 * @brief   FLASH在指定地址开始读取指定长度的数据
 * @param   pBuffer 数据存储区	
 * @param   ReadAddr 开始读取的地址	
 * @param   _rSize 要读取的字节数(最大65535)	
 * @retval  SF_OK 读取成功，SF_ERR 读取失败
 */
uint8_t sfRead(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t _rSize)
{
#ifdef USING_W25Q256
    uint8_t Tmp[5];
#else
    uint8_t Tmp[4];
#endif
    sfAddr_t sfAddr;
    sfAddr.AddrValue = ReadAddr;

    Tmp[0] = SF_CMD_READ;
#ifdef USING_W25Q256
    Tmp[1] = sfAddr.Addr.HH;
    Tmp[2] = sfAddr.Addr.H;
    Tmp[3] = sfAddr.Addr.L;
    Tmp[4] = sfAddr.Addr.LL;
    return spiSendThenRecv(FlashHandle, &Tmp, 5, pBuffer, _rSize);
#else
    Tmp[1] = sfAddr.Addr.H;
    Tmp[2] = sfAddr.Addr.L;
    Tmp[3] = sfAddr.Addr.LL;
    return spiSendThenRecv(sfHandle, &Tmp, 4, pBuffer, _rSize);
#endif
}

/**
 * @func    sfWritePage
 * @brief   FLASH在指定地址开始写入最大256字节的数据
 * @param   pBuffer 数据存储区	
 * @param   WriteAddr 开始写入的地址	
 * @param   _wSize 要写入的字节数(最大256),该数不应该超过该页的剩余字节数!	
 * @retval  无
 */
void sfWritePage(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t _wSize)
{
    uint16_t i;

#ifdef USING_W25Q256
    uint8_t Tmp[5];
#else
    uint8_t Tmp[4];
#endif
    sfAddr_t sfAddr;
    sfAddr.AddrValue = WriteAddr;

    /* SET WEL */
    sfWriteEnable();
    Tmp[0] = SF_CMD_PPG;
    if (_sfInfo.ChipID == SF_SST25VF016B_ID)
    {
        if (_wSize < 2 || _wSize % 2)
        {
            return;
        }

        Tmp[1] = sfAddr.Addr.H;
        Tmp[2] = sfAddr.Addr.L;
        Tmp[3] = sfAddr.Addr.LL;
        spiSendThenSend(sfHandle, &Tmp, 4, pBuffer, 2);

        for (i = 2; i < _wSize; i += 2)
        {
            spiSendThenSend(sfHandle, &Tmp, 1, pBuffer + i, 2);

            /* 等待写入结束 */
            sfWaitBusy();
        }
    }
    else
    {
#ifdef USING_W25Q256
        Tmp[1] = FlashAddr.Addr.HH;
        Tmp[2] = FlashAddr.Addr.H;
        Tmp[3] = FlashAddr.Addr.L;
        Tmp[4] = FlashAddr.Addr.LL;
        spiSendThenSend(FlashHandle, &Tmp, 5, pBuffer, _wSize);
#else
        Tmp[1] = sfAddr.Addr.H;
        Tmp[2] = sfAddr.Addr.L;
        Tmp[3] = sfAddr.Addr.LL;
        spiSendThenSend(sfHandle, &Tmp, 4, pBuffer, _wSize);
#endif
        /* 等待写入结束 */
        sfWaitBusy();
    }
}

/**
 * @func    sfWriteNoCheck
 * @brief   FLASH无检验写数据
 * @param   pBuffer 数据存储区	
 * @param   WriteAddr 开始写入的地址	
 * @param   _wSize 要写入的字节数(最大65535)	
 * @note    必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
 * @retval  SF_OK 写入成功，SF_ERR 写入失败
 */
uint8_t sfWriteNoCheck(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t _wSize)
{
    uint16_t PageRemain;

    /* 写入长度为0，不继续操作 */
    if (_wSize == 0)
    {
        return (uint8_t)SF_ERR;
    }

    /* 操作地址大于芯片地址范围，返回错误 */
    if (WriteAddr > _sfInfo.TotalSize)
    {
        return (uint8_t)SF_ERR;
    }

    /* 单页剩余的字节数 */
    PageRemain = 256 - WriteAddr % 256;

    if (_wSize <= PageRemain)
    {
        PageRemain = _wSize; /* 不大于256个字节 */
    }

    while (1)
    {
        sfWritePage(pBuffer, WriteAddr, PageRemain);

        if (_wSize == PageRemain)
        {
            return SF_OK; /* 写入结束了 */
        }
        else /* NumByteToWrite > pageremain */
        {
            pBuffer += PageRemain;
            WriteAddr += PageRemain;
            _wSize -= PageRemain; /* 减去已经写入了的字节数 */

            if (_wSize > 256)
            {
                PageRemain = 256; /* 一次可以写入256个字节 */
            }
            else
            {
                PageRemain = _wSize; /* 不够256个字节了 */
            }
        }
    }
}

/**
 * @func    sfWrite
 * @brief   FLASH在指定地址开始写入指定长度的数据
 * @param   pBuffer 数据存储区	
 * @param   WriteAddr 开始写入的地址	
 * @param   _wSize 要写入的字节数(最大65535) 
 * @note    该函数带擦除操作!	
 * @retval  SF_OK 写入成功，SF_ERR 写入失败
 */
uint8_t sfWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t _wSize)
{
    uint32_t SecPos;
    uint16_t SecOff;
    uint16_t SecRemain;
    uint16_t i;

    /* 写入长度为0，不继续操作 */
    if (_wSize == 0)
    {
        return (uint8_t)SF_ERR;
    }

    /* 操作地址大于芯片地址范围，返回错误 */
    if (WriteAddr > _sfInfo.TotalSize)
    {
        return (uint8_t)SF_ERR;
    }

    SecPos = WriteAddr / 4096; /* 扇区地址 */
    SecOff = WriteAddr % 4096; /* 在扇区内的偏移 */
    SecRemain = 4096 - SecOff; /* 扇区剩余空间大小 */

    if (_wSize <= SecRemain)
    {
        SecRemain = _wSize; /* 不大于4096个字节 */
    }

    while (1)
    {
        /* 读出整个扇区的内容 */
        sfRead(sfBuf, SecPos * 4096, SecRemain);

        /* 校验数据 */
        for (i = 0; i < SecRemain; i++)
        {
            /* 判断需要擦除，不为0xff就需要擦除 */
            if ((~sfBuf[i + SecOff]) & pBuffer[i])
            {
                break;
            }
        }

        /* 需要擦除 */
        if (i < SecRemain)
        {
            /* 擦除这个扇区 */
            sfEraseSector(SecPos * 4096);

            for (i = 0; i < SecRemain; i++)
            {
                sfBuf[i + SecOff] = pBuffer[i];
            }

            /* 写入整个扇区 */
            sfWriteNoCheck(sfBuf + SecOff, SecPos * 4096 + SecOff, SecRemain);
        }
        else
        {
            /* 写已经擦除了的,直接写入扇区剩余区间. */
            sfWriteNoCheck(pBuffer + SecOff, SecPos * 4096 + SecOff, SecRemain);
        }

        if (_wSize == SecRemain)
        {
            return SF_OK;
        }
        else /* 写入未结束 */
        {
            SecPos++;   /* 扇区地址增1 */
            SecOff = 0; /* 偏移位置为0 */

            pBuffer += SecRemain;   /* 指针偏移 */
            WriteAddr += SecRemain; /* 写地址偏移 */
            _wSize -= SecRemain;    /* 字节数递减 */

            if (_wSize > 4096)
                SecRemain = 4096; /* 下一个扇区还是写不完 */
            else
                SecRemain = _wSize; /* 下一个扇区可以写完了 */
        }
    };
}

/**
 * @func    sfEraseChip
 * @brief   FLASH擦除整个芯片
 * @note    等待时间超长
 * @retval  无
 */
void sfEraseChip(void)
{
    uint8_t cmd;
    /* SET WEL */
    sfWriteEnable();
    sfWaitBusy();

    cmd = SF_CMD_ERASE_CHIP;

    spiTransfer(sfHandle, &cmd, NULL, 1);

    /* 等待芯片擦除结束 */
    sfWaitBusy();
}

/**
 * @func    sfEraseSector
 * @brief   FLASH擦除一个扇区
 * @param   DesAddr 扇区地址	
 * @note    擦除一个山区的最少时间:150ms
 * @retval  无
 */
void sfEraseSector(uint32_t DesAddr)
{
    uint16_t block;
#ifdef USING_W25Q256
    uint8_t Tmp[5];
#else
    uint8_t Tmp[4];
#endif
    sfAddr_t sfAddr;

    block = DesAddr / 4096;
    DesAddr = block * 4096;

    sfAddr.AddrValue = DesAddr;

    /* SET WEL */
    sfWriteEnable();
    sfWaitBusy();

    Tmp[0] = SF_CMD_ERASE_4K;
#ifdef USING_W25Q256
    Tmp[1] = sfAddr.Addr.HH;
    Tmp[2] = sfAddr.Addr.H;
    Tmp[3] = sfAddr.Addr.L;
    Tmp[4] = sfAddr.Addr.LL;
    spiTransfer(sfHandle, Tmp, NULL, 5);
#else
    Tmp[1] = sfAddr.Addr.H;
    Tmp[2] = sfAddr.Addr.L;
    Tmp[3] = sfAddr.Addr.LL;
    spiTransfer(sfHandle, Tmp, NULL, 4);
#endif

    /* 等待擦除完成 */
    sfWaitBusy();
}

/**
 * @func    sfWaitBusy
 * @brief   FLASH等待空闲
 * @retval  无
 */
void sfWaitBusy(void)
{
    /* 等待BUSY位清空 */
    while ((sfReadSR() & 0x01) == 0x01)
        ;
}

/**
 * @func    sfPowerDown
 * @brief   FLASH进入掉电模式
 * @retval  无
 */
void sfPowerDown(void)
{
    uint8_t cmd;

    cmd = SF_CMD_POWRDON;

    spiTransfer(sfHandle, &cmd, NULL, 1);

    /* 等待TPD */
    sfDelay(3);
}

/**
 * @func    sfWakeup
 * @brief   唤醒 FLASH
 * @retval  无
 */
void sfWakeup(void)
{
    uint8_t cmd;

    cmd = SF_CMD_RPOWRDON;

    spiTransfer(sfHandle, &cmd, NULL, 1);

    /* 等待TRES1 */
    sfDelay(3);
}
