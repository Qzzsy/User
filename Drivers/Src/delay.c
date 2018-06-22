/**
 ******************************************************************************
 * @file        delay.c
 * @author      �Ž�����С��
 * @version     V1.1.2
 * @date        2018-06-19
 * @brief       ����ļ���ϵͳ��ʱʵ�ֵķ�����ʵ�ֵ���ʱ��us��ʱ��ms��ʱ������systick
                �ķ�ʽ����ʱ��֧�������uc/OS-III��uc/OS-II��rt-threadʵʱϵͳ		
 * @note        ʹ��ǰ��Ҫ����delay_init();  �����޷�ʹ�ô˷���
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
 */
	
/* Includes ------------------------------------------------------------------*/
#include "delay.h"

/* ���֧��ϵͳ */
#ifdef SYSTEM_SUPPORT_OS

/* ���ʹ��uc/OS������������ͷ�ļ� */
#if defined (OS_CRITICAL_METHOD) || defined (CPU_CFG_CRITICAL_METHOD)
#include "includes.h"

/* ���ʹ��rt-thread������������ͷ�ļ� */
#elif defined (OS_USE_RTTHREAD)	
#include "rtthread.h"					
#endif	
#endif /* SYSTEM_SUPPORT_OS */

/* global variable Declaration -----------------------------------------------*/

/* ��ʱ������Ҫ��������ֵ����װ�ڽṹ��delay_Typedef�� */
/* fac_us	ÿһ��us�������ʱ���� */
/* fac_ms	ÿһ��ms���������ʱ���� */
/* �ýṹ�嶨���ȫ�ֱ���������delay.c����ʹ�� */
Delay_t SysDelay;

/* User function Declaration --------------------------------------------------*/


/* User functions -------------------------------------------------------------*/

/* ���SYSTEM_SUPPORT_OS�����ˣ�˵��Ҫ֧��OS */
#ifdef SYSTEM_SUPPORT_OS							

/**
 * @func    DelayOsschedlock
 * @brief   us����ʱʱ���ر��������(��ֹϵͳ���ȴ��us���ӳ�)
 * @note    ��֧��ϵͳʹ��
 * @retval  ��
 */
void DelayOsschedlock(void)
{
/* ʹ��UCOSIII */
#if defined (CPU_CFG_CRITICAL_METHOD)    			
    OS_ERR Err; 
    
    /* UCOSIII�ķ�ʽ����ֹ���ȣ���ֹ���us��ʱ */
    OSSchedLock(&Err);
    
/* ʹ��UCOSII */
#elif defined (OS_CRITICAL_METHOD)		

    /* UCOSII�ķ�ʽ����ֹ���� */
    OSSchedLock();
    
/* ʹ��rt-thread */	
#elif defined (OS_USE_RTTHREAD)
	
    /* rt-thread�ķ�ʽ����ֹ���� */
    rt_enter_critical();
#endif
}

/**
 * @func    DelayOsschedunlock
 * @brief   us����ʱʱ,�ָ��������
 * @note    ��֧��ϵͳʹ��
 * @retval  ��
 */
void DelayOsschedunlock(void)
{	
#if defined (CPU_CFG_CRITICAL_METHOD)    			
    OS_ERR Err; 
	
    /* UCOSIII�ķ�ʽ,�ָ����� */
    OSSchedUnlock(&Err);	
	
#elif defined (OS_CRITICAL_METHOD)
	
    /* UCOSII�ķ�ʽ,�ָ����� */
    OSSchedUnlock();	
	
#elif defined (OS_USE_RTTHREAD)
	
    /* rt-thread�ķ�ʽ,�ָ����� */
    rt_exit_critical();
#endif
}

/**
 * @func    DelayOstimedly
 * @brief   ����OS�Դ�����ʱ������ʱ
 * @param   ticks ��ʱ�Ľ�����
 * @note    ��֧��ϵͳʹ��
 * @retval  ��
 */
