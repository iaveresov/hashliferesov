#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "hash.h"
#include "plaintext.h"
#include "quadrotree.h"
#include "queue.h"
#include "typedefs.h"

KEY_T
mix_keys (KEY_T nw, KEY_T ne, KEY_T sw, KEY_T se)
{
  KEY_T key = PI_FRAC;
  key ^= mix64 (nw + KNUTH_CONST * 1);
  key = (key * KNUTH_CONST) ^ (key >> 32);
  key ^= mix64 (ne + KNUTH_CONST * 2);
  key = (key * KNUTH_CONST) ^ (key >> 32);
  key ^= mix64 (sw + KNUTH_CONST * 3);
  key = (key * KNUTH_CONST) ^ (key >> 32);
  key ^= mix64 (se + KNUTH_CONST * 4);
  key = (key * KNUTH_CONST) ^ (key >> 32);

  return mix64 (key);
}

KEY_T
get_tree_key (KEY_T nw, KEY_T ne, KEY_T sw, KEY_T se)
{
  KEY_T key = mix_keys (nw, ne, sw, se);
  uint64_t level = LEVEL (nw);
  bool is_zero = ZERO (nw) & ZERO (ne) & ZERO (sw) & ZERO (se);

  if (is_zero)
    {
      return (1ULL << 63) | ((level + 1) << 47) | HASH_VALUE (mix64 (key));
    }
  else
    {
      return (0ULL << 63) | ((level + 1) << 47) | HASH_VALUE (key);
    }
}

KEY_T
QTree_join (HASH *hash_table, KEY_T nw, KEY_T ne, KEY_T sw, KEY_T se)
{

  KEY_T key = get_tree_key (nw, ne, sw, se);
  QTREE *joined_tree = (QTREE *)Hash_get (hash_table, key);
  while (joined_tree)
    {
      if (joined_tree->nw == nw && joined_tree->ne == ne
          && joined_tree->sw == sw && joined_tree->se == se)
        {
            
          return key;
        }
      KEY_T new_hash = mix64 (key);
      key ^= HASH_VALUE (new_hash);
      joined_tree = (QTREE *)Hash_get (hash_table, key);
    }

  if (!joined_tree)
    {

      QTREE *nw_node, *ne_node, *sw_node, *se_node;
      joined_tree = (QTREE *)calloc (1, sizeof (QTREE));
      joined_tree->node_key = key;
      joined_tree->nw = nw;
      joined_tree->ne = ne;
      joined_tree->sw = sw;
      joined_tree->se = se;

      nw_node = (QTREE *)Hash_get (hash_table, nw);
      ne_node = (QTREE *)Hash_get (hash_table, ne);
      sw_node = (QTREE *)Hash_get (hash_table, sw);
      se_node = (QTREE *)Hash_get (hash_table, se);
      if (nw_node && ne_node && sw_node && se_node)
        {
          joined_tree->population = nw_node->population + ne_node->population
                                    + sw_node->population
                                    + se_node->population;
        }
      Hash_insert (hash_table, key, (void *)joined_tree);
    }
  return key;
}

KEY_T
QTree_get_zero (HASH *hash_table, uint64_t level)
{

  if (level == 0)
    {
      return DEAD_CELL;
    }

  KEY_T key = (1ULL << 63) | (level << 47) | (HASH_VALUE (mix64 (level)));
  QTREE *probe_tree = (QTREE *)Hash_get (hash_table, key);
  if (probe_tree != NULL && probe_tree->node_key == key)
    {
      return key;
    }

  return QTree_join (hash_table, QTree_get_zero (hash_table, level - 1),
                     QTree_get_zero (hash_table, level - 1),
                     QTree_get_zero (hash_table, level - 1),
                     QTree_get_zero (hash_table, level - 1));
}

KEY_T
QTree_centre (HASH *hash_table, KEY_T tree)
{
  KEY_T zero = QTree_get_zero (hash_table, LEVEL (tree) - 1);
  QTREE *tree_node = (QTREE *)Hash_get (hash_table, tree);

  return QTree_join (hash_table,
                     QTree_join (hash_table, zero, zero, zero, tree_node->nw),
                     QTree_join (hash_table, zero, zero, tree_node->ne, zero),
                     QTree_join (hash_table, zero, tree_node->sw, zero, zero),
                     QTree_join (hash_table, tree_node->se, zero, zero, zero));
}

