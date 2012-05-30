#include "test_search.h"

uint8_t linear_search(uint8_t const key_count, uint64_t const* const keys, uint64_t key) {
  /* Few obvious cases that need to be resolved so we can start iterating at the middle */
  if (keys[0] > key) {
    return 0;
  }
  if (keys[key_count - 1] <= key) {
    return key_count - 1;
  }

  uint64_t const* const begin = keys;
  uint64_t const* const end   = begin + key_count;
  uint64_t const*       item  = begin;

  if ((key_count == 0) || (keys[0] >= key)) {
    return 0;
  }
  if (keys[key_count - 1] < key) {
    return key_count;
  }

  while (item < end) {
    if (*item >= key) {
      break;
    }
    ++item;
  }

  return item - begin;
}
