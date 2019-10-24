/**
 ******************************************************************************
 * @file        delay.c
 * @author      �Ž�����С��
 * @version     V1.2.0
 * @date        2019-08-26
 * @brief       ����ļ���ϵͳ��ʱʵ�ֵķ�����ʵ�ֵ���ʱ��us��ʱ��ms��ʱ������systick
                �ķ�ʽ����ʱ��֧�������uc/OS-III��uc/OS-II��FreeRTOS��rt-threadʵʱϵͳ		
 * @note        ʹ��ǰ��Ҫ����delay_init();  �����޷�ʹ�ô��ļ��еķ���
 * @History
 * Date           	Author    	version    				Notes
 * 2017-11-05     	ZSY       	V1.0.0              first version.
 * 2017-11-07     	ZSY       	V1.0.1              �޸��ڲ���ϵͳ��us��ʱ��׼��BUG����߼�����
 * 2017-11-13       ZSY         V1.0.2              �޸�������������us����ʱ�����BUG�������ṹ���޸���uc/OS��ʹ��ʱms����ʱ��BUG
 * 2018-01-11     	ZSY       	V1.0.3              �Ű��ʽ����������ǿ�ɶ��ԣ���Ӷ�HAL���֧��
 * 2018-01-11       ZSY         V1.0.4              �޸Ĳ��ֱ���������
 * 2018-06-07       ZSY         V1.1.0              �޸Ľṹ��ȫ���üĴ����ķ�ʽ��д�����ݼĴ����Լ�����⺯��
 * 2018-06-12       ZSY         V1.1.1              �޸Ĳ��ֱ������ƣ����ƶ�ȫϵ��оƬ��֧��
 * 2018-06-19       ZSY         V1.1.2              �޸��ڲ���Ƶ�������ms����ʱ��׼��BUG
 * 2019-08-26       ZSY         V1.2.0              ��Ӷ�FreeRTOS��֧��
 */

/* Includes ------------------------------------------------------------------*/
#include "delay.h"

/* ���֧��ϵͳ */
#ifdef SYSTEM_SUPPORT_OS

/* ���ʹ��uc/OS������������ͷ�ļ� */
#if defined(OS_CRITICAL_METHOD) || defined(CPU_CFG_CRITICAL_METHOD)
#include "includes.h"

/* ���ʹ��rt-thread������������ͷ�ļ� */
#elif defined(OS_USING_RTTHREAD)
#include "rtthread.h"

#elif defined(OS_USING_FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#endif
__weak void xPortSysTickHandler(void)
{
}

#endif /* SYSTEM_SUPPORT_OS */

/* global variable Declaration -----------------------------------------------*/

/* ��ʱ������Ҫ��������ֵ����װ�ڽṹ��delay_Typedef�� */
/* fac_us	ÿһ��us�������ʱ���� */
/* fac_ms	ÿһ��ms���������ʱ���� */
/* �ýṹ�嶨���ȫ�ֱ���������delay.c����ʹ�� */
delay_t sys_delay;

/* User function Declaration --------------------------------------------------*/

/* User functions -------------------------------------------------------------*/

/* ���SYSTEM_SUPPORT_OS�����ˣ�˵��Ҫ֧��OS */
#ifdef SYSTEM_SUPPORT_OS

/**
 * @func    delay_os_schedlock
 * @brief   us����ʱʱ���ر��������(��ֹϵͳ���ȴ��us���ӳ�)
 * @note    ��֧��ϵͳʹ��
 * @retval  ��
 */
void delay_os_schedlock(void)
{
/* ʹ��UCOSIII */
#if defined(CPU_CFG_CRITICAL_METHOD)
    OS_ERR Err;

    /* UCOSIII�ķ�ʽ����ֹ���ȣ���ֹ���us��ʱ */
    OSSchedLock(&Err);

/* ʹ��UCOSII */
#elif defined(OS_CRITICAL_METHOD)

    /* UCOSII�ķ�ʽ����ֹ���� */
    OSSchedLock();

/* ʹ��rt-thread */
#elif defined(OS_USING_RTTHREAD)

    /* rt-thread�ķ�ʽ����ֹ���� */
    rt_enter_critical();
#elif defined(OS_USING_FREERTOS)

    /* FreeRTOS�ķ�ʽ����ֹ���� */
    vPortEnterCritical();
#endif
}