uint64_t
QTree_get_cell (HASH *hash_table, KEY_T tree, uint64_t x, uint64_t y,
                uint64_t level)
{
  QTREE *tree_node = Hash_get (hash_table, tree);
  if (LEVEL (tree) == 0 || LEVEL (tree) == level)
    {
      return tree_node->population;
    }
  uint64_t size = 1 << LEVEL (tree);
  if (x >= size || y >= size)
    {
      return 0;
    }
  KEY_T nw = tree_node->nw;
  KEY_T ne = tree_node->ne;
  KEY_T sw = tree_node->sw;
  KEY_T se = tree_node->se;

  uint64_t offset = 1ULL << (LEVEL (tree) - 1);
  if (x < offset && y < offset)
    {
      return QTree_get_cell (hash_table, nw, x, y, level);
    }
  else if (x >= offset && y < offset)
    {
      return QTree_get_cell (hash_table, ne, x - offset, y, level);
    }
  else if (x < offset && y >= offset)
    {
      return QTree_get_cell (hash_table, sw, x, y - offset, level);
    }
  else
    {
      return QTree_get_cell (hash_table, se, x - offset, y - offset, level);
    }
}

void
QTree_print (HASH *hash_table, KEY_T tree)
{
  uint64_t size = 1ULL << LEVEL (tree);
  for (uint64_t i = 0; i < size; ++i)
    {
      for (uint64_t j = 0; j < size; ++j)
        {
            uint64_t cell = QTree_get_cell(hash_table, tree, j, i, 0);
            printf("%c", cell == 0 ? '.':'O');
       }
      printf ("\n");
    }
}

KEY_T
QTree_set_cell (HASH *hash_table, KEY_T tree, size_t x, size_t y, bool cell)
{
  if (LEVEL (tree) == 0)
    {
      return cell ? LIVE_CELL : DEAD_CELL;
    }

  while (x >= (1ULL << LEVEL (tree)) || y >= (1ULL << LEVEL (tree)))
    {
      KEY_T zero = QTree_get_zero (hash_table, LEVEL (tree));
      tree = QTree_join (hash_table, tree, zero, zero, zero);
    }

  QTREE *tree_node = (QTREE *)Hash_get (hash_table, tree);
  KEY_T nw = tree_node->nw;
  KEY_T ne = tree_node->ne;
  KEY_T sw = tree_node->sw;
  KEY_T se = tree_node->se;

  uint64_t offset = 1ULL << (LEVEL (tree) - 1);
  if (x < offset && y < offset)
    {
      nw = QTree_set_cell (hash_table, nw, x, y, cell);
    }
  else if (x >= offset && y < offset)
    {
      ne = QTree_set_cell (hash_table, ne, x - offset, y, cell);
    }
  else if (x < offset && y >= offset)
    {
      sw = QTree_set_cell (hash_table, sw, x, y - offset, cell);
    }
  else if (x >= offset && y >= offset)
    {
      se = QTree_set_cell (hash_table, se, x - offset, y - offset, cell);
    }
  return QTree_join (hash_table, nw, ne, sw, se);
}

KEY_T
QTree_pack (HASH *hash_table, BYTESBUFFER_T *b)
{
  KEY_T qtree_root = QTree_get_zero (hash_table, 2);

  for (size_t i = 0; i < b->h; ++i)
    {
      for (size_t j = 0; j < b->w; ++j)
        {
          bool cell = Plaintext_get (b, j, i);
          qtree_root = QTree_set_cell (hash_table, qtree_root, j, i, cell);
        }
    }
  return qtree_root;
}

