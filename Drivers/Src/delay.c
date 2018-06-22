/**
 ******************************************************************************
 * @file        delay.c
 * @author      门禁开发小组
 * @version     V1.1.2
 * @date        2018-06-19
 * @brief       这个文件是系统延时实现的方法，实现的延时有us延时和ms延时，采用systick
                的方式来延时，支持裸机、uc/OS-III、uc/OS-II和rt-thread实时系统		
 * @note        使用前需要调用delay_init();  否则无法使用此方法
 * @History
 * Date           	Author    	version    				Notes
 * 2017-11-05     	ZSY       	V1.0.0              first version.
 * 2017-11-07     	ZSY       	V1.0.1              修复在部分系统上us延时不准的BUG，提高兼容性
 * 2017-11-13       ZSY         V1.0.2              修复在裸机的情况下us级延时错误的BUG，调整结构，修复在uc/OS中使用时ms级延时的BUG
 * 2018-01-11     	ZSY       	V1.0.3              排版格式化操作，增强可读性，添加对HAL库的支持
 * 2018-01-11       ZSY         V1.0.4              修改部分变量的声明
 * 2018-06-07       ZSY         V1.1.0              修改结构，全采用寄存器的方式编写，兼容寄存器以及各类库函数
 * 2018-06-12       ZSY         V1.1.1              修改部分变量名称，完善对全系列芯片的支持
 * 2018-06-19       ZSY         V1.1.2              修复在不分频的情况下ms级延时不准的BUG
 */
	
/* Includes ------------------------------------------------------------------*/
#include "delay.h"

/* 如果支持系统 */
#ifdef SYSTEM_SUPPORT_OS

/* 如果使用uc/OS，则包括下面的头文件 */
#if defined (OS_CRITICAL_METHOD) || defined (CPU_CFG_CRITICAL_METHOD)
#include "includes.h"

/* 如果使用rt-thread，则包括下面的头文件 */
#elif defined (OS_USE_RTTHREAD)	
#include "rtthread.h"					
#endif	
#endif /* SYSTEM_SUPPORT_OS */

/* global variable Declaration -----------------------------------------------*/

/* 延时方法需要的两个数值，封装在结构体delay_Typedef内 */
/* fac_us	每一个us里包含的时钟数 */
/* fac_ms	每一个ms里面包含的时钟数 */
/* 该结构体定义的全局变量仅限于delay.c里面使用 */
Delay_t SysDelay;

/* User function Declaration --------------------------------------------------*/


/* User functions -------------------------------------------------------------*/

/* 如果SYSTEM_SUPPORT_OS定义了，说明要支持OS */
#ifdef SYSTEM_SUPPORT_OS							

/**
 * @func    DelayOsschedlock
 * @brief   us级延时时，关闭任务调度(防止系统调度打断us级延迟)
 * @note    仅支持系统使用
 * @retval  无
 */
void DelayOsschedlock(void)
{
/* 使用UCOSIII */
#if defined (CPU_CFG_CRITICAL_METHOD)    			
    OS_ERR Err; 
    
    /* UCOSIII的方式，禁止调度，防止打断us延时 */
    OSSchedLock(&Err);
    
/* 使用UCOSII */
#elif defined (OS_CRITICAL_METHOD)		

    /* UCOSII的方式，禁止调度 */
    OSSchedLock();
    
/* 使用rt-thread */	
#elif defined (OS_USE_RTTHREAD)
	
    /* rt-thread的方式，禁止调度 */
    rt_enter_critical();
#endif
}

/**
 * @func    DelayOsschedunlock
 * @brief   us级延时时,恢复任务调度
 * @note    仅支持系统使用
 * @retval  无
 */
void DelayOsschedunlock(void)
{	
#if defined (CPU_CFG_CRITICAL_METHOD)    			
    OS_ERR Err; 
	
    /* UCOSIII的方式,恢复调度 */
    OSSchedUnlock(&Err);	
	
#elif defined (OS_CRITICAL_METHOD)
	
    /* UCOSII的方式,恢复调度 */
    OSSchedUnlock();	
	
#elif defined (OS_USE_RTTHREAD)
	
    /* rt-thread的方式,恢复调度 */
    rt_exit_critical();
#endif
}

