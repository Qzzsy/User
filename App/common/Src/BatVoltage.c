#include "BatVoltage.h"
#include "Bsp_ADC.h"

#define USE_VREFINT

#ifdef USE_VREFINT
#define VREFINT_VOLTAGE         1200
#else
#define VCC_VOLTAGE             3300
#endif

#define BAT_ADC_CHANNEL         ADC_CHANNEL_9

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

uint32_t GetRealVol(uint16_t AdcVol)
{
    return AdcVol * (22.0 / 39.0) + 2500;
}

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
