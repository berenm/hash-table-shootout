#include <inttypes.h>
#include "bplus_tree2.h"

static gint intcmp(gconstpointer a,
                   gconstpointer b) {
  return (a < b) ? (-1) : ((a > b) ? 1 : 0);
}

static gint gstrcmp(gconstpointer a,
                    gconstpointer b) {
  return strcmp((char*) a, (char*) b);
}

#ifdef BPLUS_TREE_GENERIC
# define SETUP                                      \
  BplusTree * hash = bplus_tree_new(FALSE, intcmp); \
  BplusTree* str_hash = bplus_tree_new(FALSE, gstrcmp);
# define INSERT_INT_INTO_HASH(key, value) bplus_tree_insert(hash, GINT_TO_POINTER(key), &value)
# define LOOKUP_INT_FROM_HASH(key)        (void) bplus_tree_get(hash, GINT_TO_POINTER(key))
# define DELETE_INT_FROM_HASH(key)        bplus_tree_remove(hash, GINT_TO_POINTER(key))
# define INSERT_STR_INTO_HASH(key, value) bplus_tree_insert(str_hash, key, &value)
# define LOOKUP_STR_FROM_HASH(key)        (void) bplus_tree_get(str_hash, key)
# define DELETE_STR_FROM_HASH(key)        bplus_tree_remove_value(str_hash, key, &value)
#else /* ifdef BPLUS_TREE_GENERIC */
# define SETUP                              \
  BplusTree * hash = bplus_tree_new(FALSE); \
  BplusTree* str_hash = bplus_tree_new(FALSE);
# define INSERT_INT_INTO_HASH(key, value) bplus_tree_insert(hash, key, &value)
# define LOOKUP_INT_FROM_HASH(key)        (void) bplus_tree_get(hash, key)
# define DELETE_INT_FROM_HASH(key)        bplus_tree_remove(hash, key)
# define INSERT_STR_INTO_HASH(key, value) bplus_tree_insert(str_hash, atoi(key), &value)
# define LOOKUP_STR_FROM_HASH(key)        (void) bplus_tree_get(str_hash, atoi(key))
# define DELETE_STR_FROM_HASH(key)        bplus_tree_remove_value(str_hash, atoi(key), &value)
#endif /* ifdef BPLUS_TREE_GENERIC */

#define TEARDOWN bplus_tree_destroy(hash); /* bplus_tree_destroy(str_hash); */

#include "template.c"
