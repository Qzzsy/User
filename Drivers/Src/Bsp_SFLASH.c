/**
 ******************************************************************************
 * @file      Bsp_SFLASH.c
 * @author    ZSY
 * @version   V1.0.1
 * @date      2018-10-08
 * @brief     Flash����������ʵ�ֶ�SPI Flash�Ķ�д����
 * @note      4KbytesΪһ��Sector
 * @History
 * Date           Author    version    		Notes
 * 2018-06-27       ZSY     V1.0.0      first version.
 * 2018-10-08       ZSY     V1.0.1      �����������.
 */
/* Includes ------------------------------------------------------------------*/
#include "Bsp_SFLASH.h"
#include "bsp_spi_bus.h"
#include "spi.h"
#include "string.h"

/* ��ʹ��LL�⣬�붨��USER_EFFIΪ1 */
#ifndef USE_EFFI
#define USE_EFFI 0
#endif

/* ����ʹ�õ��Ƿ�ΪW25Q256 */
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

/*!< ָ��� */
#define SF_CMD_WRSR 0x01  /* д״̬�Ĵ������� */
#define SF_CMD_PPG 0x02   /* ҳ���ָ�� */
#define SF_CMD_READ 0x03  /* ������������ */
#define SF_CMD_DISWR 0x04 /* ��ֹд, �˳�AAI״̬ */
#define SF_CMD_RDSR 0x05  /* ��״̬�Ĵ������� */
#define SF_CMD_WREN 0x06  /* дʹ������ */
#define SF_CMD_FRDATA 0x0B
#define SF_CMD_ERASE_4K 0x20 /* ����4K�������� */
#define SF_CMD_FRDUAL 0x3B
#define SF_CMD_EWRSR 0x50      /* ����д״̬�Ĵ��������� */
#define SF_CMD_ERASE_32K 0x52  /* ������������ */
#define SF_CMD_RDID 0x9F       /* ������ID���� */
#define SF_CMD_DUMMY_BYTE 0xA5 /* ���������Ϊ����ֵ�����ڶ����� */
#define SF_CMD_RPOWRDON 0xAB   /* ���ѵ�Դ */
#define SF_CMD_AAI 0xAD        /* AAI �������ָ��(FOR SST25VF016B) */
#define SF_CMD_POWRDON 0xB9    /* �رյ�Դ */
#define SF_CMD_ERASE_CHIP 0xC7 /* оƬ�������� */
#define SF_CMD_ERASE_64K 0xD8  /* 64K��������� */

#define SF_WIP_FLAG 0x01 /* ״̬�Ĵ����е����ڱ�̱�־��WIP) */

#ifdef USING_W25Q256
#define SF_CMD_ENTER_4_BYTE_MODE (0XB7) /* ����4�ֽڵ�ַģʽ */
#define SF_CMD_EXIT_4_BYTE_MODE (0XE9)  /* �˳�4�ֽڵ�ַģʽ */
#define SF_CMD_READ_32B_ADDR (0X13)     /* Read Data 32bit address */
static void sfWnter4ByteMode(void);
#endif /* USING_W25Q256 */

/* �����ַ�ṹ�� */
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
uint8_t sfReadSR(void);     /* ��ȡ״̬�Ĵ��� */
void sfWriteSR(uint8_t sr); /* д״̬�Ĵ��� */
void sfWriteEnable(void);   /* дʹ�� */
void sfWriteDisable(void);  /* д���� */
void sfWaitBusy(void);      /* �ȴ����� */

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
 * @brief   FLASH��ʱ����
 * @param   us Ҫ��ʱ��us��	
 * @retval  ��
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
 * @brief   ��ʼ��SPI FLASH��IO��
 * @retval  SF_OK ��ʼ���ɹ���SF_ERR ��ʼ��ʧ��
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

    /* ��ȡFLASH ID. */
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
 * @brief   FLASH����Ϊ4-bit��ַģʽ
 * @retval  ��
 */
static void sfWnter4ByteMode(void)
{
    uint8_t cmd = 0;

    /* дʹ�� */
    sfWriteEnable();

    /* wait operation done. */
    sfWaitBusy();

    cmd = SF_CMD_ENTER_4_BYTE_MODE;
    /* ���ͽ���4byte-addressģʽ���� */
    spiTransfer(sfHandle, &cmd, NULL, 1);

    /* wait operation done. */
    sfWaitBusy();
}
#endif

/**
 * @func    sfReadSR
 * @brief   FLASH��ȡ״̬�Ĵ���
 * @note    BIT7  6   5   4   3   2   1   0
 *          SPR   RV  TB BP2 BP1 BP0 WEL BUSY
 *          SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
 *          TB,BP2,BP1,BP0:FLASH����д��������
 *          WEL:дʹ������
 *          BUSY:æ���λ(1,æ;0,����)
 *          Ĭ��:0x00
 * @retval  �Ĵ�����״̬
 */
