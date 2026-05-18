#ifndef PLAINTEXT_H
#define PLAINTEXT_H 1

#include<stdbool.h>
#include <stddef.h>

typedef struct Byte BYTE_T;
struct Byte
{
  unsigned int b0 : 1;
  unsigned int b1 : 1;
  unsigned int b2 : 1;
  unsigned int b3 : 1;
  unsigned int b4 : 1;
  unsigned int b5 : 1;
  unsigned int b6 : 1;
  unsigned int b7 : 1;
};

typedef struct BytesBuffer BYTESBUFFER_T;
struct BytesBuffer
{
  BYTE_T **bytes;
  size_t w;
  size_t h;
};

BYTESBUFFER_T *Plaintext_read (char *path);
bool Plaintext_get (BYTESBUFFER_T *buf, size_t x, size_t y);
int Plaintext_free (BYTESBUFFER_T *buf);

#endif /* ifndef PLAINTEXT_H */
