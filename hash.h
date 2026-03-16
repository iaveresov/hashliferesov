#include <stddef.h>

/*
    Hashtable
*/

#ifndef HASH_H
#define HASH_H 1

typedef unsigned long KEY;
typedef struct Hash HASH;
typedef KEY (*HASH_FUNC)(void *);

struct Hash
{
    KEY *keys;
    void *values;
    size_t size;
    HASH_FUNC function;
};

HASH Hash_init(HASH_FUNC);

/*
    returns key
*/
void Hash_insert(HASH *hash, KEY key, void *value);

/*
    returns 0 on success
*/
int Hash_remove(HASH *hash, KEY key);

void *Hash_get(HASH *hash, KEY key);

#endif
