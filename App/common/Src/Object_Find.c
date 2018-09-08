#include "Object_Find.h"

static rt_object_t object_find(char *name, rt_uint8_t type)
{
    struct rt_object_information *information;
    struct rt_object *object;
    struct rt_list_node *node;

    /* enter critical */
    if (rt_thread_self() != RT_NULL)
        rt_enter_critical();

    /* try to find device object */
    information = rt_object_get_information((enum rt_object_class_type)type);
    RT_ASSERT(information != RT_NULL);
    for (node  = information->object_list.next;
         node != &(information->object_list);
         node  = node->next)
    {
        object = rt_list_entry(node, struct rt_object, list);
        if (rt_strncmp(object->name, name, RT_NAME_MAX) == 0)
        {
            /* leave critical */
            if (rt_thread_self() != RT_NULL)
                rt_exit_critical();

            return (rt_object_t)object;
        }
    }

    /* leave critical */
    if (rt_thread_self() != RT_NULL)
        rt_exit_critical();

    /* not found */
    return RT_NULL;
}

/**
 * This function will find the specified Semaphore.
 *
 * @param name the name of Semaphore finding
 *
 * @return the found Semaphore
 *
 * @note please don't invoke this function in interrupt status.
 */
rt_sem_t rt_sem_find(char *name)
{
    RT_ASSERT(name);
    
    return (rt_sem_t)object_find(name, RT_Object_Class_Semaphore);
}

/**
 * This function will find the specified mutex.
 *
 * @param name the name of mutex finding
 *
 * @return the found mutex
 *
 * @note please don't invoke this function in interrupt status.
 */
rt_mutex_t rt_mutex_find(char *name)
{
    RT_ASSERT(name);
    
    return (rt_mutex_t)object_find(name, RT_Object_Class_Mutex);
}

/**
 * This function will find the specified MailBox.
 *
 * @param name the name of mailbox finding
 *
 * @return the found mailbox
 *
 * @note please don't invoke this function in interrupt status.
 */
rt_mailbox_t rt_mb_find(char *name)
{
    RT_ASSERT(name);
    
    return (rt_mailbox_t)object_find(name, RT_Object_Class_MailBox);
}

/**
 * This function will find the specified event.
 *
 * @param name the name of event finding
 *
 * @return the found event
 *
 * @note please don't invoke this function in interrupt status.
 */
rt_event_t rt_event_find(char *name)
{
    RT_ASSERT(name);
    
    return (rt_event_t)object_find(name, RT_Object_Class_Event);
}

/**
 * This function will find the specified MessageQueue.
 *
 * @param name the name of MessageQueue finding
 *
 * @return the found MessageQueue
 *
 * @note please don't invoke this function in interrupt status.
 */
rt_mq_t rt_mq_find(char *name)
{
    RT_ASSERT(name);
    
    return (rt_mq_t)object_find(name, RT_Object_Class_MessageQueue);
}

/**
 * This function will find the specified timer.
 *
 * @param name the name of timer finding
 *
 * @return the found timer
 *
 * @note please don't invoke this function in interrupt status.
 */
rt_timer_t rt_timer_find(char *name)
{
    RT_ASSERT(name);
    
    return (rt_timer_t)object_find(name, RT_Object_Class_Timer);
}

