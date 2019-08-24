/**
 ******************************************************************************
 * @file      Bsp_SFUB.c
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
#include "Bsp_sfub.h"
#include "bsp_spi_bus.h"
#include "spi.h"
#include "ustring.h"
#include "udef.h"

/* ����ʹ�õ��Ƿ�ΪW25Q256 */
//#define USING_W25Q256

#define SFUB_SPIX SPI1
#define SFUB_CS_PORT SPI_SFUB_CS_GPIO_Port
#define SFUB_CS_PIN SPI_SFUB_CS_Pin

#define SFUB_CS_RELEASE HAL_GPIO_WritePin(SFUB_CS_PORT, SFUB_CS_PIN, GPIO_PIN_SET)
#define SFUB_CS_TAKE HAL_GPIO_WritePin(SFUB_CS_PORT, SFUB_CS_PIN, GPIO_PIN_RESET)

#define SFUB_PAGE_SIZE          (256)
#define SFUB_SECTOR_SIZE        (4096)

/*!< ָ��� */
#define SFUB_CMD_WRSR           0x01        /* д״̬�Ĵ������� */
#define SFUB_CMD_PPG            0x02        /* ҳ���ָ�� */
#define SFUB_CMD_READ           0x03        /* ������������ */
#define SFUB_CMD_DISWR          0x04        /* ��ֹд, �˳�AAI״̬ */
#define SFUB_CMD_RDSR           0x05        /* ��״̬�Ĵ������� */
#define SFUB_CMD_WREN           0x06        /* дʹ������ */
#define SFUB_CMD_FRDATA         0x0B
#define SFUB_CMD_ERASE_4K       0x20        /* ����4K�������� */
#define SFUB_CMD_FRDUAL         0x3B
#define SFUB_CMD_EWRSR          0x50        /* ����д״̬�Ĵ��������� */
#define SFUB_CMD_ERASE_32K      0x52        /* ������������ */
#define SFUB_CMD_RDID           0x9F        /* ������ID���� */
#define SFUB_CMD_DUMMY_BYTE     0xA5        /* ���������Ϊ����ֵ�����ڶ����� */
#define SFUB_CMD_RPOWRDON       0xAB        /* ���ѵ�Դ */
#define SFUB_CMD_AAI            0xAD        /* AAI �������ָ��(FOR SST25VF016B) */
#define SFUB_CMD_POWRDON        0xB9        /* �رյ�Դ */
#define SFUB_CMD_ERASE_CHIP     0xC7        /* оƬ�������� */
#define SFUB_CMD_ERASE_64K      0xD8        /* 64K��������� */

#define SFUB_WIP_FLAG         0x01        /* ״̬�Ĵ����е����ڱ�̱�־��WIP) */

#ifdef USING_W25Q256
#define SF_CMD_ENTER_4_BYTE_MODE (0XB7) /* ����4�ֽڵ�ַģʽ */
#define SF_CMD_EXIT_4_BYTE_MODE (0XE9)  /* �˳�4�ֽڵ�ַģʽ */
#define SF_CMD_READ_32B_ADDR (0X13)     /* Read Data 32bit address */
static void sfub_enter_4byte_mode(void);
#endif /* USING_W25Q256 */

/* �����ַ�ṹ�� */
typedef union {
    uint32_t addr_value;
    struct
    {
        uint8_t ll;
        uint8_t l;
        uint8_t h;
        uint8_t hh;
    } addr;
} sfub_addr_t;

uint8_t sfub_read_info(void);
uint8_t sfub_read_SR(void);     /* ��ȡ״̬�Ĵ��� */
void sfub_write_SR(uint8_t sr); /* д״̬�Ĵ��� */
void sfub_write_enable(void);   /* дʹ�� */
void sfub_write_disable(void);  /* д���� */
void sfub_wait_busy(void);      /* �ȴ����� */

static spi_device_handle_t sfub_handle;
static uint8_t sfub_buf[4096];
sfub_info_t sfub_info;

void sfub_cs_take(void);
void sfub_cs_release(void);

