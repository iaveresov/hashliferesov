#include "args.h"
#include <errno.h>
#include <stdlib.h>

int parse_uint(char *arg, unsigned int *n) {
    char *end;
    *n = strtoul(arg, &end, 10);
    if (errno == ERANGE || *end != '\0') {
        return 1;
    }
    return 0;
}