void DelayOstimedly(uint32_t Ticks)
{
#if defined (CPU_CFG_CRITICAL_METHOD) 
    OS_ERR Err; 
	
    /* UCOSIII��ʱ��������ģʽ */
    OSTimeDly(Ticks, OS_OPT_TIME_PERIODIC, &Err);
	
#elif defined (OS_CRITICAL_METHOD)
	
    /* UCOSII��ʱ */
    OSTimeDly(Ticks);		
	
#elif defined (OS_USE_RTTHREAD)
	
    /* rt-thread��ʱ */
    rt_thread_delay(Ticks);
#endif 
}
/* ��Ϊrt-thread�����Ѿ�������SysTick_Handler�����Խ���uc/OS��Ҫ�õ��˺��� */
#if defined (OS_CRITICAL_METHOD) || defined (CPU_CFG_CRITICAL_METHOD)

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
void DelayInit(void)
{
    uint32_t SysTickFreq;
	
	/* ����RCC_ClocksTypeDef���͵ı��������ڻ�ȡϵͳ��ʱ�� */
    uint32_t HCLK_Frequency = SYSTEM_FREQUENCY;
	
/* ��Ҫ֧��OS. */
#ifdef SYSTEM_SUPPORT_OS 

/* ʹ��uc/OS */	
#if defined (OS_CRITICAL_METHOD) || defined (CPU_CFG_CRITICAL_METHOD)
    uint32_t Ticks = (uint32_t)HCLK_Frequency / DELAY_OS_TICK_PERSEC;

#if SYSTICK_DIV8 == 1
    /* Ĭ��ʹ��8��Ƶ */
    Ticks = Ticks / 8;
#endif    
    
    /* systick��ʱ������ֵ������ */
    if ((Ticks - 1UL) > 0x00FFFFFFUL)
    {
        return (1UL);                                                   /* Reload value impossible */
    }

    SysTick->LOAD  = (uint32_t)(Ticks - 1UL);                         /* set reload register */
    NVIC_SetPriority (SysTick_IRQn, (1UL << 0x00000004U) - 1UL); /* set Priority for Systick Interrupt */
    SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
    SysTick->CTRL  = 0x00000004U |
                     0x00000002U   |
                     0x00000001U;                         /* Enable SysTick IRQ and SysTick Timer */
    return (0UL);    
#endif
#endif /* SYSTEM_SUPPORT_OS */
	
/* ��ʹ��rt-thread */
#ifndef OS_USE_RTTHREAD

    if (SYSTICK_CLK == 0x00000004U)
    {
        SysTick->CTRL |= 0x00000004U;
    }
    else
    {
        SysTick->CTRL &= ~0x00000004U;
    }
#endif /* !OS_USE_RTTHREAD */
    
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
    SysDelay.fac_us = SysTickFreq / 1000000;
    
#ifdef SYSTEM_SUPPORT_OS
#if defined (OS_USE_RTTHREAD) || defined (OS_CRITICAL_METHOD) || defined (CPU_CFG_CRITICAL_METHOD)
    SysDelay.fac_ms = 1000 / DELAY_OS_TICK_PERSEC;
#endif
#else

    /* ��OS��,����ÿ��ms��Ҫ��systickʱ���� */
    SysDelay.fac_ms = (uint16_t)SysDelay.fac_us * 1000;				
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
void Delay_us(uint32_t nus)
{		
    uint32_t Ticks;
    uint32_t Told, Tnow, Tcnt = 0;
    uint32_t Reload;
    
    /* ��ȡ��ǰLOAD��ֵ */
    Reload = SysTick->LOAD;				
    
    /* ������Ҫ�Ľ����� */
    Ticks = nus * SysDelay.fac_us; 	
    
    /* ��ֹOS���ȣ���ֹ���us��ʱ */
    DelayOsschedlock();	
    
    /* ��¼��ǰ�ļ�������ֵ */
    Told = SysTick->VAL;    
    
    /* ���� ��ʱ */
    while (1)
    {
        Tnow = SysTick->VAL;	
        
        if (Tnow != Told)
        {	    
            /* ����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ����� */
            if (Tnow < Told)
                Tcnt += Told - Tnow;	
            else 
                Tcnt += Reload - Tnow + Told;	    
            Told = Tnow;
            
            /* ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳� */
            if (Tcnt >= Ticks)
                break;			
        }  
    };
    
    /* �ָ�OS���� */			
    DelayOsschedunlock();												    
}  

/**
 * @func    delay_ms
 * @brief   ��ʱnms
 * @param   nms Ҫ��ʱ��ms��	
 * @note    nms��0-65535
 * @retval  ��
 */	
void Delay_ms(uint16_t nms)
{	
    uint16_t i;
#if defined (OS_CRITICAL_METHOD) || defined (CPU_CFG_CRITICAL_METHOD)
    
    /* ���OS�Ѿ�������,���Ҳ������ж�����(�ж����治���������) */
    if (DELAY_OS_RUNNING && (DELAY_OS_INTNESTING == 0))  
    {	
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
        return ;
    }
    
/* ���ʹ��rt-thread(�ж����治���������) */
#elif defined (OS_USE_RTTHREAD)
    {
        static rt_uint8_t Delay_msLock = false;
        if((DELAY_OS_RUNNING | Delay_msLock) && (DELAY_OS_INTNESTING == 0))
        {
            
            /* ���ڱ�־ϵͳ�Ƿ��Ѿ������� */
            if (Delay_msLock == false)
            {
                Delay_msLock = true;
            }
            
            if (nms >= SysDelay.fac_ms)						
            { 
                DelayOstimedly(nms / SysDelay.fac_ms);	
            }
            
            nms %= SysDelay.fac_ms;	
        }	
        else
        {
            for (i = 0; i < nms; i++)
            {
                /* ��ͨ��ʽ��ʱ */
                Delay_us(1000);	
            }
            return ;
        }	
    }
#endif
    
    /* ��ͨ��ʽ��ʱ */
    Delay_us((uint32_t)(nms * 1000));				
}

/* ����ucosʱ */
#else  

#if USE_CUBEMX_CREAT_CODE == 1

/**
 * @func    delay_us
 * @brief   ��ʱnus
 * @param   nus Ҫ��ʱ��us��	
 * @note    nus�ķ�Χ0~204522252(���ֵ��2^32/fac_us@fac_us=21)
 * @retval  ��
 */	  								   
void Delay_us(uint32_t nus)
{		
    uint32_t Ticks;
    uint32_t Told, Tnow, Tcnt = 0;
    uint32_t Reload;
    
    /* ��ȡ��ǰLOAD��ֵ */
    Reload = SysTick->LOAD;				
    
    /* ������Ҫ�Ľ����� */
    Ticks = nus * SysDelay.fac_us; 	
    
    /* ��¼��ǰ�ļ�������ֵ */
    Told = SysTick->VAL;    
    
    /* ���� ��ʱ */
    while (1)
    {
        Tnow = SysTick->VAL;	
        
        if (Tnow != Told)
        {	    
            /* ����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ����� */
            if (Tnow < Told)
                Tcnt += Told - Tnow;	
            else 
                Tcnt += Reload - Tnow + Told;	    
            Told = Tnow;
            
            /* ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳� */
            if (Tcnt >= Ticks)
                break;			
        }  
    }											    
} 


/**
 * @func    delay_ms
 * @brief   ��ʱnms
 * @param   nms Ҫ��ʱ��ms��	
 * @note    nms��0-65535
 * @retval  ��
 */	
void Delay_ms(uint16_t nms)
{	
    HAL_Delay(nms);			
}
#else
/**
 * @func    Delay_us
 * @brief   ��ʱnus
 * @param   nus ΪҪ��ʱ��us��.	
 * @note    nus��ֵ,��Ҫ����798915us(���ֵ��2^24/fac_us@fac_us=21)
 * @retval  ��
 */	
void Delay_us(uint32_t nus)
{		
    uint32_t Temp;	    	 
    
    /* ʱ����� */
    SysTick->LOAD = nus * SysDelay.fac_us; 		
    
    /* ��ռ����� */
    SysTick->VAL = 0x00;        				
    
    /* ��ʼ���� */
    SysTick->CTRL |= (uint32_t)0x00000001U ; 	 
    
    do
    {
        Temp = SysTick->CTRL;
    } while ((Temp & 0x01 ) && !(Temp & (1 << 16)));	//�ȴ�ʱ�䵽��   
    
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
void Delay_xms(uint16_t nms)
{	 		  	  
    uint32_t Temp;		   
    
    /* ʱ�����(SysTick->LOADΪ24bit) */
    SysTick->LOAD = (uint32_t)nms * SysDelay.fac_ms;		
    
    /* ��ռ����� */
    SysTick->VAL = 0x00;           
    
    /* ��ʼ���� */
    SysTick->CTRL |= ((uint32_t)0x00000001U) ;    
    
    do
    {
        Temp = SysTick->CTRL;
    } while ((Temp & 0x01) && !(Temp & (1 << 16)));	//�ȴ�ʱ�䵽��   
    
    /* �رռ����� */
    SysTick->CTRL &= ~((uint32_t)0x00000001U);     
    
    /* ��ռ����� */
    SysTick->VAL = 0X00;     		  		  	    
} 

/**
 * @func    Delay_ms
 * @brief   ��ʱnms
 * @param   nms ΪҪ��ʱ��ms��.	
 * @note    nms:0~65535
 * @retval  ��
 */	
void Delay_ms(uint16_t nms)
{	
#if SYSTICK_DIV8 == 1
    /* ������540,�ǿ��ǵ�ĳЩ�ͻ����ܳ�Ƶʹ�� */
    uint16_t Repeat = nms / 540;	
    
    /* ���糬Ƶ��248M��ʱ��,delay_xms���ֻ����ʱ541ms���� */									
    uint16_t Remain = nms % 540;
    
    while (Repeat)
    {
        Delay_xms(540);
        Repeat--;
    }
#else
    /* ������540,�ǿ��ǵ�ĳЩ�ͻ����ܳ�Ƶʹ�� */
    uint16_t Repeat = nms / 60;	
    
    /* ���糬Ƶ��248M��ʱ��,delay_xms���ֻ����ʱ541ms���� */									
    uint16_t Remain = nms % 60;
    
    while (Repeat)
    {
        Delay_xms(60);
        Repeat--;
    }
#endif  
    if (Remain)
        Delay_xms(Remain);
} 
#endif
#endif
			 



