spi_ops_t sfub_ops =
{
    .cs_take = sfub_cs_take,
    .cs_reslease = sfub_cs_release
};

void sfub_cs_take(void)
{
    SFUB_CS_TAKE;
}

void sfub_cs_release(void)
{
    SFUB_CS_RELEASE;
}

/**
 * @func    sfub_delay
 * @brief   FLASH��ʱ����
 * @param   us Ҫ��ʱ��us��	
 * @retval  ��
 */
void sfub_delay(__IO uint32_t us)
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
 * @func    sfub_init
 * @brief   ��ʼ��SPI FLASH��IO��
 * @retval  SF_OK ��ʼ���ɹ���SF_ERR ��ʼ��ʧ��
 */
uint8_t sfub_init(void)
{
    static spi_device_t sfub_spi_device;
    static spi_bus_t spi_bus;
    static spi_conf_t sfub_spi_Conf;

    spi_bus.id = 1;
    spi_bus.ops = &sfub_ops;

    sfub_spi_Conf.data_width = 8;
    sfub_spi_Conf.mode = SPI_MODE_1;
    sfub_spi_Conf.max_freq = 80 * 1000000;

    sfub_spi_device.bus = &spi_bus;
    sfub_spi_device.config = &sfub_spi_Conf;
    sfub_spi_device.device= &hspi1;

    sfub_handle = &sfub_spi_device;

    /* ��ȡFLASH ID. */
    if (sfub_read_info() != SFUB_OK)
    {
        return (uint8_t)SFUB_ERR;
    }

#ifdef USING_W25Q256
    sfub_enter_4byte_mode();
#endif
    return SFUB_OK;
}

#ifdef USING_W25Q256
/**
 * @func    sfub_enter_4byte_mode
 * @brief   FLASH����Ϊ4-bit��ַģʽ
 * @retval  ��
 */
static void sfub_enter_4byte_mode(void)
{
    uint8_t cmd = 0;

    /* дʹ�� */
    sfub_write_enable();

    /* wait operation done. */
    sfub_wait_busy();

    cmd = SFUB_CMD_ENTER_4_BYTE_MODE;
    /* ���ͽ���4byte-addressģʽ���� */
    spi_transfer(sfub_handle, &cmd, NULL, 1);

    /* wait operation done. */
    sfub_wait_busy();
}
#endif

/**
 * @func    sfub_read_SR
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
uint8_t sfub_read_SR(void)
{
    uint8_t byte = 0;
    byte = SFUB_CMD_RDSR;
    spi_send_then_recv(sfub_handle, &byte, 1, &byte, 1); /* ���Ͷ�ȡ״̬�Ĵ������� */
    return byte;
}

/**
 * @func    sfub_write_SR
 * @brief   FLASHд״̬�Ĵ���
 * @param   SR д���״̬
 * @note    ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
 * @retval  ��
 */
void sfub_write_SR(uint8_t SR)
{
    uint8_t cmd = 0;
    if (sfub_info.chip_id == SFUB_SST25VF016B_ID)
    {
        cmd = SFUB_CMD_EWRSR;
        spi_transfer(sfub_handle, &cmd, NULL, 1);
        cmd = SFUB_CMD_WRSR;
        spi_send_then_send(sfub_handle, &cmd, 1, &SR, 1); /* ����дȡ״̬�Ĵ������� */
    }
    else
    {
        cmd = SFUB_CMD_WRSR;
        spi_send_then_send(sfub_handle, &cmd, 1, &SR, 1); /* ����дȡ״̬�Ĵ������� */
    }
}

/**
 * @func    sfub_write_enable
 * @brief   FLASHдʹ��
 * @retval  ��
 */
void sfub_write_enable(void)
{
    uint8_t cmd = 0;
    cmd = SFUB_CMD_WREN;
    spi_transfer(sfub_handle, &cmd, NULL, 1);
}

/**
 * @func    sfub_write_disable
 * @brief   FLASHд��ֹ
 * @retval  ��
 */
