#include "stm32f4xx.h"
#include "zu_string.h"

#define SRAM_BANK3
#define CHECK_BYTE_TYPE
#define CHECK_HALFWORD_TYPE
#define CHECK_WORD_TYPE
#define MEMORY_SIZE             (1 * 1024 * 1024)

#if defined (SRAM_BANK1)
#define SRAM_ADDRESS_START     (0x60000000)
#elif defined (SRAM_BANK2)
#define SRAM_ADDRESS_START     (0x64000000)
#elif defined (SRAM_BANK3)
#define SRAM_ADDRESS_START     (0x68000000)
#elif defined (SRAM_BANK4)
#define SRAM_ADDRESS_START     (0x6C000000)
#endif

#ifdef CHECK_BYTE_TYPE
uint8_t mem_check_byte_type(void)
{
    __IO uint32_t i;
    __IO uint8_t * ptr = (uint8_t *)SRAM_ADDRESS_START, tmp1, tmp2;
    zu_printf("test byte w/r.\r\nmemory test no.1\r\n");
    for (i = 0; i < MEMORY_SIZE; i++)
    {
        tmp2 = (i % 256);
        *ptr = tmp2;
        ptr++;
    }
    ptr = (uint8_t *)SRAM_ADDRESS_START;
    for (i = 0; i < MEMORY_SIZE; i++)
    {
        tmp1 = *ptr;
        tmp2 = (i % 256);
        if (tmp1 != tmp2)
        {
            zu_printf("at 0x%p:current value is 0x%02x, now it`s 0x%02x\r\n", ptr, tmp2, tmp1);
        }
        
        ptr++;
    }
    
    ptr = (uint8_t *)SRAM_ADDRESS_START;
    zu_printf("memory test no.2\r\n");
    for (i = 0; i < MEMORY_SIZE; i++)
    {
        tmp2 = ~((uint8_t)(i & 0xff));
        *ptr = tmp2;
        ptr++;
    }
    ptr = (uint8_t *)SRAM_ADDRESS_START;
    for (i = 0; i < MEMORY_SIZE; i++)
    {
        tmp1 = *ptr;
        tmp2 = ~(i % 256);
        if (tmp1 != tmp2)
        {
            zu_printf("at 0x%p:current value is 0x%02x, now it`s 0x%02x\r\n", ptr, tmp2, tmp1);
        }
        
        ptr++;
    }
    
    zu_printf("memory test no.3\r\n");
    ptr = (uint8_t *)SRAM_ADDRESS_START;
    for (i = 0; i < MEMORY_SIZE; i++)
    {
        *ptr = 0xaa;
        ptr++;
    }    
    ptr = (uint8_t *)SRAM_ADDRESS_START;
    for (i = 0; i < MEMORY_SIZE; i++)
    {
        tmp1 = *ptr;
        if (tmp1 != 0xaa)
        {
            zu_printf("at 0x%p:current value is 0x%02x, now it`s 0x%02x\r\n", ptr, 0xaa, tmp1);
        }
        
        ptr++;
    }
    
    zu_printf("memory test no.4\r\n");
    ptr = (uint8_t *)SRAM_ADDRESS_START;
    for (i = 0; i < MEMORY_SIZE; i++)
    {
        *ptr = 0x55;
        ptr++;
    }    
    ptr = (uint8_t *)SRAM_ADDRESS_START;
    for (i = 0; i < MEMORY_SIZE; i++)
    {
        tmp1 = *ptr;
        if (tmp1 != 0x55)
        {
            zu_printf("at 0x%p:current value is 0x%02x, now it`s 0x%02x\r\n", ptr, 0x55, tmp1);
        }
        
        ptr++;
    }
    zu_printf("byte test complete\r\n");
    return 0;
}
#endif


