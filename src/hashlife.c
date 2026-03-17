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
    else if (CMP("test", argv[1]))
    {   
        //TODO: write tests for hashtable
        return 0;    
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
