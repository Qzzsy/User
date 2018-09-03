/**
 ******************************************************************************
 * @file      Bsp_W25QXX.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-06-27
 * @brief     W25QXX����������ʵ�ֶ�W25QXX�Ķ�д����
 * @note      4KbytesΪһ��Sector
 * @History
 * Date           Author    version    		Notes
 * 2018-06-27       ZSY     V1.0.0      first version.
 */
/* Includes ------------------------------------------------------------------*/
#include "Bsp_W25QXX.h"

/* ��ʹ��LL�⣬�붨��USER_EFFIΪ1 */
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

//ָ���
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

/* ����ʹ�õ��Ƿ�ΪW25Q256 */
//#define USING_W25Q256

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
} BspFlashAddr_t;

#ifdef USING_W25Q256
#define FLASH_CMD_ENTER_4_BYTE_MODE (0XB7) /* ����4�ֽڵ�ַģʽ */
#define FLASH_CMD_EXIT_4_BYTE_MODE (0XE9)  /* �˳�4�ֽڵ�ַģʽ */
#define FLASH_CMD_READ_32B_ADDR (0X13)     /* Read Data 32bit address */
static void BspW25QXX_Wnter4ByteMode(void);
#endif /* USING_W25Q256 */

uint16_t BspW25QXX_ReadID(void);    /* ��ȡFLASH ID */
uint8_t BspW25QXX_ReadSR(void);     /* ��ȡ״̬�Ĵ��� */
void BspW25QXX_WriteSR(uint8_t sr); /* д״̬�Ĵ��� */
void BspW25QXX_WriteEnable(void);   /* дʹ�� */
void BspW25QXX_WriteDisable(void);  /* д���� */
void BspW25QXX_WaitBusy(void);      /* �ȴ����� */

/*---------------------------------------------------------------------------------*/
/* Ĭ�Ͼ���25Q64 */
uint16_t BspW25QXX_TYPE = W25Q256;

uint8_t BspW25QXX_BUF[4096];

/**
 * @func    BspW25QXX_Delay
 * @brief   W25QXX��ʱ����
 * @param   us Ҫ��ʱ��us��	
 * @retval  ��
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
 * @brief   ��ʼ��SPI FLASH��IO��
 * @retval  ��
 */
void BspW25QXX_Init(void)
{
    LL_SPI_Enable(W25QXX_SPIX);
    
    /* ��ȡFLASH ID. */
    BspW25QXX_TYPE = BspW25QXX_ReadID(); 

#ifdef USING_W25Q256
    BspW25QXX_Wnter4ByteMode();
#endif
}

/**
 * @func    BspW25QXX_ReadWriteData
 * @brief   W25QXX��д����
 * @param   WriteData Ҫд�������
 * @retval  ��ȡ��������
 */
