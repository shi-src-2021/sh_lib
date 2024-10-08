#ifndef __SH_LIST_H__
#define __SH_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#define sh_list_inline  static __inline

typedef struct sh_list_node {
    struct sh_list_node *prev;
    struct sh_list_node *next;
} sh_list_t;

sh_list_inline void sh_list_init(sh_list_t *list)
{
    list->next = list;
    list->prev = list;
}

sh_list_inline void sh_list_insert_before(sh_list_t *list, sh_list_t *pos)
{
    list->next = pos;
    list->prev = pos->prev;

    pos->prev->next = list;
    pos->prev = list;
}

sh_list_inline void sh_list_insert_after(sh_list_t *list, sh_list_t *pos)
{
    list->next = pos->next;
    list->prev = pos;

    pos->next->prev = list;
    pos->next = list;
}

sh_list_inline void sh_list_remove(sh_list_t *list)
{
    list->prev->next = list->next;
    list->next->prev = list->prev;

    list->next = list->prev = list;
}

sh_list_inline int sh_list_isempty(sh_list_t *list)
{
    return (list->next == list);
}

sh_list_inline int sh_list_len(sh_list_t *list)
{
    int len = 0;
    sh_list_t *p = list;

    while (p->next != list) {
        len++;
        p = p->next;
    }

    return len;
}

#define SH_LIST_INIT(list) sh_list_t list = {&list, &list}

#define sh_list_for_each(node, head) \
            for (sh_list_t *node = (head)->next; node != (head); node = node->next)

#define sh_list_for_each_safe(node, head) \
            for (sh_list_t *node = (head)->next, *n = node->next; \
                node != (head); node = n, n = node->next)

#ifdef __cplusplus
}   /* extern "C" */ 
#endif

#endif

