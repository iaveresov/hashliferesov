#include "hash.h"
#include "list.h"
#include "typedefs.h"
#include <errno.h>
#include <quadrotree.h>
#include <stdio.h>
#include <stdlib.h>

static void *
ecalloc (size_t nmemb, size_t size)
{
  void *ptr = calloc (nmemb, size);
  if (!ptr)
    {
      perror ("hash:");
      exit (errno);
    }
  return ptr;
}

struct Pair
{
  KEY_T key;
  void *value;
};

struct Hash
{
  LIST **table;
  size_t size;
  size_t capacity;
  size_t hash_hits;
  HASH_FUNC f;
};

KEY_T
mix64 (KEY_T key)
{
  KEY_T hash = key;
  hash += KNUTH_CONST;
  hash = (hash ^ (hash >> 30)) * FIRST_CONST;
  hash = (hash ^ (hash >> 27)) * SECOND_CONST;
  return hash ^ (hash >> 31);
}

void
Hash_increase_hits(HASH *hash)
{
    hash->hash_hits++;
}

size_t 
Hash_get_hits(HASH *hash)
{
    return hash->hash_hits;
}

HASH *
Hash_init (HASH_FUNC function)
{
  HASH *table = (HASH *)ecalloc (1, sizeof (HASH));
  table->table = (LIST **)calloc (TABLE_SIZE, sizeof (LIST *));
  table->f = function;
  table->size = 0;
  table->capacity = TABLE_SIZE;
  table->hash_hits = 0;

  QTREE *dead_leaf, *live_leaf;
  dead_leaf = (QTREE *)calloc (1, sizeof (QTREE));
  live_leaf = (QTREE *)calloc (1, sizeof (QTREE));
  dead_leaf->node_key = DEAD_CELL;
  live_leaf->node_key = LIVE_CELL;
  live_leaf->population = 1;
  dead_leaf->population = 0;

  Hash_insert (table, DEAD_CELL, (void *)dead_leaf);
  Hash_insert (table, LIVE_CELL, (void *)live_leaf);

  return table;
}

size_t
Hash_size (HASH *hash)
{
  return hash->size;
}

size_t
Hash_capacity (HASH *hash)
{
  return hash->capacity;
}

bool
has_key (void *element, void *load)
{
  return ((PAIR *)element)->key == *((KEY_T *)load);
}

void
Hash_insert (HASH *hash, KEY_T key, void *value)
{

  // resize if load factor is too big
  
  if (hash->size * 2 >= hash->capacity)
    {
      Hash_resize (&hash);
    }

   
  size_t index = hash->f (key, hash->capacity);
  PAIR *key_value;
  LIST **list = &(hash->table[index]);

  if (List_search (*list, &has_key, &key))
    {
      return;
    }

  key_value = (PAIR *)ecalloc (1, sizeof (PAIR));
  key_value->key = key;
  key_value->value = value;
  List_prepend (list, key_value);
  hash->size++;
}

void
Hash_resize (HASH **hash)
{
  HASH *current_hash = *hash;
  size_t old_capacity = current_hash->capacity;
  current_hash->capacity *= 2;
  LIST **current_table = current_hash->table;
  current_hash->table
      = (LIST **)ecalloc (current_hash->capacity, sizeof (LIST *));

  for (size_t i = 0; i < old_capacity; ++i)
    {
      LIST *node = current_table[i];
      while (node)
        {
          PAIR *pair = (PAIR *)List_get_value (node);
          KEY_T key = pair->key;
          void *value = pair->value;
          Hash_insert (*hash, key, value);
          LIST *temp = List_next (node);
          free (pair);
          free (node);
          node = temp;
        }
    }
  free (current_table);
}

// int Hash_remove(HASH *hash, KEY key);

void *
Hash_get (HASH *hash, KEY_T key)
{
  size_t index = hash->f (key, hash->capacity);
  LIST *list;
  PAIR *key_value;

  if (!hash->table[index])
    {
      return NULL;
    }
  else
    {
      list = hash->table[index];
      if ((key_value = (PAIR *)List_search (list, &has_key, &key)))
        {
          return key_value->value;
        }

      return NULL;
    }
}
