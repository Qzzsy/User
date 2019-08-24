/**
 * @file UserMain.c
 * @author ZSY (zhushiye@live.com)
 * @brief user main file
 * @version 0.1
 * @date 2019-07-23
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#define LOG_TAG    "main"

#include "STM32F1xx.h"
#include "bsp_uart.h"
#include "ustring.h"
#include "global.h"
#include "sfud.h"
#include "drv_onchip_flash.h"
#include "VerManage.h"
#include "IAP.h"
#include "main.h"
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include <elog.h>
#include "delay.h"

typedef void (*pFunc)(void);

/* get cmd, bootloader send to user */
uint8_t get_cmd = 0;

sfud_flash * sfud_dev = UNULL;
void IAP_error_return(uint8_t error_cede);
/**
 * @brief Write firmware data to onchip flash
 * 
 * @param addr write address
 * @param buf write buffer
 * @param size write size
 * @return err code
 */
iap_err_t write_onchip_flash(uint32_t addr, const void * buf, uint32_t size)
{
    err_t retval;
    if (buf == UNULL)
    {
        return IAP_ERROR;
    }
    if ((addr + size) < ONCHIP_FLASH_END_ADDRESS) 
    {
        retval = onchip_flash_write(addr, buf, size);
        if (retval != size)
        {
            log_e("data write to onchip flash error!");
            log_e("program fault!");
            return IAP_ERROR;
        }
        return IAP_OK;
    }
    else
    {
        log_e("destination address over onchip flash end address!  0x%08x", addr);
        return IAP_ERROR;
    } 
}

/**
 * @brief Get the send cmd object
 * 
 * @param data: pack data
 * @param Size: data size
 */
iap_err_t get_send_cmd(const void *data, uint32_t Size)
{
    uint8_t *p_buf;
    p_buf = (uint8_t *)data;
    firm_pack_t pack = *((firm_pack_t *)data);
    if (pack.cmd == IAP_CMD_GET_FIRM_INFO)
    {
        get_cmd = IAP_CMD_GET_FIRM_INFO;
    }
    else if (pack.cmd == IAP_CMD_GET_FIRM_DATA)
    {
        get_cmd = IAP_CMD_GET_FIRM_DATA;
    }
    else if (pack.cmd == IAP_CMD_WARNING)
    {
    }
    else if (pack.cmd == IAP_CMD_ERR)
    {
        log_e("iap send error cmd!!!");
        IAP_error_return(p_buf[13]);
    }
    return IAP_OK;
}

/**
 * @brief read firmware data from ext flash
 * 
 * @param pack_num now need read pack num
 * @param buf read buffer
 * @param size will read size
 * @return uint32_t realy read size, return 0, read error
 */
iap_err_t read_firmware_data(uint32_t pack_num, uint32_t offiset, void * buf, uint32_t size)
{
    /* firmware data size */
    static uint32_t firm_data_size = 0;
    firm_pack_t *pack = (firm_pack_t *)buf;
    if (buf == UNULL)
    {
        log_e("read buf is NULL");
        return IAP_ERROR;
    }

    pack->size = size;
    if ((get_cmd == IAP_CMD_GET_FIRM_INFO) && (pack_num == 0))
    {
        pack->cmd = IAP_CMD_GET_FIRM_INFO;
        pack->pack_num = pack_num;
        sfud_read(sfud_dev, FIRM_DATA_BASE_ADDR, pack->size, ((uint8_t *)buf + 12));
        firm_data_size = *((uint32_t *)((uint8_t * )buf + 284 + 12)) + 512;
    }
    else if ((get_cmd == IAP_CMD_GET_FIRM_DATA) && (pack_num != 0))
    {
        pack->cmd = IAP_CMD_GET_FIRM_DATA;
        pack->pack_num = pack_num;

        if ((offiset + size) <= firm_data_size)
        {
            if ((FIRM_DATA_BASE_ADDR + offiset + pack->size) < FIRM_DATA_END_ADDR)
            {
                sfud_read(sfud_dev, FIRM_DATA_BASE_ADDR + offiset, pack->size, ((uint8_t *)buf + 12));
            }
            else
            {
                log_e("destination address over onchip flash end address!  0x%08x", FIRM_DATA_BASE_ADDR + offiset);

                return IAP_ERROR;
            }
        }
        else
        {
            log_e("read firmware data fault, firmware size: %d, but read offiset: %d, read size: %d", firm_data_size, offiset, size);
            return IAP_ERROR;
        }
    }
    //log_d("read ok, size: %d", size);
    return pack->size;
}

