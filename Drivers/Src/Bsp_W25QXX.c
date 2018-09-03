/**
 ******************************************************************************
 * @file      Bsp_W25QXX.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-06-27
 * @brief     W25QXX的驱动程序，实现对W25QXX的读写数据
 * @note      4Kbytes为一个Sector
 * @History
 * Date           Author    version    		Notes
 * 2018-06-27       ZSY     V1.0.0      first version.
 */
/* Includes ------------------------------------------------------------------*/
#include "Bsp_W25QXX.h"

/* 若使用LL库，请定义USER_EFFI为1 */
#ifndef USER_EFFI
#define USER_EFFI 0
#endif

#define W25QXX_SPIX SPI1
#define W25QXX_CS_PORT Flash_CS_GPIO_Port
#define W25QXX_CS_PIN Flash_CS_Pin

#if USER_EFFI == 1
#define W25QXX_CS_WRITE_H LL_GPIO_SetOutputPin(W25QXX_CS_PORT, W25QXX_CS_PIN)
#define W25QXX_CS_WRITE_L LL_GPIO_ResetOutputPin(W25QXX_CS_PORT, W25QXX_CS_PIN)
#else
#define W25QXX_CS_WRITE_H HAL_GPIO_WritePin(W25QXX_CS_PORT, W25QXX_CS_PIN, GPIO_PIN_SET)
#define W25QXX_CS_WRITE_L HAL_GPIO_WritePin(W25QXX_CS_PORT, W25QXX_CS_PIN, GPIO_PIN_RESET)
#endif

//指令表
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData           0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID           0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 

/* 定义使用的是否为W25Q256 */
//#define USING_W25Q256

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
} BspFlashAddr_t;

#ifdef USING_W25Q256
#define FLASH_CMD_ENTER_4_BYTE_MODE (0XB7) /* 进入4字节地址模式 */
#define FLASH_CMD_EXIT_4_BYTE_MODE (0XE9)  /* 退出4字节地址模式 */
#define FLASH_CMD_READ_32B_ADDR (0X13)     /* Read Data 32bit address */
static void BspW25QXX_Wnter4ByteMode(void);
#endif /* USING_W25Q256 */

uint16_t BspW25QXX_ReadID(void);    /* 读取FLASH ID */
uint8_t BspW25QXX_ReadSR(void);     /* 读取状态寄存器 */
void BspW25QXX_WriteSR(uint8_t sr); /* 写状态寄存器 */
void BspW25QXX_WriteEnable(void);   /* 写使能 */
void BspW25QXX_WriteDisable(void);  /* 写保护 */
void BspW25QXX_WaitBusy(void);      /* 等待空闲 */

/*---------------------------------------------------------------------------------*/
/* 默认就是25Q64 */
uint16_t BspW25QXX_TYPE = W25Q256;

uint8_t BspW25QXX_BUF[4096];

/**
 * @func    BspW25QXX_Delay
 * @brief   W25QXX延时函数
 * @param   us 要延时的us数	
 * @retval  无
 */
void BspW25QXX_Delay(__IO uint32_t us)
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
 * @func    BspW25QXX_Init
 * @brief   初始化SPI FLASH的IO口
 * @retval  无
 */
void BspW25QXX_Init(void)
{
    LL_SPI_Enable(W25QXX_SPIX);
    
    /* 读取FLASH ID. */
    BspW25QXX_TYPE = BspW25QXX_ReadID(); 

#ifdef USING_W25Q256
    BspW25QXX_Wnter4ByteMode();
#endif
}

/**
 * @func    BspW25QXX_ReadWriteData
 * @brief   W25QXX读写数据
 * @param   WriteData 要写入的数据
 * @retval  读取到的数据
 */
static uint8_t BspW25QXX_ReadWriteData(uint8_t WriteData)
{
    __IO uint8_t Retry = 0;
    /* 检查指定的SPI标志位设置与否:发送缓存空标志位 */
    while (LL_SPI_IsActiveFlag_TXE(W25QXX_SPIX) == RESET)
    {
        Retry++;
        if (Retry > 200)
        {
            return 0;
        }
    }
    LL_SPI_TransmitData8(W25QXX_SPIX, WriteData);

    Retry = 0;
    /* 检查指定的SPI标志位设置与否:发送缓存空标志位 */
    while (LL_SPI_IsActiveFlag_RXNE(W25QXX_SPIX) == RESET)
    {
        Retry++;
        if (Retry > 200)
        {
            return 0;
        }
    }

    return LL_SPI_ReceiveData8(W25QXX_SPIX);
}

