/**
 *   \file private_list.h
 *   \brief 根据库自定义一些函数
 */
#ifndef _PRIVATE_LIST_H_
#define _PRIVATE_LIST_H_

#include "list.h"

list_node_t *private_list_find(list_t *self, void *val,
                               int (*match)(void *a, void *b));

list_node_t *private_list_next(list_t *self, list_iterator_t **it);

#define PRIVATE_LIST_WHILE_EACH(self)                                          \
    list_node_t *node;                                                         \
    list_iterator_t *it = NULL;                                                \
    while ((node = private_list_next(self, &it)))

#define PRIVATE_LIST_DESTROY_IT(it)                                            \
    if (NULL != it) {                                                          \
        list_iterator_destroy(it);                                             \
        it = NULL;                                                             \
    }

#define PRIVATE_LIST_FOR_EACH(self, it, node)                                  \
    while ((node = private_list_next(self, &it)))

#endif /* _PRIVATE_LIST_H_ */
