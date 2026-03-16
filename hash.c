#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "hash.h"
#include "list.h"

static void* ecalloc(size_t nmemb, size_t size)
{
    void *ptr = calloc(nmemb, size); 
    if (!ptr)
    {
        perror("hash:");
        exit(errno);
    } 
    return ptr;
}

struct Pair
{
    KEY key;
    void* value;
};

struct Hash
{
    LIST **table;
    size_t size;
    HASH_FUNC f;

};

HASH *Hash_init(HASH_FUNC function)
{
    HASH *table = (HASH *)ecalloc(1, sizeof(HASH));

    table->table = (LIST **)calloc(TABLE_SIZE, sizeof(LIST *));
    table->f = function;
    table->size = 0;

    return table;
}

size_t Hash_size(HASH *hash)
{
    return hash->size;
}

bool has_key(void *element, void *load)
{
    return ((PAIR *)element)->key == *((KEY *)load);
}

void Hash_insert(HASH *hash, KEY key, void *value)
{
    size_t index = hash->f(key);     
    
    PAIR* key_value;
    LIST **list = &(hash->table[index]);

    if (List_search(*list, &has_key, &key)) 
    {
        return;
    }

    key_value = (PAIR *)ecalloc(1, sizeof(PAIR));
    key_value->key = key;
    key_value->value = value;
    List_prepend(list, key_value);
}

//int Hash_remove(HASH *hash, KEY key);

void *Hash_get(HASH *hash, KEY key)
{
    size_t index = hash->f(key);     
    LIST *list;
    PAIR *key_value;
    
    if (!hash->table[index])
    {
        return NULL;
    }
    else
    {
        list = hash->table[index];
        if ((key_value = (PAIR *)List_search(list, &has_key, &key)))
        {
            return key_value->value;
        } 

        return NULL;
    }
}


