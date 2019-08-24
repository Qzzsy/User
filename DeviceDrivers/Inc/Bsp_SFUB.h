/**
 ******************************************************************************
 * @Copyright     (C) 2017 - 2018 guet-sctc-hardwarepart Team
 * @filename      Bsp_SFUB.h
 * @author        ZSY
 * @version       V1.0.1
 * @date          2018-10-08
 * @Description   ������SPI FLASH�ĳ���������API
 * @Others
 * @History
 * Date           Author    version    		Notes
 * 2017-11-01     ZSY       V1.0.0      first version.
 * 2018-10-08     ZSY       V1.0.1      �����������.
 */
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _BSP_SFUB_H_
#define _BSP_SFUB_H_

#include "udef.h"
#include "uconfig.h"

#if defined STM32F1
#include "STM32f1xx.h"
#elif defined STM32F4
#include "STM32f4xx.h"
#endif

enum
{
    SFUB_ERR = -1,
    SFUB_OK = 0
};

/* JEDEC Device ID: Memory type and Capacity */
enum
{
    SFUB_W25Q80_BV          = 0xEF4014, /* W25Q80BV */
    SFUB_W25Q16_BV_CL_CV    = 0xEF4015, /* W25Q16BV W25Q16CL W25Q16CV  */
    SFUB_W25Q16_DW          = 0xEF6015, /* W25Q16DW  */
    SFUB_W25Q32_BV          = 0xEF4016, /* W25Q32BV */
    SFUB_W25Q32_DW          = 0xEF6016, /* W25Q32DW */
    SFUB_W25Q64_DW_BV_CV    = 0xEF4017, /* W25Q64DW W25Q64BV W25Q64CV */
    SFUB_W25Q128_BV         = 0xEF4018, /* W25Q128BV */
    SFUB_W25Q256_FV         = 0xEF4019, /* W25Q256FV */
	SFUB_SST25VF016B_ID     = 0xBF2541,
	SFUB_MX25L1606E_ID      = 0xC22015,
};

typedef struct
{
    uint32_t total_size;
    uint32_t chip_id;
    uint16_t page_size;
    uint16_t sector_size;
    char chip_name[20];
}sfub_info_t;

/* оƬ��Ϣ */
extern sfub_info_t sfub_info;

uint8_t sfub_init(void);
uint8_t sfub_write_no_check(uint8_t * p_buf, uint32_t write_addr, uint16_t w_size);
uint8_t sfub_read(uint8_t * p_buf, uint32_t read_addr, uint16_t r_size);   //��ȡflash
uint8_t sfub_write(uint8_t * p_buf, uint32_t write_addr, uint16_t w_size);//д��flash
void sfub_erase_chip(void);    	  //��Ƭ����
void sfub_erase_sector(uint32_t des_addr);//��������
void sfub_power_down(void);           //�������ģʽ
void sfub_wakeup(void);			  //����

#endif
