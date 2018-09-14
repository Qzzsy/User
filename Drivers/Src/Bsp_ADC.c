#include "Bsp_ADC.h"
#include "adc.h"

#define USE_ADC1
#define USE_ADC2

#ifdef USE_ADC1
ADC_HandleTypeDef *GetAdc1Handle(void)
{
    return &hadc1;
}
#endif

#ifdef USE_ADC2
ADC_HandleTypeDef *GetAdc2Handle(void)
{
    return &hadc2;
}
#endif

uint8_t SetAdcConvChannel(ADC_HandleTypeDef * _Handle, uint32_t Channel, uint32_t SamplingTime)
{
    ADC_ChannelConfTypeDef sConfig;
    sConfig.Channel = Channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = SamplingTime;
    if (HAL_ADC_ConfigChannel(_Handle, &sConfig) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }
}

uint16_t GetAdcValue(ADC_HandleTypeDef *_Handle)
{
    uint32_t State = HAL_ERROR;
    uint32_t Value;

    HAL_ADC_Start(_Handle);

    State = HAL_ADC_PollForConversion(_Handle, 1000);
    if (State != HAL_OK)
    {
        _Handle->ErrorCode = HAL_ERROR;
        return (uint16_t)(-1);
    }

    State = HAL_ADC_GetState(_Handle);
    if (State & HAL_ADC_STATE_REG_EOC)
    {
        Value = HAL_ADC_GetValue(_Handle);
        _Handle->ErrorCode = HAL_OK;
        return Value;
    }

    _Handle->ErrorCode = HAL_ERROR;
    return (uint16_t)(-1);
}
