#include <inttypes.h>
#include "bplus_tree_generic.h"
#define SETUP                           \
  BplusTree *hash = bplus_tree_new(g_direct_equal); \
  BplusTree* str_hash = bplus_tree_new(g_direct_equal);
#define INSERT_INT_INTO_HASH(key, value) bplus_tree_insert(hash, GINT_TO_POINTER(key), &value)
#define LOOKUP_INT_FROM_HASH(key)        (void) bplus_tree_get(hash, GINT_TO_POINTER(key))
#define DELETE_INT_FROM_HASH(key)        bplus_tree_remove_value(hash, GINT_TO_POINTER(key), &value)
#define INSERT_STR_INTO_HASH(key, value) bplus_tree_insert(str_hash, key, &value)
#define LOOKUP_STR_FROM_HASH(key)        (void) bplus_tree_get(str_hash, key)
#define DELETE_STR_FROM_HASH(key)        bplus_tree_remove_value(str_hash, key, &value)
#include "template.c"
