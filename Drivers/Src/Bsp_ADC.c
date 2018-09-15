/**
 ******************************************************************************
 * @file      Bsp_ADC.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-09-14
 * @brief     该文件提供了ADC操作相关的API，使得底层与应用层更加分离
 * @History
 * Date           Author    version    		Notes
 * 2018-09-14       ZSY     V1.0.0      first version.
 */

/* Includes ------------------------------------------------------------------*/
#include "Bsp_ADC.h"
#include "adc.h"

/*!< 定义是否使用对应的ADC */
#define USE_ADC1
#define USE_ADC2

#ifdef USE_ADC1
/**
 * @func    GetAdc1Handle
 * @brief   获取ADC1的句柄
 * @retval  ADC1的句柄
 */
ADC_HandleTypeDef *GetAdc1Handle(void)
{
    return &hadc1;
}
#endif

#ifdef USE_ADC2
/**
 * @func    GetAdc2Handle
 * @brief   获取ADC2的句柄
 * @retval  ADC2的句柄
 */
ADC_HandleTypeDef *GetAdc2Handle(void)
{
    return &hadc2;
}
#endif

/**
 * @func    SetAdcConvChannel
 * @brief   设置ADC的转换通道
 * @param   _Handle ADC的句柄
 * @param   Channel 通道
 * @param   SamplingTime 转换时间
 * @retval  HAL_OK
 */
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
    return HAL_OK;
}

/**
 * @func    GetAdcValue
 * @brief   获取ADC的转换值
 * @param   _Handle ADC的句柄
 * @retval  ADC采集到的值
 */
uint16_t GetAdcValue(ADC_HandleTypeDef *_Handle)
{
    uint32_t State = HAL_ERROR;
    uint32_t Value;
    
    _Handle->ErrorCode = HAL_OK;

    _Handle->ErrorCode = HAL_ADC_Start(_Handle);

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