#ifdef USING_W25Q256
/**
 * @func    BspW25QXX_Wnter4ByteMode
 * @brief   W25QXX设置为4-bit地址模式
 * @retval  无
 */
static void BspW25QXX_Wnter4ByteMode(void)
{
    uint8_t SendBuffer[1];

    /* 写使能 */
    BspW25QXX_WriteEnable();

    /* wait operation done. */
    BspW25QXX_WaitBusy();

    /* 设置模式 */
    W25QXX_CS_WRITE_L;

    /* 发送进入4byte-address模式命令 */
    BspW25QXX_ReadWriteData(FLASH_CMD_ENTER_4_BYTE_MODE);
    W25QXX_CS_WRITE_H;

    /* wait operation done. */
    BspW25QXX_WaitBusy();
}
#endif

/**
 * @func    BspW25QXX_ReadSR
 * @brief   W25QXX读取状态寄存器
 * @note    BIT7  6   5   4   3   2   1   0
 *          SPR   RV  TB BP2 BP1 BP0 WEL BUSY
 *          SPR:默认0,状态寄存器保护位,配合WP使用
 *          TB,BP2,BP1,BP0:FLASH区域写保护设置
 *          WEL:写使能锁定
 *          BUSY:忙标记位(1,忙;0,空闲)
 *          默认:0x00
 * @retval  寄存器的状态
 */
uint8_t BspW25QXX_ReadSR(void)
{
    uint8_t byte = 0;
    W25QXX_CS_WRITE_L;                      
    BspW25QXX_ReadWriteData(W25X_ReadStatusReg); /* 发送读取状态寄存器命令 */
    byte = BspW25QXX_ReadWriteData(0xff);        /* 读取一个字节 */
    W25QXX_CS_WRITE_H;                        
    return byte;
}

/**
 * @func    BspW25QXX_WriteSR
 * @brief   W25QXX写状态寄存器
 * @param   SR 写入的状态
 * @note    只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
 * @retval  无
 */
void BspW25QXX_WriteSR(uint8_t SR)
{
    W25QXX_CS_WRITE_L;                            
    BspW25QXX_ReadWriteData(W25X_WriteStatusReg); /* 发送写取状态寄存器命令 */
    BspW25QXX_ReadWriteData(SR);                  /* 写入一个字节 */
    W25QXX_CS_WRITE_H;                            
}

/**
 * @func    BspW25QXX_WriteEnable
 * @brief   W25QXX写使能
 * @retval  无
 */
void BspW25QXX_WriteEnable(void)
{
    W25QXX_CS_WRITE_L;                         
    BspW25QXX_ReadWriteData(W25X_WriteEnable); 
    W25QXX_CS_WRITE_H;                         
}

/**
 * @func    BspW25QXX_WriteDisable
 * @brief   W25QXX写禁止
 * @retval  无
 */
void BspW25QXX_WriteDisable(void)
{
    W25QXX_CS_WRITE_L;                          
    BspW25QXX_ReadWriteData(W25X_WriteDisable); 
    W25QXX_CS_WRITE_H;                          
}

/**
 * @func    BspW25QXX_ReadID
 * @brief   W25QXX读取芯片ID
 * @retval  芯片的ID代号
 */
uint16_t BspW25QXX_ReadID(void)
{
    __IO uint16_t W25QXX_ID = 0;

    W25QXX_CS_WRITE_L;
    BspW25QXX_ReadWriteData(0x90); /* 发送读取ID命令 */
    BspW25QXX_ReadWriteData(0x00);
    BspW25QXX_ReadWriteData(0x00);
    BspW25QXX_ReadWriteData(0x00);

    W25QXX_ID = BspW25QXX_ReadWriteData(0xff) << 8;
    W25QXX_ID |= BspW25QXX_ReadWriteData(0xff);

    W25QXX_CS_WRITE_H;
    return W25QXX_ID;
}

/**
 * @func    BspW25QXX_Read
 * @brief   W25QXX在指定地址开始读取指定长度的数据
 * @param   pBuffer 数据存储区	
 * @param   ReadAddr 开始读取的地址	
 * @param   NumByteToRead 要读取的字节数(最大65535)	
 * @retval  无
 */
void BspW25QXX_Read(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
    uint16_t i;

    BspFlashAddr_t FlashAddr;
    FlashAddr.AddrValue = ReadAddr;

    W25QXX_CS_WRITE_L;    

    /* 发送读取命令 */
    BspW25QXX_ReadWriteData(W25X_ReadData); 
#ifdef USING_W25Q256
    BspW25QXX_ReadWriteData(FlashAddr.Addr.HH); /* 发送32bit地址 */
    BspW25QXX_ReadWriteData(FlashAddr.Addr.H);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.L);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.LL);
