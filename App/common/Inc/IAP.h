#ifndef _IAP_H_
#define _IAP_H_

#if defined STM32F1
#include "stm32f1xx.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#endif

enum
{
    IAP_ERR_TIMEOUT = 0,
    IAP_ERR_CMD_ERROR = 1,
    IAP_ERR_FIRM_ERROR = 2,
    IAP_ERR_FIRM_ALREADY = 3,
    IAP_ERR_MEM_IS_FULL = 4,
    IAP_ERR_FLASH_IS_FULL = 5
};

enum
{
    IAP_CMD_GET_FIRM_INFO = 0,
    IAP_CMD_GET_FIRM_DATA = 1,
    IAP_CMD_OK = 3,
    IAP_CMD_ERR = 4
};

void GetFirmVer(void);
void Bootloader(void);
void SetBootloaderHooks(void (*SendData)(const void *Data, uint32_t Size),
                        void (*GetDataSize)(uint32_t *Size),
                        void (*ReadData)(void *rData));

#endif

