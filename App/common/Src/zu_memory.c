/**
 ******************************************************************************
 * @file      myMemory.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-10-08
 * @brief     实现内存池内存管理方法
 * @History
 * Date           Author    version    		Notes
 * 2018-10-08     ZSY       V1.0.0          first version.
 */
	
/* Includes ------------------------------------------------------------------*/
#include "myMemory.h"

/* Functions */
MemInfo_t MemInfo;

/**
 * @func    MemInit
 * @brief   初始化内存池
 * @param   pool 内存池指针
 * @param   size 内存池大小
 * @note    
 * @retval  错误代码
 */
U32 MemInit(void *pool, U32 Size)
{
    MEMP *ptr;

    if ((pool == NULL) || (Size < sizeof(MEMP)))
    {
        return (1U);
    }

    MemInfo.hPool = pool;
    MemInfo.Size = Size;
    MemInfo.Available = Size;
    MemInfo.MaxUse = 0;

    ptr = (MEMP *)pool;
    ptr->next = (MEMP *)((U32)pool + Size - sizeof(MEMP *));
    ptr->next->next = NULL;
    ptr->len = 0U;

    return (0U);
}

/**
 * @func    MemAlloc
 * @brief   从内存池中申请内存
 * @param   size 内存块大小
 * @note    
 * @retval  内存指针
 */
void *MemMalloc(U32 size)
{
    MEMP *p, *p_search, *p_new;
    U32 hole_size;

    if ((MemInfo.hPool == NULL) || (size == 0U))
    {
        return NULL;
    }

    /* Add header offset to 'size' */
    size += sizeof(MEMP);
    /* Make sure that block is 4-byte aligned  */
    size = (size + 3U) & ~(U32)3U;

    p_search = (MEMP *)MemInfo.hPool;
    while (1)
    {
        hole_size = (U32)p_search->next - (U32)p_search;
        hole_size -= p_search->len;
        /* Check if hole size is big enough */
        if (hole_size >= size)
        {
            break;
        }
        p_search = p_search->next;
        if (p_search->next == NULL)
        {
            /* Failed, we are at the end of the list */
            return NULL;
        }
    }

    if (p_search->len == 0U)
    {
        /* No block is allocated, set the Length of the first element */
        p_search->len = size;
        p = (MEMP *)(((U32)p_search) + sizeof(MEMP));
    }
    else
    {
        /* Insert new list element into the memory list */
        p_new = (MEMP *)((U32)p_search + p_search->len);
        p_new->next = p_search->next;
        p_new->len = size;
        p_search->next = p_new;
        p = (MEMP *)(((U32)p_new) + sizeof(MEMP));
    }

    MemInfo.Available -= size;
    if ((MemInfo.Size - MemInfo.Available) > MemInfo.MaxUse)
    {
        MemInfo.MaxUse = MemInfo.Size - MemInfo.Available;
    }
    return (p);
}

/**
 * @func    MemCalloc
 * @brief   从内存池中申请内存并清零
 * @param   size 内存块大小
 * @note    
 * @retval  内存指针
 */
void *MemCalloc(U32 size)
{
    char *p;
    p = (char *)MemMalloc(size);
    if (p != NULL)
    {
        while(size--)
        {
            *p = 0;
            p++;
        }
    }
    return p;
}

/**
 * @func    MemFree
 * @brief   释放内存回到内存池
 * @param   mem 内存块指针
 * @note    
 * @retval  内存指针
 */
void MemFree(void *mem)
{
    MEMP *p_search, *p_prev, *p_return;

    if ((MemInfo.hPool == NULL) || (mem == NULL))
    {
        return;
    }

    p_return = (MEMP *)((U32)mem - sizeof(MEMP));

    /* Set list header */
    p_prev = NULL;
    p_search = (MEMP *)MemInfo.hPool;
    while (p_search != p_return)
    {
        p_prev = p_search;
        p_search = p_search->next;
        if (p_search == NULL)
        {
            /* Valid Memory block not found */
            return;
        }
    }
    
    MemInfo.Available += p_search->len;

    if (p_prev == NULL)
    {
        /* First block to be released, only set length to 0 */
        p_search->len = 0U;
    }
    else
    {
        /* Discard block from chain list */
        p_prev->next = p_search->next;
    }

    return;
}

/**
 * @func    MemUsage
 * @brief   统计内存使用率
 * @param   UsageMajor 整数部分
 * @param   Usageminor 小数部分
 * @note    
 * @retval  无
 */
void MemUsage(U8 *UsageMajor, U8 *Usageminor)
{
    U32 MemUsed;
    /* 计算内存使用 */
    MemUsed = MemInfo.Size - MemInfo.Available;
    
    *UsageMajor = (MemUsed * 100) / MemInfo.Size;
    *Usageminor = ((MemUsed * 100) % MemInfo.Size) * 100 / MemInfo.Size;
}
