#include "queue.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node
{
  struct Node *next;
  void *value;
};

// multi-dimensional queue
struct Queue
{
  NODE **heads;
  NODE **tails;
  size_t *line_lens;
  size_t line_num;
  bool *is_empty;
};

QUEUE *
Queue_init (size_t line_num)
{
  QUEUE *queue = (QUEUE *)calloc (1, sizeof (QUEUE));
  queue->heads = (NODE **)calloc (line_num, sizeof (NODE *));
  queue->tails = (NODE **)calloc (line_num, sizeof (NODE *));
  queue->line_lens = (size_t *)calloc (line_num, sizeof (size_t));
  queue->is_empty = (bool *)calloc(line_num, sizeof(bool));
  memset(queue->is_empty, 1, line_num * sizeof(bool)); 

  return queue;
}

void
Queue_append (QUEUE *queue, void *value, size_t line)
{
  if (line > queue->line_num)
    {
      queue->heads
          = (NODE **)realloc (queue->heads, (line + 1) * sizeof (NODE *));
      if (!queue->heads)
        {
          perror ("Queue_append");
          exit (errno);
        }

      queue->tails
          = (NODE **)realloc (queue->tails, (line + 1) * sizeof (NODE *));

      if (!queue->tails)
        {
          perror ("Queue_append");
          exit (errno);
        }
      // line indexing starts from 0
      queue->line_num = line + 1;
    }

  NODE *new = (NODE *)calloc (1, sizeof (NODE));

  ++queue->line_lens[line];
  new->value = value;
  if (queue->line_lens[line] == 1)
    {
      queue->heads[line] = new;
      queue->tails[line] = new;
      return;
    }

  queue->tails[line]->next = new;
  queue->tails[line] = new;
}

void *
Queue_pop (QUEUE *queue, size_t line)
{
  if (!queue->line_lens[line] || queue->line_num < line)
    {
      errno = ENOKEY;
      perror ("Queue_pop()");
      exit (errno);
    }

  NODE *head = queue->heads[line];
  queue->line_lens[line]--;
  void *value = head->value;
  queue->heads[line] = head->next;
  free (head);
  if (!queue->line_lens)
    {
      queue->is_empty[line] = 1;
    }
  return value;
}

bool
Queue_is_empty (QUEUE *queue)
{
  bool res = 1;
  for (int i = 0; i < queue->line_num; ++i)
    {
      res &= queue->is_empty[i];

      if (!res)
        {
          return res;
        }
    }
  return res;
}

// debug print selected floor of queue
void
Queue_print (QUEUE *queue, size_t line)
{

  NODE *head = queue->heads[line];
  printf ("line %lu: ", line);
  while (head)
    {
      printf ("%d ", *(int *)head->value);
      head = head->next;
    }
  printf ("\n");
}

void
Queue_free (QUEUE *queue)
{
  NODE *current;
  for (size_t i; i < queue->line_num; ++i)
    {
      current = queue->heads[i];
      while (current)
        {
          NODE *next = current->next;
          free (current);
          current = next;
        }
    }
  free (queue->line_lens);
  free (queue);
}
