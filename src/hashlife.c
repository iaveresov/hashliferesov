#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "config.h"
#include "hash.h"
#include "quadrotree.h"
#include "typedefs.h"

#define CMP(str, arg) (memcmp (str, arg, sizeof (str)) == 0)

void
usage (char *name)
{
  printf ("usage:\n"
          "\t%s init DEPTH [LIMIT]\n"
          "\t%s --version|-v\n"
          , name, name);
}

int
main (int argc, char **argv)
{
  if (argc < 2)
    {
      goto error;
    }
  if (CMP ("init", argv[1]))
    {
      if (argc < 3)
        {
          goto error;
        }

      unsigned int depth, limit = 0;
      if (parse_uint (argv[2], &depth))
        {
          goto error;
        }

      if (argc > 3)
        {
          if (argc > 3 && parse_uint (argv[3], &limit))
            {
              goto error;
            }
        }

      HASH *hash = Hash_init (&QTree_hash);
      QTREE *tree = QTree_init (depth, hash);
      QTree_print (tree, limit);
      printf ("hash size: %lu\n", Hash_size (hash));
      // QTree_free(tree);
    }
  else if (CMP ("--version", argv[1]) || CMP ("-v", argv[1]))
    {
      printf ("%s\n", PACKAGE_STRING);
      return 0;
    }
  else
    {
      goto error;
    }
  return 0;

error:
  errno = EINVAL;
  perror ("hashlife");
  usage (argv[0]);
  exit (errno);
}
