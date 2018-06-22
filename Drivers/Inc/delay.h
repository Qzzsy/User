/**
******************************************************************************
 * @Copyright       (C) 2017 - 2018 guet-sctc-hardwarepart Team
 * @filename        delay.h
 * @author          门禁开发小组
 * @version         V1.1.2
 * @date            2018-06-19
 * @Description     delay文件，包含了是否使用系统的宏定义选择以及使用系统的类型
 * @Others
 * @History
 * Date           	Author    	version                 Notes
 * 2017-11-05     	ZSY       	V1.0.0              first version.
 * 2017-11-13       ZSY         V1.0.1              调整结构
 * 2018-01-11       ZSY         V1.0.2              排版格式化操作，增强在不同缩进情况下的可读性，增加HAL库的支持
 * 2018-01-11       ZSY         V1.0.3              修改部分变量的声明
 * 2018-06-07       ZSY         V1.1.0              修改结构，全采用寄存器的方式编写，兼容寄存器以及各类库函数
 * 2018-06-12       ZSY         V1.1.1              修改部分变量名称，完善对全系列芯片的支持
 * 2018-06-19       ZSY         V1.1.2              修复在不分频的情况下ms级延时不准的BUG
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

/* 根据芯片的内核频率更改 */
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

/* 根据分频要求更改 */
#define SYSTICK_DIV8        0
#if SYSTICK_DIV8 == 1
#define SYSTICK_CLK  0x00000000U
#else
#define SYSTICK_CLK  0x00000004U
#endif

/* Public macro Definition ---------------------------------------------------*/
/**
 * 当delay_us/delay_ms需要支持OS的时候需要三个与OS相关的宏定义和函数来支持		
 * 首先是3个宏定义:
 * SYSTEM_SUPPORT_OS            是否使用系统
	
 * OS_USE_RTTHREAD              是否使用rt-thread
	
 * OS_CRITICAL_METHOD           是否使用uc/OS-II
	
 * CPU_CFG_CRITICAL_METHOD      是否使用uc/OS-III
	
	
 * DELAY_OS_RUNNING             用于表示OS当前是否正在运行,以决定是否可以使用相关函数
 * DELAY_OS_TICK_PERSEC         用于表示OS设定的时钟节拍,delay_init将根据这个参数来初始哈systick
 * DELAY_OS_INTNESTING          用于表示OS中断嵌套级别,因为中断里面不可以调度,delay_ms使用该参数来决定如何运行
 */

/* 0为不使用系统，1为使用系统 */
#define SYSTEM_SUPPORT_OS       0					
#if SYSTEM_SUPPORT_OS == 0
#undef SYSTEM_SUPPORT_OS
#else

/* 0为不使用rt-thread系统，1为使用rt-thread系统 */
#define OS_USE_RTTHREAD         1
#if OS_USE_RTTHREAD == 0
#undef OS_USE_RTTHREAD
#else
#define DELAY_OS_RUNNING rt_tick_get()
#define DELAY_OS_TICK_PERSEC RT_TICK_PER_SECOND         //OS时钟节拍,即每秒调度次数
#define DELAY_OS_INTNESTING rt_interrupt_get_nest()     //中断嵌套级别,即中断嵌套次数
#endif /* OS_USE_RTTHREAD */

/* 0为不使用uc/OS-II系统，1为使用uc/OS-II系统 */
#define OS_CRITICAL_METHOD      0
#if OS_CRITICAL_METHOD == 0
#undef OS_CRITICAL_METHOD
#else
#define DELAY_OS_RUNNING OSRunning                  //OS是否运行标记,0,不运行;1,在运行
#define DELAY_OS_TICK_PERSEC OS_TICKS_PER_SEC       //OS时钟节拍,即每秒调度次数
#define DELAY_OS_INTNESTING OSIntNesting            //中断嵌套级别,即中断嵌套次数
#endif /* OS_CRITICAL_METHOD */

/* 0为不使用uc/OS-III系统，1为使用uc/OS-III系统 */
#define CPU_CFG_CRITICAL_METHOD     0
#if CPU_CFG_CRITICAL_METHOD == 0
#undef CPU_CFG_CRITICAL_METHOD
#else
#define DELAY_OS_RUNNING OSRunning                  //OS是否运行标记,0,不运行;1,在运行
#define DELAY_OS_TICK_PERSEC OSCfg_TickRate_Hz      //OS时钟节拍,即每秒调度次数
#define DELAY_OS_INTNESTING OSIntNestingCtr         //中断嵌套级别,即中断嵌套次数
#endif /* CPU_CFG_CRITICAL_METHOD */
#endif /* SYSTEM_SUPPORT_OS */

/* End public macro Definition -----------------------------------------------*/

/* UserCode start ------------------------------------------------------------*/

/* 延时属性的结构体 */
typedef struct Delay
{
    uint16_t fac_us;            //us延时倍乘数			   
    uint32_t fac_ms;            //ms延时倍乘数,在os下,代表每个节拍的ms数
}Delay_t;

/* Member method APIs --------------------------------------------------------*/
void DelayInit(void);
void Delay_ms(uint16_t nms);
void Delay_us(uint32_t nus);
/* End Member Method APIs ----------------------------------------------------*/

#endif /* __DELAY_H_ */





