/**
 * @func    delay_os_schedunlock
 * @brief   us����ʱʱ,�ָ��������
 * @note    ��֧��ϵͳʹ��
 * @retval  ��
 */
void delay_os_schedunlock(void)
{
#if defined(CPU_CFG_CRITICAL_METHOD)
    OS_ERR Err;

    /* UCOSIII�ķ�ʽ,�ָ����� */
    OSSchedUnlock(&Err);

#elif defined(OS_CRITICAL_METHOD)

    /* UCOSII�ķ�ʽ,�ָ����� */
    OSSchedUnlock();

#elif defined(OS_USING_RTTHREAD)

    /* rt-thread�ķ�ʽ,�ָ����� */
    rt_exit_critical();

#elif defined(OS_USING_FREERTOS)
    vPortExitCritical();
#endif
}

/**
 * @func    delay_ostimedly
 * @brief   ����OS�Դ�����ʱ������ʱ
 * @param   ticks ��ʱ�Ľ�����
 * @note    ��֧��ϵͳʹ��
 * @retval  ��
 */
void delay_ostimedly(uint32_t ticks)
{
#if defined(CPU_CFG_CRITICAL_METHOD)
    OS_ERR Err;

    /* UCOSIII��ʱ��������ģʽ */
    OSTimeDly(ticks, OS_OPT_TIME_PERIODIC, &Err);

#elif defined(OS_CRITICAL_METHOD)

    /* UCOSII��ʱ */
    OSTimeDly(ticks);

#elif defined(OS_USING_RTTHREAD)

    /* rt-thread��ʱ */
    rt_thread_delay(ticks);
#elif defined(OS_USING_FREERTOS)

    /* FreeRTOS��ʱ */
    vTaskDelay(ticks);
#endif
}

/* ��Ϊrt-thread�����Ѿ�������SysTick_Handler�����Խ���uc/OS��FreeRTOS��Ҫ�õ��˺��� */
#if defined(OS_CRITICAL_METHOD) || defined(CPU_CFG_CRITICAL_METHOD) || defined(OS_USING_FREERTOS)

/**
 * @func    SysTick_Handler
 * @brief   systick�жϷ�����,ʹ��OSʱ�õ�
 * @note    ��ǰ��֧��uc/OSϵͳʹ��
 * @retval  ��
 */
