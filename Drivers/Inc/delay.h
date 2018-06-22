/**
******************************************************************************
 * @Copyright       (C) 2017 - 2018 guet-sctc-hardwarepart Team
 * @filename        delay.h
 * @author          �Ž�����С��
 * @version         V1.1.2
 * @date            2018-06-19
 * @Description     delay�ļ����������Ƿ�ʹ��ϵͳ�ĺ궨��ѡ���Լ�ʹ��ϵͳ������
 * @Others
 * @History
 * Date           	Author    	version                 Notes
 * 2017-11-05     	ZSY       	V1.0.0              first version.
 * 2017-11-13       ZSY         V1.0.1              �����ṹ
 * 2018-01-11       ZSY         V1.0.2              �Ű��ʽ����������ǿ�ڲ�ͬ��������µĿɶ��ԣ�����HAL���֧��
 * 2018-01-11       ZSY         V1.0.3              �޸Ĳ��ֱ���������
 * 2018-06-07       ZSY         V1.1.0              �޸Ľṹ��ȫ���üĴ����ķ�ʽ��д�����ݼĴ����Լ�����⺯��
 * 2018-06-12       ZSY         V1.1.1              �޸Ĳ��ֱ������ƣ����ƶ�ȫϵ��оƬ��֧��
 * 2018-06-19       ZSY         V1.1.2              �޸��ڲ���Ƶ�������ms����ʱ��׼��BUG
 * @verbatim  
 */
	
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DELAY_H_
#define __DELAY_H_ 		

/* Includes ------------------------------------------------------------------*/
#if (defined STM32F10X_HD) || (defined STM32F10X_LD_VL) || (defined STM32F10X_MD_VL) || (defined STM32F10X_HD_VL) || (defined STM32F10X_XL) ||\
    (defined STM32F100xB) || (defined STM32F100xE) || (defined STM32F101x6) || \
    (defined STM32F101xB) || (defined STM32F101xE) || (defined STM32F101xG) || (defined STM32F102x6) || (defined STM32F102xB) || (defined STM32F103x6) || \
    (defined STM32F103xB) || (defined STM32F103xE) || (defined STM32F103xG) || (defined STM32F105xC) || (defined STM32F107xC)
#include "stm32f10x.h" 
#ifndef STM32F1
    #define STM32F1
#endif
#elif (defined STM32F405xx) || (defined STM32F415xx) || (defined STM32F407xx) || (defined STM32F417xx) || \
        (defined STM32F427xx) || (defined STM32F437xx) || (defined STM32F429xx) || (defined STM32F439xx) || \
        (defined STM32F401xC) || (defined STM32F401xE) || (defined STM32F410Tx) || (defined STM32F410Cx) || \
        (defined STM32F410Rx) || (defined STM32F411xE) || (defined STM32F446xx) || (defined STM32F469xx) || \
        (defined STM32F479xx) || (defined STM32F412Cx) || (defined STM32F412Rx) || (defined STM32F412Vx) || \
        (defined STM32F412Zx) || (defined STM32F413xx) || (defined STM32F423xx) ||\
        (defined STM32F40_41xxx) || (defined STM32F412xG) || (defined STM32F413_423xx) || (defined STM32F427_437xx) ||\
        (defined STM32F429_439xx) || (defined STM32F446xx) || (defined STM32F469_479xx)
#include <stm32f4xx.h>	
#ifndef STM32F4
    #define STM32F4
#endif
#elif defined (STM32F756xx) || defined (STM32F746xx) || defined (STM32F745xx) || defined (STM32F767xx) || \
    defined (STM32F769xx) || defined (STM32F777xx) || defined (STM32F779xx) || defined (STM32F722xx) || \
	defined (STM32F723xx) || defined (STM32F732xx) || defined (STM32F733xx)
#include "stm32f7xx.h"
#ifndef STM32F7
    #define STM32F7
#endif
#endif

/* ����оƬ���ں�Ƶ�ʸ��� */
#ifdef STM32F1
#define DEFAULT_SYSTEM_FREQUENCY            72000000U
#elif defined STM32F4
#define DEFAULT_SYSTEM_FREQUENCY            168000000U
#elif defined STM32F7
#define DEFAULT_SYSTEM_FREQUENCY            216000000U
#else
#define DEFAULT_SYSTEM_FREQUENCY            72000000U
#endif

#define SYSTEM_FREQUENCY    DEFAULT_SYSTEM_FREQUENCY

#define USE_CUBEMX_CREAT_CODE       0