/**
 * @func    DelayOstimedly
 * @brief   调用OS自带的延时函数延时
 * @param   ticks 延时的节拍数
 * @note    仅支持系统使用
 * @retval  无
 */
void DelayOstimedly(uint32_t Ticks)
{
#if defined (CPU_CFG_CRITICAL_METHOD) 
    OS_ERR Err; 
	
    /* UCOSIII延时采用周期模式 */
    OSTimeDly(Ticks, OS_OPT_TIME_PERIODIC, &Err);
	
#elif defined (OS_CRITICAL_METHOD)
	
    /* UCOSII延时 */
    OSTimeDly(Ticks);		
	
#elif defined (OS_USE_RTTHREAD)
	
    /* rt-thread延时 */
    rt_thread_delay(Ticks);
#endif 
}
/* 因为rt-thread本身已经包含了SysTick_Handler，所以仅有uc/OS需要用到此函数 */
#if defined (OS_CRITICAL_METHOD) || defined (CPU_CFG_CRITICAL_METHOD)

/**
 * @func    SysTick_Handler
 * @brief   systick中断服务函数,使用OS时用到
 * @note    当前仅支持uc/OS系统使用
 * @retval  无
 */
void SysTick_Handler(void)
{	
#ifdef USE_HAL_DRIVER
    HAL_IncTick();
#endif
    /* OS开始跑了,才执行正常的调度处理 */
    if (DELAY_OS_RUNNING == 1)					
    {
        /* 进入中断 */
        OSIntEnter();						
        
        /* 调用ucos的时钟服务程序 */
        OSTimeTick();      
        
        /* 触发任务切换软中断 */    
        OSIntExit();       	 				
    }
}
#endif
#endif /* SYSTEM_SUPPORT_OS */

/**
 * @func    DelayInit
 * @brief   初始化延迟函数
 * @note    当前仅支持uc/OS系统使用
            当使用uc/OS的时候,此函数会初始化uc/OS的时钟节拍
            SYSTICK的时钟固定为AHB时钟的1/8
 * @retval  无
 */			   
void DelayInit(void)
{
    uint32_t SysTickFreq;
	
	/* 定义RCC_ClocksTypeDef类型的变量，用于获取系统的时钟 */
    uint32_t HCLK_Frequency = SYSTEM_FREQUENCY;
	
/* 需要支持OS. */
#ifdef SYSTEM_SUPPORT_OS 

/* 使用uc/OS */	
#if defined (OS_CRITICAL_METHOD) || defined (CPU_CFG_CRITICAL_METHOD)
    uint32_t Ticks = (uint32_t)HCLK_Frequency / DELAY_OS_TICK_PERSEC;

#if SYSTICK_DIV8 == 1
    /* 默认使用8分频 */
    Ticks = Ticks / 8;
#endif    
    
    /* systick的时钟重载值的配置 */
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
	
/* 不使用rt-thread */
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
        /* 获取systick的时钟，SysTick直接取HCLK不分频 */
        SysTickFreq = (uint32_t)HCLK_Frequency;
    }
    else
    {
        /* 获取systick的时钟，默认是系统时钟的8分频 */
        SysTickFreq = (uint32_t)HCLK_Frequency / 8;
    }
    
    /* 计算每个us包含的时钟数 */
    SysDelay.fac_us = SysTickFreq / 1000000;
    
#ifdef SYSTEM_SUPPORT_OS
#if defined (OS_USE_RTTHREAD) || defined (OS_CRITICAL_METHOD) || defined (CPU_CFG_CRITICAL_METHOD)
    SysDelay.fac_ms = 1000 / DELAY_OS_TICK_PERSEC;
