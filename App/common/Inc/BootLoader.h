

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
    BOOTLOADER_OK = 0,
    BOOTLOADER_ERASEKO,
    BOOTLOADER_WRITINGCTRL_ERROR,
    BOOTLOADER_WRITING_ERROR,
    BOOTLOADER_PROTECTION_ERRROR
};

/* protection type */  
enum{
    BOOTLOADER_PROTECTION_NONE         = 0,
    BOOTLOADER_PROTECTION_PCROPENABLED = 0x1,
    BOOTLOADER_PROTECTION_WRPENABLED   = 0x2,
    BOOTLOADER_PROTECTION_RDPENABLED   = 0x4,
};

/* protection update */
enum {
	BOOTLOADER_WRP_ENABLE,
	BOOTLOADER_WRP_DISABLE
};

/* Define the address from where user application will be loaded.
   Note: this area is reserved for the IAP code                  */
#define FLASH_PAGE_STEP         FLASH_PAGE_SIZE           /* Size of page : 2 Kbytes */
#define APPLICATION_ADDRESS     (uint32_t)0x08008000      /* Start user code address: ADDR_FLASH_PAGE_8 */

/* Notable Flash addresses */
#define USER_FLASH_END_ADDRESS        0x08100000

/* Define the user application size */
#define USER_FLASH_SIZE               ((uint32_t)0x00003000) /* Small default template application */


/* Exported macro ------------------------------------------------------------*/
/* ABSoulute value */
#define ABS_RETURN(x,y)               ((x) < (y)) ? ((y)-(x)) : ((x)-(y))

/* Get the number of sectors from where the user program will be loaded */
#define FLASH_SECTOR_NUMBER           ((uint32_t)(ABS_RETURN(APPLICATION_ADDRESS,FLASH_START_BANK1))>>12)

/* Compute the mask to test if the Flash memory, where the user program will be
  loaded, is write protected */
#define FLASH_PROTECTED_SECTORS       (~(uint32_t)((1 << FLASH_SECTOR_NUMBER) - 1))
/* Exported functions ------------------------------------------------------- */
void Bootloader_Init(void);
uint32_t Bootloader_Erase(uint32_t StartAddr, uint32_t EndAddr);
uint32_t Bootloader_GetWriteProtectionStatus(uint32_t Address);
uint32_t Bootloader_Write(uint32_t Destination, uint32_t *pSource, uint32_t Length);
uint32_t Bootloader_WriteProtectionConfig(uint32_t ProtectionState, uint32_t Address);

#endif  /* __FLASH_IF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