void SysTick_Handler(void)
{
#ifdef USE_HAL_DRIVER
    HAL_IncTick();
#endif

#if defined(OS_CRITICAL_METHOD) || defined(CPU_CFG_CRITICAL_METHOD)
    /* OS��ʼ����,��ִ�������ĵ��ȴ��� */
    if (DELAY_OS_RUNNING == 1)
    {
        /* �����ж� */
        OSIntEnter();

        /* ����ucos��ʱ�ӷ������ */
        OSTimeTick();

        /* ���������л����ж� */
        OSIntExit();
    }
#elif defined(OS_USING_FREERTOS)
    if (DELAY_OS_RUNNING != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
#endif
}
#endif
#endif /* SYSTEM_SUPPORT_OS */

/**
 * @func    DelayInit
 * @brief   ��ʼ���ӳٺ���
 * @note    ��ǰ��֧��uc/OSϵͳʹ��
            ��ʹ��uc/OS��ʱ��,�˺������ʼ��uc/OS��ʱ�ӽ���
            SYSTICK��ʱ�ӹ̶�ΪAHBʱ�ӵ�1/8
 * @retval  ��
 */
void delay_init(void)
{
    uint32_t SysTickFreq;

    /* ����RCC_ClocksTypeDef���͵ı��������ڻ�ȡϵͳ��ʱ�� */
    uint32_t HCLK_Frequency = SYSTEM_FREQUENCY;

/* ��Ҫ֧��OS. */
#ifdef SYSTEM_SUPPORT_OS

/* ʹ��uc/OS */
#if defined(OS_CRITICAL_METHOD) || defined(CPU_CFG_CRITICAL_METHOD) || defined(OS_USING_FREERTOS)
    uint32_t ticks = (uint32_t)HCLK_Frequency / DELAY_OS_TICK_PERSEC;

#if SYSTICK_DIV8 == 1
    /* Ĭ��ʹ��8��Ƶ */
    ticks = ticks / 8;
#endif

    /* systick��ʱ������ֵ������ */
    if ((ticks - 1UL) > 0x00FFFFFFUL)
    {
        return; /* Reload value impossible */
    }

    SysTick->LOAD = (uint32_t)(ticks - 1UL); /* set reload register */
    NVIC_SetPriority(SysTick_IRQn, 0);       /* set Priority for Systick Interrupt */
    SysTick->VAL = 0UL;                      /* Load the SysTick Counter Value */
    SysTick->CTRL = 0x00000004U |
                    0x00000002U |
                    0x00000001U; /* Enable SysTick IRQ and SysTick Timer */
#endif
#else  /* ��֧��ϵͳ��ֱ�ӹص�оƬ�δ��ж� */
    SysTick->CTRL &= ~(0x00000002u);
#endif /* SYSTEM_SUPPORT_OS */

/* ��ʹ��rt-thread */
#ifndef OS_USING_RTTHREAD

    if (SYSTICK_CLK == 0x00000004U)
    {
        SysTick->CTRL |= 0x00000004U;
    }
    else
    {
        SysTick->CTRL &= ~0x00000004U;
    }
#endif /* !OS_USING_RTTHREAD */

    if (((SysTick->CTRL) >> 2) & 0x01)
    {
        /* ��ȡsystick��ʱ�ӣ�SysTickֱ��ȡHCLK����Ƶ */
        SysTickFreq = (uint32_t)HCLK_Frequency;
    }
    else
    {
        /* ��ȡsystick��ʱ�ӣ�Ĭ����ϵͳʱ�ӵ�8��Ƶ */
        SysTickFreq = (uint32_t)HCLK_Frequency / 8;
    }

    /* ����ÿ��us������ʱ���� */
    sys_delay.fac_us = SysTickFreq / 1000000;

#ifdef SYSTEM_SUPPORT_OS
    sys_delay.fac_ms = 1000 / DELAY_OS_TICK_PERSEC;
#else

    /* ��OS��,����ÿ��ms��Ҫ��systickʱ���� */
    sys_delay.fac_ms = (uint16_t)sys_delay.fac_us * 1000;
#endif /* SYSTEM_SUPPORT_OS */
}

/* �����Ҫ֧��OS. */
#if SYSTEM_SUPPORT_OS

/**
 * @func    delay_us
 * @brief   ��ʱnus
 * @param   nus Ҫ��ʱ��us��	
 * @note    nus�ķ�Χ0~204522252(���ֵ��2^32/fac_us@fac_us=21)
 * @retval  ��
 */
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload;

    /* ��ȡ��ǰLOAD��ֵ */
    reload = SysTick->LOAD;

    /* ������Ҫ�Ľ����� */
    ticks = nus * sys_delay.fac_us;

    /* ��ֹOS���ȣ���ֹ���us��ʱ */
    delay_os_schedlock();

    /* ��¼��ǰ�ļ�������ֵ */
    told = SysTick->VAL;

    /* ���� ��ʱ */
    while (1)
    {
        tnow = SysTick->VAL;

        if (tnow != told)
        {
            /* ����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ����� */
            if (tnow < told)
                tcnt += told - tnow;
            else
                tcnt += reload - tnow + told;
            told = tnow;

            /* ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳� */
            if (tcnt >= ticks)
                break;
        }
    };

    /* �ָ�OS���� */
    delay_os_schedunlock();
}

/**
 * @func    delay_ms
 * @brief   ��ʱnms
 * @param   nms Ҫ��ʱ��ms��	
 * @note    nms��0-65535
 * @retval  ��
 */
