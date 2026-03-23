#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>

#include "plaintext.h"
#define PLAINTEXT_BUF_SIZE 8

enum _FSM_STMT
{
  START = 0,
  COMMENT,
  NL_COMMENT,
  PATTERN,
  NL_PATTERN,
  ERROR
};

typedef struct STMT
{
  int stmt_num; /*statement; value from enum*/
  bool unget;   /*bool flag indicating whether to read new char*/
} STMT;

static void
_update_sizes (size_t *length, size_t *width, size_t *height)
{
  (*height)++;
  (*width) = MAX (*length, *width);
  *length = 0;
}

static void
_read_pattern_chunk (STMT *stmt, char c, size_t *length, size_t *width,
                     size_t *height)
{
  switch (c)
    {
    case '.':
    case 'O':
      (*length)++;
      stmt->stmt_num = PATTERN;
      break;

    case '\r':
      stmt->stmt_num = NL_PATTERN;
      _update_sizes (length, width, height);
      break;

    case '\n':
      stmt->stmt_num = PATTERN;
      _update_sizes (length, width, height);
      break;

    default:
      stmt->stmt_num = ERROR;
    }
}

static void
_check_newline (STMT *stmt, char c)
{
  stmt->unget = (c == '\n') ? (false) : (true);
  if (stmt->stmt_num == NL_PATTERN)
    {
      stmt->stmt_num = PATTERN;
    }
  else if (stmt->stmt_num == NL_COMMENT)
    {
      stmt->stmt_num = START;
    }
}

static void
_read_comment_chunk (STMT *stmt, char c)
{
  switch (c)
    {
    case '\r':
      stmt->stmt_num = NL_COMMENT;
      break;
    case '\n':
      stmt->stmt_num = START;
      break;
    default:
      stmt->stmt_num = COMMENT;
      break;
    }
}

BYTESBUFFER_T *
Plaintext_read (char *path)
{
  FILE *text = fopen (path, "r");
  if (text == NULL)
    {
      perror ("plaintext");
      exit (errno);
    }

  char *line = (char *)malloc (PLAINTEXT_BUF_SIZE);
  size_t width = 0, /* max length of line in pattern file */
      height = 0;   /* number of pattern file lines excluding comments */
  size_t length = 0, read;
  char *ptr, c;

  STMT *stmt = (STMT *)malloc (sizeof (STMT)); /*statement struct for FSM*/
  stmt->stmt_num = START;
  stmt->unget = false;

  while ((read = fread (line, 1, PLAINTEXT_BUF_SIZE, text)))
    {
      ptr = line;
      stmt->unget = false;

      while (ptr < line + read)
        {
          if (!stmt->unget)
            {
              c = *(ptr++);
            }
          else
            {
              c = *ptr;
              stmt->unget = false;
            }

          switch (stmt->stmt_num)
            {
            case START:
              switch (c)
                {
                case '!':
                  stmt->stmt_num = COMMENT;
                  break;
                case '.':
                case 'O':
                case '\n':
                case '\r':
                  stmt->stmt_num = PATTERN;
                  stmt->unget = true;
                  break;
                default:
                  stmt->stmt_num = ERROR;
                  break;
                }
              break;

            case COMMENT:
              _read_comment_chunk (stmt, c);
              break;

            case PATTERN:
              _read_pattern_chunk (stmt, c, &length, &width, &height);
              break;

            case NL_COMMENT:
            case NL_PATTERN:
              _check_newline (stmt, c);
              break;

            case ERROR:
              fclose (text);
              errno = EINVAL;
              perror ("plaintext");
              exit (errno);
            }
        }
    }

  if (!feof (text) || ferror (text))
    {
      perror ("plaintext");
      exit (errno);
    }

  printf ("max_line_len: %lu\n", width);
  printf ("linenum: %lu\n", height);
  // TODO: now init BYTESBUFFER with this params

  free (stmt);
  free (line);

  return NULL;
}

int Plaintext_get (BYTESBUFFER_T *buf, size_t x, size_t y);
