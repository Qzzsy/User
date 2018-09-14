/**
 ******************************************************************************
 * @file      Bsp_TM1639.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-09-14
 * @brief     ���ļ��ṩ��TM1639������ص������Լ�API
 * @History
 * Date           Author    version    		Notes
 * 2018-09-14       ZSY     V1.0.0      first version.
 */

/* Includes ------------------------------------------------------------------*/
#include "Bsp_TM1639.h"

/* ��ʹ��LL�⣬�붨��USER_EFFIΪ1 */
#ifndef USER_EFFI
#define USER_EFFI 0
#endif

#define TM1639_CLK_PIN      TM1639_CLK_Pin
#define TM1639_DIO_PIN      TM1639_DIO_Pin  
#define TM1639_CS_PIN       TM1639_CS_Pin

#define TM1639_CLK_PORT	    TM1639_CLK_GPIO_Port
#define TM1639_DIO_PORT	    TM1639_DIO_GPIO_Port
#define TM1639_CS_PORT	    TM1639_CS_GPIO_Port

#if USER_EFFI == 1
#define TM1639_CLK_WL       LL_GPIO_ResetOutputPin(TM1639_CLK_PORT, TM1639_CLK_PIN)
#define TM1639_CLK_WH       LL_GPIO_SetOutputPin(TM1639_CLK_PORT, TM1639_CLK_PIN)
#define TM1639_DIO_WL       LL_GPIO_ResetOutputPin(TM1639_DIO_PORT, TM1639_DIO_PIN)  
#define TM1639_DIO_WH       LL_GPIO_SetOutputPin(TM1639_DIO_PORT, TM1639_DIO_PIN)
#define TM1639_CS_WL        LL_GPIO_ResetOutputPin(TM1639_CS_PORT, TM1639_CS_PIN)
#define TM1639_CS_WH        LL_GPIO_SetOutputPin(TM1639_CS_PORT, TM1639_CS_PIN)

#define TM1639_DIO_R        LL_GPIO_IsInputPinSet(TM1639_DIO_PORT, TM1639_DIO_PIN)
#elif USER_EFFI == 0
#define TM1639_CLK_WL       HAL_GPIO_WritePin(TM1639_CLK_PORT, TM1639_CLK_PIN, GPIO_PIN_RESET)
#define TM1639_CLK_WH       HAL_GPIO_WritePin(TM1639_CLK_PORT, TM1639_CLK_PIN, GPIO_PIN_SET)
#define TM1639_DIO_WL       HAL_GPIO_WritePin(TM1639_DIO_PORT, TM1639_DIO_PIN, GPIO_PIN_RESET)  
#define TM1639_DIO_WH       HAL_GPIO_WritePin(TM1639_DIO_PORT, TM1639_DIO_PIN, GPIO_PIN_SET) 
#define TM1639_CS_WL        HAL_GPIO_WritePin(TM1639_CS_PORT, TM1639_CS_PIN, GPIO_PIN_RESET)
#define TM1639_CS_WH        HAL_GPIO_WritePin(TM1639_CS_PORT, TM1639_CS_PIN, GPIO_PIN_SET)

#define TM1639_DIO_R        HAL_GPIO_ReadPin(TM1639_DIO_PORT, TM1639_DIO_PIN)
#endif

/*!< �����Զ���ַ��һ�ķ�ʽд��ʾ���� */
#define CMD_AUTO_ADDR       0x40

/*!< ���ù̶���ģʽд���� */
#define CMD_STATIONARY_ADDR 0x44

#define CMD_WRITE_DISP      0X40
#define CMD_READ_KEY        0X42

/*!< ��ʼ��ַ */
#define CMD_START_ADDR      0xC0

#define CMD_DISP_ON         0x81
#define CMD_DISP_OFF        0x80

#if defined STM32F1
/* IO�������� */
#define SET_TM1639_DIO_IN()  {TM1639_DIO_PORT->CRL &= 0X0FFFFFFF; TM1639_DIO_PORT->CRL |= (uint32_t)8 << 28;}
#define SET_TM1639_DIO_OUT() {TM1639_DIO_PORT->CRL &= 0X0FFFFFFF; TM1639_DIO_PORT->CRL |= (uint32_t)3 << 28;}
#elif defined STM32F4
/* IO�������� */
#define SET_TM1639_DIO_IN()  {TM1639_DIO_PORT->MODER &= ~(3 << (5 * 2)); TM1639_DIO_PORT->MODER |= (0 << (5 * 2));}
#define SET_TM1639_DIO_OUT() {TM1639_DIO_PORT->MODER &= ~(3 << (5 * 2)); TM1639_DIO_PORT->MODER |= (1 << (5 * 2));} 
#endif