static uint8_t BspW25QXX_ReadWriteData(uint8_t WriteData)
{
    __IO uint8_t Retry = 0;
    /* ���ָ����SPI��־λ�������:���ͻ���ձ�־λ */
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
    /* ���ָ����SPI��־λ�������:���ͻ���ձ�־λ */
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
 * @brief   W25QXX����Ϊ4-bit��ַģʽ
 * @retval  ��
 */
static void BspW25QXX_Wnter4ByteMode(void)
{
    uint8_t SendBuffer[1];

    /* дʹ�� */
    BspW25QXX_WriteEnable();

    /* wait operation done. */
    BspW25QXX_WaitBusy();

    /* ����ģʽ */
    W25QXX_CS_WRITE_L;

    /* ���ͽ���4byte-addressģʽ���� */
    BspW25QXX_ReadWriteData(FLASH_CMD_ENTER_4_BYTE_MODE);
    W25QXX_CS_WRITE_H;

    /* wait operation done. */
    BspW25QXX_WaitBusy();
}
#endif

/**
 * @func    BspW25QXX_ReadSR
 * @brief   W25QXX��ȡ״̬�Ĵ���
 * @note    BIT7  6   5   4   3   2   1   0
 *          SPR   RV  TB BP2 BP1 BP0 WEL BUSY
 *          SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
 *          TB,BP2,BP1,BP0:FLASH����д��������
 *          WEL:дʹ������
 *          BUSY:æ���λ(1,æ;0,����)
 *          Ĭ��:0x00
 * @retval  �Ĵ�����״̬
 */
uint8_t BspW25QXX_ReadSR(void)
{
    uint8_t byte = 0;
    W25QXX_CS_WRITE_L;                      
    BspW25QXX_ReadWriteData(W25X_ReadStatusReg); /* ���Ͷ�ȡ״̬�Ĵ������� */
    byte = BspW25QXX_ReadWriteData(0xff);        /* ��ȡһ���ֽ� */
    W25QXX_CS_WRITE_H;                        
    return byte;
}

/**
 * @func    BspW25QXX_WriteSR
 * @brief   W25QXXд״̬�Ĵ���
 * @param   SR д���״̬
 * @note    ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
 * @retval  ��
 */
void BspW25QXX_WriteSR(uint8_t SR)
{
    W25QXX_CS_WRITE_L;                            
    BspW25QXX_ReadWriteData(W25X_WriteStatusReg); /* ����дȡ״̬�Ĵ������� */
    BspW25QXX_ReadWriteData(SR);                  /* д��һ���ֽ� */
    W25QXX_CS_WRITE_H;                            
}

/**
 * @func    BspW25QXX_WriteEnable
 * @brief   W25QXXдʹ��
 * @retval  ��
 */
void BspW25QXX_WriteEnable(void)
{
    W25QXX_CS_WRITE_L;                         
    BspW25QXX_ReadWriteData(W25X_WriteEnable); 
    W25QXX_CS_WRITE_H;                         
}

/**
 * @func    BspW25QXX_WriteDisable
 * @brief   W25QXXд��ֹ
 * @retval  ��
 */
void BspW25QXX_WriteDisable(void)
{
    W25QXX_CS_WRITE_L;                          
    BspW25QXX_ReadWriteData(W25X_WriteDisable); 
    W25QXX_CS_WRITE_H;                          
}

/**
 * @func    BspW25QXX_ReadID
 * @brief   W25QXX��ȡоƬID
 * @retval  оƬ��ID����
 */
uint16_t BspW25QXX_ReadID(void)
{
    __IO uint16_t W25QXX_ID = 0;

    W25QXX_CS_WRITE_L;
    BspW25QXX_ReadWriteData(0x90); /* ���Ͷ�ȡID���� */
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
 * @brief   W25QXX��ָ����ַ��ʼ��ȡָ�����ȵ�����
 * @param   pBuffer ���ݴ洢��	
 * @param   ReadAddr ��ʼ��ȡ�ĵ�ַ	
 * @param   NumByteToRead Ҫ��ȡ���ֽ���(���65535)	
 * @retval  ��
 */
void BspW25QXX_Read(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
    uint16_t i;

    BspFlashAddr_t FlashAddr;
    FlashAddr.AddrValue = ReadAddr;

    W25QXX_CS_WRITE_L;    

    /* ���Ͷ�ȡ���� */
    BspW25QXX_ReadWriteData(W25X_ReadData); 
#ifdef USING_W25Q256
    BspW25QXX_ReadWriteData(FlashAddr.Addr.HH); /* ����32bit��ַ */
    BspW25QXX_ReadWriteData(FlashAddr.Addr.H);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.L);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.LL);
#else
    BspW25QXX_ReadWriteData(FlashAddr.Addr.H);  /* ����24bit��ַ */
    BspW25QXX_ReadWriteData(FlashAddr.Addr.L);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.LL);
#endif
    for (i = 0; i < NumByteToRead; i++)
    {
        pBuffer[i] = BspW25QXX_ReadWriteData(0xff); /* ѭ������ */
    }
    W25QXX_CS_WRITE_H;
}

/**
 * @func    BspW25QXX_WritePage
 * @brief   ��ָ����ַ��ʼд�����256�ֽڵ�����
 * @param   pBuffer ���ݴ洢��	
 * @param   WriteAddr ��ʼд��ĵ�ַ	
 * @param   NumByteToWrite Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!	
 * @retval  ��
 */
