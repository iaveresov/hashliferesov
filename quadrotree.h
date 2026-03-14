#ifndef QUADROTREE_H
#define QUADROTREE_H 1

#define LEAF 4

/*
    0 | 1
    ----- 
    2 | 3
*/

typedef struct QTree QTREE;

struct QTree
{
    union
    {
        QTREE *leaf[LEAF];
        // nodes of level 0 could represent real cells
        int bits;
    } data;
    int level;
};

QTREE *QTree_init(int depth);
void QTree_print(QTREE *tree, int limit);
// void QTree_free(QTREE *tree);

#endif

