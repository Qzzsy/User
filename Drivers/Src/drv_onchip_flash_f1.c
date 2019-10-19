#include "drv_onchip_flash.h"

#ifdef USING_ONCHIP_DEBUG
#include "ustring.h"
#endif

/**
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
static uint32_t get_page_address(uint32_t addr)
{
    uint32_t page = 0;
    if (FLASH_PAGE_SIZE == 0x400u)
    {
        page = addr >> 10;
    }
    else if (FLASH_PAGE_SIZE == 0x800u)
    {
        page = addr >> 11;
    }
    return (page << 11);
}

/**
 * Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
int onchip_flash_read(uint32_t addr, uint8_t *buf, size_t size)
{
    size_t i;

    if ((addr + size) > ONCHIP_FLASH_END_ADDRESS)
    {
#ifdef USING_ONCHIP_DEBUG
        uprintf("read outrange flash size! addr is (0x%p)", (void *)(addr + size));
#endif
        return -UEINVAL;
    }

    for (i = 0; i < size; i++, buf++, addr++)
    {
        *buf = *(uint8_t *) addr;
    }

    return size;
}

/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr flash address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
int onchip_flash_write(uint32_t addr, const uint8_t *buf, size_t size)
{
    err_t result        = UEOK;
    uint32_t end_addr   = addr + size;

    if (addr % 2 != 0)
    {
#ifdef USING_ONCHIP_DEBUG
        uprintf("write addr must be 4-byte alignment");
#endif
        return -UEINVAL;
    }

    if ((end_addr) > ONCHIP_FLASH_END_ADDRESS)
    {
#ifdef USING_ONCHIP_DEBUG
        uprintf("write outrange flash size! addr is (0x%p)", (void *)(addr + size));
#endif
        return -UEINVAL;
    }

    HAL_FLASH_Unlock();

    while (addr < end_addr)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, *((uint16_t *)buf)) == HAL_OK)
        {
            if (*(uint16_t *)addr != *(uint16_t *)buf)
            {
                result = -UERROR;
                break;
            }
            addr += 2;
            buf  += 2;
        }
        else
        {
            result = -UERROR;
            break;
        }
    }

    HAL_FLASH_Lock();

    if (result != UEOK)
    {
        return result;
    }

    return size;
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
int onchip_flash_erase(uint32_t addr, size_t size)
{
    err_t result = UEOK;
    uint32_t PAGEError = 0;

    /*Variable used for Erase procedure*/
    FLASH_EraseInitTypeDef EraseInitStruct;

    if ((addr + size) > ONCHIP_FLASH_END_ADDRESS)
    {
#ifdef USING_ONCHIP_DEBUG
        uprintf("ERROR: erase outrange flash size! addr is (0x%p)\n", (void *)(addr + size));
#endif
        return -UEINVAL;
    }

    HAL_FLASH_Unlock();

    /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = get_page_address(addr);
    EraseInitStruct.NbPages     = (size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
    {
        result = -UERROR;
        goto __exit;
    }

__exit:
    HAL_FLASH_Lock();

    if (result != UEOK)
    {
        return result;
    }

#ifdef USING_ONCHIP_DEBUG
    uprintf("erase done: addr (0x%p), size %d\n", (void *)addr, size);
#endif
    return size;
}

