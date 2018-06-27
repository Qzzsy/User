/**
 ******************************************************************************
 * @Copyright     (C) 2017 - 2018 guet-sctc-hardwarepart Team
 * @filename      Bsp_W25QXX.h
 * @author        ZSY
 * @version       V1.0.1
 * @date          2018-06-27
 * @Description   定义了W25QXX的常规命令以API
 * @Others
 * @History
 * Date           Author    version    		Notes
 * 2017-11-01     ZSY       V1.0.0      first version.
 */
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _BSP_W25QXX_H_
#define _BSP_W25QXX_H_

#include "STM32f4xx.h"

#define W25Q80 	0XEF13 	
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128 0XEF17
#define W25Q256 0xEF18

/* 定义我们使用的flash芯片型号 */
extern uint16_t BspW25QXX_TYPE;

void BspW25QXX_Init(void);
void BspW25QXX_WriteNoCheck(uint8_t * pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void BspW25QXX_WritePage(uint8_t * pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void BspW25QXX_Read(uint8_t * pBuffer, uint32_t WriteAddr, uint16_t NumByteToRead);   //读取flash
void BspW25QXX_Write(uint8_t * pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);//写入flash
void BspW25QXX_EraseChip(void);    	  //整片擦除
void BspW25QXX_EraseSector(uint32_t Dst_Addr);//扇区擦除
void BspW25QXX_PowerDown(void);           //进入掉电模式
void BspW25QXX_WAKEUP(void);			  //唤醒

#endif
