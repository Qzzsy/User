/**
 ******************************************************************************
 * @file      queue.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-10-08
 * @brief     文件内包含着队列的操作方法
 * @History
 * Date           Author    version    		Notes
 * 2018-10-08     ZSY       V1.0.0          first version.
 */
	
/* Includes ------------------------------------------------------------------*/
#include "queue.h"
#include "list.h"

/* 内存操作方法 */
typedef struct
{
    void *(*QueueMalloc)(size_t Size);
    void (*QueueFree)(void *Ptr);
} QueueHooks_t;

/* 初始化内存操作方法 */
static QueueHooks_t Hooks =
    {
        .QueueMalloc = malloc,
        .QueueFree = free};

/**
 * @func    QueueInitHooks
 * @brief   初始化队列的钩子方法
 * @param   Malloc 内存申请
 * @param   Free 内存释放
 * @note    
 * @retval  无
 */
void QueueInitHooks(void *(*Malloc)(size_t Size), void (*Free)(void *prt))
{
    if (Malloc != NULL && Free != NULL)
    {
        Hooks.QueueMalloc = Malloc;
        Hooks.QueueFree = Free;
    }
}

/**
 * @func    QueueInit
 * @brief   队列初始化
 * @param   queue 队列对象
 * @param   size 最大节点数量
 * @note    
 * @retval  错误代码
 */
err_t QueueInit(Queue_t *queue, uint16_t size)
{
    List_t *l;
    err_t result;
    if (queue == NULL)
    {
        result = (err_t)QUEUE_ERR;
        goto __exit;
    }

    queue->Size = size;
    queue->Count = 0;

    l = (List_t *)Hooks.QueueMalloc(sizeof(List_t));
    if (l == NULL)
    {
        result = (err_t)QUEUE_ERR;
        goto __exit;
    }

    ListInit(l);

    queue->front = l->Next;
    queue->rear = l->Prev;
    result = QUEUE_OK;

__exit:
    return result;
}

/**
 * @func    Enqueue
 * @brief   向队列的末端添加一个元素
 * @param   queue 队列对象
 * @param   Ptr 队列节点对象的内容
 * @note    
 * @retval  错误代码
 */
err_t Enqueue(Queue_t *queue, const void *Ptr)
{
    DataItem_t *pNew;
    uint8_t result;

    if (queue == NULL || Ptr == NULL)
    {
        result = (err_t)QUEUE_ERR;
        goto __exit;
    }

    result = QUEUE_OK;

    if (queue->Count == queue->Size)
    {
        result = QUEUE_IS_FULL;
        goto __exit;
    }

    pNew = (DataItem_t *)Hooks.QueueMalloc(sizeof(DataItem_t));
    if (pNew == NULL)
    {
        result = (err_t)QUEUE_ERR;

        goto __exit;
    }

    pNew->DataPtr = Ptr;
    queue->Count++;

    ListInsertAfter(queue->front, &pNew->List);

    queue->front = queue->front->Next;
__exit:
    return result;
}

/**
 * @func    Dequeue
 * @brief   出队，提取当前队列的第一个元素
 * @param   queue 队列对象
 * @param   Ptr 数据指针
 * @note    
 * @retval  错误代码
 */
err_t Dequeue(Queue_t *queue, const void **Ptr)
{
    err_t result;

    DataItem_t *pOut;

    if (queue == NULL || Ptr == NULL)
    {
        result = (err_t)QUEUE_ERR;
        goto __exit;
    }

    result = QUEUE_OK;

    if (queue->Count == 0 || isListEmpty(queue->rear))
    {
        result = QUEUE_IS_EMPTY;
        goto __exit;
    }

    pOut = (DataItem_t *)LIST_ENTRY(queue->rear->Next, DataItem_t, List);

    *Ptr = pOut->DataPtr;

    ListRemove(queue->rear->Next);
    queue->Count--;
    if (queue->Count == 0)
    {
        queue->front = queue->rear;
    }

    Hooks.QueueFree(pOut);
    result = QUEUE_OK;

__exit:

    return result;
}

/**
 * @func    QueueCount
 * @brief   获取当前队列的节点数量
 * @param   queue 队列对象
 * @param   Count 节点数量指针
 * @note    
 * @retval  错误代码
 */
err_t QueueCount(Queue_t *queue, uint32_t *Count)
{
    *Count = queue->Count;

    return QUEUE_OK;
}

/**
 * @func    QueueClear
 * @brief   清除队列节点
 * @param   queue 队列对象
 * @note    可能会造成内存泄漏，小心使用
 * @retval  无
 */
void QueueClear(Queue_t *queue)
{
    DataItem_t *pOut;
    /* resume on pop list */
    while (queue->Count != 0)
    {
        pOut = (DataItem_t *)LIST_ENTRY(queue->rear->Next, DataItem_t, List);

        ListRemove(queue->rear->Next);

        Hooks.QueueFree(pOut);
        queue->Count--;
    }
}

/**
 * @func    isQueueEmpty
 * @brief   获取当前队列的状态
 * @param   queue 队列对象
 * @note    
 * @retval  队列状态
 */
uint8_t isQueueEmpty(Queue_t *queue)
{
    return queue->Count == 0;
}

