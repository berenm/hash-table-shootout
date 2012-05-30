#ifndef __TEST_SEARCH_H__
#define __TEST_SEARCH_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define ARRAY_POWER 5
#define ARRAY_SIZE  (1 << ARRAY_POWER)

typedef struct _array_t {
  uint8_t  size;
  uint64_t values[ARRAY_SIZE];
} array_t;

typedef uint8_t (search_t)(uint8_t const key_count, uint64_t const* const keys, uint64_t key);

#endif // ifndef __TEST_SEARCH_H__
