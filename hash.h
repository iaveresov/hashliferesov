#include <stddef.h>

/*
    Hashtable
*/

#ifndef HASH_H
#define HASH_H 1
#define TABLE_SIZE 4096

typedef unsigned long KEY;
typedef struct Hash HASH;
typedef struct Pair PAIR;
typedef size_t (*HASH_FUNC)(KEY);


HASH *Hash_init(HASH_FUNC function);

/*
    returns key
*/
void Hash_insert(HASH *hash, KEY key, void *value);

/*
    returns 0 on success
*/
int Hash_remove(HASH *hash, KEY key);

void *Hash_get(HASH *hash, KEY key);

size_t Hash_size(HASH *hash);

#endif
