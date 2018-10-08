/**
 ******************************************************************************
 * @Copyright     (C) 2017 - 2018 guet-sctc-hardwarepart Team
 * @filename      list.h
 * @author        ZSY
 * @version       V1.0.0
 * @date          2018-10-08
 * @Description   双向环形链表实现
 * @Others
 * @History
 * Date           Author    version    		Notes
 * 2018-10-08      ZSY      V1.0.0      first version.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _LIST_H_
#define _LIST_H_

#ifndef NULL
#define NULL ((void *)0)
#endif
/**
 * rt_container_of - return the member address of ptr, if the type of ptr is the
 * struct type.
 */
#define CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/**
 * @brief get the struct for this entry
 * @param node the entry point
 * @param type the type of structure
 * @param member the name of list in structure
 */
#define LIST_ENTRY(node, type, member) \
    CONTAINER_OF(node, type, member)

/**
 * LIST_FOR_EACH_ENTRY  -   iterate over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define LIST_FOR_EACH_ENTRY(pos, head, member)                 \
    for (pos = LIST_ENTRY((head)->next, typeof(*pos), member); \
         &pos->member != (head);                               \
         pos = LIST_ENTRY(pos->member.next, typeof(*pos), member))

/**
 * LIST_FOR_EACH_ENTRY_SAFE - iterate over list of given type safe against removal of list entry
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define LIST_FOR_EACH_ENTRY_SAFE(pos, n, head, member)          \
    for (pos = LIST_ENTRY((head)->next, typeof(*pos), member),  \
        n = LIST_ENTRY(pos->member.next, typeof(*pos), member); \
         &pos->member != (head);                                \
         pos = n, n = LIST_ENTRY(n->member.next, typeof(*n), member))

/**
 * LIST_FIRST_ENTRY - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define LIST_FIRST_ENTRY(ptr, type, member) \
    LIST_ENTRY((ptr)->next, type, member)

/**
 * @brief get the struct for this single list node
 * @param node the entry point
 * @param type the type of structure
 * @param member the name of list in structure
 */
#define SLIST_ENTRY(node, type, member) \
    CONTAINER_OF(node, type, member)

/**
 * SLIST_FOR_EACH_ENTRY  -   iterate over single list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your single list.
 * @member: the name of the list_struct within the struct.
 */
#define SLIST_FOR_EACH_ENTRY(pos, head, member)                 \
    for (pos = SLIST_ENTRY((head)->next, typeof(*pos), member); \
         &pos->member != (RT_NULL);                             \
         pos = SLIST_ENTRY(pos->member.next, typeof(*pos), member))

/**
 * SLIST_FIRST_ENTRY - get the first element from a slist
 * @ptr:    the slist head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the slist_struct within the struct.
 *
 * Note, that slist is expected to be not empty.
 */
#define SLIST_FIRST_ENTRY(ptr, type, member) \
    SLIST_ENTRY((ptr)->next, type, member)

/**
 * Double List structure
 */
struct ListNode
{
    struct ListNode *Next; /**< point to next node. */
    struct ListNode *Prev; /**< point to prev node. */
};
typedef struct ListNode List_t; /**< Type for lists. */

/**
 * Single List structure
 */
struct SlistNode
{
    struct SlistNode *Next; /**< point to next node. */
};
typedef struct SlistNode Slist_t; /**< Type for single list. */

/**
 * @brief initialize a list
 *
 * @param l list to be initialized
 */
static inline void ListInit(List_t *l)
{
    l->Next = l->Prev = l;
}
/**
 * @brief insert a node after a list
 *
 * @param l list to insert it
 * @param n new node to be inserted
 */
static inline void ListInsertAfter(List_t *l, List_t *n)
{
    l->Next->Prev = n;
    n->Next = l->Next;

    l->Next = n;
    n->Prev = l;
}

/**
 * @brief insert a node before a list
 *
 * @param n new node to be inserted
 * @param l list to insert it
 */
static inline void ListInsertBefore(List_t *l, List_t *n)
{
    l->Prev->Next = n;
    n->Prev = l->Prev;

    l->Prev = n;
    n->Next = l;
}

/**
 * @brief remove node from list.
 * @param n the node to remove from the list.
 */
static inline void ListRemove(List_t *n)
{
    n->Next->Prev = n->Prev;
    n->Prev->Next = n->Next;

    n->Next = n->Prev = n;
}

/**
 * @brief tests whether a list is empty
 * @param l the list to test.
 */
static inline int isListEmpty(const List_t *l)
{
    return l->Next == l;
}

/**
 * @brief get the list length
 * @param l the list to get.
 */
static inline unsigned int ListLen(const List_t *l)
{
    unsigned int len = 0;
    const List_t *p = l;
    while (p->Next != l)
    {
        p = p->Next;
        len++;
    }

    return len;
}

/**
 * @brief initialize a single list
 *
 * @param l the single list to be initialized
 */
static inline void sListInit(Slist_t *l)
{
    l->Next = NULL;
}

static inline void SlistAppend(Slist_t *l, Slist_t *n)
{
    struct SlistNode *node;

    node = l;
    while (node->Next)
        node = node->Next;

    /* append the node to the tail */
    node->Next = n;
    n->Next = NULL;
}

static inline void SlistInsert(Slist_t *l, Slist_t *n)
{
    n->Next = l->Next;
    l->Next = n;
}

static inline unsigned int SlistLen(const Slist_t *l)
{
    unsigned int len = 0;
    const Slist_t *list = l->Next;
    while (list != NULL)
    {
        list = list->Next;
        len++;
    }

    return len;
}

static inline Slist_t *SlistRemove(Slist_t *l, Slist_t *n)
{
    /* remove slist head */
    struct SlistNode *node = l;
    while (node->Next && node->Next != n)
        node = node->Next;

    /* remove node */
    if (node->Next != (Slist_t *)0)
        node->Next = node->Next->Next;

    return l;
}

static inline int isSlistEmpty(Slist_t *l)
{
    return l->Next == NULL;
}

#endif