void sfub_write_disable(void)
{
    uint8_t cmd = 0;
    cmd = SFUB_CMD_DISWR;
    spi_transfer(sfub_handle, &cmd, NULL, 1);
}

/**
 * @func    sfub_read_id
 * @brief   FLASH��ȡоƬID
 * @retval  оƬ��ID����
 */
uint32_t sfub_read_id(void)
{
    uint8_t Buf[3] = {'\0'}, cmd;

    cmd = SFUB_CMD_RDID;

    spi_send_then_recv(sfub_handle, &cmd, 1, Buf, 3);

    return (Buf[0] << 16) | (Buf[1] << 8) | Buf[2];
}

/**
 * @func    sfub_read_info
 * @brief   FLASH��ȡоƬ��Ϣ
 * @note    
 * @retval  SF_OK ��ȡ��оƬ�źż���Ϣ��SF_ERR ��ȡ������Ϣ��Ϣ���鿴�Ƿ�֧�֣�
 */
uint8_t sfub_read_info(void)
{
    sfub_info.chip_id = sfub_read_id();

    switch (sfub_info.chip_id)
    {
    case SFUB_W25Q80_BV:
        ustrncpy(sfub_info.chip_name, "W25Q80_BV", sizeof("W25Q80_BV"));
        sfub_info.total_size = 1 * 1024 * 1024;
        sfub_info.page_size = 256;
        sfub_info.sector_size = 4 * 1024;
        break;
    case SFUB_W25Q16_BV_CL_CV:
        ustrncpy(sfub_info.chip_name, "W25Q16_BV_CL_CV", sizeof("W25Q16_BV_CL_CV"));
        sfub_info.total_size = 2 * 1024 * 1024;
        sfub_info.page_size = 256;
        sfub_info.sector_size = 4 * 1024;
        break;
    case SFUB_W25Q16_DW:
        ustrncpy(sfub_info.chip_name, "W25Q16_DW", sizeof("W25Q16_DW"));
        sfub_info.total_size = 2 * 1024 * 1024;
        sfub_info.page_size = 256;
        sfub_info.sector_size = 4 * 1024;
        break;
    case SFUB_W25Q32_BV:
        ustrncpy(sfub_info.chip_name, "W25Q32_BV", sizeof("W25Q32_BV"));
        sfub_info.total_size = 4 * 1024 * 1024;
        sfub_info.page_size = 256;
        sfub_info.sector_size = 4 * 1024;
        break;
    case SFUB_W25Q32_DW:
        ustrncpy(sfub_info.chip_name, "W25Q32_DW", sizeof("W25Q32_DW"));
        sfub_info.total_size = 4 * 1024 * 1024;
        sfub_info.page_size = 256;
        sfub_info.sector_size = 4 * 1024;
        break;
    case SFUB_W25Q64_DW_BV_CV:
        ustrncpy(sfub_info.chip_name, "W25Q64_DW_BV_CV", sizeof("W25Q64_DW_BV_CV"));
        sfub_info.total_size = 8 * 1024 * 1024;
        sfub_info.page_size = 256;
        sfub_info.sector_size = 4 * 1024;
        break;
    case SFUB_W25Q128_BV:
        ustrncpy(sfub_info.chip_name, "W25Q128_BV", sizeof("W25Q128_BV"));
        sfub_info.total_size = 16 * 1024 * 1024;
        sfub_info.page_size = 256;
        sfub_info.sector_size = 4 * 1024;
        break;
    case SFUB_W25Q256_FV:
        ustrncpy(sfub_info.chip_name, "W25Q256_FV", sizeof("W25Q256_FV"));
        sfub_info.total_size = 32 * 1024 * 1024;
        sfub_info.page_size = 256;
        sfub_info.sector_size = 4 * 1024;
        break;
    case SFUB_SST25VF016B_ID:
        ustrncpy(sfub_info.chip_name, "SST25VF016B_ID", sizeof("SST25VF016B_ID"));
        sfub_info.total_size = 2 * 1024 * 1024;
        sfub_info.page_size = 256;
        sfub_info.sector_size = 4 * 1024;
        break;
    case SFUB_MX25L1606E_ID:
        ustrncpy(sfub_info.chip_name, "MX25L1606E_ID", sizeof("MX25L1606E_ID"));
        sfub_info.total_size = 2 * 1024 * 1024;
        sfub_info.page_size = 256;
        sfub_info.sector_size = 4 * 1024;
        break;
    default:
        uprintf("unknow sfub chip ID.\r\n");
        return (uint8_t)SFUB_ERR;
    }
    uprintf("JEDEC device ID: 0x%04x, chip name: %s.\r\n", sfub_info.chip_id, sfub_info.chip_name);
    return SFUB_OK;
}

