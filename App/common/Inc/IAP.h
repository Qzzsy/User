/**
 ******************************************************************************
 * @Copyright     (C) 2017 - 2018 guet-sctc-hardwarepart Team
 * @filename      iap.h
 * @author        ZSY
 * @version       V1.0.0
 * @date          2018-10-25
 * @Description   实现IAP功能。
 * @Others
 * @History
 * Date           Author    version    		    Notes
 * 2018-06-22      ZSY      V1.0.0          first version.
 */
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _IAP_H_
#define _IAP_H_

#include "uconfig.h"
#include "udef.h"

#if defined STM32F1
#include "stm32f1xx.h"
#elif defined STM32F4
#include "stm32f4xx.h"
#endif

typedef uint32_t iap_err_t;

/* 枚举错误代码 */
enum
{
    IAP_ERROR                           = 0xffffffff,
    IAP_OK                              = 0,
    IAP_ERR_CMD_ERROR                   = 1,
    IAP_ERR_FIRM_ERROR                  = 3,
    IAP_ERR_FIRM_ALREADY                = 4,
    IAP_ERR_MEM_IS_FULL                 = 5,
    IAP_ERR_FLASH_IS_FULL               = 6,
    
    IAP_ERR_APIS_ERROR                  = 100,
    IAP_ERR_UNKNOW_FIRM                 = 101,
    IAP_ERR_CHECK_FAULT                 = 102,
    IAP_ERR_UPDATE_FAULT                = 103,
    IAP_ERR_GET_ONCHIP_DATA_FAULT       = 106,
    IAP_ERR_ERASE_FAULT                 = 107,
    IAP_ERR_CHECK_IS_EMPTY_FAULT        = 108,
    IAP_ERR_GET_EXT_DATA_FAULT          = 109,   

    IAP_ERR_HARDFAULT,
};

/* 枚举命令 */
enum
{
    IAP_CMD_GET_FIRM_INFO   = 0,
    IAP_CMD_GET_FIRM_DATA   = 1,
    IAP_CMD_ERASE           = 2,
    IAP_CMD_OK              = 3,
    IAP_CMD_ERR             = 4,
    IAP_CMD_WARNING         = 5
};

/* 固件数据包结构体 */
#pragma pack(4)
typedef struct
{
    uint32_t cmd;
    uint32_t pack_num;
    uint32_t size;
    void *data;
} firm_pack_t;
#pragma pack()

/* APIs */
void Bootloader(void);
void set_bootloader_hooks(iap_err_t (*_send_cmd)(const void *data, uint32_t Size),
                        iap_err_t (*_read_firm_data)(uint32_t pack_num, uint32_t offiset, void *data, uint32_t size),
                        iap_err_t (*_send_to_onchip_flash)(uint32_t addr, const void *data, uint32_t size),
                        iap_err_t (*_read_from_onchip_flash)(uint32_t addr, void *data, uint32_t size),
                        iap_err_t (*_control_onchip_flash)(uint8_t cmd, void *vargs),
                        void (*_jump_app)(uint32_t startup_addr));

#endif

