#include <stdlib.h>
#include <string.h>

#ifdef BNPPP_MEMORY_USE_GLIB

# include <glib.h>

# define BNPPP_MEMORY_ALLOC(struct_m)           g_slice_new(struct_m)
# define BNPPP_MEMORY_ALLOC_ZEROED(struct_m)    g_slice_new0(struct_m)
# define BNPPP_MEMORY_FREE(struct_m, pointer_m) g_slice_free(struct_m, pointer_m)

# define BNPPP_MEMORY_ALLOC_N(count_m, struct_m)           (struct_m*) malloc(count_m * sizeof(struct_m))
# define BNPPP_MEMORY_FREE_N(count_m, struct_m, pointer_m) free((struct_m*) pointer_m)

#else // ifdef BNPPP_MEMORY_USE_GLIB

# define BNPPP_MEMORY_ALLOC(struct_m)           (struct_m*) malloc(sizeof(struct_m))
# define BNPPP_MEMORY_ALLOC_ZEROED(struct_m)    (struct_m*) calloc(1, sizeof(struct_m))
# define BNPPP_MEMORY_FREE(struct_m, pointer_m) free((struct_m*) pointer_m)

# define BNPPP_MEMORY_ALLOC_N(count_m, struct_m)           (struct_m*) malloc(count_m * sizeof(struct_m))
# define BNPPP_MEMORY_FREE_N(count_m, struct_m, pointer_m) free((struct_m*) pointer_m)

#endif // ifdef BNPPP_MEMORY_USE_GLIB

#define BNPPP_ERROR_IF(...) \
    do                      \
    {                       \
    }                       \
    while (0)
#define L_TRACE(...) \
    do               \
    {                \
    }                \
    while (0)
#define L_WARN(...) \
    do              \
    {               \
    }               \
    while (0)

struct _BNPPP_ListItem
{
    void* data;
};
typedef struct _BNPPP_ListItem BNPPP_ListItem;

struct _BNPPP_List
{
    size_t          size;
    size_t          capacity;
    BNPPP_ListItem* items;
};
typedef struct _BNPPP_List BNPPP_List;

BNPPP_List* BNPPP_ListCreate(size_t const capacity)
{
    BNPPP_List* list = BNPPP_MEMORY_ALLOC(BNPPP_List);
    L_TRACE("create list %p", list);
    BNPPP_ERROR_IF(return 0, !list, "create list failed, unable to allocate memory of size %zu.", sizeof(BNPPP_List));

    list->size     = 0;
    list->capacity = capacity;
    list->items    = BNPPP_MEMORY_ALLOC_N(capacity, BNPPP_ListItem);

    return list;
}

void BNPPP_ListDestroy(BNPPP_List* list)
{
    BNPPP_ERROR_IF(return , !list, "bnppp_list_destroy with null list");

    L_TRACE("destroy list %p", list);
    BNPPP_MEMORY_FREE_N(list->capacity, BNPPP_ListItem, list->items);
    BNPPP_MEMORY_FREE(BNPPP_List, list);
}

size_t BNPPP_ListSize(BNPPP_List const* list)
{
    return list->size;
}

void BNPPP_ListAppend(BNPPP_List* list, void* data)
{
    BNPPP_ERROR_IF(return , !list, "bnppp_list_append with null list");
    BNPPP_ERROR_IF(return , list->capacity <= list->size, "bnppp_list_append with full list");
    L_TRACE("list %p append %p", list, data);

    list->items[list->size++].data = data;
}

void BNPPP_ListClear(BNPPP_List* list)
{
    list->size = 0;
}

BNPPP_ListItem* BNPPP_ListFirst(BNPPP_List* list)
{
    BNPPP_ERROR_IF(return 0, !list, "bnppp_list_first with null list");

    if (list->size > 0)
    {
        return list->items;
    }
    else
    {
        return 0;
    }
}

BNPPP_ListItem* BNPPP_ListNext(BNPPP_List* list, BNPPP_ListItem* item)
{
    BNPPP_ERROR_IF(return 0, !list, "bnppp_list_next with null list");
    BNPPP_ERROR_IF(return 0, !list, "bnppp_list_next with null item");

    if (item >= list->items + list->size - 1)
    {
        return 0;
    }

    return ++item;
}

BNPPP_ListItem* BNPPP_ListAt(BNPPP_List* list, size_t const index)
{
    BNPPP_ERROR_IF(return 0, !list, "bnppp_list_next with null list");
    BNPPP_ERROR_IF(return 0, !list, "bnppp_list_next with null item");

    if (index >= list->size)
    {
        return 0;
    }

    return list->items + index;
}

void BNPPP_ListRemoveRange(BNPPP_List* list, size_t const index_from, size_t const index_to)
{
    BNPPP_ERROR_IF(return , !list, "bnppp_list_remove_range with null list");
    BNPPP_ERROR_IF(return , index_to > list->size, "bnppp_list_remove_range with index_to > size");
    BNPPP_ERROR_IF(return , index_from > list->size, "bnppp_list_remove_range with index_from > size");
    BNPPP_ERROR_IF(return , index_to < index_from, "bnppp_list_remove_range with index_from > index_to");

    if (index_to > index_from)
    {
        size_t const move_size = list->size - index_to;
        list->size = index_from + move_size;

        if (move_size > 0)
        {
            memmove(list->items + index_from, list->items + index_to, move_size * sizeof(BNPPP_ListItem));
        }
    }
}

static void BNPPP_ListItemPrint(BNPPP_List* list, BNPPP_ListItem* item)
{
    BNPPP_ERROR_IF(return , !list, "bnppp_list_item_print with null list");
    BNPPP_ERROR_IF(return , !list, "bnppp_list_item_print with null item");

    printf("[%p]->", item->data);
}

void BNPPP_ListPrint(BNPPP_List* list)
{
    BNPPP_ERROR_IF(return , !list, "bnppp_list_print with null list");

    BNPPP_ListItem* item = BNPPP_ListFirst(list);
    while (item)
    {
        BNPPP_ListItemPrint(list, item);
        item = BNPPP_ListNext(list, item);
    }
    printf("\n");
}

#define SETUP                BNPPP_List * array = BNPPP_ListCreate(element_count);
#define INSERT(index, value) BNPPP_ListAppend(array, &value)
#define LOOKUP(index)        if(index < BNPPP_ListSize(array)) (void) BNPPP_ListAt(array, index)
#define DELETE(index)        if(index < BNPPP_ListSize(array)) BNPPP_ListRemoveRange(array, index, index + 1)
#include "template.c"