/**
 * @func    sfub_read
 * @brief   FLASH��ָ����ַ��ʼ��ȡָ�����ȵ�����
 * @param   pBuffer ���ݴ洢��	
 * @param   ReadAddr ��ʼ��ȡ�ĵ�ַ	
 * @param   _rSize Ҫ��ȡ���ֽ���(���65535)	
 * @retval  SF_OK ��ȡ�ɹ���SF_ERR ��ȡʧ��
 */
uint8_t sfub_read(uint8_t *p_buf, uint32_t read_addr, uint16_t r_size)
{
#ifdef USING_W25Q256
    uint8_t tmp[5];
#else
    uint8_t tmp[4];
#endif
    sfub_addr_t sfub_addr;
    sfub_addr.addr_value = read_addr;

    tmp[0] = SFUB_CMD_READ;
#ifdef USING_W25Q256
    tmp[1] = sfub_addr.addr.hh;
    tmp[2] = sfub_addr.addr.h;
    tmp[3] = sfub_addr.addr.l;
    tmp[4] = sfub_addr.addr.ll;
    return spi_send_then_recv(sfub_handle, &tmp, 5, p_buf, r_size);
#else
    tmp[1] = sfub_addr.addr.h;
    tmp[2] = sfub_addr.addr.l;
    tmp[3] = sfub_addr.addr.ll;
    return spi_send_then_recv(sfub_handle, &tmp, 4, p_buf, r_size);
#endif
}

/**
 * @func    sfub_write_page
 * @brief   FLASH��ָ����ַ��ʼд�����256�ֽڵ�����
 * @param   pBuffer ���ݴ洢��	
 * @param   WriteAddr ��ʼд��ĵ�ַ	
 * @param   _wSize Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!	
 * @retval  ��
 */
void sfub_write_page(uint8_t *p_buf, uint32_t write_addr, uint16_t w_size)
{
    uint16_t i;

#ifdef USING_W25Q256
    uint8_t tmp[5];
#else
    uint8_t tmp[4];
#endif
    sfub_addr_t sfub_addr;
    sfub_addr.addr_value = write_addr;

    /* SET WEL */
    sfub_write_enable();
    tmp[0] = SFUB_CMD_PPG;
    if (sfub_info.chip_id == SFUB_SST25VF016B_ID)
    {
        if (w_size < 2 || w_size % 2)
        {
            return;
        }

        tmp[1] = sfub_addr.addr.h;
        tmp[2] = sfub_addr.addr.l;
        tmp[3] = sfub_addr.addr.ll;
        spi_send_then_send(sfub_handle, &tmp, 4, p_buf, 2);

        for (i = 2; i < w_size; i += 2)
        {
            spi_send_then_send(sfub_handle, &tmp, 1, p_buf + i, 2);

            /* �ȴ�д����� */
            sfub_wait_busy();
        }
    }
    else
    {
#ifdef USING_W25Q256
        tmp[1] = sfub_addr.addr.hh;
        tmp[2] = sfub_addr.addr.h;
        tmp[3] = sfub_addr.addr.l;
        tmp[4] = sfub_addr.addr.ll;
        spi_send_then_send(sfub_handle, &tmp, 5, p_buf, w_size);
#else
        tmp[1] = sfub_addr.addr.h;
        tmp[2] = sfub_addr.addr.l;
        tmp[3] = sfub_addr.addr.ll;
        spi_send_then_send(sfub_handle, &tmp, 4, p_buf, w_size);
#endif
        /* �ȴ�д����� */
        sfub_wait_busy();
    }
}