/**
 * @brief Get the onchip data object
 * 
 * @param addr read data address
 * @param buf read data buffer
 * @param size read size
 */
iap_err_t get_onchip_data(uint32_t addr, void * buf, uint32_t size)
{
    if (buf == UNULL)
    {
        return IAP_ERROR;
    }
    if ((addr + size) < ONCHIP_FLASH_END_ADDRESS)
    {
        onchip_flash_read(addr, buf, size);
    }
    else
    {
        uprintf("destination address over onchip flash end address!  0x%08x\r\n", addr);
        return IAP_ERROR;
    } 
    return size;
}

/**
 * @brief control flash on chip
 * 
 * @param cmd cmd control
 * @param vargs user parameger
 */
iap_err_t control_onchip_flash(uint8_t cmd, void * vargs)
{
    uint32_t p_data[2];
    p_data[0] = *((uint32_t *)vargs);
    p_data[1] = *((uint32_t *)vargs + 1);

    if (cmd == IAP_CMD_ERASE)
    {
        onchip_flash_erase(p_data[0], p_data[1]);
    }
    return IAP_OK;
}

/**
 * @brief it will jump to appliction program, it`t user program
 * 
 * @param startup_addr user program startup address
 */
void jump_to_app(uint32_t startup_addr)
{
    uint32_t jump_address;
    pFunc jump_to_application;
    uint32_t addr = *(uint32_t *)startup_addr;
    addr &= (~(ONCHIP_RAM_SIZE | (ONCHIP_RAM_SIZE - 1)));
    
    delay_ms(100);
    
    /* 判断堆栈的有效性 */
    if (addr == ONCHIP_RAM_START_ADDRESS)
    {
        /* 获取复位地址 */
        jump_address = *(__IO uint32_t *)(startup_addr + 4);

        jump_to_application = (pFunc)jump_address;

        /* 设置堆栈地址 */
        __set_MSP(*(__IO uint32_t *)startup_addr);
        
        
		for(int i = 0; i < 8; i++)
		{			
			NVIC->ICER[i] = 0xFFFFFFFF;	/* 关闭中断*/
			NVIC->ICPR[i] = 0xFFFFFFFF;	/* 清除中断标志位 */
        }
        
        HAL_UART_MspDeInit(uart1_dev.huart);
        //HAL_SPI_MspDeInit(&hspi1);
        
        delay_ms(100);
        /* 执行跳转 */
        jump_to_application();
    }
    else
    {
        uprintf("heap tack address nullity!\r\n");
        while (true)
        {
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_RESET);
            delay_ms(200);
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_SET);
            delay_ms(200);
            
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_RESET);
            delay_ms(200);
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_SET);
            delay_ms(200);
            
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_RESET);
            delay_ms(2000);
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_SET);
            delay_ms(2000);
        }
    }
}

