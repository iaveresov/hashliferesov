#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "hash.h"
#include "plaintext.h"
#include "quadrotree.h"
#include "queue.h"
#include "typedefs.h"

#define CMP(str, arg) (memcmp (str, arg, sizeof (str)) == 0)

size_t
hash_fn (KEY_T key, size_t table_size)
{
  return key % table_size;
}

void
usage (char *name)
{
  printf ("usage:\n"
          "\t%s read FILE generations\n"
          "\t%s --version|-v\n",
          // TODO: how long could it be?
          name, name);
}

int
main (int argc, char **argv)
{
  if (argc < 2)
    {
      goto error;
    }

  if (CMP ("--version", argv[1]) || CMP ("-v", argv[1]))
    {
      printf ("%s\n", PACKAGE_STRING);
      return 0;
    }
  else if (CMP ("read", argv[1]))
    {
      if (argc < 4)
        {
          goto error;
        }
      char *endptr;
      uint64_t generations = strtoull (argv[3], &endptr, 10);

      BYTESBUFFER_T *buf = Plaintext_read (argv[2]);
      HASH *hash_table = Hash_init (hash_fn);
      KEY_T qtree_root = QTree_pack (hash_table, buf);
      printf ("\ninit tree\n");
      qtree_root = QTree_centre (hash_table, qtree_root);
      QTree_print (hash_table, qtree_root);

      printf ("init population: "
              "%" PRIu64 "\n",
              ((QTREE *)Hash_get (hash_table, qtree_root))->population);

      clock_t start = clock ();

      KEY_T result = QTree_make_step (hash_table, qtree_root, generations);

      clock_t stop = clock ();

      printf ("\nresult after 2^%" PRIu64 " steps\n", generations);
      QTree_print (hash_table, result);

      double elapsed_time = ((double)(stop - start)) / CLOCKS_PER_SEC;
      printf ("elapsed_time %.6fs\n", elapsed_time);

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
