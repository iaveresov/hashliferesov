#include <stddef.h>
#ifndef LIST_H
#define LIST_H 1

#include <stdbool.h>

typedef struct List LIST;

size_t List_get_size(void);
void List_prepend (LIST **list, void *value);
void *List_search (LIST *list, bool (*predicate) (void *element, void *load),
                   void *load);
void *List_value(LIST *node);
void List_remove (LIST *list, void *element);
void List_free (LIST *list);
LIST *List_next(LIST *list);
void *List_get_value(LIST *list);

#endif