// recursion base for 4x4 node
KEY_T
QTree_life_base (HASH *hash_table, KEY_T tree)
{
  if (LEVEL (tree) != 2)
    {
      errno = EINVAL;
      perror ("QTree_life_base");
      exit (1);
    }

  QTREE *tree_node = (QTREE *)Hash_get (hash_table, tree);
  QTREE *nw = (QTREE *)Hash_get (hash_table, tree_node->nw);
  QTREE *ne = (QTREE *)Hash_get (hash_table, tree_node->ne);
  QTREE *sw = (QTREE *)Hash_get (hash_table, tree_node->sw);
  QTREE *se = (QTREE *)Hash_get (hash_table, tree_node->se);

#define LIVE(node) (node == LIVE_CELL)
  int pop_nw = LIVE (nw->nw) + LIVE (nw->ne) + LIVE (nw->sw) + LIVE (ne->nw)
               + LIVE (ne->sw) + LIVE (sw->nw) + LIVE (sw->ne) + LIVE (se->nw);

  int pop_ne = LIVE (nw->ne) + LIVE (ne->nw) + LIVE (ne->ne) + LIVE (ne->se)
               + LIVE (se->ne) + LIVE (se->nw) + LIVE (sw->ne) + LIVE (nw->se);

  int pop_sw = LIVE (nw->sw) + LIVE (nw->se) + LIVE (ne->sw) + LIVE (se->nw)
               + LIVE (se->sw) + LIVE (sw->se) + LIVE (sw->sw) + LIVE (sw->nw);

  int pop_se = LIVE (nw->se) + LIVE (ne->sw) + LIVE (ne->se) + LIVE (se->ne)
               + LIVE (se->se) + LIVE (se->sw) + LIVE (sw->se) + LIVE (sw->ne);
#undef LIVE

#define LIVE_RULE(pop_sum, center)                                            \
  (((pop_sum == 3) || (pop_sum == 2 && center == LIVE_CELL)) ? (LIVE_CELL)    \
                                                             : (DEAD_CELL))

  return QTree_join (hash_table, LIVE_RULE (pop_nw, nw->se),
                     LIVE_RULE (pop_ne, ne->sw), LIVE_RULE (pop_sw, sw->ne),
                     LIVE_RULE (pop_se, se->nw));

#undef LIVE_RULE
}

KEY_T
QTree_get_result (HASH *hash_table, KEY_T tree, uint64_t steps)
{
  QTREE *tree_node = (QTREE *)Hash_get (hash_table, tree);

  QTREE *cached_node
      = (QTREE *)Hash_get (hash_table, mix_keys (tree, steps, tree, steps));
  if (cached_node && cached_node->source == tree
      && cached_node->steps == steps)
    {
      return cached_node->result;
    }
  return (KEY_T)NULL;
}

void
QTree_cache_result (HASH *hash_table, KEY_T tree, KEY_T result, uint64_t steps)
{
  KEY_T cached_key = mix_keys (tree, steps, tree, steps);
  QTREE *cached_node = (QTREE *)Hash_get (hash_table, cached_key);
  if (cached_node)
    {
      cached_node->source = tree;
      cached_node->result = result;
      cached_node->steps = steps;
    }
  else
    {
      cached_node = (QTREE *)calloc (1, sizeof (QTREE));
      cached_node->node_key = cached_key;
      cached_node->source = tree;
      cached_node->result = result;
      cached_node->steps = steps;
      Hash_insert (hash_table, cached_key, (void *)cached_node);
    }
}

KEY_T
QTree_join_result (HASH *hash_table, KEY_T nw, KEY_T ne, KEY_T sw, KEY_T se,
                   uint64_t steps)
{
  return QTree_result (hash_table, QTree_join (hash_table, nw, ne, sw, se),
                       steps);
}