#endif
#else

    /* 非OS下,代表每个ms需要的systick时钟数 */
    SysDelay.fac_ms = (uint16_t)SysDelay.fac_us * 1000;				
#endif /* SYSTEM_SUPPORT_OS */
}		

/* 如果需要支持OS. */
#if SYSTEM_SUPPORT_OS 

/**
 * @func    delay_us
 * @brief   延时nus
 * @param   nus 要延时的us数	
 * @note    nus的范围0~204522252(最大值即2^32/fac_us@fac_us=21)
 * @retval  无
 */	  								   
void Delay_us(uint32_t nus)
{		
    uint32_t Ticks;
    uint32_t Told, Tnow, Tcnt = 0;
    uint32_t Reload;
    
    /* 获取当前LOAD的值 */
    Reload = SysTick->LOAD;				
    
    /* 计算需要的节拍数 */
    Ticks = nus * SysDelay.fac_us; 	
    
    /* 阻止OS调度，防止打断us延时 */
    DelayOsschedlock();	
    
    /* 记录当前的计数器的值 */
    Told = SysTick->VAL;    
    
    /* 进行 延时 */
    while (1)
    {
        Tnow = SysTick->VAL;	
        
        if (Tnow != Told)
        {	    
            /* 这里注意一下SYSTICK是一个递减的计数器就可以了 */
            if (Tnow < Told)
                Tcnt += Told - Tnow;	
            else 
                Tcnt += Reload - Tnow + Told;	    
            Told = Tnow;
            
            /* 时间超过/等于要延迟的时间,则退出 */
            if (Tcnt >= Ticks)
                break;			
        }  
    };
    
    /* 恢复OS调度 */			
    DelayOsschedunlock();												    
}  

/**
 * @func    delay_ms
 * @brief   延时nms
 * @param   nms 要延时的ms数	
 * @note    nms的0-65535
 * @retval  无
 */	
void Delay_ms(uint16_t nms)
{	
    uint16_t i;
#if defined (OS_CRITICAL_METHOD) || defined (CPU_CFG_CRITICAL_METHOD)
    
    /* 如果OS已经在跑了,并且不是在中断里面(中断里面不能任务调度) */
    if (DELAY_OS_RUNNING && (DELAY_OS_INTNESTING == 0))  
    {	
        /* 延时的时间大于OS的最少时间周期 */
        if (nms >= sys_delay.fac_ms)						
        { 
            /* OS延时 */
            delay_ostimedly(nms / sys_delay.fac_ms);	
        }
        
        /* OS已经无法提供这么小的延时了,采用普通方式延时 */
        nms %= sys_delay.fac_ms;						 
    }
    else
    {
        for (i = 0; i < nms; i++)
        {
            /* 普通方式延时 */
            delay_us(1000);	
        }
        return ;
    }
    
/* 如果使用rt-thread(中断里面不能任务调度) */
#elif defined (OS_USE_RTTHREAD)
    {
        static rt_uint8_t Delay_msLock = false;
        if((DELAY_OS_RUNNING | Delay_msLock) && (DELAY_OS_INTNESTING == 0))
        {
            
            /* 用于标志系统是否已经在运行 */
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
                /* 普通方式延时 */
                Delay_us(1000);	
            }
            return ;
        }	
    }
#endif
    
    /* 普通方式延时 */
    Delay_us((uint32_t)(nms * 1000));				
}

/* 不用ucos时 */
#else  

#if USE_CUBEMX_CREAT_CODE == 1

/**
 * @func    delay_us
 * @brief   延时nus
 * @param   nus 要延时的us数	
 * @note    nus的范围0~204522252(最大值即2^32/fac_us@fac_us=21)
 * @retval  无
 */	  								   
