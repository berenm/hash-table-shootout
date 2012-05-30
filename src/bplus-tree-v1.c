#include <inttypes.h>

#include "../bplus-tree-v1/test/bplus_tree_all.c"

#define SETUP BplusTree * hash = bplus_tree_create();
#define INSERT_INT_INTO_HASH(key, value) bplus_tree_insert(hash, key, &value)
#define LOOKUP_INT_FROM_HASH(key)        (void) bplus_tree_get(hash, key)
#define DELETE_INT_FROM_HASH(key)        bplus_tree_remove_first(hash, key)
#define TEARDOWN bplus_tree_destroy(hash);

#include "template.c"