const uint8_t Dofly[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x58, 0x5e, 0x79, 0x71};

/**
 * @func    TM1639_SendData
 * @brief   ��TM1639��������
 * @param   Data Ҫ���͵�����
 * @retval  ��
 */
void TM1639_SendData(uint8_t Data)
{
    uint8_t i;

    SET_TM1639_DIO_OUT();

    for (i = 0; i < 8; i++)
    {
        TM1639_CLK_WL;

        if ((Data >> i) & 0x01)
            TM1639_DIO_WH;
        else
            TM1639_DIO_WL;
        
        for (int j = 10; j > 0; j--);

        TM1639_CLK_WH;
        
        for (int j = 10; j > 0; j--);
    }
}

/**
 * @func    TM1639_ReadData
 * @brief   ��TM1639�ж�ȡ����
 * @retval  ��ȡ��������
 */
uint8_t TM1639_ReadData(void)
{
    uint8_t i, RevData = 0x00;

    SET_TM1639_DIO_IN();

    for (i = 0; i < 8; i++)
    {
        TM1639_CLK_WL;
        
        for (int j = 10; j > 0; j--);
        TM1639_CLK_WH;

        if (TM1639_DIO_R)
            RevData |= 1 << i;
        
        for (int j = 10; j > 0; j--);
    }

    return RevData;
}

/**
 * @func    TM1639_Start
 * @brief   ʹ��TM1639
 * @retval  ��
 */
void TM1639_Start(void)
{
    TM1639_CS_WL;

    TM1639_CLK_WH;
}

/**
 * @func    TM1639_Stop
 * @brief   ʧ��TM1639
 * @retval  ��
 */
void TM1639_Stop(void)
{
    TM1639_CS_WH;
    
    TM1639_CLK_WH;
}

/**
 * @func    TM1639_Control
 * @brief   ��TM1639д�����ź�
 * @param   Cmd ����
 * @retval  ��
 */
void TM1639_Control(uint8_t Cmd)
{
    TM1639_Start();

    TM1639_SendData(Cmd);

    TM1639_Stop();
}

/**
 * @func    TM1639_DispOn
 * @brief   TM1639����ʾ
 * @retval  ��
 */
void TM1639_DispOn(void)
{
    TM1639_Control(CMD_DISP_ON);
}

/**
 * @func    TM1639_DispOff
 * @brief   TM1639�ر���ʾ
 * @retval  ��
 */
void TM1639_DispOff(void)
{
    TM1639_Control(CMD_DISP_OFF);
}

/**
 * @func    TM1639_Disp
 * @brief   TM1639ˢ���������ʾ������
 * @param   Data ���ݵ�ָ��
 * @param   Mode ��ʾ��ģʽ
 * @retval  ��
 */
void TM1639_Disp(uint8_t * Data, uint8_t Mode)
{
    uint8_t i;

    __disable_fault_irq();

    TM1639_Control(CMD_AUTO_ADDR);

    TM1639_Start();

    if (Mode == TM_MODE_DISP_DIG)
    {
        TM1639_SendData(DIG0);

        for (int j = 10; j > 0; j--);
        for (i = 0; i < 2; i++)
        {
            TM1639_SendData(Dofly[Data[i]] & 0x0f);

            TM1639_SendData(((Dofly[Data[i]]) >> 4) & 0x0f);
        }
    }
    else if (Mode == TM_MODE_DISP_LED)
    {
        TM1639_SendData(DIG2);

        for (int j = 10; j > 0; j--);
        TM1639_SendData((*Data) & 0x0f);
        
        TM1639_SendData(((*Data) >> 4) & 0x0f);
    }

    TM1639_Stop();

    TM1639_Control(LEVEL_1);
    
    __enable_fault_irq();
}

/**
 * @func    TM1639_ReadKeyValue
 * @brief   ��ȡ��������Чֵ
 * @param   KeyValue ����ֵ
 * @retval  ��
 */
void TM1639_ReadKeyValue(uint32_t * KeyValue)
{
    uint8_t * Temp = (uint8_t *)KeyValue;

    __disable_fault_irq();

    TM1639_Control(CMD_AUTO_ADDR);

    TM1639_Start();

    TM1639_SendData(CMD_READ_KEY);

    for (int j = 10; j > 0; j--);
    
    Temp[0] = TM1639_ReadData();
    Temp[1] = TM1639_ReadData();
    
    TM1639_Stop();

    __enable_fault_irq();
}

