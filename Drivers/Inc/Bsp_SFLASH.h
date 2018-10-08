/**
 ******************************************************************************
 * @Copyright     (C) 2017 - 2018 guet-sctc-hardwarepart Team
 * @filename      Bsp_SFLASH.h
 * @author        ZSY
 * @version       V1.0.1
 * @date          2018-10-08
 * @Description   定义了SPI FLASH的常规命令以API
 * @Others
 * @History
 * Date           Author    version    		Notes
 * 2017-11-01     ZSY       V1.0.0      first version.
 * 2018-10-08     ZSY       V1.0.1      完善驱动框架.
 */
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _BSP_SFLASH_H_
#define _BSP_SFLASH_H_

#if defined STM32F1
#include "STM32f1xx.h"
#elif defined STM32F4
#include "STM32f4xx.h"
#endif

enum
{
    SF_ERR = -1,
    SF_OK = 0
};

/* JEDEC Device ID: Memory type and Capacity */
enum
{
    SF_W25Q80_BV  = 0xEF4014, /* W25Q80BV */
    SF_W25Q16_BV_CL_CV = 0xEF4015, /* W25Q16BV W25Q16CL W25Q16CV  */
    SF_W25Q16_DW = 0xEF6015,/* W25Q16DW  */
    SF_W25Q32_BV = 0xEF4016,/* W25Q32BV */
    SF_W25Q32_DW = 0xEF6016,/* W25Q32DW */
    SF_W25Q64_DW_BV_CV = 0xEF4017, /* W25Q64DW W25Q64BV W25Q64CV */
    SF_W25Q128_BV = 0xEF4018, /* W25Q128BV */
    SF_W25Q256_FV = 0xEF4019,    /* W25Q256FV */
	SF_SST25VF016B_ID = 0xBF2541,
	SF_MX25L1606E_ID  = 0xC22015,
};

typedef struct
{
    uint32_t TotalSize;
    uint32_t ChipID;
    uint16_t PageSize;
    char ChipName[20];
}sfInfo_t;

/* 芯片信息 */
extern sfInfo_t _sfInfo;

uint8_t sfInit(void);
uint8_t sfWriteNoCheck(uint8_t * pBuffer, uint32_t WriteAddr, uint16_t _wSize);
void sfWritePage(uint8_t * pBuffer, uint32_t WriteAddr, uint16_t _wSize);
uint8_t sfRead(uint8_t * pBuffer, uint32_t WriteAddr, uint16_t _rSize);   //读取flash
uint8_t sfWrite(uint8_t * pBuffer, uint32_t WriteAddr, uint16_t _wSize);//写入flash
void sfEraseChip(void);    	  //整片擦除
void sfEraseSector(uint32_t Dst_Addr);//扇区擦除
void sfPowerDown(void);           //进入掉电模式
void sfWakeup(void);			  //唤醒

#endif
