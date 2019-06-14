#include "STM32F4xx.h"
#include "memory_check.h"
#include "bsp_uart.h"
#include "zu_string.h"
#include "global.h"

void UserMainFunc(void)
{
    zu_strncpy(usart1_dev.name, "usart1", 6);
    usart1_dev.mode = ZU_MODE_IDLE_INT;
    usart1_dev.usart_buf.recv_buf = usart1_recv_buf;
    usart1_dev.usart_buf.recv_buf_size = RECV_BUF_SIZE;
    usart_init(&usart1_dev);
    zu_set_console_device(console_out);
    
    //mem_check();
    while (true)
    {
        HAL_Delay(1000);
    }
}
