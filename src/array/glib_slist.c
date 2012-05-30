#include <glib.h>
#define SETUP                GSList * array = g_slist_alloc();
#define INSERT(index, value) array = g_slist_append(array, &value)
#define LOOKUP(index)        if(index < g_slist_length(array)) (void) g_slist_nth_data(array, index)
#define DELETE(index)        if(index < g_slist_length(array)) array = g_slist_delete_link(array, g_slist_nth(array, index))
#include "template.c"
