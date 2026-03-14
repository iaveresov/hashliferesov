#include <stdio.h>
#include <errno.h>  
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "quadrotree.h"

#define CMP(str, arg) (memcmp(str, arg, sizeof(str)) == 0)

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        goto error;
    }
    if (CMP("quad", argv[1]))
    {
        QTREE *tree = QTree_init(10);
        QTree_print(tree, 2);
        // QTree_free(tree);
    } else {
        goto error;
    }
    return 0;

error:
    errno = 2;
    perror("hashlife");
    exit(errno);
}
