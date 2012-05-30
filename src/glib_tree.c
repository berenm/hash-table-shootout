#include <glib.h>

static gint intcmp(gconstpointer a, gconstpointer b)
{
    return a - b;
}

#define SETUP GTree * hash = g_tree_new(&intcmp);
#define INSERT_INT_INTO_HASH(key, value) g_tree_insert(hash, GINT_TO_POINTER(key), &value)
#define LOOKUP_INT_FROM_HASH(key)        (void) g_tree_lookup(hash, GINT_TO_POINTER(key))
#define DELETE_INT_FROM_HASH(key)        g_tree_remove(hash, GINT_TO_POINTER(key))

#include "template.c"
