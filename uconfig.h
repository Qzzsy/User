#ifndef _ZU_CONFIG_H_
#define _ZU_CONFIG_H_

#define STM32F1
#define OBJECT_NAME_MAX         16

#define PROJECT_NAME            "DY-STCPLOCK-RT"
#define DEVICE_FIRM_LEVEL       "level1"
#define DEVICE_CHIP_NAME        "STM32F103VCT6"

#define VERSION                 "1.0.0"

#define MAJVERSION                   1L              /* major version number */
#define SUBVERSION                   0L              /* minor version number */
#define REVISION                     0L              /* revise version number */

/* �����Ƿ�ʹ���Զ������������ */
#define USING_USER_BASE_TYPE

/* ����Ƭ��ROM��RAM�Ĵ�С�Լ�������ַ */
#define USING_ON_CHIP_FLASH
#define ONCHIP_FLASH_START_ADDRESS  0x08000000
#define ONCHIP_FLASH_SIZE           0x00040000
#define ONCHIP_FLASH_END_ADDRESS    0x08040000

#define ONCHIP_RAM_START_ADDRESS    0x20000000
#define ONCHIP_RAM_SIZE             0x0000c000

/* ����ʹ�õĴ��ڶ˿� */
#define USING_USART1
//#define USING_USART_DMA
//#define USING_USART1_DMA_RX
//#define USE_UART2
#define USING_CONSOLE
#define CONSOLE_DEVICE      "usart1"    
#define RECV_BUF_SIZE       16

/* ����1K�Ŀռ����ڷ����ַ��� */
#define PRINTF_SEND_BUF_SIZE        256

/* �̼���Ϣ��ַ��Ƭ�� */
#define FIRM_INFO_BASE_ADDR 0x08008000
/* ���������ⲿ��FLASH  256K-512K��οռ��� */
#define FIRM_DATA_BASE_ADDR 0x002000
#define FIRM_DATA_END_ADDR  0x046000
#define FIRM_HEAD_INFO_SIZE 512
#define FIRM_RECV_BUF_SIZE  2048     

#endif