/**
 * @func    sfub_write_no_check
 * @brief   FLASH�޼���д����
 * @param   pBuffer ���ݴ洢��	
 * @param   WriteAddr ��ʼд��ĵ�ַ	
 * @param   _wSize Ҫд����ֽ���(���65535)	
 * @note    ����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
 * @retval  SF_OK д��ɹ���SF_ERR д��ʧ��
 */
uint8_t sfub_write_no_check(uint8_t *p_buf, uint32_t write_addr, uint16_t w_size)
{
    uint16_t page_remain;

    /* д�볤��Ϊ0������������ */
    if (w_size == 0)
    {
        return (uint8_t)SFUB_ERR;
    }

    /* ������ַ����оƬ��ַ��Χ�����ش��� */
    if (write_addr > sfub_info.total_size)
    {
        return (uint8_t)SFUB_ERR;
    }

    /* ��ҳʣ����ֽ��� */
    page_remain = 256 - (write_addr & 0xff);

    if (w_size <= page_remain)
    {
        page_remain = w_size; /* ������256���ֽ� */
    }

    while (1)
    {
        sfub_write_page(p_buf, write_addr, page_remain);

        if (w_size == page_remain)
        {
            return SFUB_OK; /* д������� */
        }
        else /* NumByteToWrite > pageremain */
        {
            p_buf += page_remain;
            write_addr += page_remain;
            w_size -= page_remain; /* ��ȥ�Ѿ�д���˵��ֽ��� */

            if (w_size > 256)
            {
                page_remain = 256; /* һ�ο���д��256���ֽ� */
            }
            else
            {
                page_remain = w_size; /* ����256���ֽ��� */
            }
        }
    }

    return SFUB_OK; /* д������� */
}

/**
 * @func    sfub_write
 * @brief   FLASH��ָ����ַ��ʼд��ָ�����ȵ�����
 * @param   pBuffer ���ݴ洢��	
 * @param   WriteAddr ��ʼд��ĵ�ַ	
 * @param   _wSize Ҫд����ֽ���(���65535) 
 * @note    �ú�������������!	
 * @retval  SF_OK д��ɹ���SF_ERR д��ʧ��
 */
uint8_t sfub_write(uint8_t *p_buf, uint32_t write_addr, uint16_t w_size)
{
    uint32_t sector_pos;
    uint16_t sector_off;
    uint16_t sector_remain;
    uint16_t i;

    /* д�볤��Ϊ0������������ */
    if (w_size == 0)
    {
        return (uint8_t)SFUB_ERR;
    }

    /* ������ַ����оƬ��ַ��Χ�����ش��� */
    if (write_addr > sfub_info.total_size)
    {
        return (uint8_t)SFUB_ERR;
    }

    sector_pos = (write_addr >> 12); /* ������ַ */
    sector_off = write_addr & 0x0fff; /* �������ڵ�ƫ�� */
    sector_remain = 4096 - sector_off; /* ����ʣ��ռ��С */

    if (w_size <= sector_remain)
    {
        sector_remain = w_size; /* ������4096���ֽ� */
    }

    while (1)
    {
        /* ������������������ */
        sfub_read(sfub_buf, (sector_pos << 12), sector_remain);

        /* У������ */
        for (i = 0; i < sector_remain; i++)
        {
            /* �ж���Ҫ��������Ϊ0xff����Ҫ���� */
            if ((~sfub_buf[i + sector_off]) & p_buf[i])
            {
                break;
            }
        }

        /* ��Ҫ���� */
        if (i < sector_remain)
        {
            /* ����������� */
            sfub_erase_sector(sector_pos << 12);

            for (i = 0; i < sector_remain; i++)
            {
                sfub_buf[i + sector_off] = p_buf[i];
            }

            /* д���������� */
            sfub_write_no_check(sfub_buf + sector_off, (sector_pos << 12) + sector_off, sector_remain);
        }
        else
        {
            /* д�Ѿ������˵�,ֱ��д������ʣ������. */
            sfub_write_no_check(p_buf + sector_off, (sector_pos << 12) + sector_off, sector_remain);
        }

        if (w_size == sector_remain)
        {
            return SFUB_OK;
        }
        else /* д��δ���� */
        {
            sector_pos++;   /* ������ַ��1 */
            sector_off = 0; /* ƫ��λ��Ϊ0 */

            p_buf += sector_remain;   /* ָ��ƫ�� */
            write_addr += sector_remain; /* д��ַƫ�� */
            w_size -= sector_remain;    /* �ֽ����ݼ� */

            if (w_size > 4096)
                sector_remain = 4096; /* ��һ����������д���� */
            else
                sector_remain = w_size; /* ��һ����������д���� */
        }
    }
    return SFUB_OK; /* д������� */
}