#ifdef CHECK_HALFWORD_TYPE
uint8_t mem_check_halfword_type(void)
{
    __IO uint32_t i;
    __IO uint16_t * ptr = (uint16_t *)SRAM_ADDRESS_START, tmp1, tmp2;
    zu_printf("test halfword w/r.\r\nmemory test no.1\r\n");
    for (i = 0; i < (MEMORY_SIZE >> 1); i++)
    {
        tmp2 = (i & 0xffff);
        *ptr = tmp2;
        ptr++;
    }
    ptr = (uint16_t *)SRAM_ADDRESS_START;
    for (i = 0; i < (MEMORY_SIZE >> 1); i++)
    {
        tmp1 = *ptr;
        tmp2 = (i & 0xffff);
        if (tmp1 != tmp2)
        {
            zu_printf("at 0x%p:current value is 0x%04x, now it`s 0x%04x\r\n", ptr, tmp2, tmp1);
        }
        
        ptr++;
    }
    
    ptr = (uint16_t *)SRAM_ADDRESS_START;
    zu_printf("memory test no.2\r\n");
    for (i = 0; i < (MEMORY_SIZE >> 1); i++)
    {
        tmp2 = ~((uint16_t)(i & 0xffff));
        *ptr = tmp2;
        ptr++;
    }
    ptr = (uint16_t *)SRAM_ADDRESS_START;
    for (i = 0; i < (MEMORY_SIZE >> 1); i++)
    {
        tmp1 = *ptr;
        tmp2 = ~(i & 0xffff);
        if (tmp1 != tmp2)
        {
            zu_printf("at 0x%p:current value is 0x%04x, now it`s 0x%04x\r\n", ptr, tmp2, tmp1);
        }
        
        ptr++;
    }
    
    zu_printf("memory test no.3\r\n");
    ptr = (uint16_t *)SRAM_ADDRESS_START;
    for (i = 0; i < (MEMORY_SIZE >> 1); i++)
    {
        *ptr = 0xaaaa;
        ptr++;
    }    
    ptr = (uint16_t *)SRAM_ADDRESS_START;
    for (i = 0; i < (MEMORY_SIZE >> 1); i++)
    {
        tmp1 = *ptr;
        if (tmp1 != 0xaaaa)
        {
            zu_printf("at 0x%p:current value is 0x%04x, now it`s 0x%04x\r\n", ptr, 0xaaaa, tmp1);
        }
        
        ptr++;
    }
    
    zu_printf("memory test no.4\r\n");
    ptr = (uint16_t *)SRAM_ADDRESS_START;
    for (i = 0; i < (MEMORY_SIZE >> 1); i++)
    {
        *ptr = 0x5555;
        ptr++;
    }    
    ptr = (uint16_t *)SRAM_ADDRESS_START;
    for (i = 0; i < (MEMORY_SIZE >> 1); i++)
    {
        tmp1 = *ptr;
        if (tmp1 != 0x5555)
        {
            zu_printf("at 0x%p:current value is 0x%04x, now it`s 0x%04x\r\n", ptr, 0x5555, tmp1);
        }
        
        ptr++;
    }
    zu_printf("halfword test complete\r\n");
    return 0;
}
#endif

#ifdef CHECK_WORD_TYPE
uint8_t mem_check_word_type(void)
{
    __IO uint32_t i;
    __IO uint32_t * ptr = (uint32_t *)SRAM_ADDRESS_START, tmp1, tmp2;
    zu_printf("test word w/r.\r\nmemory test no.1\r\n");
    for (i = 0; i < (MEMORY_SIZE >> 2); i++)
    {
        *ptr = i;
        ptr++;
    }
    ptr = (uint32_t *)SRAM_ADDRESS_START;
    for (i = 0; i < (MEMORY_SIZE >> 2); i++)
    {
        tmp1 = *ptr;
        if (tmp1 != i)
        {
            zu_printf("at 0x%p:current value is 0x%08x, now it`s 0x%08x\r\n", ptr, i, tmp1);
        }
        
        ptr++;
    }
    
    ptr = (uint32_t *)SRAM_ADDRESS_START;
    zu_printf("memory test no.2\r\n");
    for (i = 0; i < (MEMORY_SIZE >> 2); i++)
    {
        tmp1 = ~i;
        *ptr = tmp1;
        ptr++;
    }
    ptr = (uint32_t *)SRAM_ADDRESS_START;
    for (i = 0; i < (MEMORY_SIZE >> 2); i++)
    {
        tmp1 = *ptr;
        if (tmp1 != (~i))
        {
            zu_printf("at 0x%p:current value is 0x%08x, now it`s 0x%08x\r\n", ptr, (~i), tmp1);
        }
        
        ptr++;
    }
    
    zu_printf("memory test no.3\r\n");
    ptr = (uint32_t *)SRAM_ADDRESS_START;
    for (i = 0; i < (MEMORY_SIZE >> 2); i++)
    {
        *ptr = 0xaa55aa55;
        ptr++;
    }    
    ptr = (uint32_t *)SRAM_ADDRESS_START;
    for (i = 0; i < (MEMORY_SIZE >> 2); i++)
    {
        tmp1 = *ptr;
        if (tmp1 != 0xaa55aa55)
        {
            zu_printf("at 0x%p:current value is 0x%08x, now it`s 0x%08x\r\n", ptr, 0xaa55aa55, tmp1);
        }
        
        ptr++;
    }
    
    zu_printf("memory test no.4\r\n");
    ptr = (uint32_t *)SRAM_ADDRESS_START;
    for (i = 0; i < (MEMORY_SIZE >> 2); i++)
    {
        *ptr = 0x55aa55aa;
        ptr++;
    }    
    ptr = (uint32_t *)SRAM_ADDRESS_START;
    for (i = 0; i < (MEMORY_SIZE >> 2); i++)
    {
        tmp1 = *ptr;
        if (tmp1 != 0x55aa55aa)
        {
            zu_printf("at 0x%p:current value is 0x%08x, now it`s 0x%08x\r\n", ptr, 0x55aa55aa, tmp1);
        }
        
        ptr++;
    }
    zu_printf("halfword test complete\r\n");
    return 0;
}
#endif

void mem_check(void)
{
    mem_check_byte_type();
    mem_check_halfword_type();
    mem_check_word_type();
}



