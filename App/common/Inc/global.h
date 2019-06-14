#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "zu_config.h"
#include "zu_def.h"
#include "bsp_uart.h"

zu_serial_t usart1_dev;
uint8_t usart1_recv_buf[RECV_BUF_SIZE];

#endif

