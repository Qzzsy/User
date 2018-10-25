

/** @addtogroup STM32L4xx_IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "Bsp_IntFlash.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/*  1M0 flash 1 * 1024 * 1024 */
#define FLASH_START_ADRESS 0x08000000

#if defined STM32F4
/* 每个Bank区所包含的山区数 */
#define FLASH_PAGE_NBPERBANK 12
#endif
#define FLASH_SIZE 1 * 1024 * 1024

#if defined(STM32F427xx) || defined(STM32F437xx) || defined(STM32F429xx) || defined(STM32F439xx) || \
    defined(STM32F469xx) || defined(STM32F479xx)
#define FLASH_BANK_NUMBER 2
#else
#define FLASH_BANK_NUMBER 1
#endif

#if defined(STM32F4)
/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0 ((uint32_t)0x08000000)  /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1 ((uint32_t)0x08004000)  /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2 ((uint32_t)0x08008000)  /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3 ((uint32_t)0x0800C000)  /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4 ((uint32_t)0x08010000)  /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5 ((uint32_t)0x08020000)  /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6 ((uint32_t)0x08040000)  /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7 ((uint32_t)0x08060000)  /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8 ((uint32_t)0x08080000)  /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9 ((uint32_t)0x080A0000)  /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10 ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11 ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */
#if defined(STM32F427xx) || defined(STM32F437xx) || defined(STM32F429xx) || defined(STM32F439xx) || \
    defined(STM32F469xx) || defined(STM32F479xx)
/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_12 ((uint32_t)0x08100000) /* Base @ of Sector 12, 16 Kbytes */
#define ADDR_FLASH_SECTOR_13 ((uint32_t)0x08104000) /* Base @ of Sector 13, 16 Kbytes */
#define ADDR_FLASH_SECTOR_14 ((uint32_t)0x08108000) /* Base @ of Sector 14, 16 Kbytes */
#define ADDR_FLASH_SECTOR_15 ((uint32_t)0x0810C000) /* Base @ of Sector 15, 16 Kbytes */
#define ADDR_FLASH_SECTOR_16 ((uint32_t)0x08110000) /* Base @ of Sector 16, 64 Kbytes */
#define ADDR_FLASH_SECTOR_17 ((uint32_t)0x08120000) /* Base @ of Sector 17, 128 Kbytes */
#define ADDR_FLASH_SECTOR_18 ((uint32_t)0x08140000) /* Base @ of Sector 18, 128 Kbytes */
#define ADDR_FLASH_SECTOR_19 ((uint32_t)0x08160000) /* Base @ of Sector 19, 128 Kbytes */
#define ADDR_FLASH_SECTOR_20 ((uint32_t)0x08180000) /* Base @ of Sector 20, 128 Kbytes */
#define ADDR_FLASH_SECTOR_21 ((uint32_t)0x081A0000) /* Base @ of Sector 21, 128 Kbytes */
#define ADDR_FLASH_SECTOR_22 ((uint32_t)0x081C0000) /* Base @ of Sector 22, 128 Kbytes */
#define ADDR_FLASH_SECTOR_23 ((uint32_t)0x081E0000) /* Base @ of Sector 23, 128 Kbytes */
#endif
#elif defined STM32F7
/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0 ((uint32_t)0x08000000)  /* Base @ of Sector 0, 32 Kbytes */
#define ADDR_FLASH_SECTOR_1 ((uint32_t)0x08008000)  /* Base @ of Sector 1, 32 Kbytes */
#define ADDR_FLASH_SECTOR_2 ((uint32_t)0x08010000)  /* Base @ of Sector 2, 32 Kbytes */
#define ADDR_FLASH_SECTOR_3 ((uint32_t)0x08018000)  /* Base @ of Sector 3, 32 Kbytes */
#define ADDR_FLASH_SECTOR_4 ((uint32_t)0x08020000)  /* Base @ of Sector 4, 128 Kbytes */
#define ADDR_FLASH_SECTOR_5 ((uint32_t)0x08040000)  /* Base @ of Sector 5, 256 Kbytes */
#define ADDR_FLASH_SECTOR_6 ((uint32_t)0x08080000)  /* Base @ of Sector 6, 256 Kbytes */
#define ADDR_FLASH_SECTOR_7 ((uint32_t)0x080C0000)  /* Base @ of Sector 7, 256 Kbytes */
#endif



