#include <assert.h>
#include <errno.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "hash.h"
#include "list.h"
#include "plaintext.h"
#include "quadrotree.h"
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
          "\t%s read IFILE generations [OFILE]\n"
          "\t%s --version|-v\n",
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

      FILE *of = NULL;
      if (argc == 5)
        {
          of = fopen (argv[4], "w");

          printf ("%s\n", argv[4]);

          if (!of)
            {
              perror ("hashlife");
              exit (1);
            }
        }

      printf ("test");
      char *endptr;
      uint64_t generations = strtoull (argv[3], &endptr, 10);

      BYTESBUFFER_T *buf = Plaintext_read (argv[2]);
      HASH *hash_table = Hash_init (hash_fn);
      KEY_T qtree_root = QTree_pack (hash_table, buf);
      printf ("\ninit tree\n");
      qtree_root = QTree_centre (hash_table, qtree_root);
      QTree_print (hash_table, qtree_root, NULL);

      printf ("init population: "
              "%" PRIu64 "\n",
              ((QTREE *)Hash_get (hash_table, qtree_root))->population);

      clock_t start = clock ();

      KEY_T result = QTree_make_step (hash_table, qtree_root, generations);

      clock_t stop = clock ();

      printf ("\nresult after %" PRIu64 " steps\n", generations);
      QTree_print (hash_table, result, of);

      double elapsed_time = ((double)(stop - start)) / CLOCKS_PER_SEC;
      printf ("elapsed time %.6fs\n", elapsed_time);
      size_t List_size = List_get_size ();
      size_t Hash_size_fin = Hash_size (hash_table);
      size_t Hash_capacity_fin = Hash_capacity (hash_table);
      size_t QTree_size = sizeof (struct QTree);
      size_t elapsed_memory = (Hash_capacity_fin - Hash_size_fin) * List_size
                              + Hash_size_fin * (QTree_size + List_size);
      printf ("elapsed memory %zu bytes", elapsed_memory);

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