/* ���ݷ�ƵҪ����� */
#define SYSTICK_DIV8        0
#if SYSTICK_DIV8 == 1
#define SYSTICK_CLK  0x00000000U
#else
#define SYSTICK_CLK  0x00000004U
#endif

/* Public macro Definition ---------------------------------------------------*/
/**
 * ��delay_us/delay_ms��Ҫ֧��OS��ʱ����Ҫ������OS��صĺ궨��ͺ�����֧��		
 * ������3���궨��:
 * SYSTEM_SUPPORT_OS            �Ƿ�ʹ��ϵͳ
	
 * OS_USE_RTTHREAD              �Ƿ�ʹ��rt-thread
	
 * OS_CRITICAL_METHOD           �Ƿ�ʹ��uc/OS-II
	
 * CPU_CFG_CRITICAL_METHOD      �Ƿ�ʹ��uc/OS-III
	
	
 * DELAY_OS_RUNNING             ���ڱ�ʾOS��ǰ�Ƿ���������,�Ծ����Ƿ����ʹ����غ���
 * DELAY_OS_TICK_PERSEC         ���ڱ�ʾOS�趨��ʱ�ӽ���,delay_init�����������������ʼ��systick
 * DELAY_OS_INTNESTING          ���ڱ�ʾOS�ж�Ƕ�׼���,��Ϊ�ж����治���Ե���,delay_msʹ�øò����������������
 */

/* 0Ϊ��ʹ��ϵͳ��1Ϊʹ��ϵͳ */
#define SYSTEM_SUPPORT_OS       0					
#if SYSTEM_SUPPORT_OS == 0
#undef SYSTEM_SUPPORT_OS
#else

/* 0Ϊ��ʹ��rt-threadϵͳ��1Ϊʹ��rt-threadϵͳ */
#define OS_USE_RTTHREAD         1
#if OS_USE_RTTHREAD == 0
#undef OS_USE_RTTHREAD
#else
#define DELAY_OS_RUNNING rt_tick_get()
#define DELAY_OS_TICK_PERSEC RT_TICK_PER_SECOND         //OSʱ�ӽ���,��ÿ����ȴ���
#define DELAY_OS_INTNESTING rt_interrupt_get_nest()     //�ж�Ƕ�׼���,���ж�Ƕ�״���
#endif /* OS_USE_RTTHREAD */

/* 0Ϊ��ʹ��uc/OS-IIϵͳ��1Ϊʹ��uc/OS-IIϵͳ */
#define OS_CRITICAL_METHOD      0
#if OS_CRITICAL_METHOD == 0
#undef OS_CRITICAL_METHOD
#else
#define DELAY_OS_RUNNING OSRunning                  //OS�Ƿ����б��,0,������;1,������
#define DELAY_OS_TICK_PERSEC OS_TICKS_PER_SEC       //OSʱ�ӽ���,��ÿ����ȴ���
#define DELAY_OS_INTNESTING OSIntNesting            //�ж�Ƕ�׼���,���ж�Ƕ�״���
#endif /* OS_CRITICAL_METHOD */

/* 0Ϊ��ʹ��uc/OS-IIIϵͳ��1Ϊʹ��uc/OS-IIIϵͳ */
#define CPU_CFG_CRITICAL_METHOD     0
#if CPU_CFG_CRITICAL_METHOD == 0
#undef CPU_CFG_CRITICAL_METHOD
#else
#define DELAY_OS_RUNNING OSRunning                  //OS�Ƿ����б��,0,������;1,������
#define DELAY_OS_TICK_PERSEC OSCfg_TickRate_Hz      //OSʱ�ӽ���,��ÿ����ȴ���
#define DELAY_OS_INTNESTING OSIntNestingCtr         //�ж�Ƕ�׼���,���ж�Ƕ�״���
#endif /* CPU_CFG_CRITICAL_METHOD */
#endif /* SYSTEM_SUPPORT_OS */

/* End public macro Definition -----------------------------------------------*/

/* UserCode start ------------------------------------------------------------*/

/* ��ʱ���ԵĽṹ�� */
typedef struct Delay
{
    uint16_t fac_us;            //us��ʱ������			   
    uint32_t fac_ms;            //ms��ʱ������,��os��,����ÿ�����ĵ�ms��
}Delay_t;

/* Member method APIs --------------------------------------------------------*/
void DelayInit(void);
void Delay_ms(uint16_t nms);
void Delay_us(uint32_t nus);
/* End Member Method APIs ----------------------------------------------------*/

#endif /* __DELAY_H_ */





























