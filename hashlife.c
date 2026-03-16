#include <assert.h>
#include <stdio.h>
#include <errno.h>  
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "quadrotree.h"
#include "args.h"
#include "hash.h"

#define CMP(str, arg) (memcmp(str, arg, sizeof(str)) == 0)

size_t hash_fn(KEY key)
{
    return (size_t)(key % TABLE_SIZE);
}


void usage(char *name)
{
    printf(
        "usage:\n"
        "\t%s init DEPTH [LIMIT]\n",
        name
    );
}

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        goto error;
    }
    if (CMP("init", argv[1]))
    {
        if (argc < 3)
        {
            goto error;
        }

        unsigned int depth, limit = 0;
        if (parse_uint(argv[2], &depth)) {
            goto error;
        }

        if (argc > 3) {
            if (argc > 3 && parse_uint(argv[3], &limit)) {
                goto error;
            }
        }


        QTREE *tree = QTree_init(depth);
        QTree_print(tree, limit);
        // QTree_free(tree);
    }
    else if (CMP("hash", argv[1]))
    {
        HASH *hash = Hash_init(&hash_fn);
        int *value = (int *)malloc(sizeof(int));
        *value = 1600;
        Hash_insert(hash, 1337, value);
        printf("%d\n", *(int *)Hash_get(hash, 1337));
        int sval = 2000;
        Hash_insert(hash, 5433, &sval);
        printf("%d\n", *(int *)Hash_get(hash, 5433));
        printf("%d\n", *(int *)Hash_get(hash, 1337));
    }
    else {
        goto error;
    }
    return 0;

error:
    errno = EINVAL;
    perror("hashlife");
    usage(argv[0]);
    exit(errno);
}
