#include <stdlib.h>

#include "list.h"

struct List
{
  void *value;
  LIST *next;
};

void
List_prepend (LIST **list, void *value)
{
  LIST *node = malloc (sizeof (LIST));
  node->value = value;
  node->next = *(list);
  *(list) = node;
}

LIST *
List_next (LIST *list)
{
  if (list == NULL || list->next == NULL)
    {
      return NULL;
    }
  return list->next;
}

void *
List_get_value (LIST *list)
{
  return list->value;
}

void *
List_search (LIST *list, bool (*predicate) (void *element, void *load),
             void *load)
{
  if (!list)
    {
      return NULL;
    }

  do
    {
      if (predicate (list->value, load))
        {
          return list->value;
        }
      list = list->next;
    }
  while (list);

  return NULL;
}