void BspW25QXX_WritePage(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint16_t i;

    BspFlashAddr_t FlashAddr;
    FlashAddr.AddrValue = WriteAddr;

    /* SET WEL */
    BspW25QXX_WriteEnable(); 

    W25QXX_CS_WRITE_L;

    /* ����дҳ���� */
    BspW25QXX_ReadWriteData(W25X_PageProgram); 
#ifdef USING_W25Q256
    BspW25QXX_ReadWriteData(FlashAddr.Addr.HH); /* ����32bit��ַ */
    BspW25QXX_ReadWriteData(FlashAddr.Addr.H);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.L);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.LL);
#else
    BspW25QXX_ReadWriteData(FlashAddr.Addr.H);  /* ����24bit��ַ */
    BspW25QXX_ReadWriteData(FlashAddr.Addr.L);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.LL);
#endif

    for (i = 0; i < NumByteToWrite; i++)
    {
        BspW25QXX_ReadWriteData(pBuffer[i]); /* ѭ��д�� */
    }
    W25QXX_CS_WRITE_H;   

    /* �ȴ�д����� */
    BspW25QXX_WaitBusy(); 
}

/**
 * @func    BspW25QXX_WriteNoCheck
 * @brief   W25QXX�޼���д����
 * @param   pBuffer ���ݴ洢��	
 * @param   WriteAddr ��ʼд��ĵ�ַ	
 * @param   NumByteToWrite Ҫд����ֽ���(���65535)	
 * @note    ����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
 * @retval  ��
 */
void BspW25QXX_WriteNoCheck(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint16_t pageremain;
    pageremain = 256 - WriteAddr % 256; /* ��ҳʣ����ֽ��� */

    if (NumByteToWrite <= pageremain)
    {
        pageremain = NumByteToWrite;    /* ������256���ֽ� */
    }

    while (1)
    {
        BspW25QXX_WritePage(pBuffer, WriteAddr, pageremain);

        if (NumByteToWrite == pageremain)
        {
            break; /* д������� */
        }
        else /* NumByteToWrite > pageremain */
        {
            pBuffer += pageremain;
            WriteAddr += pageremain;
            NumByteToWrite -= pageremain;       /* ��ȥ�Ѿ�д���˵��ֽ��� */

            if (NumByteToWrite > 256)
            {
                pageremain = 256;               /* һ�ο���д��256���ֽ� */
            }
            else
            {
                pageremain = NumByteToWrite;    /* ����256���ֽ��� */
            }
        }
    }
}

/**
 * @func    BspW25QXX_Write
 * @brief   W25QXX��ָ����ַ��ʼд��ָ�����ȵ�����
 * @param   pBuffer ���ݴ洢��	
 * @param   WriteAddr ��ʼд��ĵ�ַ	
 * @param   NumByteToWrite Ҫд����ֽ���(���65535) 
 * @note    �ú�������������!	
 * @retval  ��
 */