/**
 * @func    sfub_erase_chip
 * @brief   FLASH��������оƬ
 * @note    �ȴ�ʱ�䳬��
 * @retval  ��
 */
void sfub_erase_chip(void)
{
    uint8_t cmd;
    /* SET WEL */
    sfub_write_enable();
    sfub_wait_busy();

    cmd = SFUB_CMD_ERASE_CHIP;

    spi_transfer(sfub_handle, &cmd, NULL, 1);

    /* �ȴ�оƬ�������� */
    sfub_wait_busy();
}

/**
 * @func    sfub_erase_sector
 * @brief   FLASH����һ������
 * @param   DesAddr ������ַ	
 * @note    ����һ��ɽ��������ʱ��:150ms
 * @retval  ��
 */
void sfub_erase_sector(uint32_t des_addr)
{
    uint16_t sector;
#ifdef USING_W25Q256
    uint8_t Tmp[5];
#else
    uint8_t Tmp[4];
#endif
    sfub_addr_t sfub_addr;

    sector = des_addr >> 12;
    des_addr = sector << 12;

    sfub_addr.addr_value = des_addr;

    /* SET WEL */
    sfub_write_enable();
    sfub_wait_busy();

    Tmp[0] = SFUB_CMD_ERASE_4K;
#ifdef USING_W25Q256
    Tmp[1] = sfub_addr.addr.hh;
    Tmp[2] = sfub_addr.addr.h;
    Tmp[3] = sfub_addr.addr.l;
    Tmp[4] = sfub_addr.addr.ll;
    spi_transfer(sfub_handle, Tmp, NULL, 5);
#else
    Tmp[1] = sfub_addr.addr.h;
    Tmp[2] = sfub_addr.addr.l;
    Tmp[3] = sfub_addr.addr.ll;
    spi_transfer(sfub_handle, Tmp, NULL, 4);
#endif

    /* �ȴ�������� */
    sfub_wait_busy();
}

/**
 * @func    sfub_wait_busy
 * @brief   FLASH�ȴ�����
 * @retval  ��
 */
void sfub_wait_busy(void)
{
    /* �ȴ�BUSYλ��� */
    while ((sfub_read_SR() & 0x01) == 0x01)
        ;
}

/**
 * @func    sfub_power_down
 * @brief   FLASH�������ģʽ
 * @retval  ��
 */
void sfub_power_down(void)
{
    uint8_t cmd;

    cmd = SFUB_CMD_POWRDON;

    spi_transfer(sfub_handle, &cmd, NULL, 1);

    /* �ȴ�TPD */
    sfub_delay(3);
}

/**
 * @func    sfub_wakeup
 * @brief   ���� FLASH
 * @retval  ��
 */
void sfub_wakeup(void)
{
    uint8_t cmd;

    cmd = SFUB_CMD_RPOWRDON;

    spi_transfer(sfub_handle, &cmd, NULL, 1);

    /* �ȴ�TRES1 */
    sfub_delay(3);
}
