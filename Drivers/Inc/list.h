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
#define ZU_CONTAINER_OF(ptr, type, member) \
     ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/**
 * @brief get the struct for this entry
 * @param node the entry point
 * @param type the type of structure
 * @param member the name of list in structure
 */
#define ZU_LIST_ENTRY(node, type, member) \
      ZU_CONTAINER_OF(node, type, member)

/**
 * LIST_FOR_EACH_ENTRY  -   iterate over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define ZU_LIST_FOR_EACH_ENTRY(pos, head, member)                 \
      for (pos = ZU_LIST_ENTRY((head)->next, typeof(*pos), member); \
              &pos->member != (head);                               \
           pos = ZU_LIST_ENTRY(pos->member.next, typeof(*pos), member))

/**
 * LIST_FOR_EACH_ENTRY_SAFE - iterate over list of given type safe against removal of list entry
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define ZU_LIST_FOR_EACH_ENTRY_SAFE(pos, n, head, member)                    \
           for (pos = ZU_LIST_ENTRY((head)->next, typeof(*pos), member),     \
                  n = ZU_LIST_ENTRY(pos->member.next, typeof(*pos), member); \
                   &pos->member != (head);                                   \
         pos = n, n = ZU_LIST_ENTRY(n->member.next, typeof(*n), member))

/**
 * LIST_FIRST_ENTRY - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define ZU_LIST_FIRST_ENTRY(ptr, type, member) \
              ZU_LIST_ENTRY((ptr)->next, type, member)

/**
 * @brief get the struct for this single list node
 * @param node the entry point
 * @param type the type of structure
 * @param member the name of list in structure
 */
#define ZU_SLIST_ENTRY(node, type, member) \
       ZU_CONTAINER_OF(node, type, member)

/**
 * SLIST_FOR_EACH_ENTRY  -   iterate over single list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your single list.
 * @member: the name of the list_struct within the struct.
 */
#define ZU_SLIST_FOR_EACH_ENTRY(pos, head, member)                  \
      for (pos = ZU_SLIST_ENTRY((head)->next, typeof(*pos), member);  \
           &pos->member != (NULL);                                    \
           pos = ZU_SLIST_ENTRY(pos->member.next, typeof(*pos), member))

/**
 * SLIST_FIRST_ENTRY - get the first element from a slist
 * @ptr:    the slist head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the slist_struct within the struct.
 *
 * Note, that slist is expected to be not empty.
 */
#define ZU_SLIST_FIRST_ENTRY(ptr, type, member) \
              ZU_SLIST_ENTRY((ptr)->next, type, member)

/**
 * Double List structure
 */
struct zu_list_node
{
    struct zu_list_node *next; /**< point to next node. */
    struct zu_list_node *prev; /**< point to prev node. */
};
typedef struct zu_list_node zu_list_t; /**< Type for lists. */
typedef struct zu_list_node *p_zu_list_t; /**< Type for lists. */

/**
 * Single List structure
 */
struct zu_slist_node
{
    struct zu_slist_node *next; /**< point to next node. */
};
typedef struct zu_slist_node zu_slist_t; /**< Type for single list. */
typedef struct zu_slist_node *p_zu_slist_t; /**< Type for single list. */

/**
 * @brief initialize a list
 *
 * @param l list to be initialized
 */
static inline void list_init(p_zu_list_t l)
{
    l->next = l->prev = l;
}
/**
 * @brief insert a node after a list
 *
 * @param l list to insert it
 * @param n new node to be inserted
 */
static inline void list_insert_after(p_zu_list_t l, p_zu_list_t n)
{
    l->next->prev = n;
    n->next = l->next;

    l->next = n;
    n->prev = l;
}

/**
 * @brief insert a node before a list
 *
 * @param n new node to be inserted
 * @param l list to insert it
 */
static inline void list_insert_before(p_zu_list_t l, p_zu_list_t n)
{
    l->prev->next = n;
    n->prev = l->prev;

    l->prev = n;
    n->next = l;
}

/**
 * @brief remove node from list.
 * @param n the node to remove from the list.
 */
static inline void list_remove(p_zu_list_t n)
{
    n->next->prev = n->prev;
    n->prev->next = n->next;

    n->next = n->prev = n;
}

/**
 * @brief tests whether a list is empty
 * @param l the list to test.
 */
static inline int is_list_empty(const p_zu_list_t l)
{
    return l->next == l;
}

/**
 * @brief get the list length
 * @param l the list to get.
 */
static inline unsigned int list_len(const p_zu_list_t l)
{
    unsigned int len = 0;
    p_zu_list_t p = l;
    while (p->next != l)
    {
        p = p->next;
        len++;
    }

    return len;
}

/**
 * @brief initialize a single list
 *
 * @param l the single list to be initialized
 */
static inline void sList_init(p_zu_slist_t l)
{
    l->next = NULL;
}

static inline void slist_append(p_zu_slist_t l, p_zu_slist_t n)
{
    p_zu_slist_t node;

    node = l;
    while (node->next)
        node = node->next;

    /* append the node to the tail */
    node->next = n;
    n->next = NULL;
}

static inline void slist_insert(p_zu_slist_t l, p_zu_slist_t n)
{
    n->next = l->next;
    l->next = n;
}

static inline unsigned int slist_len(const p_zu_slist_t l)
{
    unsigned int len = 0;
    p_zu_slist_t list = l->next;
    while (list != NULL)
    {
        list = list->next;
        len++;
    }

    return len;
}

static inline zu_slist_t *slist_remove(p_zu_slist_t l, p_zu_slist_t n)
{
    /* remove slist head */
    zu_slist_t *node = l;
    while (node->next && node->next != n)
        node = node->next;

    /* remove node */
    if (node->next != (p_zu_slist_t)0)
        node->next = node->next->next;

    return l;
}

static inline int is_slist_empty(p_zu_slist_t l)
{
    return l->next == NULL;
}

#endif
