#include <inttypes.h>
#include "bplus_tree_dicho.h"
#define SETUP                           \
  BNPPP_Map * hash = BNPPP_MapCreate(); \
  BNPPP_Map* str_hash = BNPPP_MapCreate();
#define INSERT_INT_INTO_HASH(key, value) BNPPP_MapInsert(hash, key, &value)
#define LOOKUP_INT_FROM_HASH(key)        (void) BNPPP_MapGet(hash, key)
#define DELETE_INT_FROM_HASH(key)        BNPPP_MapRemove(hash, key, &value)
#define INSERT_STR_INTO_HASH(key, value) BNPPP_MapInsert(str_hash, atoi(key), &value)
#define LOOKUP_STR_FROM_HASH(key)        (void) BNPPP_MapGet(str_hash, atoi(key))
#define DELETE_STR_FROM_HASH(key)        BNPPP_MapRemove(str_hash, atoi(key), &value)
#include "template.c"
