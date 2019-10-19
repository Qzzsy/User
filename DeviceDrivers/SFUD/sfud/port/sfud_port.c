/*
 * This file is part of the Serial Flash Universal Driver Library.
 *
 * Copyright (c) 2016-2018, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2016-04-23
 */

#include <sfud.h>
#include <stdarg.h>
#include "bsp_spi_bus.h"
#include "spi.h"
#include "ustring.h"

#ifndef USING_EFFI
#define USING_EFFI   1
#endif

#define SFUD_SPIX SPI1
#define SFUD_CS_PORT SPI_SFUD_CS_GPIO_Port
#define SFUD_CS_PIN SPI_SFUD_CS_Pin

#if (USING_EFFI == 1)
#define SFUD_CS_RELEASE LL_GPIO_SetOutputPin(SFUD_CS_PORT, SFUD_CS_PIN)
#define SFUD_CS_TAKE LL_GPIO_ResetOutputPin(SFUD_CS_PORT, SFUD_CS_PIN)
#else
#define SFUD_CS_RELEASE HAL_GPIO_WritePin(SFUD_CS_PORT, SFUD_CS_PIN, GPIO_PIN_SET)
#define SFUD_CS_TAKE HAL_GPIO_WritePin(SFUD_CS_PORT, SFUD_CS_PIN, GPIO_PIN_RESET)
#endif


static char log_buf[256];

void sfud_log_debug(const char *file, const long line, const char *format, ...);

void sfud_cs_take(void);
void sfud_cs_release(void);

spi_ops_t sfud_ops =
    {
        .cs_take = sfud_cs_take,
        .cs_reslease = sfud_cs_release};

void sfud_cs_take(void)
{
    SFUD_CS_TAKE;
}

void sfud_cs_release(void)
{
    SFUD_CS_RELEASE;
}

static void spi_lock(const sfud_spi *spi)
{
    __disable_irq();
}

static void spi_unlock(const sfud_spi *spi)
{
    __enable_irq();
}

/* about 100 microsecond delay */
static void retry_delay_100us(void)
{
    uint32_t delay = 120;
    while (delay--)
        ;
}

/**
 * SPI write data then read data
 */
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
                               size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;
    uint8_t send_data, read_data;
    spi_device_handle_t sfub_handle = (spi_device_handle_t)spi->user_data;

    spi_send_then_recv(sfub_handle, write_buf, write_size, read_buf, read_size);
    /**
     * add your spi write and read code
     */

    return result;
}

#ifdef SFUD_USING_QSPI
/**
 * read flash data by QSPI
 */
static sfud_err qspi_read(const struct __sfud_spi *spi, uint32_t addr, sfud_qspi_read_cmd_format *qspi_read_cmd_format,
                          uint8_t *read_buf, size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;

    /**
     * add your qspi read flash data code
     */

    return result;
}
#endif /* SFUD_USING_QSPI */

sfud_err sfud_spi_port_init(sfud_flash *flash)
{
    static spi_device_t sfud_spi_device;
    static spi_bus_t spi_bus;
    static spi_conf_t sfud_spi_Conf;

    sfud_err result = SFUD_SUCCESS;

    spi_bus.id = 1;
    spi_bus.ops = &sfud_ops;

    sfud_spi_Conf.data_width = 8;
    sfud_spi_Conf.mode = SPI_MODE_1;
    sfud_spi_Conf.max_freq = 80 * 1000000;

    sfud_spi_device.bus = &spi_bus;
    sfud_spi_device.config = &sfud_spi_Conf;
    sfud_spi_device.device = SPI1;

    switch (flash->index)
    {
        case SFUD_W25QXX_DEVICE_INDEX:
        {

            /* 同步 Flash 移植所需的接口及数据 */
            flash->spi.wr = spi_write_read;
            flash->spi.lock = spi_lock;
            flash->spi.unlock = spi_unlock;
            flash->spi.user_data = &sfud_spi_device;
            /* about 100 microsecond delay */
            flash->retry.delay = retry_delay_100us;
            /* adout 60 seconds timeout */
            flash->retry.times = 60 * 10000;

            break;
        }
    }

    return result;
}

/**
 * This function is print debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 */
void sfud_log_debug(const char *file, const long line, const char *format, ...)
{
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    uprintf("[SFUD](%s:%ld) ", file, line);
    /* must use vprintf to print */
    uvsnprintf(log_buf, sizeof(log_buf), format, args);
    uprintf("%s\r\n", log_buf);
    va_end(args);
}

/**
 * This function is print routine info.
 *
 * @param format output format
 * @param ... args
 */
void sfud_log_info(const char *format, ...)
{
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    uprintf("[SFUD]");
    /* must use vprintf to print */
    uvsnprintf(log_buf, sizeof(log_buf), format, args);
    uprintf("%s\r\n", log_buf);
    va_end(args);
}
