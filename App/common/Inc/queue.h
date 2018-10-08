/**
 ******************************************************************************
 * @Copyright     (C) 2017 - 2018 guet-sctc-hardwarepart Team
 * @filename      queue.h
 * @author        ZSY
 * @version       V1.0.0
 * @date          2018-10-08
 * @Description   队列接口文件
 * @Others
 * @History
 * Date           Author    version    		Notes
 * 2018-10-08      ZSY      V1.0.0      first version.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "list.h"
#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"

/* 枚举错误代码 */
enum
{
    QUEUE_ERR = -1,
    QUEUE_OK = 0,
    QUEUE_IS_FULL = 1,
    QUEUE_IS_EMPTY = 2
};

/* 数据节点 */
typedef struct
{
    const void *DataPtr;
    List_t List;
} DataItem_t;

/* data queue implementation */
struct queue
{
    uint16_t Size;
    uint16_t Count;

    List_t *front;
    List_t *rear;
};
typedef struct queue Queue_t;

typedef uint8_t err_t;

/* APIs */
void QueueInitHooks(void *(*Malloc)(size_t Size), void (*Free)(void *prt));
err_t QueueInit(Queue_t *queue, uint16_t size);
err_t Enqueue(Queue_t *queue, const void *Ptr);
err_t Dequeue(Queue_t *queue, const void **Ptr);
err_t QueueCount(Queue_t *queue, uint32_t *Count);
void QueueClear(Queue_t *queue);
uint8_t isQueueEmpty(Queue_t *queue);

#endif