uint8_t sfReadSR(void)
{
    uint8_t byte = 0;
    byte = SF_CMD_RDSR;
    spiSendThenRecv(sfHandle, &byte, 1, &byte, 1); /* ���Ͷ�ȡ״̬�Ĵ������� */
    return byte;
}

/**
 * @func    sfWriteSR
 * @brief   FLASHд״̬�Ĵ���
 * @param   SR д���״̬
 * @note    ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
 * @retval  ��
 */
void sfWriteSR(uint8_t SR)
{
    uint8_t cmd = 0;
    if (_sfInfo.ChipID == SF_SST25VF016B_ID)
    {
        cmd = SF_CMD_EWRSR;
        spiTransfer(sfHandle, &cmd, NULL, 1);
        cmd = SF_CMD_WRSR;
        spiSendThenSend(sfHandle, &cmd, 1, &SR, 1); /* ����дȡ״̬�Ĵ������� */
    }
    else
    {
        cmd = SF_CMD_WRSR;
        spiSendThenSend(sfHandle, &cmd, 1, &SR, 1); /* ����дȡ״̬�Ĵ������� */
    }
}

/**
 * @func    sfWriteEnable
 * @brief   FLASHдʹ��
 * @retval  ��
 */
void sfWriteEnable(void)
{
    uint8_t cmd = 0;
    cmd = SF_CMD_WREN;
    spiTransfer(sfHandle, &cmd, NULL, 1);
}

/**
 * @func    sfWriteDisable
 * @brief   FLASHд��ֹ
 * @retval  ��
 */
void sfWriteDisable(void)
{
    uint8_t cmd = 0;
    cmd = SF_CMD_DISWR;
    spiTransfer(sfHandle, &cmd, NULL, 1);
}

/**
 * @func    sfReadID
 * @brief   FLASH��ȡоƬID
 * @retval  оƬ��ID����
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
 * @brief   FLASH��ȡоƬ��Ϣ
 * @note    
 * @retval  SF_OK ��ȡ��оƬ�źż���Ϣ��SF_ERR ��ȡ������Ϣ��Ϣ���鿴�Ƿ�֧�֣�
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
 * @brief   FLASH��ָ����ַ��ʼ��ȡָ�����ȵ�����
 * @param   pBuffer ���ݴ洢��	
 * @param   ReadAddr ��ʼ��ȡ�ĵ�ַ	
 * @param   _rSize Ҫ��ȡ���ֽ���(���65535)	
 * @retval  SF_OK ��ȡ�ɹ���SF_ERR ��ȡʧ��
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
 * @brief   FLASH��ָ����ַ��ʼд�����256�ֽڵ�����
 * @param   pBuffer ���ݴ洢��	
 * @param   WriteAddr ��ʼд��ĵ�ַ	
 * @param   _wSize Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!	
 * @retval  ��
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

            /* �ȴ�д����� */
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
        /* �ȴ�д����� */
        sfWaitBusy();
    }
}

/**
 * @func    sfWriteNoCheck
 * @brief   FLASH�޼���д����
 * @param   pBuffer ���ݴ洢��	
 * @param   WriteAddr ��ʼд��ĵ�ַ	
 * @param   _wSize Ҫд����ֽ���(���65535)	
 * @note    ����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
 * @retval  SF_OK д��ɹ���SF_ERR д��ʧ��
 */
uint8_t sfWriteNoCheck(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t _wSize)
{
    uint16_t PageRemain;

    /* д�볤��Ϊ0������������ */
    if (_wSize == 0)
    {
        return (uint8_t)SF_ERR;
    }

    /* ������ַ����оƬ��ַ��Χ�����ش��� */
    if (WriteAddr > _sfInfo.TotalSize)
    {
        return (uint8_t)SF_ERR;
    }

    /* ��ҳʣ����ֽ��� */
    PageRemain = 256 - WriteAddr % 256;

    if (_wSize <= PageRemain)
    {
        PageRemain = _wSize; /* ������256���ֽ� */
    }

    while (1)
    {
        sfWritePage(pBuffer, WriteAddr, PageRemain);

        if (_wSize == PageRemain)
        {
            return SF_OK; /* д������� */
        }
        else /* NumByteToWrite > pageremain */
        {
            pBuffer += PageRemain;
            WriteAddr += PageRemain;
            _wSize -= PageRemain; /* ��ȥ�Ѿ�д���˵��ֽ��� */

            if (_wSize > 256)
            {
                PageRemain = 256; /* һ�ο���д��256���ֽ� */
            }
            else
            {
                PageRemain = _wSize; /* ����256���ֽ��� */
            }
        }
    }
}

