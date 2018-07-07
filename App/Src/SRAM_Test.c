#include "stm32f4xx.h"

#define SRAM_ADDR       0x68040000

uint32_t EXT_SRAM_SIZE = 0x100000;
///*
//*********************************************************************************************************
//*	�� �� ��: bsp_TestExtSRAM
//*	����˵��: ɨ������ⲿSRAM
//*	��    ��: ��
//*	�� �� ֵ: 0 ��ʾ����ͨ���� ����0��ʾ����Ԫ�ĸ�����
//*********************************************************************************************************
//*/
uint16_t data[4096] __attribute__((section(".ARM.__at_0x68040000"))) = {0};
uint8_t bsp_TestExtSRAM(void)
{
	uint32_t i;
	uint32_t *pSRAM;
    uint16_t *pSRAM16; 
	uint8_t *pBytes;
	uint32_t err;
	const uint8_t ByteBuf[4] = {0x55, 0xA5, 0x5A, 0xAA};

	/* дSRAM */
	pSRAM16 = (uint16_t *)SRAM_ADDR;
	for (i = 0; i < EXT_SRAM_SIZE / 4; i++)
	{
		*pSRAM16++ = i;
	}

	/* ��SRAM */
	err = 0;
	pSRAM16 = (uint16_t *)SRAM_ADDR;
	for (i = 0; i < EXT_SRAM_SIZE / 4; i++)
	{
		if (*pSRAM16++ != i)
		{
			err++;
		}
	}
	if (err >  0)
	{
		return  (4 * err);
	}

	/* ��SRAM �������󷴲�д�� */
	pSRAM = (uint32_t *)SRAM_ADDR;
	for (i = 0; i < EXT_SRAM_SIZE / 4; i++)
	{
		*pSRAM = ~*pSRAM;
		pSRAM++;
	}

	/* �ٴαȽ�SRAM������ */
	err = 0;
	pSRAM = (uint32_t *)SRAM_ADDR;
	for (i = 0; i < EXT_SRAM_SIZE / 4; i++)
	{
		if (*pSRAM++ != (~i))
		{
			err++;
		}
	}

	if (err >  0)
	{
		return (4 * err);
	}

	/* ���԰��ֽڷ�ʽ����, Ŀ������֤ FSMC_NBL0 �� FSMC_NBL1 ���� */
	pBytes = (uint8_t *)SRAM_ADDR;
	for (i = 0; i < sizeof(ByteBuf); i++)
	{
		*pBytes++ = ByteBuf[i];
	}

	/* �Ƚ�SRAM������ */
	err = 0;
	pBytes = (uint8_t *)SRAM_ADDR;
	for (i = 0; i < sizeof(ByteBuf); i++)
	{
		if (*pBytes++ != ByteBuf[i])
		{
			err++;
		}
	}
	if (err >  0)
	{
		return err;
	}
	return 0;
}
