#include "hash.h"
#include "plaintext.h"
#ifndef QUADROTREE_H
#define QUADROTREE_H 1

#include "typedefs.h"
#include <stddef.h>
#include <stdint.h>

#define LEAF 4
// node id: [zero flag: 1][level: 16][hash: 47]

#define ZERO(node) (((node >> 63) & 0x1ULL))
#define LEVEL(node) (((node >> 47) & 0xFFFFULL))
#define HASH_VALUE(node) ((node & 0x7fffffffffffULL))

/*
    nw | ne
    -----
    sw | se

*/

struct QTree
{
  KEY_T node_key;
  KEY_T nw, ne, sw, se;
  KEY_T source;
  KEY_T result;
  // unsigned int level;
  uint64_t population;
  uint64_t steps;
};

KEY_T QTree_join (HASH *hash_table, KEY_T nw, KEY_T ne, KEY_T se, KEY_T sw);
KEY_T QTree_get_zero (HASH *hash_table, uint64_t level);
KEY_T QTree_pack (HASH *hash_table, BYTESBUFFER_T *b);
KEY_T QTree_set_cell (HASH *hash_table, KEY_T tree, size_t x, size_t y,
                      bool cell);
uint64_t QTree_get_cell (HASH *hash_table, KEY_T tree, uint64_t x, uint64_t y,
                         uint64_t level);
KEY_T QTree_centre (HASH *hash_table, KEY_T tree);
KEY_T QTree_life_base (HASH *hash_table, KEY_T tree);
void QTree_print (HASH *hash_table, KEY_T tree);
KEY_T QTree_result (HASH *hash_table, KEY_T tree, uint64_t steps);
KEY_T QTree_make_step (HASH *hash_table, KEY_T tree, uint64_t steps);
// void QTree_free(QTREE *tree);
#endif
