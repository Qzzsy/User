#ifndef _ZU_CONFIG_H_
#define _ZU_CONFIG_H_

#define ZU_NAME_MAX         16

/* 定义使用的串口端口 */
#define USE_USART1
//#define USE_UART2
#define USE_CONSOLE
#define CONSOLE_DEVICE      "usart1"    

#define RECV_BUF_SIZE   8192

/* 开辟4K的空间用于发送字符串 */
#define PRINTF_SEND_BUF_SIZE        1024

#endif