void BspW25QXX_Write(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint32_t SecPos;
    uint16_t SecOff;
    uint16_t SecRemain;
    uint16_t i;

    SecPos = WriteAddr / 4096; /* ������ַ */
    SecOff = WriteAddr % 4096; /* �������ڵ�ƫ�� */
    SecRemain = 4096 - SecOff; /* ����ʣ��ռ��С */

    if (NumByteToWrite <= SecRemain)
        SecRemain = NumByteToWrite; /* ������4096���ֽ� */
    while (1)
    {
        /* ������������������ */
        BspW25QXX_Read(BspW25QXX_BUF, SecPos * 4096, 4096);

        /* У������ */
        for (i = 0; i < SecRemain; i++)
        {
            /* �ж���Ҫ��������Ϊ0xff����Ҫ���� */
            if (BspW25QXX_BUF[SecOff + i] != 0XFF)
                break;
        }

        /* ��Ҫ���� */
        if (i < SecRemain)
        {
            /* ����������� */
            BspW25QXX_EraseSector(SecPos);

            for (i = 0; i < SecRemain; i++)
            {
                BspW25QXX_BUF[i + SecOff] = pBuffer[i];
            }

            /* д���������� */
            BspW25QXX_WriteNoCheck(BspW25QXX_BUF, SecPos * 4096, 4096);
        }
        else
        {
            /* д�Ѿ������˵�,ֱ��д������ʣ������. */
            BspW25QXX_WriteNoCheck(pBuffer, WriteAddr, SecRemain);
        }

        if (NumByteToWrite == SecRemain)
            break;
        else /* д��δ���� */
        {
            SecPos++;   /* ������ַ��1 */
            SecOff = 0; /* ƫ��λ��Ϊ0 */

            pBuffer += SecRemain;        /* ָ��ƫ�� */
            WriteAddr += SecRemain;      /* д��ַƫ�� */
            NumByteToWrite -= SecRemain; /* �ֽ����ݼ� */

            if (NumByteToWrite > 4096)
                SecRemain = 4096; /* ��һ����������д���� */
            else
                SecRemain = NumByteToWrite; /* ��һ����������д���� */
        }
    };
}

/**
 * @func    BspW25QXX_EraseChip
 * @brief   W25QXX��������оƬ
 * @note    �ȴ�ʱ�䳬��
 * @retval  ��
 */
void BspW25QXX_EraseChip(void)
{
    /* SET WEL */
    BspW25QXX_WriteEnable();
    BspW25QXX_WaitBusy();
    W25QXX_CS_WRITE_L;
    BspW25QXX_ReadWriteData(W25X_ChipErase);
    W25QXX_CS_WRITE_H;

    /* �ȴ�оƬ�������� */
    BspW25QXX_WaitBusy();
}

/**
 * @func    BspW25QXX_EraseSector
 * @brief   W25QXX����һ������
 * @param   DesAddr ������ַ	
 * @note    ����һ��ɽ��������ʱ��:150ms
 * @retval  ��
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
    BspW25QXX_ReadWriteData(W25X_SectorErase);  /* ������������ָ�� */
    BspW25QXX_ReadWriteData(FlashAddr.Addr.HH); /* ����32bit��ַ */
    BspW25QXX_ReadWriteData(FlashAddr.Addr.H);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.L);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.LL);
#else
    BspW25QXX_ReadWriteData(W25X_SectorErase); /* ������������ָ�� */
    BspW25QXX_ReadWriteData(FlashAddr.Addr.H); /* ����24bit��ַ */
    BspW25QXX_ReadWriteData(FlashAddr.Addr.L);
    BspW25QXX_ReadWriteData(FlashAddr.Addr.LL);
#endif
    W25QXX_CS_WRITE_H;

    /* �ȴ�������� */
    BspW25QXX_WaitBusy();
}

/**
 * @func    BspW25QXX_WaitBusy
 * @brief   W25QXX�ȴ�����
 * @retval  ��
 */
void BspW25QXX_WaitBusy(void)
{
    /* �ȴ�BUSYλ��� */
    while ((BspW25QXX_ReadSR() & 0x01) == 0x01)
        ;
}

/**
 * @func    BspW25QXX_PowerDown
 * @brief   W25QXX�������ģʽ
 * @retval  ��
 */
void BspW25QXX_PowerDown(void)
{
    W25QXX_CS_WRITE_L;

    /* ���͵������� */
    BspW25QXX_ReadWriteData(W25X_PowerDown);
    W25QXX_CS_WRITE_H;

    /* �ȴ�TPD */
    BspW25QXX_Delay(3);
}

/**
 * @func    BspW25QXX_WAKEUP
 * @brief   ���� W25QXX
 * @retval  ��
 */
void BspW25QXX_WAKEUP(void)
{
    W25QXX_CS_WRITE_L;

    /* send W25X_PowerDown command 0xAB */
    BspW25QXX_ReadWriteData(W25X_ReleasePowerDown);
    W25QXX_CS_WRITE_H;

    /* �ȴ�TRES1 */
    BspW25QXX_Delay(3);
}
