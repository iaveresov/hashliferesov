#include "list.h"
#include <stdlib.h>

struct List
{
    void *value;
    LIST *next;
};

LIST *List_init(void)
{
    return (LIST *)malloc(sizeof(LIST));
}

void List_prepend(LIST **list, void *value)
{
    LIST *node = malloc(sizeof(LIST));
    node->value = value;
    node->next = *(list);
    *(list) = node;
}

void *List_search(LIST *list, bool (*predicate)(void *element, void *load), void *load)
{
    if (!list)
    {
        return NULL;
    }

    do {
        if (predicate(list->value, load))
        {
            return list->value;
        }
        list = list->next;
    } while (list);

    return NULL;
}

