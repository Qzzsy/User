/**
 ******************************************************************************
 * @file      BatVoltage.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-09-14
 * @brief     实现了电池电压的获取
 * @History
 * Date           Author    version    		Notes
 * 2018-09-14       ZSY     V1.0.0      first version.
 */

/* Includes ------------------------------------------------------------------*/
#include "BatVoltage.h"
#include "Bsp_ADC.h"
#include "GUIDr.h"

/*!< 是否使用内部基准电压 */
#define USE_VREFINT

#ifdef USE_VREFINT
/*!< 内部基准电压的大小（mV） */
#define VREFINT_VOLTAGE 1185
#else
#define VCC_VOLTAGE 3300
#endif

/*!< 测量电池电压的ADC 通道 */
#define BAT_ADC_CHANNEL ADC_CHANNEL_9

uint16_t FilterBuf[30] = {0};
uint16_t Filter(uint16_t input)
{
    uint32_t i = 0, j = 0, arvg = 0, buff = 0;
    uint16_t FilterMirror[30] = {0};
    for (i = 29; i > 0; i--)
    {
        FilterBuf[i] = FilterBuf[i - 1];
    }

    FilterBuf[0] = input;

    for (i = 0; i < 30; i++)
    {
        FilterMirror[i] = FilterBuf[i];
    }

    for (i = 0; i < 29; i++)
    {
        for (j = i + 1; j < 30; j++)
        {
            if (FilterMirror[i] < FilterMirror[j])
            {
                buff = FilterMirror[i];
                FilterMirror[i] = FilterMirror[j];
                FilterMirror[j] = buff;
            }
        }
    }

    for (i = 2; i < 27; i++)
    {
        arvg += FilterMirror[i];
    }

    return (arvg / 25.0);
}

/**
 * @func    GetRealVol
 * @brief   获取当前真实的电压
 * @param   AdcVol 测量到的电压
 * @retval  返回真实的电压
 */
uint32_t GetRealVol(uint16_t AdcVol)
{
    return AdcVol * (22.0 / 39.0) + 2495;
}

/**
 * @func    GetBatVoltage
 * @brief   获取ADC的电压
 * @retval  返回测量得到的电压
 */
uint16_t GetBatVoltage(void)
{
    uint16_t Voltage;
    static uint32_t AdcVal = 0, vAdcVal = 0;

    ADC_HandleTypeDef *_hAdc;

    _hAdc = GetAdc1Handle();

#ifdef USE_VREFINT
    SetAdcConvChannel(_hAdc, ADC_CHANNEL_VREFINT, ADC_SAMPLETIME_239CYCLES_5);
    vAdcVal = GetAdcValue(_hAdc);
#endif

    SetAdcConvChannel(_hAdc, BAT_ADC_CHANNEL, ADC_SAMPLETIME_239CYCLES_5);
    AdcVal = GetAdcValue(_hAdc);

#ifdef USE_VREFINT
    Voltage = VREFINT_VOLTAGE * ((double)AdcVal / vAdcVal);
#else
    Voltage = ((uint32_t)VCC_VOLTAGE * AdcVal) >> 12;
#endif

    return (uint16_t)GetRealVol(Voltage);
}

void BatElecDisp(void)
{
    static uint8_t _FirstDispFlag = true, Cnt = 0, LastDispSize = 0;
    __IO uint16_t _BatVolgate = 0, _BatVolgateFir = 0;
    uint8_t DispSize = 0;

    if (_FirstDispFlag == true)
    {
        _FirstDispFlag = false;

        GuiDrawRectRound(73, 3, 20, 10, 0xffff, 3);
        GuiDrawFillRectRound(92, 6, 3, 4, 0xffff, 1);
    }

    _BatVolgate = GetBatVoltage();
    _BatVolgateFir = Filter(_BatVolgate);

    if (_BatVolgate >= 4150)
    {
        _BatVolgate = 4150;
    }
    else if (_BatVolgate <= 3400)
    {
        _BatVolgate = 3400;
    }

    if (_BatVolgateFir >= 4150)
    {
        _BatVolgateFir = 4150;
    }
    else if (_BatVolgateFir <= 3400)
    {
        _BatVolgateFir = 3400;
    }

    if (Cnt >= 100)
    {
        DispSize = (18.0 / 750.0) * (_BatVolgateFir - 3400);
        
        if (DispSize < LastDispSize)
        {
            GuiDrawFillRect(74 + DispSize - 1, 4, LastDispSize - DispSize + 1, 8, 0x0000);
            GuiDrawRectRound(73, 3, 20, 10, 0xffff, 3);
            GuiDrawFillRectRound(92, 6, 3, 4, 0xffff, 1);
        }
        if (_BatVolgateFir >= 3700)
        {
            GuiDrawFillRectRound(74, 4, DispSize, 8, 0x07e0, 1);
        }
        else
        {
            GuiDrawFillRectRound(74, 4, DispSize, 8, 0xfc00, 1);
        }
    }
    else
    {
        DispSize = (18.0 / 750.0) * (_BatVolgate - 3400);

        if (DispSize < LastDispSize)
        {
            GuiDrawFillRect(74 + DispSize - 1, 4, LastDispSize - DispSize + 1, 8, 0x0000);
            GuiDrawRectRound(73, 3, 20, 10, 0xffff, 3);
            GuiDrawFillRectRound(92, 6, 3, 4, 0xffff, 1);
        }
        if (_BatVolgate >= 3700)
        {
            GuiDrawFillRectRound(74, 4, DispSize, 8, 0x07e0, 1);
        }
        else
        {
            GuiDrawFillRectRound(74, 4, DispSize, 8, 0xfc00, 1);
        }
    }

    LastDispSize = DispSize;

    if (Cnt < 100)
        Cnt++;
}
