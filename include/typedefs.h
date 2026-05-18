#ifndef TYPEDEFS_H
#define TYPEDEFS_H 1
#include <inttypes.h>
#include <stdint.h>

#define DEAD_CELL ((1ULL << 63) | (0ULL << 47) | (HASH_VALUE (mix64 (0))))
#define LIVE_CELL ((0ULL << 63) | (0ULL << 47) | (HASH_VALUE (mix64 (1))))

#define KNUTH_CONST 0x9e3779b97f4a7c15ULL
#define FIRST_CONST 0xbf58476d1ce4e5b9ULL
#define SECOND_CONST 0x94d049bb133111ebULL
#define PI_FRAC 0x243f6a8885a308d3ULL


typedef struct Hash HASH;
typedef struct QTree QTREE;

#endif
