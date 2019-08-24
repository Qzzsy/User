#ifndef __DRV_FLASH_H__
#define __DRV_FLASH_H__

#include "uconfig.h"
#include "udef.h"

#include "stm32f1xx.h"

#ifdef __cplusplus
extern "C" {
#endif

int onchip_flash_read(uint32_t addr, uint8_t *buf, size_t size);
int onchip_flash_write(uint32_t addr, const uint8_t *buf, size_t size);
int onchip_flash_erase(uint32_t addr, size_t size);

#ifdef __cplusplus
}
#endif

#endif  /* __DRV_FLASH_H__ */
