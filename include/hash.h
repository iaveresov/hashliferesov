#ifndef HASH_H
#define HASH_H 1

#include <stddef.h>

#include "typedefs.h"
#define TABLE_SIZE 4096 
#define KNUTH_CONST 0x9e3779b97f4a7c15ULL
#define FIRST_CONST 0xbf58476d1ce4e5b9ULL
#define SECOND_CONST 0x94d049bb133111ebULL

typedef unsigned long KEY_T;

typedef struct Pair PAIR;
typedef size_t (*HASH_FUNC)(KEY_T, size_t);

KEY_T mix64(KEY_T key);
HASH *Hash_init(HASH_FUNC function);
void Hash_insert(HASH *hash, KEY_T key, void *value);
int Hash_remove(HASH *hash, KEY_T key);
void *Hash_get(HASH *hash, KEY_T key);
size_t Hash_size(HASH *hash);
size_t Hash_capacity(HASH *hash_table);
void Hash_resize(HASH **hash);

#endif