#else
    BspW25QXX_ReadWriteData(FlashAddr.Addr.H);  /* 发送24bit地址 */
    BspW25QXX_ReadWriteData(FlashAddr.Addr.L);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.LL);
#endif
    for (i = 0; i < NumByteToRead; i++)
    {
        pBuffer[i] = BspW25QXX_ReadWriteData(0xff); /* 循环读数 */
    }
    W25QXX_CS_WRITE_H;
}

/**
 * @func    BspW25QXX_WritePage
 * @brief   在指定地址开始写入最大256字节的数据
 * @param   pBuffer 数据存储区	
 * @param   WriteAddr 开始写入的地址	
 * @param   NumByteToWrite 要写入的字节数(最大256),该数不应该超过该页的剩余字节数!	
 * @retval  无
 */
void BspW25QXX_WritePage(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint16_t i;

    BspFlashAddr_t FlashAddr;
    FlashAddr.AddrValue = WriteAddr;

    /* SET WEL */
    BspW25QXX_WriteEnable(); 

    W25QXX_CS_WRITE_L;

    /* 发送写页命令 */
    BspW25QXX_ReadWriteData(W25X_PageProgram); 
#ifdef USING_W25Q256
    BspW25QXX_ReadWriteData(FlashAddr.Addr.HH); /* 发送32bit地址 */
    BspW25QXX_ReadWriteData(FlashAddr.Addr.H);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.L);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.LL);
#else
    BspW25QXX_ReadWriteData(FlashAddr.Addr.H);  /* 发送24bit地址 */
    BspW25QXX_ReadWriteData(FlashAddr.Addr.L);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.LL);
#endif

    for (i = 0; i < NumByteToWrite; i++)
    {
        BspW25QXX_ReadWriteData(pBuffer[i]); /* 循环写数 */
    }
    W25QXX_CS_WRITE_H;   

    /* 等待写入结束 */
    BspW25QXX_WaitBusy(); 
}

/**
 * @func    BspW25QXX_WriteNoCheck
 * @brief   W25QXX无检验写数据
 * @param   pBuffer 数据存储区	
 * @param   WriteAddr 开始写入的地址	
 * @param   NumByteToWrite 要写入的字节数(最大65535)	
 * @note    必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
 * @retval  无
 */
void BspW25QXX_WriteNoCheck(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint16_t pageremain;
    pageremain = 256 - WriteAddr % 256; /* 单页剩余的字节数 */

    if (NumByteToWrite <= pageremain)
    {
        pageremain = NumByteToWrite;    /* 不大于256个字节 */
    }

    while (1)
    {
        BspW25QXX_WritePage(pBuffer, WriteAddr, pageremain);

        if (NumByteToWrite == pageremain)
        {
            break; /* 写入结束了 */
        }
        else /* NumByteToWrite > pageremain */
        {
            pBuffer += pageremain;
            WriteAddr += pageremain;
            NumByteToWrite -= pageremain;       /* 减去已经写入了的字节数 */

            if (NumByteToWrite > 256)
            {
                pageremain = 256;               /* 一次可以写入256个字节 */
            }
            else
            {
                pageremain = NumByteToWrite;    /* 不够256个字节了 */
            }
        }
    }
}

/**
 * @func    BspW25QXX_Write
 * @brief   W25QXX在指定地址开始写入指定长度的数据
 * @param   pBuffer 数据存储区	
 * @param   WriteAddr 开始写入的地址	
 * @param   NumByteToWrite 要写入的字节数(最大65535) 
 * @note    该函数带擦除操作!	
 * @retval  无
 */
