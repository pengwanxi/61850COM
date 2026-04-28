#include "private_list.h"

list_node_t *private_list_find(list_t *self, void *val,
                               int (*match)(void *a, void *b))
{
    list_iterator_t *it = list_iterator_new(self, LIST_HEAD);
    list_node_t *node;

    if (!match) {
        return NULL;
    }

    while ((node = list_iterator_next(it))) {
        if (NULL == val || NULL == node->val) {
            return NULL;
        }
        if (match(val, node->val)) {
            list_iterator_destroy(it);
            return node;
        }
        else {
            if (val == node->val) {
                list_iterator_destroy(it);
                return node;
            }
        }
    }

    list_iterator_destroy(it);
    return NULL;
}

list_node_t *private_list_next(list_t *self, list_iterator_t **it)
{
	if ( NULL == *it ) {
		*it = list_iterator_new(self, LIST_HEAD);
	}
    list_node_t *node = list_iterator_next(*it);
	if ( NULL == node ) {
		list_iterator_destroy(*it);
	}

    return node;
}