void delay_ms(uint32_t nms)
{
    uint16_t i;
#if defined(OS_CRITICAL_METHOD) || defined(CPU_CFG_CRITICAL_METHOD) || defined(OS_USING_FREERTOS)
#if defined(OS_USING_FREERTOS)
    /* ���OS�Ѿ�������,���Ҳ������ж�����(�ж����治���������) */
    if (DELAY_OS_RUNNING != taskSCHEDULER_NOT_STARTED)
    {
#else
    if (DELAY_OS_RUNNING == 1)
    {
#endif
        /* ��ʱ��ʱ�����OS������ʱ������ */
        if (nms >= sys_delay.fac_ms)
        {
            /* OS��ʱ */
            delay_ostimedly(nms / sys_delay.fac_ms);
        }

        /* OS�Ѿ��޷��ṩ��ôС����ʱ��,������ͨ��ʽ��ʱ */
        nms %= sys_delay.fac_ms;
    }
    else
    {
        for (i = 0; i < nms; i++)
        {
            /* ��ͨ��ʽ��ʱ */
            delay_us(1000);
        }
        return;
    }

/* ���ʹ��rt-thread(�ж����治���������) */
#elif defined(OS_USING_RTTHREAD)
    {
        static rt_uint8_t delay_Lock = false;
        if ((DELAY_OS_RUNNING | delay_Lock) && (DELAY_OS_INTNESTING == 0))
        {
            /* ���ڱ�־ϵͳ�Ƿ��Ѿ������� */
            if (delay_Lock == false)
            {
                delay_Lock = true;
            }

            if (nms >= sys_delay.fac_ms)
            {
                delay_ostimedly(nms / sys_delay.fac_ms);
            }

            nms %= sys_delay.fac_ms;
        }
        else
        {
            for (i = 0; i < nms; i++)
            {
                /* ��ͨ��ʽ��ʱ */
                delay_us(1000);
            }
            return;
        }
    }
#endif

    /* ��ͨ��ʽ��ʱ */
    delay_us((uint32_t)(nms * 1000));
}

/* ����ϵͳʱ */
#else
/**
 * @func    Delay_us
 * @brief   ��ʱnus
 * @param   nus ΪҪ��ʱ��us��.	
 * @note    nus��ֵ,��Ҫ����798915us(���ֵ��2^24/fac_us@fac_us=21)
 * @retval  ��
 */
void delay_us(uint32_t nus)
{
    uint32_t temp;

    /* ʱ����� */
    SysTick->LOAD = nus * sys_delay.fac_us;

    /* ��ռ����� */
    SysTick->VAL = 0x00;

    /* ��ʼ���� */
    SysTick->CTRL |= (uint32_t)0x00000001U;

    do
    {
        temp = SysTick->CTRL;
    } while ((Temp & 0x01) && !(temp & (1 << 16))); //�ȴ�ʱ�䵽��

    /* �رռ����� */
    SysTick->CTRL &= ~((uint32_t)0x00000001U);

    /* ��ռ����� */
    SysTick->VAL = 0X00;
}

/**
 * @func    Delay_xms
 * @brief   ��ʱnms
 * @param   nms ΪҪ��ʱ��ms��.	
 * @note    ע��nms�ķ�Χ
            SysTick->LOADΪ24λ�Ĵ���,����,�����ʱΪ:
            nms<=0xffffff*8*1000/SYSCLK
            SYSCLK��λΪHz,nms��λΪms
            ��168M������,nms<=798ms 
 * @retval  ��
 */
static void delay_1ms(void)
{
    uint32_t temp;

    /* ʱ�����(SysTick->LOADΪ24bit) */
    SysTick->LOAD = sys_delay.fac_ms;

    /* ��ռ����� */
    SysTick->VAL = 0x00;

    /* ��ʼ���� */
    SysTick->CTRL |= ((uint32_t)0x00000001U);

    do
    {
        temp = SysTick->CTRL;
    } while ((Temp & 0x01) && !(temp & (1 << 16))); //�ȴ�ʱ�䵽��

    /* �رռ����� */
    SysTick->CTRL &= ~((uint32_t)0x00000001U);

    /* ��ռ����� */
    SysTick->VAL = 0X00;
}

/**
 * @func    delay_ms
 * @brief   ��ʱnms
 * @param   nms ΪҪ��ʱ��ms��.	
 * @note    nms:0~65535
 * @retval  ��
 */
void delay_ms(uint32_t nms)
{
    while (nms--)
    {
        delay_1ms();
    }
}
#endif
