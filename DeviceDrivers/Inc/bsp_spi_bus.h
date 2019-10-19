/**
 ******************************************************************************
 * @Copyright     (C) 2017 - 2018 guet-sctc-hardwarepart Team
 * @filename      Bsp_SPI_BUS.h
 * @author        ZSY
 * @version       V1.0.0
 * @date          2018-10-08
 * @Description   定义了SPI总线的常规命令以API
 * @Others
 * @History
 * Date           Author    version    		Notes
 * 2017-10-08     ZSY       V1.0.0      first version.
 */
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _BSP_SPI_BUS_H_
#define _BSP_SPI_BUS_H_

#include "udef.h"
#include "uconfig.h"

#if defined (STM32F1)
#include "stm32f1xx.h"
#elif defined (STM32F4)
#include "stm32f4xx.h"
#endif

//#define SOFT_SPI		/* 定义此行表示使用GPIO模拟SPI接口 */
#define HARD_SPI 		/* 定义此行表示使用CPU的硬件SPI接口 */

#define SPI_BUS_BUSY		0x00
#define SPI_BUS_NOBUSY		0x01

#define SPI_CPHA (1 << 0) /* bit[0]:CPHA, clock phase */
#define SPI_CPOL (1 << 1) /* bit[1]:CPOL, clock polarity */

#define SPI_OK		0x00
#define SPI_ERROR	(unsigned long)(-1)

/**
 * At CPOL=0 the base value of the clock is zero
 *  - For CPHA=0, data are captured on the clock's rising edge (low→high transition)
 *    and data are propagated on a falling edge (high→low clock transition).
 *  - For CPHA=1, data are captured on the clock's falling edge and data are
 *    propagated on a rising edge.
 * At CPOL=1 the base value of the clock is one (inversion of CPOL=0)
 *  - For CPHA=0, data are captured on clock's falling edge and data are propagated
 *    on a rising edge.
 *  - For CPHA=1, data are captured on clock's rising edge and data are propagated
 *    on a falling edge.
 */
#define SPI_LSB 		(0 << 2) /* bit[2]: 0-LSB */
#define SPI_MSB 		(1 << 2) /* bit[2]: 1-MSB */

#define SPI_MASTER 		(0 << 3) /* SPI master device */
#define SPI_SLAVE 		(1 << 3)  /* SPI slave device */

#define SPI_MODE_0 		(0 | 0)				 /* CPOL = 0, CPHA = 0 */
#define SPI_MODE_1 		(0 | SPI_CPHA)		 /* CPOL = 0, CPHA = 1 */
#define SPI_MODE_2 		(SPI_CPOL | 0)		 /* CPOL = 1, CPHA = 0 */
#define SPI_MODE_3 		(SPI_CPOL | SPI_CPHA) /* CPOL = 1, CPHA = 1 */

#define SPI_MODE_MASK (SPI_CPHA | SPI_CPOL | SPI_MSB)

#define SPI_CS_HIGH 	(1 << 4) /* Chipselect active high */
#define SPI_NO_CS 		(1 << 5)   /* No chipselect */
#define SPI_3WIRE 		(1 << 6)   /* SI/SO pin shared */
#define SPI_READY 		(1 << 7)   /* Slave pulls low to pause */

/**
 * SPI message structure
 */
typedef struct
{
	const void *send_buf;
	void *recv_buf;
	uint32_t length;
	
	uint8_t cs_take;
	uint8_t cs_release;
	uint16_t timeout;
	uint8_t flag;
}spi_message_t;

/**
 * SPI configuration structure
 */
typedef struct 
{
	uint8_t mode;
	uint8_t data_width;
	uint16_t reserved;

	uint32_t max_freq;
}spi_conf_t;

struct spi_ops;

typedef struct
{
	const struct spi_ops *ops;
    uint16_t id;
}spi_bus_t;

typedef struct 
{
	spi_bus_t *bus;
	spi_conf_t *config;
	void *device;
}spi_device_t;

typedef spi_device_t * spi_device_handle_t;

typedef struct spi_ops
{
	void (*cs_take)(void);
	void (*cs_reslease)(void);
}spi_ops_t;

/*
	【SPI时钟最快是2分频，不支持不分频】
	如果是SPI1，2分频时SCK时钟 = 42M，4分频时SCK时钟 = 21M
	如果是SPI3, 2分频时SCK时钟 = 21M
*/
#define SPI_SPEED_42M SPI_BaudRatePrescaler_2
#define SPI_SPEED_21M SPI_BaudRatePrescaler_4
#define SPI_SPEED_5_2M SPI_BaudRatePrescaler_8
#define SPI_SPEED_2_6M SPI_BaudRatePrescaler_16
#define SPI_SPEED_1_3M SPI_BaudRatePrescaler_32
#define SPI_SPEED_0_6M SPI_BaudRatePrescaler_64

// void bsp_init_SPI_bus(void);

// void bsp_spiWrite0(uint8_t _ucByte);
// uint8_t bsp_spiRead0(void);

// void bsp_spiWrite1(uint8_t _ucByte);
// uint8_t bsp_spiRead1(void);

// uint8_t bsp_SpiBusBusy(void);

// void bsp_SPI_Init(uint16_t _cr1);

// void bsp_SpiBusEnter(void);
// void bsp_SpiBusExit(void);
// uint8_t bsp_SpiBusBusy(void);
// void bsp_SetSpiSck(uint8_t _data);

void spi_bus_take(void);
void spi_bus_release(void);
uint8_t spi_get_bus_busy(void);
uint8_t spi_configure(spi_device_handle_t _spi_device_handle);
uint8_t spi_send_then_send(spi_device_handle_t _spi_device_handle,
                         const void           *send_buf1,
                         uint32_t             send_length1,
                         const void           *send_buf2,
                         uint32_t             send_length2);
uint8_t spi_send_then_recv(spi_device_handle_t _spi_device_handle,
                               const void           *send_buf,
                               uint32_t             send_length,
                               void                 *recv_buf,
                               uint32_t             recvl_ength);
uint8_t spi_transfer(spi_device_handle_t _spi_device_handle,
                          const void           *send_buf,
                          void                 *recv_buf,
                          uint32_t             Llength);		
						  					   
#endif

