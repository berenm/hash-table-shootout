#include <glib.h>
#define SETUP                GArray * array = g_array_sized_new(0, 0, sizeof(int), element_count);
#define INSERT(index, value) g_array_append_val(array, value)
#define LOOKUP(index)        if(index < array->len) (void) g_array_index(array, int, index)
#define DELETE(index)        if(index < array->len) g_array_remove_index(array, index)
#include "template.c"
