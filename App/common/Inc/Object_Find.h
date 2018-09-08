#ifndef _OBJECT_FIND_
#define _OBJECT_FIND_

#include "rtthread.h"

rt_sem_t rt_sem_find(char *name);
rt_mutex_t rt_mutex_find(char *name);
rt_mailbox_t rt_mb_find(char *name);
rt_event_t rt_event_find(char *name);
rt_mq_t rt_mq_find(char *name);
rt_timer_t rt_timer_find(char *name);

#endif

