#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "quadrotree.h"

QTREE *QTree_init(int depth)
{
    assert(depth > 0);

    QTREE *tree, *leaf;

    tree = (QTREE *)malloc(sizeof(QTREE));
    tree->bits = 0;
    tree->level = 0;

    for (int l = 1; l < depth; l++)
    {
        leaf = tree;
        tree = (QTREE *)malloc(sizeof(QTREE));
        tree->level = l;
        for (int i = 0; i < LEAF; i++)
        {
            tree->leaf[i] = leaf;
        }
    }

    return tree;
}

void _print_impl(QTREE *root, int tabs, int x, int y, int limit)
{
    printf("(%d, %d) x %d\n", x, y, 1 << root->level);
    for (int i = 0; i < LEAF; i++)
    {
        if (root->level > limit)
        {
            for (int t = 0; t < tabs + 1; t++)
            {
                printf("\t");
            }
            printf("%d: ", i);

            const int size = 1 << root->leaf[i]->level;
            const int _x = x + size * (i % 2);
            const int _y = y + size * (i > 1);
            _print_impl(root->leaf[i], tabs + 1, _x, _y, limit);
        }
    }
}

void QTree_print(QTREE *tree, int limit)
{
    _print_impl(tree, 0, 0, 0, limit);
}

// void QTree_free(QTREE *tree);