/**
 * @func    sfWrite
 * @brief   FLASH��ָ����ַ��ʼд��ָ�����ȵ�����
 * @param   pBuffer ���ݴ洢��	
 * @param   WriteAddr ��ʼд��ĵ�ַ	
 * @param   _wSize Ҫд����ֽ���(���65535) 
 * @note    �ú�������������!	
 * @retval  SF_OK д��ɹ���SF_ERR д��ʧ��
 */
uint8_t sfWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t _wSize)
{
    uint32_t SecPos;
    uint16_t SecOff;
    uint16_t SecRemain;
    uint16_t i;

    /* д�볤��Ϊ0������������ */
    if (_wSize == 0)
    {
        return (uint8_t)SF_ERR;
    }

    /* ������ַ����оƬ��ַ��Χ�����ش��� */
    if (WriteAddr > _sfInfo.TotalSize)
    {
        return (uint8_t)SF_ERR;
    }

    SecPos = WriteAddr / 4096; /* ������ַ */
    SecOff = WriteAddr % 4096; /* �������ڵ�ƫ�� */
    SecRemain = 4096 - SecOff; /* ����ʣ��ռ��С */

    if (_wSize <= SecRemain)
    {
        SecRemain = _wSize; /* ������4096���ֽ� */
    }

    while (1)
    {
        /* ������������������ */
        sfRead(sfBuf, SecPos * 4096, SecRemain);

        /* У������ */
        for (i = 0; i < SecRemain; i++)
        {
            /* �ж���Ҫ��������Ϊ0xff����Ҫ���� */
            if ((~sfBuf[i + SecOff]) & pBuffer[i])
            {
                break;
            }
        }

        /* ��Ҫ���� */
        if (i < SecRemain)
        {
            /* ����������� */
            sfEraseSector(SecPos * 4096);

            for (i = 0; i < SecRemain; i++)
            {
                sfBuf[i + SecOff] = pBuffer[i];
            }

            /* д���������� */
            sfWriteNoCheck(sfBuf + SecOff, SecPos * 4096 + SecOff, SecRemain);
        }
        else
        {
            /* д�Ѿ������˵�,ֱ��д������ʣ������. */
            sfWriteNoCheck(pBuffer + SecOff, SecPos * 4096 + SecOff, SecRemain);
        }

        if (_wSize == SecRemain)
        {
            return SF_OK;
        }
        else /* д��δ���� */
        {
            SecPos++;   /* ������ַ��1 */
            SecOff = 0; /* ƫ��λ��Ϊ0 */

            pBuffer += SecRemain;   /* ָ��ƫ�� */
            WriteAddr += SecRemain; /* д��ַƫ�� */
            _wSize -= SecRemain;    /* �ֽ����ݼ� */

            if (_wSize > 4096)
                SecRemain = 4096; /* ��һ����������д���� */
            else
                SecRemain = _wSize; /* ��һ����������д���� */
        }
    };
}

/**
 * @func    sfEraseChip
 * @brief   FLASH��������оƬ
 * @note    �ȴ�ʱ�䳬��
 * @retval  ��
 */
void sfEraseChip(void)
{
    uint8_t cmd;
    /* SET WEL */
    sfWriteEnable();
    sfWaitBusy();

    cmd = SF_CMD_ERASE_CHIP;

    spiTransfer(sfHandle, &cmd, NULL, 1);

    /* �ȴ�оƬ�������� */
    sfWaitBusy();
}

/**
 * @func    sfEraseSector
 * @brief   FLASH����һ������
 * @param   DesAddr ������ַ	
 * @note    ����һ��ɽ��������ʱ��:150ms
 * @retval  ��
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

    /* �ȴ�������� */
    sfWaitBusy();
}

/**
 * @func    sfWaitBusy
 * @brief   FLASH�ȴ�����
 * @retval  ��
 */
void sfWaitBusy(void)
{
    /* �ȴ�BUSYλ��� */
    while ((sfReadSR() & 0x01) == 0x01)
        ;
}

/**
 * @func    sfPowerDown
 * @brief   FLASH�������ģʽ
 * @retval  ��
 */
void sfPowerDown(void)
{
    uint8_t cmd;

    cmd = SF_CMD_POWRDON;

    spiTransfer(sfHandle, &cmd, NULL, 1);

    /* �ȴ�TPD */
    sfDelay(3);
}

/**
 * @func    sfWakeup
 * @brief   ���� FLASH
 * @retval  ��
 */
void sfWakeup(void)
{
    uint8_t cmd;

    cmd = SF_CMD_RPOWRDON;

    spiTransfer(sfHandle, &cmd, NULL, 1);

    /* �ȴ�TRES1 */
    sfDelay(3);
}
