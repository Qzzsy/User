/**
 ******************************************************************************
 * @file      Bsp_SFUB.c
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
#include "Bsp_sfub.h"
#include "bsp_spi_bus.h"
#include "spi.h"
#include "ustring.h"
#include "udef.h"

/* 定义使用的是否为W25Q256 */
//#define USING_W25Q256

#define SFUB_SPIX SPI1
#define SFUB_CS_PORT SPI_SFUB_CS_GPIO_Port
#define SFUB_CS_PIN SPI_SFUB_CS_Pin

#define SFUB_CS_RELEASE HAL_GPIO_WritePin(SFUB_CS_PORT, SFUB_CS_PIN, GPIO_PIN_SET)
#define SFUB_CS_TAKE HAL_GPIO_WritePin(SFUB_CS_PORT, SFUB_CS_PIN, GPIO_PIN_RESET)

#define SFUB_PAGE_SIZE          (256)
#define SFUB_SECTOR_SIZE        (4096)

/*!< 指令表 */
#define SFUB_CMD_WRSR           0x01        /* 写状态寄存器命令 */
#define SFUB_CMD_PPG            0x02        /* 页编程指令 */
#define SFUB_CMD_READ           0x03        /* 读数据区命令 */
#define SFUB_CMD_DISWR          0x04        /* 禁止写, 退出AAI状态 */
#define SFUB_CMD_RDSR           0x05        /* 读状态寄存器命令 */
#define SFUB_CMD_WREN           0x06        /* 写使能命令 */
#define SFUB_CMD_FRDATA         0x0B
#define SFUB_CMD_ERASE_4K       0x20        /* 擦除4K扇区命令 */
#define SFUB_CMD_FRDUAL         0x3B
#define SFUB_CMD_EWRSR          0x50        /* 允许写状态寄存器的命令 */
#define SFUB_CMD_ERASE_32K      0x52        /* 擦除扇区命令 */
#define SFUB_CMD_RDID           0x9F        /* 读器件ID命令 */
#define SFUB_CMD_DUMMY_BYTE     0xA5        /* 哑命令，可以为任意值，用于读操作 */
#define SFUB_CMD_RPOWRDON       0xAB        /* 唤醒电源 */
#define SFUB_CMD_AAI            0xAD        /* AAI 连续编程指令(FOR SST25VF016B) */
#define SFUB_CMD_POWRDON        0xB9        /* 关闭电源 */
#define SFUB_CMD_ERASE_CHIP     0xC7        /* 芯片擦除命令 */
#define SFUB_CMD_ERASE_64K      0xD8        /* 64K块擦除命令 */

#define SFUB_WIP_FLAG         0x01        /* 状态寄存器中的正在编程标志（WIP) */

#ifdef USING_W25Q256
#define SF_CMD_ENTER_4_BYTE_MODE (0XB7) /* 进入4字节地址模式 */
#define SF_CMD_EXIT_4_BYTE_MODE (0XE9)  /* 退出4字节地址模式 */
#define SF_CMD_READ_32B_ADDR (0X13)     /* Read Data 32bit address */
static void sfub_enter_4byte_mode(void);
#endif /* USING_W25Q256 */

/* 定义地址结构体 */
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
uint8_t sfub_read_SR(void);     /* 读取状态寄存器 */
void sfub_write_SR(uint8_t sr); /* 写状态寄存器 */
void sfub_write_enable(void);   /* 写使能 */
void sfub_write_disable(void);  /* 写保护 */
void sfub_wait_busy(void);      /* 等待空闲 */

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
 * @brief   FLASH延时函数
 * @param   us 要延时的us数	
 * @retval  无
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
 * @brief   初始化SPI FLASH的IO口
 * @retval  SF_OK 初始化成功，SF_ERR 初始化失败
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

    /* 读取FLASH ID. */
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
 * @brief   FLASH设置为4-bit地址模式
 * @retval  无
 */
static void sfub_enter_4byte_mode(void)
{
    uint8_t cmd = 0;

    /* 写使能 */
    sfub_write_enable();

    /* wait operation done. */
    sfub_wait_busy();

    cmd = SFUB_CMD_ENTER_4_BYTE_MODE;
    /* 发送进入4byte-address模式命令 */
    spi_transfer(sfub_handle, &cmd, NULL, 1);

    /* wait operation done. */
    sfub_wait_busy();
}
#endif

/**
 * @func    sfub_read_SR
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
uint8_t sfub_read_SR(void)
{
    uint8_t byte = 0;
    byte = SFUB_CMD_RDSR;
    spi_send_then_recv(sfub_handle, &byte, 1, &byte, 1); /* 发送读取状态寄存器命令 */
    return byte;
}

