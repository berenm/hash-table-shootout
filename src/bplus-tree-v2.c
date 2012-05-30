#include <inttypes.h>

#include "../bplus-tree-v2/test/bplus_tree_all.c"

#ifdef BPLUS_TREE_GENERIC

static gint intcmp(gconstpointer a, gconstpointer b)
{
    return a - b;
}

# define SETUP BplusTree * hash = bplus_tree_new(FALSE, intcmp);
# define INSERT_INT_INTO_HASH(key, value) bplus_tree_insert(hash, GINT_TO_POINTER(key), &value)
# define LOOKUP_INT_FROM_HASH(key)        (void) bplus_tree_get(hash, GINT_TO_POINTER(key))
# define DELETE_INT_FROM_HASH(key)        bplus_tree_remove(hash, GINT_TO_POINTER(key))

#else /* ifdef BPLUS_TREE_GENERIC */

# define SETUP BplusTree * hash = bplus_tree_new(FALSE);
# define INSERT_INT_INTO_HASH(key, value) bplus_tree_insert(hash, key, &value)
# define LOOKUP_INT_FROM_HASH(key)        (void) bplus_tree_get(hash, key)
# define DELETE_INT_FROM_HASH(key)        bplus_tree_remove(hash, key)

#endif /* ifdef BPLUS_TREE_GENERIC */

#define TEARDOWN bplus_tree_destroy(hash); /* bplus_tree_destroy(str_hash); */

#include "template.c"
