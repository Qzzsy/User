/**
 ******************************************************************************
 * @file      bsp_timer.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-09-14
 * @brief     ���ļ��ṩ��Timer������ص�API��ʹ�õײ���Ӧ�ò���ӷ���
 * @History
 * Date           Author    version    		Notes
 * 2018-09-14       ZSY     V1.0.0      first version.
 */

/* Includes ------------------------------------------------------------------*/
#include "bsp_timer.h"

/**
 * @func    LED_TickInc
 * @brief   LED_TickInc��������
 * @retval  ��
 */
__weak void LED_TickInc(void)
{
}

/**
 * @func    KeyScan
 * @brief   KeyScan��������
 * @retval  ��
 */
__weak void KeyScan(void)
{
}

/**
 * @func    TimerStart
 * @brief   ������ʱ�����ᴥ���ж�
 * @param   tim_baseHandle ��ʱ���ľ��
 * @retval  ��
 */
void TimerStartIT(TIM_HandleTypeDef * tim_baseHandle)
{
    HAL_TIM_Base_Start_IT(tim_baseHandle);
}

/**
 * @func    TimerStart
 * @brief   ������ʱ��
 * @param   tim_baseHandle ��ʱ���ľ��
 * @retval  ��
 */
void TimerStart(TIM_HandleTypeDef * tim_baseHandle)
{
    HAL_TIM_Base_Start(tim_baseHandle);
}

/**
 * @func    TimerStartPWM
 * @brief   ������Ӧͨ����PWM
 * @param   tim_baseHandle ��ʱ���ľ��
 * @param   Channel ͨ��
 * @retval  ��
 */
void TimerStartPWM(TIM_HandleTypeDef * tim_baseHandle, uint16_t Channel)
{
    HAL_TIM_PWM_Start(tim_baseHandle, Channel);
}

/**
 * @func    PwmSetCompare
 * @brief   ����PWM��ռ�ձ�
 * @param   tim_baseHandle ��ʱ���ľ��
 * @param   Channel ͨ��
 * @param   IndexWave ռ�ձ�
 * @retval  ��
 */
void PwmSetCompare(TIM_HandleTypeDef * tim_baseHandle, uint16_t Channel, uint16_t IndexWave)
{
    __HAL_TIM_SET_COMPARE(tim_baseHandle, Channel, IndexWave);
}

/**
 * @func    HAL_TIM_PeriodElapsedCallback
 * @brief   ��ʱ���Ļص�����
 * @param   tim_baseHandle ��ʱ���ľ��
 * @retval  ��
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * tim_baseHandle)
{
    if (tim_baseHandle->Instance == TIM1)
    {
        /* code */
        return ;
    }
    else if (tim_baseHandle->Instance == TIM2)
    {
        /* code */
        return ;
    }
    else if (tim_baseHandle->Instance == TIM3)
    {
        LED_TickInc();
        KeyScan();
        /* code */
        return ;
    }
    else if (tim_baseHandle->Instance == TIM4)
    {
        /* code */
        return ;
    }
    
    /*!< more Timer add in this; */
}
