#include <stdbool.h>
#include <stddef.h>
#ifndef QUEUE_H
#define QUEUE_H 1

typedef struct Queue QUEUE;
typedef struct Node NODE;

QUEUE *Queue_init (size_t line_num);
void Queue_append (QUEUE *queue, void *value, size_t line);
void *Queue_pop (QUEUE *queue, size_t line);
bool Queue_is_empty (QUEUE *queue);
void Queue_print(QUEUE *queue, size_t line);
void Queue_free (QUEUE *queue);

#endif
