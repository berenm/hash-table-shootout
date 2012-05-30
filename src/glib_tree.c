#include <glib.h>

static gint intcmp(gconstpointer a,
                   gconstpointer b) {
  return (a < b) ? (-1) : ((a > b) ? 1 : 0);
}

static gint gstrcmp(gconstpointer a,
                    gconstpointer b) {
  return strcmp((char*) a, (char*) b);
}

#define SETUP                         \
  GTree * hash = g_tree_new(&intcmp); \
  GTree* str_hash = g_tree_new(&gstrcmp);
#define INSERT_INT_INTO_HASH(key, value) g_tree_insert(hash, GINT_TO_POINTER(key), &value)
#define LOOKUP_INT_FROM_HASH(key)        (void) g_tree_lookup(hash, GINT_TO_POINTER(key))
#define DELETE_INT_FROM_HASH(key)        g_tree_remove(hash, GINT_TO_POINTER(key))
#define INSERT_STR_INTO_HASH(key, value) g_tree_insert(str_hash, key, &value)
#define LOOKUP_STR_FROM_HASH(key)        (void) g_tree_lookup(str_hash, key)
#define DELETE_STR_FROM_HASH(key)        g_tree_remove(str_hash, key)
#include "template.c"