/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
uint32_t GetSector(uint32_t Address)
{
#if defined (STM32F1)
    return (Address - FLASH_START_ADRESS) / 2048;
#elif defined(STM32F4)
    if ((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
    {
        return 0;
    }
    else if ((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
    {
        return 1;
    }
    else if ((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
    {
        return 2;
    }
    else if ((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
    {
        return 3;
    }
    else if ((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
    {
        return 4;
    }
    else if ((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
    {
        return 5;
    }
    else if ((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
    {
        return 6;
    }
    else if ((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
    {
        return 7;
    }
    else if ((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
    {
        return 8;
    }
    else if ((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
    {
        return 9;
    }
    else if ((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
    {
        return 10;
    }
#if !defined(STM32F427xx) || defined(STM32F437xx) || defined(STM32F429xx) || defined(STM32F439xx) || \
    defined(STM32F469xx) || defined(STM32F479xx)
    /*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
    else
    {
        return 11;
    }
#endif
#if defined(STM32F427xx) || defined(STM32F437xx) || defined(STM32F429xx) || defined(STM32F439xx) || \
    defined(STM32F469xx) || defined(STM32F479xx)
    else if ((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11))
    {
        return 11;
    }
    else if ((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12))
    {
        return 12;
    }
    else if ((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13))
    {
        return 13;
    }
    else if ((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14))
    {
        return 14;
    }
    else if ((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15))
    {
        return 15;
    }
    else if ((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16))
    {
        return 16;
    }
    else if ((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17))
    {
        return 17;
    }
    else if ((Address < ADDR_FLASH_SECTOR_19) && (Address >= ADDR_FLASH_SECTOR_18))
    {
        return 18;
    }
    else if ((Address < ADDR_FLASH_SECTOR_20) && (Address >= ADDR_FLASH_SECTOR_19))
    {
        return 19;
    }
    else if ((Address < ADDR_FLASH_SECTOR_21) && (Address >= ADDR_FLASH_SECTOR_20))
    {
        return 20;
    }
    else if ((Address < ADDR_FLASH_SECTOR_22) && (Address >= ADDR_FLASH_SECTOR_21))
    {
        return 21;
    }
    else if ((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_22))
    {
        return 22;
    }
    else /*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
    {
        return 23;
    }
#endif
#elif defined STM32F7
    if ((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
    {
        return 0;
    }
    else if ((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
    {
        return 1;
    }
    else if ((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
    {
        return 2;
    }
    else if ((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
    {
        return 3;
    }
    else if ((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
    {
        return 4;
    }
    else if ((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
    {
        return 5;
    }
    else if ((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
    {
        return 6;
    }
    else
    {
        return 7;
    }
#endif
}
    
uint32_t GetSectorStartAddr(uint32_t Address)
{
#if defined (STM32F1)
    return ((Address - FLASH_START_ADRESS) / 2048) * 2048 + FLASH_START_ADRESS;
#endif
#if defined (STM32F4) || defined (STM32F7)
    switch (GetSector(Address))
    {
#elif defined STM32F4
        case 0: return ADDR_FLASH_SECTOR_0;
        case 1: return ADDR_FLASH_SECTOR_1;
        case 2: return ADDR_FLASH_SECTOR_2;
        case 3: return ADDR_FLASH_SECTOR_3;
        case 4: return ADDR_FLASH_SECTOR_4;
        case 5: return ADDR_FLASH_SECTOR_5;
        case 6: return ADDR_FLASH_SECTOR_6;
        case 7: return ADDR_FLASH_SECTOR_7;
        case 8: return ADDR_FLASH_SECTOR_8;
        case 9: return ADDR_FLASH_SECTOR_9;
        case 10: return ADDR_FLASH_SECTOR_10;
        case 11: return ADDR_FLASH_SECTOR_11;
#if defined(STM32F427xx) || defined(STM32F437xx) || defined(STM32F429xx) || defined(STM32F439xx) || \
    defined(STM32F469xx) || defined(STM32F479xx)
        case 12: return ADDR_FLASH_SECTOR_12;
        case 13: return ADDR_FLASH_SECTOR_13;
        case 14: return ADDR_FLASH_SECTOR_14;
        case 15: return ADDR_FLASH_SECTOR_15;
        case 16: return ADDR_FLASH_SECTOR_16;
        case 17: return ADDR_FLASH_SECTOR_17;
        case 18: return ADDR_FLASH_SECTOR_18;
        case 19: return ADDR_FLASH_SECTOR_19;
        case 20: return ADDR_FLASH_SECTOR_20;
        case 21: return ADDR_FLASH_SECTOR_21;
        case 22: return ADDR_FLASH_SECTOR_22;
        case 23: return ADDR_FLASH_SECTOR_23;
#endif
#elif defined STM32F7
        case 0: return ADDR_FLASH_SECTOR_0;
        case 1: return ADDR_FLASH_SECTOR_1;
        case 2: return ADDR_FLASH_SECTOR_2;
        case 3: return ADDR_FLASH_SECTOR_3;
        case 4: return ADDR_FLASH_SECTOR_4;
        case 5: return ADDR_FLASH_SECTOR_5;
        case 6: return ADDR_FLASH_SECTOR_6;
        case 7: return ADDR_FLASH_SECTOR_7;
#endif
        default : break;
    }
#endif
    return 0;
}
/**
  * @brief  Unlocks Flash for write access
  * @param  None
  * @retval None
  */
void IntFlashInit(void)
{
    /* Unlock the Program memory */
    HAL_FLASH_Unlock();

    /* Clear all FLASH flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_OPERR);
    /* Unlock the Program memory */
    HAL_FLASH_Lock();
}

/**
  * @brief  This function does an erase of all user flash area
  * @param  start: start of user flash area
  * @retval FLASHIF_OK : user flash area successfully erased
  *         FLASHIF_ERASEKO : error occurred
  */
uint32_t IntFlashErase(uint32_t StartAddr, uint32_t EndAddr)
{
    uint32_t NbrOfSectors = 0;
    uint8_t StartSectors = 0;
    uint8_t MulBankFlag = false;
    uint32_t PageError = 0;
    FLASH_EraseInitTypeDef pEraseInit;
    HAL_StatusTypeDef status = HAL_OK;

    if (StartAddr < FLASH_START_ADRESS)
    {
        return HAL_ERROR;
    }

    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();

    /* Get the number of page to  erase */
    StartSectors = GetSector(StartAddr);
    NbrOfSectors = GetSector(EndAddr) - StartSectors + 1;

    if (StartSectors < FLASH_PAGE_NBPERBANK)
    {
        if (StartSectors + NbrOfSectors > FLASH_PAGE_NBPERBANK)
        {
            MulBankFlag = true;
            pEraseInit.NbSectors = FLASH_PAGE_NBPERBANK - StartSectors;
            pEraseInit.Sector = StartSectors;
            StartSectors = FLASH_PAGE_NBPERBANK;
            NbrOfSectors -= (FLASH_PAGE_NBPERBANK - StartSectors);
        }
        else
        {
            pEraseInit.NbSectors = NbrOfSectors;
            pEraseInit.Sector = StartSectors;
        }
        pEraseInit.Banks = FLASH_BANK_1;
        pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
        pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);
    }
    else
    {
        MulBankFlag = true;
    }
#if (FLASH_BANK_NUMBER == 2)
    if (status == HAL_OK && MulBankFlag == true)
    {
        if (StartSectors + NbrOfSectors > FLASH_PAGE_NBPERBANK)
        {
            return HAL_ERROR;
        }
        pEraseInit.Banks = FLASH_BANK_2;
        pEraseInit.NbSectors = NbrOfSectors;
        pEraseInit.Sector = StartSectors;
        pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
        pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_4;
        status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);
    }
#endif

    /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();

    if (status != HAL_OK)
    {
        /* Error occurred while page erase */
        return INTFLASH_ERASEKO;
    }

    return INTFLASH_OK;
}

/* Public functions ---------------------------------------------------------*/
/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  destination: start address for target location
  * @param  p_source: pointer on buffer with data to write
  * @param  length: length of data buffer (unit is 32-bit word)
  * @retval uint32_t 0: Data successfully written to Flash memory
  *         1: Error occurred while writing data in Flash memory
  *         2: Written Data in flash memory is different from expected one
  */
uint32_t IntFlashWrite(uint32_t Destination, uint8_t *pSource, uint32_t Length)
{
    uint32_t status = INTFLASH_OK;
    uint32_t i = 0;

    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();
    __HAL_FLASH_DATA_CACHE_DISABLE();

    /* DataLength must be a multiple of 64 bit */
    for (i = 0; i < Length; i++)
    {
        /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
        be done by word */
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, Destination, *(pSource + i)) == HAL_OK)
        {
            /* Check the written value */
            if (*(uint8_t *)Destination != *(pSource + i))
            {
                /* Flash content doesn't match SRAM content */
                status = INTFLASH_WRITINGCTRL_ERROR;
                break;
            }
            /* Increment FLASH destination address */
            Destination++;
        }
        else
        {
            /* Error occurred while writing data in Flash memory */
            status = INTFLASH_WRITING_ERROR;
            break;
        }
    }

    /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    __HAL_FLASH_DATA_CACHE_ENABLE();
    HAL_FLASH_Lock();

    return status;
}

uint32_t IntFlashRead(uint32_t Destination, uint8_t *Buf, uint32_t Length)
{
    while (Length--)
    {
        *Buf = *(uint8_t *)Destination;
        Buf++;
        ((uint8_t *)Destination)++;
    }
}
/**
  * @brief  Returns the write protection status of application flash area.
  * @param  None
  * @retval If a sector in application area is write-protected returned value is a combinaison
            of the possible values : FLASHIF_PROTECTION_WRPENABLED, FLASHIF_PROTECTION_PCROPENABLED, ...
  *         If no sector is write-protected FLASHIF_PROTECTION_NONE is returned.
  */
uint32_t IntFlashGetWriteProtectionStatus(uint32_t Address)
{
    uint32_t ProtectedPAGE = INTFLASH_PROTECTION_NONE;
    FLASH_OBProgramInitTypeDef OptionsBytesStruct;
    uint32_t Sectors;

    Sectors = GetSector(Address);
    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();

    /* Check if there are write protected sectors inside the user flash area ***/
    HAL_FLASHEx_OBGetConfig(&OptionsBytesStruct);

    /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();

#if defined(STM32F4)
    /* check WRP */
    if (OptionsBytesStruct.WRPSector & (1 < Sectors) == HAL_OK)
    {
        ProtectedPAGE |= INTFLASH_PROTECTION_WRPENABLED;
    }

    if (OptionsBytesStruct.RDPLevel != 0xAA)
    {
        ProtectedPAGE |= INTFLASH_PROTECTION_RDPENABLED;
    }
#endif
    return ProtectedPAGE;
}

/**
  * @brief  Configure the write protection status of user flash area.
  * @param  protectionstate : FLASHIF_WRP_DISABLE or FLASHIF_WRP_ENABLE the protection
  * @retval uint32_t FLASHIF_OK if change is applied.
  */
uint32_t IntFlashWriteProtectionConfig(uint32_t ProtectionState, uint32_t Address)
{
    FLASH_OBProgramInitTypeDef OptionsBytesStruct;
    HAL_StatusTypeDef status;
    uint32_t Sectors;

    Sectors = GetSector(Address);
    /* Unlock the Flash to enable the flash control register access *************/
    status = HAL_FLASH_Unlock();

    /* Unlock the Options Bytes *************************************************/
    status |= HAL_FLASH_OB_Unlock();

#if defined(STM32F4)
    OptionsBytesStruct.OptionType = OPTIONBYTE_WRP;
    if (ProtectionState == INTFLASH_WRP_ENABLE)
    {
        OptionsBytesStruct.WRPSector = 1 << Sectors;
        OptionsBytesStruct.WRPState = OB_WRPSTATE_ENABLE;
    }
    else
    {
        OptionsBytesStruct.WRPSector = 1 << Sectors;
        OptionsBytesStruct.WRPState = OB_WRPSTATE_DISABLE;
    }

    status |= HAL_FLASHEx_OBProgram(&OptionsBytesStruct);

    OptionsBytesStruct.RDPLevel = OB_RDP_LEVEL_0;
    OptionsBytesStruct.OptionType = OPTIONBYTE_RDP;
    status |= HAL_FLASHEx_OBProgram(&OptionsBytesStruct);
#endif
    return (status == HAL_OK ? INTFLASH_OK : INTFLASH_PROTECTION_ERRROR);
}