/**
 * @func    sfub_write_SR
 * @brief   FLASH写状态寄存器
 * @param   SR 写入的状态
 * @note    只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
 * @retval  无
 */
void sfub_write_SR(uint8_t SR)
{
    uint8_t cmd = 0;
    if (sfub_info.chip_id == SFUB_SST25VF016B_ID)
    {
        cmd = SFUB_CMD_EWRSR;
        spi_transfer(sfub_handle, &cmd, NULL, 1);
        cmd = SFUB_CMD_WRSR;
        spi_send_then_send(sfub_handle, &cmd, 1, &SR, 1); /* 发送写取状态寄存器命令 */
    }
    else
    {
        cmd = SFUB_CMD_WRSR;
        spi_send_then_send(sfub_handle, &cmd, 1, &SR, 1); /* 发送写取状态寄存器命令 */
    }
}

/**
 * @func    sfub_write_enable
 * @brief   FLASH写使能
 * @retval  无
 */
void sfub_write_enable(void)
{
    uint8_t cmd = 0;
    cmd = SFUB_CMD_WREN;
    spi_transfer(sfub_handle, &cmd, NULL, 1);
}

/**
 * @func    sfub_write_disable
 * @brief   FLASH写禁止
 * @retval  无
 */
void sfub_write_disable(void)
{
    uint8_t cmd = 0;
    cmd = SFUB_CMD_DISWR;
    spi_transfer(sfub_handle, &cmd, NULL, 1);
}

/**
 * @func    sfub_read_id
 * @brief   FLASH读取芯片ID
 * @retval  芯片的ID代号
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
 * @brief   FLASH获取芯片信息
 * @note    
 * @retval  SF_OK 获取到芯片信号及信息，SF_ERR 获取不到信息信息（查看是否支持）
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
 * @brief   FLASH在指定地址开始读取指定长度的数据
 * @param   pBuffer 数据存储区	
 * @param   ReadAddr 开始读取的地址	
 * @param   _rSize 要读取的字节数(最大65535)	
 * @retval  SF_OK 读取成功，SF_ERR 读取失败
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
 * @brief   FLASH在指定地址开始写入最大256字节的数据
 * @param   pBuffer 数据存储区	
 * @param   WriteAddr 开始写入的地址	
 * @param   _wSize 要写入的字节数(最大256),该数不应该超过该页的剩余字节数!	
 * @retval  无
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

            /* 等待写入结束 */
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
        /* 等待写入结束 */
        sfub_wait_busy();
    }
}

/**
 * @func    sfub_write_no_check
 * @brief   FLASH无检验写数据
 * @param   pBuffer 数据存储区	
 * @param   WriteAddr 开始写入的地址	
 * @param   _wSize 要写入的字节数(最大65535)	
 * @note    必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
 * @retval  SF_OK 写入成功，SF_ERR 写入失败
 */
uint8_t sfub_write_no_check(uint8_t *p_buf, uint32_t write_addr, uint16_t w_size)
{
    uint16_t page_remain;

    /* 写入长度为0，不继续操作 */
    if (w_size == 0)
    {
        return (uint8_t)SFUB_ERR;
    }

    /* 操作地址大于芯片地址范围，返回错误 */
    if (write_addr > sfub_info.total_size)
    {
        return (uint8_t)SFUB_ERR;
    }

    /* 单页剩余的字节数 */
    page_remain = 256 - (write_addr & 0xff);

    if (w_size <= page_remain)
    {
        page_remain = w_size; /* 不大于256个字节 */
    }

    while (1)
    {
        sfub_write_page(p_buf, write_addr, page_remain);

        if (w_size == page_remain)
        {
            return SFUB_OK; /* 写入结束了 */
        }
        else /* NumByteToWrite > pageremain */
        {
            p_buf += page_remain;
            write_addr += page_remain;
            w_size -= page_remain; /* 减去已经写入了的字节数 */

            if (w_size > 256)
            {
                page_remain = 256; /* 一次可以写入256个字节 */
            }
            else
            {
                page_remain = w_size; /* 不够256个字节了 */
            }
        }
    }

    return SFUB_OK; /* 写入结束了 */
}

/**
 * @func    sfub_write
 * @brief   FLASH在指定地址开始写入指定长度的数据
 * @param   pBuffer 数据存储区	
 * @param   WriteAddr 开始写入的地址	
 * @param   _wSize 要写入的字节数(最大65535) 
 * @note    该函数带擦除操作!	
 * @retval  SF_OK 写入成功，SF_ERR 写入失败
 */
