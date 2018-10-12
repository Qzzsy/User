/**
 ******************************************************************************
 * @file      queue.c
 * @author    ZSY
 * @version   V1.0.0
 * @date      2018-10-08
 * @brief     �ļ��ڰ����Ŷ��еĲ�������
 * @History
 * Date           Author    version    		Notes
 * 2018-10-08     ZSY       V1.0.0          first version.
 */
	
/* Includes ------------------------------------------------------------------*/
#include "queue.h"
#include "list.h"

/* �ڴ�������� */
typedef struct
{
    void *(*QueueMalloc)(size_t Size);
    void (*QueueFree)(void *Ptr);
} QueueHooks_t;

/* ��ʼ���ڴ�������� */
static QueueHooks_t Hooks =
    {
        .QueueMalloc = malloc,
        .QueueFree = free};

/**
 * @func    QueueInitHooks
 * @brief   ��ʼ�����еĹ��ӷ���
 * @param   Malloc �ڴ�����
 * @param   Free �ڴ��ͷ�
 * @note    
 * @retval  ��
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
 * @brief   ���г�ʼ��
 * @param   queue ���ж���
 * @param   size ���ڵ�����
 * @note    
 * @retval  �������
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
 * @brief   ����е�ĩ�����һ��Ԫ��
 * @param   queue ���ж���
 * @param   Ptr ���нڵ���������
 * @note    
 * @retval  �������
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
 * @brief   ���ӣ���ȡ��ǰ���еĵ�һ��Ԫ��
 * @param   queue ���ж���
 * @param   Ptr ����ָ��
 * @note    
 * @retval  �������
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
 * @brief   ��ȡ��ǰ���еĽڵ�����
 * @param   queue ���ж���
 * @param   Count �ڵ�����ָ��
 * @note    
 * @retval  �������
 */
err_t QueueCount(Queue_t *queue, uint32_t *Count)
{
    *Count = queue->Count;

    return QUEUE_OK;
}

/**
 * @func    QueueClear
 * @brief   ������нڵ�
 * @param   queue ���ж���
 * @note    ���ܻ�����ڴ�й©��С��ʹ��
 * @retval  ��
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
 * @brief   ��ȡ��ǰ���е�״̬
 * @param   queue ���ж���
 * @note    
 * @retval  ����״̬
 */
uint8_t isQueueEmpty(Queue_t *queue)
{
    return queue->Count == 0;
}