void Delay_us(uint32_t nus)
{		
    uint32_t Ticks;
    uint32_t Told, Tnow, Tcnt = 0;
    uint32_t Reload;
    
    /* 获取当前LOAD的值 */
    Reload = SysTick->LOAD;				
    
    /* 计算需要的节拍数 */
    Ticks = nus * SysDelay.fac_us; 	
    
    /* 记录当前的计数器的值 */
    Told = SysTick->VAL;    
    
    /* 进行 延时 */
    while (1)
    {
        Tnow = SysTick->VAL;	
        
        if (Tnow != Told)
        {	    
            /* 这里注意一下SYSTICK是一个递减的计数器就可以了 */
            if (Tnow < Told)
                Tcnt += Told - Tnow;	
            else 
                Tcnt += Reload - Tnow + Told;	    
            Told = Tnow;
            
            /* 时间超过/等于要延迟的时间,则退出 */
            if (Tcnt >= Ticks)
                break;			
        }  
    }											    
} 


/**
 * @func    delay_ms
 * @brief   延时nms
 * @param   nms 要延时的ms数	
 * @note    nms的0-65535
 * @retval  无
 */	
void Delay_ms(uint16_t nms)
{	
    HAL_Delay(nms);			
}
#else
/**
 * @func    Delay_us
 * @brief   延时nus
 * @param   nus 为要延时的us数.	
 * @note    nus的值,不要大于798915us(最大值即2^24/fac_us@fac_us=21)
 * @retval  无
 */	
void Delay_us(uint32_t nus)
{		
    uint32_t Temp;	    	 
    
    /* 时间加载 */
    SysTick->LOAD = nus * SysDelay.fac_us; 		
    
    /* 清空计数器 */
    SysTick->VAL = 0x00;        				
    
    /* 开始倒数 */
    SysTick->CTRL |= (uint32_t)0x00000001U ; 	 
    
    do
    {
        Temp = SysTick->CTRL;
    } while ((Temp & 0x01 ) && !(Temp & (1 << 16)));	//等待时间到达   
    
    /* 关闭计数器 */
    SysTick->CTRL &= ~((uint32_t)0x00000001U); 
    
    /* 清空计数器 */
    SysTick->VAL = 0X00; 
          				
}

/**
 * @func    Delay_xms
 * @brief   延时nms
 * @param   nms 为要延时的ms数.	
 * @note    注意nms的范围
            SysTick->LOAD为24位寄存器,所以,最大延时为:
            nms<=0xffffff*8*1000/SYSCLK
            SYSCLK单位为Hz,nms单位为ms
            对168M条件下,nms<=798ms 
 * @retval  无
 */	
void Delay_xms(uint16_t nms)
{	 		  	  
    uint32_t Temp;		   
    
    /* 时间加载(SysTick->LOAD为24bit) */
    SysTick->LOAD = (uint32_t)nms * SysDelay.fac_ms;		
    
    /* 清空计数器 */
    SysTick->VAL = 0x00;           
    
    /* 开始倒数 */
    SysTick->CTRL |= ((uint32_t)0x00000001U) ;    
    
    do
    {
        Temp = SysTick->CTRL;
    } while ((Temp & 0x01) && !(Temp & (1 << 16)));	//等待时间到达   
    
    /* 关闭计数器 */
    SysTick->CTRL &= ~((uint32_t)0x00000001U);     
    
    /* 清空计数器 */
    SysTick->VAL = 0X00;     		  		  	    
} 

/**
 * @func    Delay_ms
 * @brief   延时nms
 * @param   nms 为要延时的ms数.	
 * @note    nms:0~65535
 * @retval  无
 */	
void Delay_ms(uint16_t nms)
{	
#if SYSTICK_DIV8 == 1
    /* 这里用540,是考虑到某些客户可能超频使用 */
    uint16_t Repeat = nms / 540;	
    
    /* 比如超频到248M的时候,delay_xms最大只能延时541ms左右 */									
    uint16_t Remain = nms % 540;
    
    while (Repeat)
    {
        Delay_xms(540);
        Repeat--;
    }
#else
    /* 这里用540,是考虑到某些客户可能超频使用 */
    uint16_t Repeat = nms / 60;	
    
    /* 比如超频到248M的时候,delay_xms最大只能延时541ms左右 */									
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
			 



