uint8_t sfub_write(uint8_t *p_buf, uint32_t write_addr, uint16_t w_size)
{
    uint32_t sector_pos;
    uint16_t sector_off;
    uint16_t sector_remain;
    uint16_t i;

    /* 写入长度为0，不继续操作 */
    if (w_size == 0)
    {
        return (uint8_t)SFUB_ERR;
    }

    /* 操作地址大于芯片地址范围，返回错误 */
    if (write_addr > sfub_info.total_size)
    {
        return (uint8_t)SFUB_ERR;
    }

    sector_pos = (write_addr >> 12); /* 扇区地址 */
    sector_off = write_addr & 0x0fff; /* 在扇区内的偏移 */
    sector_remain = 4096 - sector_off; /* 扇区剩余空间大小 */

    if (w_size <= sector_remain)
    {
        sector_remain = w_size; /* 不大于4096个字节 */
    }

    while (1)
    {
        /* 读出整个扇区的内容 */
        sfub_read(sfub_buf, (sector_pos << 12), sector_remain);

        /* 校验数据 */
        for (i = 0; i < sector_remain; i++)
        {
            /* 判断需要擦除，不为0xff就需要擦除 */
            if ((~sfub_buf[i + sector_off]) & p_buf[i])
            {
                break;
            }
        }

        /* 需要擦除 */
        if (i < sector_remain)
        {
            /* 擦除这个扇区 */
            sfub_erase_sector(sector_pos << 12);

            for (i = 0; i < sector_remain; i++)
            {
                sfub_buf[i + sector_off] = p_buf[i];
            }

            /* 写入整个扇区 */
            sfub_write_no_check(sfub_buf + sector_off, (sector_pos << 12) + sector_off, sector_remain);
        }
        else
        {
            /* 写已经擦除了的,直接写入扇区剩余区间. */
            sfub_write_no_check(p_buf + sector_off, (sector_pos << 12) + sector_off, sector_remain);
        }

        if (w_size == sector_remain)
        {
            return SFUB_OK;
        }
        else /* 写入未结束 */
        {
            sector_pos++;   /* 扇区地址增1 */
            sector_off = 0; /* 偏移位置为0 */

            p_buf += sector_remain;   /* 指针偏移 */
            write_addr += sector_remain; /* 写地址偏移 */
            w_size -= sector_remain;    /* 字节数递减 */

            if (w_size > 4096)
                sector_remain = 4096; /* 下一个扇区还是写不完 */
            else
                sector_remain = w_size; /* 下一个扇区可以写完了 */
        }
    }
    return SFUB_OK; /* 写入结束了 */
}

/**
 * @func    sfub_erase_chip
 * @brief   FLASH擦除整个芯片
 * @note    等待时间超长
 * @retval  无
 */
void sfub_erase_chip(void)
{
    uint8_t cmd;
    /* SET WEL */
    sfub_write_enable();
    sfub_wait_busy();

    cmd = SFUB_CMD_ERASE_CHIP;

    spi_transfer(sfub_handle, &cmd, NULL, 1);

    /* 等待芯片擦除结束 */
    sfub_wait_busy();
}

/**
 * @func    sfub_erase_sector
 * @brief   FLASH擦除一个扇区
 * @param   DesAddr 扇区地址	
 * @note    擦除一个山区的最少时间:150ms
 * @retval  无
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

    /* 等待擦除完成 */
    sfub_wait_busy();
}

/**
 * @func    sfub_wait_busy
 * @brief   FLASH等待空闲
 * @retval  无
 */
void sfub_wait_busy(void)
{
    /* 等待BUSY位清空 */
    while ((sfub_read_SR() & 0x01) == 0x01)
        ;
}

/**
 * @func    sfub_power_down
 * @brief   FLASH进入掉电模式
 * @retval  无
 */
void sfub_power_down(void)
{
    uint8_t cmd;

    cmd = SFUB_CMD_POWRDON;

    spi_transfer(sfub_handle, &cmd, NULL, 1);

    /* 等待TPD */
    sfub_delay(3);
}

/**
 * @func    sfub_wakeup
 * @brief   唤醒 FLASH
 * @retval  无
 */
void sfub_wakeup(void)
{
    uint8_t cmd;

    cmd = SFUB_CMD_RPOWRDON;

    spi_transfer(sfub_handle, &cmd, NULL, 1);

    /* 等待TRES1 */
    sfub_delay(3);
}
