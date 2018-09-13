#include "CalCrc.h"

#ifndef uint8_t
#define uint8_t unsigned char
#endif
#ifndef uint16_t
#define uint16_t unsigned short int
#endif
#ifndef uint32_t
#define uint32_t unsigned int
#endif
#ifndef __IO
#define __IO volatile
#endif

uint8_t CalCrc8_Maxim(void *pBuf, uint16_t pLen)
{
    uint8_t *PData = (uint8_t *)pBuf;

    __IO uint8_t i;
    __IO uint8_t pCrcValue = 0;

    while (pLen--)
    {
        pCrcValue ^= *PData++;

        for (i = 0; i < 8; i++)
        {
            if (pCrcValue & 0x01)
            {
                pCrcValue = (pCrcValue >> 1) ^ 0x8c;
            }
            else
            {
                pCrcValue = pCrcValue >> 1;
            }
        }
    }

    return pCrcValue;
}