void IAP_error_return(uint8_t error_cede)
{
    if (error_cede == IAP_ERR_APIS_ERROR)
    {
        while (true)
        {
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_RESET);
            delay_ms(1000);
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_SET);
            delay_ms(2000);
        }
    }
    else if (error_cede == IAP_ERR_CHECK_FAULT)
    {
        while (true)
        {
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_RESET);
            delay_ms(200);
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_SET);
            delay_ms(200);
            
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_RESET);
            delay_ms(200);
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_SET);
            delay_ms(2000);
        }
    }
    else if (error_cede == IAP_ERR_UNKNOW_FIRM)
    {
        while (true)
        {
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_RESET);
            delay_ms(200);
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_SET);
            delay_ms(200);

            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_RESET);
            delay_ms(200);
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_SET);
            delay_ms(200);
            
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_RESET);
            delay_ms(200);
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_SET);
            delay_ms(2000);
        }
    }
    else if (error_cede == IAP_ERR_UPDATE_FAULT)
    {
        while (true)
        {
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_RESET);
            delay_ms(200);
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_SET);
            delay_ms(2000);
        }
    }
    else if (error_cede == IAP_ERR_GET_ONCHIP_DATA_FAULT)
    {
        while (true)
        {
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_RESET);
            delay_ms(200);
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_SET);
            delay_ms(200);

            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_RESET);
            delay_ms(200);
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_SET);
            delay_ms(200);

            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_RESET);
            delay_ms(2000);
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_SET);
            delay_ms(2000);

            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_RESET);
            delay_ms(2000);
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_SET);
            delay_ms(2000);
        }
    }
    else if (error_cede == IAP_ERR_HARDFAULT)
    {
        while (true)
        {
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_RESET);
            delay_ms(200);
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_SET);
            delay_ms(200);
        }
    }
    else
    {
        while (true)
        {
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_RESET);
            delay_ms(5000);
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_SET);
            delay_ms(5000);
        }
    }
}

/**
 * @brief user main
 * 
 */
int UserMainFunc(void)
{
    uint32_t chip_uid[3];
    delay_init();
    
    delay_ms(100);
    
    ustrncpy(uart1_dev.name, "usart1", 6);
    uart1_dev.mode = UART_MODE_IDLE_INT;
    uart1_dev.uart_buf.recv_buf = uart1_recv_buf;
    uart1_dev.uart_buf.recv_buf_size = RECV_BUF_SIZE;
    uart_init(&uart1_dev);
    set_console_device(console_out);

    uprintf("\r\n");
    uprintf("\033[33;22m *******   **    **       ******     *******     *******   **********\033[0m\r\n");
    uprintf("\033[33;22m/**////** //**  **       /*////**   **/////**   **/////** /////**/// \033[0m\r\n");
    uprintf("\033[33;22m/**    /** //****        /*   /**  **     //** **     //**    /**    \033[0m\r\n");
    uprintf("\033[33;22m/**    /**  //**    *****/******  /**      /**/**      /**    /**    \033[0m\r\n");
    uprintf("\033[33;22m/**    /**   /**   ///// /*//// **/**      /**/**      /**    /**    \033[0m\r\n");
    uprintf("\033[33;22m/**    **    /**         /*    /**//**     ** //**     **     /**    \033[0m\r\n");
    uprintf("\033[33;22m/*******     /**         /*******  //*******   //*******      /**    \033[0m\r\n");
    uprintf("\033[33;22m///////      //          ///////    ///////     ///////       //     \033[0m\r\n");
    uprintf("\r\n\033[33;22mBootloader Firmware version: %s\033[0m\r\n", Ver);
    uprintf("\033[33;22mproject building time: %s %s\033[0m\r\n\r\n", BuiltDate, BuiltTime);
    
    //HAL_GetUID(chip_uid);
    uprintf("\033[35;22mchip uid: %08x%08x%08x\033[0m\r\n\r\n", chip_uid[0], chip_uid[1], chip_uid[2]);

    elog_init();
    
    /* set EasyLogger log format */
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_T_INFO | ELOG_FMT_P_INFO | ELOG_FMT_TIME));
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_FUNC | ELOG_FMT_T_INFO | ELOG_FMT_P_INFO | ELOG_FMT_TIME));
    /* start EasyLogger */
    elog_set_text_color_enabled(true);
    
    elog_start();
        
    delay_ms(200);
    
    if (sfud_init() != SFUD_SUCCESS)
    {
        log_e("sfud init fault");
    }

    sfud_dev = sfud_get_device(0);
    if (sfud_dev == UNULL)
    {
        log_e("can`t find sfud device!");
        
        while (true)
        {
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_RESET);
            delay_ms(1000);
            HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, GPIO_PIN_SET);
            delay_ms(3000);
        }

        while (true);
    }

    set_bootloader_hooks(get_send_cmd,
                        read_firmware_data,
                        write_onchip_flash,
                        get_onchip_data,
                        control_onchip_flash,
                        jump_to_app);
                        
    Bootloader();
    
    return 0;
}
