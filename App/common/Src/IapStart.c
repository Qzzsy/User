#include "IapStart.h"
#include "Bsp_IntFlash.h"

#define FIRM_INFO_BASE_ADDR 0x08004000

void IapStartInit()
{
    IntFlashInit();
}

void SetUpdateFlag()
{
    uint8_t Buf = 0;
    IntFlashWrite(FIRM_INFO_BASE_ADDR + 16 * 1024 - 14, (uint8_t *)&Buf, 1);
}

void SysReset()
{
    __disable_irq();
    NVIC_SystemReset();
}

