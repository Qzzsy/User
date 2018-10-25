

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BOOTLOADER_H_
#define __BOOTLOADER_H_

/* Includes ------------------------------------------------------------------*/
#if defined STM32F1
#include "stm32f10x.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#elif defined STM32F7
#include "stm32f7xx.h"
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/


/* Error code */
enum 
{
    INTFLASH_OK = 0,
    INTFLASH_ERASEKO,
    INTFLASH_WRITINGCTRL_ERROR,
    INTFLASH_WRITING_ERROR,
    INTFLASH_PROTECTION_ERRROR
};
/* protection type */  
enum{
    INTFLASH_PROTECTION_NONE         = 0,
    INTFLASH_PROTECTION_PCROPENABLED = 0x1,
    INTFLASH_PROTECTION_WRPENABLED   = 0x2,
    INTFLASH_PROTECTION_RDPENABLED   = 0x4,
};

/* protection update */
enum {
	INTFLASH_WRP_ENABLE,
	INTFLASH_WRP_DISABLE
};

/* Define the address from where user application will be loaded.
   Note: this area is reserved for the IAP code                  */
#define FLASH_PAGE_STEP         FLASH_PAGE_SIZE           /* Size of page : 2 Kbytes */


/* Exported macro ------------------------------------------------------------*/
/* ABSoulute value */
#define ABS_RETURN(x,y)               ((x) < (y)) ? ((y)-(x)) : ((x)-(y))

/* Get the number of sectors from where the user program will be loaded */
#define FLASH_SECTOR_NUMBER           ((uint32_t)(ABS_RETURN(APPLICATION_ADDRESS,FLASH_START_BANK1))>>12)

/* Compute the mask to test if the Flash memory, where the user program will be
  loaded, is write protected */
#define FLASH_PROTECTED_SECTORS       (~(uint32_t)((1 << FLASH_SECTOR_NUMBER) - 1))

/* Exported functions ------------------------------------------------------- */
void IntFlashInit(void);
uint32_t GetSectorStartAddr(uint32_t Address);
uint32_t IntFlashErase(uint32_t StartAddr, uint32_t EndAddr);
uint32_t IntFlashWrite(uint32_t Destination, uint8_t *pSource, uint32_t Length);
uint32_t IntFlashRead(uint32_t Destination, uint8_t *Buf, uint32_t Length);
uint32_t IntFlashGetWriteProtectionStatus(uint32_t Address);
uint32_t IntFlashWriteProtectionConfig(uint32_t ProtectionState, uint32_t Address);

#endif  /* __FLASH_IF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
