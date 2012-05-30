#include <glib.h>

#define SETUP GHashTable * hash = g_hash_table_new(g_direct_hash, g_direct_equal);
#define INSERT_INT_INTO_HASH(key, value) g_hash_table_insert(hash, GINT_TO_POINTER(key), &value)
#define LOOKUP_INT_FROM_HASH(key)        (void) g_hash_table_lookup(hash, GINT_TO_POINTER(key))
#define DELETE_INT_FROM_HASH(key)        g_hash_table_remove(hash, GINT_TO_POINTER(key))

#include "template.c"