void BspW25QXX_Write(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint32_t SecPos;
    uint16_t SecOff;
    uint16_t SecRemain;
    uint16_t i;

    SecPos = WriteAddr / 4096; /* 扇区地址 */
    SecOff = WriteAddr % 4096; /* 在扇区内的偏移 */
    SecRemain = 4096 - SecOff; /* 扇区剩余空间大小 */

    if (NumByteToWrite <= SecRemain)
        SecRemain = NumByteToWrite; /* 不大于4096个字节 */
    while (1)
    {
        /* 读出整个扇区的内容 */
        BspW25QXX_Read(BspW25QXX_BUF, SecPos * 4096, 4096);

        /* 校验数据 */
        for (i = 0; i < SecRemain; i++)
        {
            /* 判断需要擦除，不为0xff就需要擦除 */
            if (BspW25QXX_BUF[SecOff + i] != 0XFF)
                break;
        }

        /* 需要擦除 */
        if (i < SecRemain)
        {
            /* 擦除这个扇区 */
            BspW25QXX_EraseSector(SecPos);

            for (i = 0; i < SecRemain; i++)
            {
                BspW25QXX_BUF[i + SecOff] = pBuffer[i];
            }

            /* 写入整个扇区 */
            BspW25QXX_WriteNoCheck(BspW25QXX_BUF, SecPos * 4096, 4096);
        }
        else
        {
            /* 写已经擦除了的,直接写入扇区剩余区间. */
            BspW25QXX_WriteNoCheck(pBuffer, WriteAddr, SecRemain);
        }

        if (NumByteToWrite == SecRemain)
            break;
        else /* 写入未结束 */
        {
            SecPos++;   /* 扇区地址增1 */
            SecOff = 0; /* 偏移位置为0 */

            pBuffer += SecRemain;        /* 指针偏移 */
            WriteAddr += SecRemain;      /* 写地址偏移 */
            NumByteToWrite -= SecRemain; /* 字节数递减 */

            if (NumByteToWrite > 4096)
                SecRemain = 4096; /* 下一个扇区还是写不完 */
            else
                SecRemain = NumByteToWrite; /* 下一个扇区可以写完了 */
        }
    };
}

/**
 * @func    BspW25QXX_EraseChip
 * @brief   W25QXX擦除整个芯片
 * @note    等待时间超长
 * @retval  无
 */
void BspW25QXX_EraseChip(void)
{
    /* SET WEL */
    BspW25QXX_WriteEnable();
    BspW25QXX_WaitBusy();
    W25QXX_CS_WRITE_L;
    BspW25QXX_ReadWriteData(W25X_ChipErase);
    W25QXX_CS_WRITE_H;

    /* 等待芯片擦除结束 */
    BspW25QXX_WaitBusy();
}

/**
 * @func    BspW25QXX_EraseSector
 * @brief   W25QXX擦除一个扇区
 * @param   DesAddr 扇区地址	
 * @note    擦除一个山区的最少时间:150ms
 * @retval  无
 */
void BspW25QXX_EraseSector(uint32_t DesAddr)
{
    uint16_t block;
    BspFlashAddr_t FlashAddr;

    block = DesAddr / 4096;
    DesAddr = block * 4096;

    FlashAddr.AddrValue = DesAddr;

    /* SET WEL */
    BspW25QXX_WriteEnable();
    BspW25QXX_WaitBusy();
    W25QXX_CS_WRITE_L;
#ifdef USING_W25Q256
    BspW25QXX_ReadWriteData(W25X_SectorErase);  /* 发送扇区擦除指令 */
    BspW25QXX_ReadWriteData(FlashAddr.Addr.HH); /* 发送32bit地址 */
    BspW25QXX_ReadWriteData(FlashAddr.Addr.H);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.L);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.LL);
#else
    BspW25QXX_ReadWriteData(W25X_SectorErase); /* 发送扇区擦除指令 */
    BspW25QXX_ReadWriteData(FlashAddr.Addr.H); /* 发送24bit地址 */
    BspW25QXX_ReadWriteData(FlashAddr.Addr.L);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.LL);
#endif
    W25QXX_CS_WRITE_H;

    /* 等待擦除完成 */
    BspW25QXX_WaitBusy();
}

/**
 * @func    BspW25QXX_WaitBusy
 * @brief   W25QXX等待空闲
 * @retval  无
 */
void BspW25QXX_WaitBusy(void)
{
    /* 等待BUSY位清空 */
    while ((BspW25QXX_ReadSR() & 0x01) == 0x01)
        ;
}

/**
 * @func    BspW25QXX_PowerDown
 * @brief   W25QXX进入掉电模式
 * @retval  无
 */
void BspW25QXX_PowerDown(void)
{
    W25QXX_CS_WRITE_L;

    /* 发送掉电命令 */
    BspW25QXX_ReadWriteData(W25X_PowerDown);
    W25QXX_CS_WRITE_H;

    /* 等待TPD */
    BspW25QXX_Delay(3);
}

/**
 * @func    BspW25QXX_WAKEUP
 * @brief   唤醒 W25QXX
 * @retval  无
 */
void BspW25QXX_WAKEUP(void)
{
    W25QXX_CS_WRITE_L;

    /* send W25X_PowerDown command 0xAB */
    BspW25QXX_ReadWriteData(W25X_ReleasePowerDown);
    W25QXX_CS_WRITE_H;

    /* 等待TRES1 */
    BspW25QXX_Delay(3);
}