KEY_T
QTree_result (HASH *hash_table, KEY_T tree, uint64_t steps)
{

  uint64_t level = LEVEL (tree);
  KEY_T result = QTree_get_result (hash_table, tree, steps);

  if (steps == 0 || steps >= level - 2)
    {
      steps = level - 2;
    }

  if (result)
    {
      return result;
    }

  if (ZERO (tree))
    {
      return ((QTREE *)Hash_get (hash_table, tree))->nw;
    }

  if (level == 2)
    {
      KEY_T result = QTree_life_base (hash_table, tree);
      QTREE *result_node = (QTREE *)Hash_get (hash_table, result);
      QTree_cache_result (hash_table, tree, result, steps);
      return result;
    }

  QTREE *tree_node = Hash_get (hash_table, tree);
  QTREE nw = *(QTREE *)Hash_get (hash_table, tree_node->nw);
  QTREE ne = *(QTREE *)Hash_get (hash_table, tree_node->ne);
  QTREE sw = *(QTREE *)Hash_get (hash_table, tree_node->sw);
  QTREE se = *(QTREE *)Hash_get (hash_table, tree_node->se);

  // nine temp nodes for recursion step in result callc
  KEY_T t1 = QTree_result (hash_table, nw.node_key, steps);
  KEY_T t2 = QTree_join_result (hash_table, nw.ne, ne.nw, nw.se, ne.sw, steps);
  KEY_T t3 = QTree_result (hash_table, ne.node_key, steps);
  KEY_T t4 = QTree_join_result (hash_table, ne.sw, ne.se, se.nw, se.ne, steps);
  KEY_T t5 = QTree_result (hash_table, se.node_key, steps);
  KEY_T t6 = QTree_join_result (hash_table, sw.ne, se.nw, sw.se, se.sw, steps);
  KEY_T t7 = QTree_result (hash_table, sw.node_key, steps);
  KEY_T t8 = QTree_join_result (hash_table, nw.sw, nw.se, sw.nw, sw.ne, steps);
  KEY_T t9 = QTree_join_result (hash_table, nw.se, ne.sw, sw.ne, se.nw, steps);

  // stop skipping 2^j steps
  if (steps < level - 2)
    {

      QTREE t1n = *(QTREE *)Hash_get (hash_table, t1);
      QTREE t2n = *(QTREE *)Hash_get (hash_table, t2);
      QTREE t3n = *(QTREE *)Hash_get (hash_table, t3);
      QTREE t4n = *(QTREE *)Hash_get (hash_table, t4);
      QTREE t5n = *(QTREE *)Hash_get (hash_table, t5);
      QTREE t6n = *(QTREE *)Hash_get (hash_table, t6);
      QTREE t7n = *(QTREE *)Hash_get (hash_table, t7);
      QTREE t8n = *(QTREE *)Hash_get (hash_table, t8);
      QTREE t9n = *(QTREE *)Hash_get (hash_table, t9);

      result = QTree_join (
          hash_table, QTree_join (hash_table, t1n.se, t2n.sw, t8n.ne, t9n.nw),
          QTree_join (hash_table, t2n.se, t3n.sw, t9n.ne, t4n.nw),
          QTree_join (hash_table, t8n.se, t9n.sw, t7n.ne, t6n.nw),
          QTree_join (hash_table, t9n.se, t4n.sw, t6n.ne, t5n.nw));
      QTree_cache_result (hash_table, tree, result, steps);
      return result;
    }

  // calculate to the bottom of the recursion
  else
    {
      KEY_T tnw = QTree_join_result (hash_table, t1, t2, t8, t9, steps);
      KEY_T tne = QTree_join_result (hash_table, t2, t3, t9, t4, steps);
      KEY_T tsw = QTree_join_result (hash_table, t8, t9, t7, t6, steps);
      KEY_T tse = QTree_join_result (hash_table, t9, t4, t6, t5, steps);
      result = QTree_join (hash_table, tnw, tne, tsw, tse);
      QTree_cache_result (hash_table, tree, result, steps);
      return result;
    }
}

bool
QTree_is_padded (HASH *hash_table, KEY_T tree)
{
  QTREE *tree_node = Hash_get (hash_table, tree);
  QTREE *nw = Hash_get (hash_table, tree_node->nw);
  QTREE *ne = Hash_get (hash_table, tree_node->ne);
  QTREE *sw = Hash_get (hash_table, tree_node->sw);
  QTREE *se = Hash_get (hash_table, tree_node->se);

  bool nw_se = (nw->population
                == ((QTREE *)Hash_get (hash_table, nw->se))->population);
  bool ne_sw = (ne->population
                == ((QTREE *)Hash_get (hash_table, ne->sw))->population);
  bool sw_ne = (sw->population
                == ((QTREE *)Hash_get (hash_table, sw->ne))->population);
  bool se_nw = (se->population
                == ((QTREE *)Hash_get (hash_table, se->nw))->population);

  return nw_se && ne_sw && sw_ne && se_nw;
}

KEY_T
QTree_crop (HASH *hash_table, KEY_T tree)
{

  while (LEVEL (tree) > 3 && QTree_is_padded (hash_table, tree))
    {
      QTREE *tree_node = Hash_get (hash_table, tree);
      QTREE nw = *(QTREE *)Hash_get (hash_table, tree_node->nw);
      QTREE ne = *(QTREE *)Hash_get (hash_table, tree_node->ne);
      QTREE sw = *(QTREE *)Hash_get (hash_table, tree_node->sw);
      QTREE se = *(QTREE *)Hash_get (hash_table, tree_node->se);
      tree = QTree_join (hash_table, nw.se, ne.sw, sw.ne, se.nw);
    }
  return tree;
}

KEY_T
QTree_make_step (HASH *hash_table, KEY_T tree, uint64_t steps)
{
  while ((1ULL << (LEVEL (tree) - 2)) < steps)
    {
      tree = QTree_centre (hash_table, tree);
    }
  tree = QTree_centre (hash_table, tree);

  for (uint64_t j = 1; steps > 0; j++, steps >>= 1)
    {
      if (steps & 1)
        {
          tree = QTree_centre (hash_table, QTree_result (hash_table, tree, j));
        }
    }
  return QTree_crop (hash_table, tree);

}

// void QTree_free(QTREE *tree);
