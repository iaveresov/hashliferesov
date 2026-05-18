#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/types.h>

#include "plaintext.h"
#define PLAINTEXT_BUF_SIZE 8

typedef enum
{
  START = 0,
  COMMENT,
  NL_COMMENT,
  PATTERN,
  NL_PATTERN,
  ERROR
} STATE;

typedef struct STATE_T
{
  STATE state; /* statement; value from enum */
  bool stay;   /* flag indicating whether to read new char */
} STATE_T;

BYTESBUFFER_T
*Buffer_init (void)
{
  BYTESBUFFER_T *bytes_buffer
      = (BYTESBUFFER_T *)calloc (1, sizeof (BYTESBUFFER_T));

  bytes_buffer->bytes = (BYTE_T **)malloc (sizeof (BYTE_T *));
  bytes_buffer->bytes[0] = (BYTE_T *)malloc (sizeof (BYTE_T));
  bytes_buffer->w = 0;
  bytes_buffer->h = 0;
  return bytes_buffer;
}

void
Buffer_set_bit (BYTESBUFFER_T *b, bool value)
{
#define current_byte (b->bytes[b->h][b->w / 8])
  switch (b->w % 8)
    {
    case 0:
      current_byte.b0 = value;
      break;
    case 1:
      current_byte.b1 = value;
      break;
    case 2:
      current_byte.b2 = value;
      break;
    case 3:
      current_byte.b3 = value;
      break;
    case 4:
      current_byte.b4 = value;
      break;
    case 5:
      current_byte.b5 = value;
      break;
    case 6:
      current_byte.b6 = value;
      break;
    case 7:
      current_byte.b7 = value;
      break;
    }
#undef current_byte

  if (b->w / 8 < ((b->w) + 1) / 8) 
    {
      /* reallocate current line if byte border reached */
      b->bytes[b->h] = (BYTE_T *)realloc (b->bytes[b->h],
                                          (b->w / 8 + 1) * sizeof (BYTE_T));
    }
  b->w++;
}

static void
_update_sizes (size_t *max_width, BYTESBUFFER_T *buffer)
{
  *max_width = MAX (buffer->w, *max_width);
  buffer->w = 0;

  /* appending new string with one new zero byte */
  buffer->bytes = (BYTE_T **)realloc (buffer->bytes,
                                      (++buffer->h + 1) * sizeof (BYTE_T *));
  buffer->bytes[buffer->h] = (BYTE_T *)calloc (1, sizeof (BYTE_T));
}

static void
_read_pattern_chunk (STATE_T *stmt, char c, size_t *max_width,
                     BYTESBUFFER_T *buffer)
{
  switch (c)
    {
    case 'O':
    case '.':
      Buffer_set_bit (buffer, c == 'O');
      stmt->state = PATTERN;
      break;

    case '\r':
      stmt->state = NL_PATTERN;
      _update_sizes (max_width, buffer);
      break;

    case '\n':
      stmt->state = PATTERN;
      _update_sizes (max_width, buffer);
      break;

    default:
      stmt->state = ERROR;
    }
}

static void
_check_newline (STATE_T *stmt, char c)
{
  stmt->stay = (c != '\n');
  if (stmt->state == NL_PATTERN)
    {
      stmt->state = PATTERN;
    }
  else if (stmt->state == NL_COMMENT)
    {
      stmt->state = START;
    }
}

static void
_read_comment_chunk (STATE_T *stmt, char c)
{
  switch (c)
    {
    case '\r':
      stmt->state = NL_COMMENT;
      break;
    case '\n':
      stmt->state = START;
      break;
    default:
      stmt->state = COMMENT;
      break;
    }
}

BYTESBUFFER_T *
Plaintext_read (char *path)
{
  /* final buffer for storing pattern bit-per-cell; empty initially */
  BYTESBUFFER_T *bytes_buffer = Buffer_init ();

  FILE *text = fopen (path, "r");
  if (text == NULL)
    {
      perror ("plaintext");
      exit (errno);
    }

  char *line = (char *)malloc (PLAINTEXT_BUF_SIZE);
  size_t max_width = 0; /* max length of line in pattern file */
  size_t read;
  char *ptr, c;

  STATE_T *stmt
      = (STATE_T *)malloc (sizeof (STATE_T)); /*statement struct for FSM*/
  stmt->state = START;
  stmt->stay = false;

  while ((read = fread (line, 1, PLAINTEXT_BUF_SIZE, text)))
    {
      ptr = line;
      stmt->stay = false;

      while (ptr < line + read)
        {
          c = *ptr;
          switch (stmt->state)
            {
            case START:
              switch (c)
                {
                case '!':
                  stmt->state = COMMENT;
                  break;
                case '.':
                case 'O':
                case '\n':
                case '\r':
                  stmt->state = PATTERN;
                  stmt->stay = true;
                  break;
                default:
                  stmt->state = ERROR;
                  break;
                }
              break;

            case COMMENT:
              _read_comment_chunk (stmt, c);
              break;

            case PATTERN:
              _read_pattern_chunk (stmt, c, &max_width, bytes_buffer);
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

          if (!stmt->stay)
            ptr++;
          stmt->stay = false;
        }
    }

  if (!feof (text) || ferror (text))
    {
      perror ("plaintext");
      exit (errno);
    }

  printf ("max_line_len: %lu\n", max_width);
  printf ("linenum: %lu\n", bytes_buffer->h);
  // min size of line is 8 bits
  int extra_byte = (max_width % 8 == 0) ? 0 : 1;
  bytes_buffer->w = (max_width / 8 + extra_byte) * 8;
  for (size_t i = 0; i < bytes_buffer->h; ++i)
    {

      bytes_buffer->bytes[i] = (BYTE_T *)realloc (
          bytes_buffer->bytes[i], (bytes_buffer->w / 8 + 1) * sizeof (BYTE_T));
    }
  
  bytes_buffer->w = max_width;
  fclose (text);
  free (stmt);
  free (line);
  return bytes_buffer;
}

bool
Plaintext_get (BYTESBUFFER_T *buf, size_t x, size_t y)
{
  if (y > buf->h || x > buf->w)
    {
      return 0;
    }

  BYTE_T req_byte = buf->bytes[y][x / 8];
  switch (x % 8)
    {
    case 0:
      return req_byte.b0;
      break;
    case 1:
      return req_byte.b1;
      break;
    case 2:
      return req_byte.b2;
      break;
    case 3:
      return req_byte.b3;
      break;
    case 4:
      return req_byte.b4;
      break;
    case 5:
      return req_byte.b5;
      break;
    case 6:
      return req_byte.b6;
      break;
    case 7:
      return req_byte.b7;
      break;
    };
  return 0;
}
