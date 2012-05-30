#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <execinfo.h>

#include <stdlib.h>
#include <assert.h>

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

typedef uint64_t BNPPP_Key;
struct _BNPPP_MapItem {
  uint64_t key;
  void*    data;
};

typedef struct _BNPPP_MapItem BNPPP_MapItem;

typedef struct _BPlusTree         BPlusTree;
typedef struct _BPlusTreeNode     BPlusTreeNode;
typedef struct _BNPPP_MapItem     BPlusTreeItem;
typedef struct _BPlusTreeIterator BPlusTreeIterator;

#define BNPPP_ERROR_IF(...) do {} while (0)

// #define L_TRACE(format_m, ...) printf(format_m "\n", ## __VA_ARGS__)
#define L_TRACE(...) do {} while (0)
#define L_WARN(...)  do {} while (0)

BPlusTree* BPlusTreeCreate();
void       BPlusTreeDestroy(BPlusTree* tree);
void       BPlusTreeDestroyEach(BPlusTree* tree,
                                void (* function)(void const* tree, BNPPP_Key key, void* data, void* argument),
                                void* argument);

void  BPlusTreeInsert(BPlusTree* tree, BNPPP_Key const key, void* data);
void  BPlusTreeRemove(BPlusTree* tree, BNPPP_Key const key, void* data);
void* BPlusTreeRemoveFirst(BPlusTree* tree, BNPPP_Key const key);
void  BPlusTreeRemoveAll(BPlusTree* tree, BNPPP_Key const key);

BPlusTreeItem const* BPlusTreeAt(BPlusTree const* tree, size_t const index);
BPlusTreeItem const* BPlusTreeGet(BPlusTree const* tree, BNPPP_Key const key);

BPlusTreeIterator*   BPlusTreeIteratorNext(BPlusTreeIterator* iterator);
void                 BPlusTreeIteratorEnd(BPlusTreeIterator* iterator);
BPlusTreeItem const* BPlusTreeIteratorGetItem(BPlusTreeIterator const* iterator);

BPlusTreeIterator* BPlusTreeFirst(BPlusTree const* tree);
BPlusTreeIterator* BPlusTreeFirstWithKey(BPlusTree const* tree, BNPPP_Key const key);
BPlusTreeIterator* BPlusTreeRangeFirst(BPlusTree const* tree, BNPPP_Key const key_from, BNPPP_Key const key_to);

void BPlusTreeDebug(BPlusTree const* tree, FILE* file);
void Test_BPlusTree();

#ifndef BPLUS_TREE_TEST_COUNT
# define BPLUS_TREE_TEST_COUNT 1000000
#endif /* ifndef BPLUS_TREE_TEST_COUNT */

#ifndef BPLUS_TREE_ORDER
# define BPLUS_TREE_ORDER 32
#endif /* ifndef BPLUS_TREE_ORDER */

#ifdef BNPPP_MEMORY_USE_GLIB
# include <glib.h>
#endif /* ifdef BNPPP_MEMORY_USE_GLIB */

/* -- node functions ------------------------------------------------------------------------------------------------------------------- */

static BPlusTreeNode* BPlusTreeNodeCreate(BPlusTree* tree, uint8_t const create_leaf, BPlusTreeNode* left_leaf);
static void           BPlusTreeNodeDestroy(BPlusTree* tree, BPlusTreeNode* node);

static uint8_t BPlusTreeNodeIsLeaf(BPlusTreeNode const* node);
static void    BPlusTreeNodeMoveAndResizeData(BPlusTreeNode* node, size_t const to_offset, size_t const from_offset);
static void    BPlusTreeNodeCopyData(BPlusTreeNode* to_node,
                                     size_t const   to_offset,
                                     BPlusTreeNode* from_node,
                                     size_t const   from_offset,
                                     size_t         count);
static void BPlusTreeNodeUpdateParent(BPlusTreeNode* node);
static void BPlusTreeNodePropagateMinimalKey(BPlusTreeNode* node);

static size_t BPlusTreeNodeGetKeyIndexFirstOrBefore(BPlusTreeNode const* node, BNPPP_Key const key);
static size_t BPlusTreeNodeGetKeyIndexAfterLast(BPlusTreeNode const* node, BNPPP_Key const key);

static void BPlusTreeNodeInsert(BPlusTree* tree, BPlusTreeNode* node, BNPPP_Key const key, void* data);
static void BPlusTreeNodeRemoveRange(BPlusTree*     tree,
                                     BPlusTreeNode* node,
                                     size_t const   index_from,
                                     size_t const   index_to);
static void BPlusTreeNodeSplit(BPlusTree* tree, BPlusTreeNode* left_node);
static void BPlusTreeNodeCollapseRoot(BPlusTree* tree);

static size_t         BPlusTreeLeafFindKey(BPlusTreeNode const* node, BNPPP_Key const key);
static size_t         BPlusTreeLeafFindKeyData(BPlusTreeNode const* node, BNPPP_Key const key, void const* data);
static BPlusTreeNode* BPlusTreeLeafGetNotEmpty(BPlusTreeNode* leaf);
static void           BPlusTreeLeafRemoveKeyData(BPlusTree*      tree,
                                                 BPlusTreeNode*  node,
                                                 BNPPP_Key const key,
                                                 void const*     data);
static void* BPlusTreeLeafRemoveKeyFirst(BPlusTree* tree, BPlusTreeNode* node, BNPPP_Key const key);

static void BPlusTreeNodeDebug(BPlusTreeNode* node, void* arguments);

/* -- tree functions -------------------------------------------------------------------------------------------------------------------- */

static BPlusTreeNode* BPlusTreeGetLeaf(BPlusTree const* tree, BNPPP_Key const key);

static BPlusTreeNode*       BPlusTreeFindLeafWithKey(BPlusTree const* tree, BNPPP_Key const key);
static BPlusTreeNode*       BPlusTreeFindLeafWithKeyData(BPlusTree const* tree, BNPPP_Key const key, void* data);
static BPlusTreeItem const* BPlusTreeLeafGetNextToLastWithKey(BPlusTree const* tree, BNPPP_Key const key);

static void BPlusTreeNodeForeachNode(BPlusTree const* tree, BPlusTreeNode* node, void (* function)(BPlusTreeNode*,
                                                                                                   void*),
                                     void* argument);
static void BPlusTreeForeachNode(BPlusTree const* tree, void (* function)(BPlusTreeNode*, void*), void* argument);

static BPlusTreeIterator* BPlusTreeIteratorCreate(BPlusTree const*     tree,
                                                  BPlusTreeNode const* leaf,
                                                  BPlusTreeItem const* item,
                                                  BPlusTreeItem const* end);
static void BPlusTreeIteratorDestroy(BPlusTreeIterator* iterator);

static void BPlusTreeDebugToFile(BPlusTree* tree);

struct _BPlusTreeIterator {
  BPlusTreeNode const* leaf;
  BPlusTreeItem const* leaf_end;
  BPlusTreeItem const* item;
  BPlusTreeItem const* end;
};

typedef struct _BPlusTreeLeafLink {
  BPlusTreeNode* next;
  BPlusTreeNode* previous;
} BPlusTreeLeafLink;

struct _BPlusTreeNode {
  BPlusTreeNode*     parent;
  BPlusTreeLeafLink* leaf_link;

  BNPPP_Key key;

  size_t        size;
  BPlusTreeItem items[BPLUS_TREE_ORDER];
};

#define BPlusTreeNodeFirst(node)     (node->items)
#define BPlusTreeNodeAt(node, index) (node->items + index)
#define BPlusTreeNodeEnd(node)       (node->items + node->size)
#define BPlusTreeNodeLast(node)      (BPlusTreeNodeEnd(node) - 1)

struct _BPlusTree {
  BPlusTreeNode* root;
  BPlusTreeNode* first;
  BPlusTreeNode* last;

  BPlusTreeIterator iterator;
};

static BPlusTreeNode* BPlusTreeNodeCreate(BPlusTree* tree, uint8_t const create_leaf, BPlusTreeNode* left_leaf) {
  BNPPP_ERROR_IF(return 0, !tree, "bplus_tree_node_create with null tree");
  BNPPP_ERROR_IF(return 0,
                 create_leaf && left_leaf && !BPlusTreeNodeIsLeaf(left_leaf),
                 "bplus_tree_node_create with left leaf which is not a leaf");

  BPlusTreeNode* node = BNPPP_MEMORY_ALLOC(BPlusTreeNode);
  node->key       = 0;
  node->size      = 0;
  node->parent    = 0;
  node->leaf_link = 0;
  memset(node->items, 0xFF, BPLUS_TREE_ORDER * sizeof(*node->items));

  L_TRACE("create node %p tree %p", node, tree);
  BNPPP_ERROR_IF(return 0, !node, "create node failed, unable to allocate memory of size %zu.", sizeof(BPlusTreeNode));

  if (create_leaf) {
    node->leaf_link           = BNPPP_MEMORY_ALLOC(BPlusTreeLeafLink);
    node->leaf_link->next     = 0;
    node->leaf_link->previous = 0;

    BNPPP_ERROR_IF(return 0,
                   !node->leaf_link,
                   "create node leaf link failed, unable to allocate memory of size %zu.",
                   sizeof(BPlusTreeLeafLink));

    /* Update leaf link list if needed */
    if (left_leaf) {
      BPlusTreeNode* right_leaf = left_leaf->leaf_link->next;
      if (right_leaf) {
        node->leaf_link->next           = right_leaf;
        right_leaf->leaf_link->previous = node;
      }

      node->leaf_link->previous  = left_leaf;
      left_leaf->leaf_link->next = node;

      if (tree->last == left_leaf) {
        tree->last = node;
      }
    }
  }

  return node;
}

static void BPlusTreeNodeDestroy(BPlusTree* tree, BPlusTreeNode* node) {
  BNPPP_ERROR_IF(return , !tree, "bplus_tree_node_destroy with null tree");
  BNPPP_ERROR_IF(return , !node, "bplus_tree_node_destroy with null node");

  L_TRACE("destroy node %p tree %p", node, tree);

  if (node->leaf_link) {
    BPlusTreeNode* left_leaf  = node->leaf_link->previous;
    BPlusTreeNode* right_leaf = node->leaf_link->next;

    /* Update leaf link list if needed */
    if (left_leaf) {
      left_leaf->leaf_link->next = right_leaf;

      if (tree->last == node) {
        tree->last = left_leaf;
      }
    }
    if (right_leaf) {
      right_leaf->leaf_link->previous = left_leaf;

      if (tree->first == node) {
        tree->first = right_leaf;
      }
    }

    BNPPP_MEMORY_FREE(BPlusTreeLeafLink, node->leaf_link);
  }

  BNPPP_MEMORY_FREE(BPlusTreeNode, node);
}

static uint8_t BPlusTreeNodeIsLeaf(BPlusTreeNode const* node) {
  BNPPP_ERROR_IF(return 0, !node, "bplus_tree_node_is_leaf with null node");

  /* Leaf nodes do have a leaf link */
  return node->leaf_link != 0;
}

static void BPlusTreeNodeMoveAndResizeData(BPlusTreeNode* node, size_t const to_offset, size_t const from_offset) {
  BNPPP_ERROR_IF(return , !node, "bplus_tree_node_move_data with null node");
  BNPPP_ERROR_IF(BPlusTreeNodeDebug(node, stderr);
                 assert(0);
                 return ,
                 from_offset > node->size,
                 "bplus_tree_node_move_data with from_offset greater than node size");

  /* Move data if needed and if the source offset is inside the current data */
  if ((from_offset != to_offset) && (from_offset < node->size)) {
    memmove(node->items + to_offset, node->items + from_offset, (node->size - from_offset) * sizeof(*node->items));
  }

  /* Update the node size accordingly */
  if (from_offset < to_offset) {
    node->size += to_offset - from_offset;
  } else if (from_offset > to_offset) {
    node->size -= from_offset - to_offset;
    memset(node->items + node->size, 0xFF, (from_offset - to_offset) * sizeof(*node->items));
  }

  BNPPP_ERROR_IF(return ,
                 node->size > BPLUS_TREE_ORDER,
                 "node %p size is unexpectedly big after moving data from %zu to %zu",
                 node, from_offset, to_offset);
}

static void BPlusTreeNodeCopyData(BPlusTreeNode* to_node,
                                  size_t const   to_offset,
                                  BPlusTreeNode* from_node,
                                  size_t const   from_offset,
                                  size_t         count) {
  BNPPP_ERROR_IF(return , to_node == from_node, "bplus_tree_node_copy_data with from node == to node");

  /* Make room in the destination node */
  BPlusTreeNodeMoveAndResizeData(to_node, to_offset + count, to_offset);

  /* Copy data from source node to destination node */
  memcpy(to_node->items + to_offset, from_node->items + from_offset, count * sizeof(*from_node->items));

  /* Shrink the source node */
  BPlusTreeNodeMoveAndResizeData(from_node, from_offset, from_offset + count);

  /* Update the destination node's child parent links */
  BPlusTreeNodeUpdateParent(to_node);
}

static size_t BPlusTreeNodeGetKeyIndexFirstOrBefore(BPlusTreeNode const* node, BNPPP_Key const key) {
  BNPPP_ERROR_IF(return -1, !node, "bplus_tree_node_get_key_position with null node");

  /* Few obvious cases that need to be resolved so we can start iterating at the middle */
  if (node->items[0].key >= key) {
    return 0;
  }
  if (node->items[node->size - 1].key < key) {
    return node->size - 1;
  }

  /* Jump by power of two steps, smaller and smaller, starting at the middle of the array.
   *
   * We might jump on the actual result without knowing it until we've checked the lower values
   * so we might have to correct the result at the end.
   */
  size_t  index = BPLUS_TREE_ORDER >> 1;
  uint8_t step  = BPLUS_TREE_ORDER >> 2;
  do {
    if (node->items[index].key >= key) {
      index -= step;
    } else {
      index += step;
    }

    step >>= 1;
  } while (step > 0);

  /* Correct if we've missed the exact position */
  if (node->items[index].key < key) {
    index++;
  }
  if (node->items[index].key > key) {
    index--;
  }

  L_TRACE("find first or before %lu at %zu in node %p (k: %lu, size: %lu, keys: %lu,%lu,%lu,%lu)",
          key,
          index,
          node,
          node->key,
          node->size,
          node->items[0].key,
          node->items[1].key,
          node->items[2].key,
          node->items[3].key);
  return index;
}

static size_t BPlusTreeNodeGetKeyIndexAfterLast(BPlusTreeNode const* node, BNPPP_Key const key) {
  BNPPP_ERROR_IF(return -1, !node, "bplus_tree_node_get_key_position with null node");

  /* Few obvious cases that need to be resolved so we can start iterating at the middle */
  if (node->items[0].key > key) {
    return 0;
  }
  if (node->items[node->size - 1].key <= key) {
    return node->size;
  }

  /* Jump by power of two steps, smaller and smaller, starting at the middle of the array.
   *
   * We might jump on the actual result without knowing it until we've checked the lower values
   * so we might have to correct the result at the end.
   */
  size_t  index = BPLUS_TREE_ORDER >> 1;
  uint8_t step  = BPLUS_TREE_ORDER >> 2;
  do {
    if (node->items[index].key > key) {
      index -= step;
    } else {
      index += step;
    }

    step >>= 1;
  } while (step > 0);

  /* Correct if we've missed the exact position */
  if (node->items[index].key <= key) {
    index++;
  }

  L_TRACE("find after last %lu at %zu in node %p (k: %lu, size: %lu, keys: %lu,%lu,%lu,%lu)",
          key,
          index,
          node,
          node->key,
          node->size,
          node->items[0].key,
          node->items[1].key,
          node->items[2].key,
          node->items[3].key);

  return index;
} // BPlusTreeNodeGetKeyIndexAfterLast

static size_t BPlusTreeLeafFindKey(BPlusTreeNode const* node, BNPPP_Key const key) {
  BNPPP_ERROR_IF(return -1, !node, "bplus_tree_leaf_find_key with null node");

  /* Return the index of the first item with the given key, or node->size if key is not found */
  size_t const index = BPlusTreeNodeGetKeyIndexFirstOrBefore(node, key);
  if (node->items[index].key == key) {
    return index;
  }

  return node->size;
}

static size_t BPlusTreeLeafFindKeyData(BPlusTreeNode const* node, BNPPP_Key const key, void const* data) {
  BNPPP_ERROR_IF(return -1, !node, "bplus_tree_leaf_find_key_data with null node");

  /* Return the index of the first item with the given key and given data, or node->size if key is not found */
  BPlusTreeItem const* const begin = node->items;
  BPlusTreeItem const* const end   = begin + node->size;
  BPlusTreeItem const*       item  = begin + BPlusTreeLeafFindKey(node, key);
  while (item < end && item->key == key && item->data != data) {
    ++item;
  }

  if ((item < end) && (item->key == key)) {
    L_TRACE("find %lu=>%p at %zu in node %p (k: %lu, size: %lu, keys: %lu,%lu,%lu,%lu)",
            key,
            data,
            item - begin,
            node,
            node->key,
            node->size,
            node->items[0].key,
            node->items[1].key,
            node->items[2].key,
            node->items[3].key);
    return item - begin;
  }

  return node->size;
}

static BPlusTreeNode* BPlusTreeLeafGetNotEmpty(BPlusTreeNode* leaf) {
  /* Iterate over the leaf link list until leaf is null or size is not 0 */
  while (leaf && leaf->size == 0) {
    leaf = leaf->leaf_link->next;
  }

  return leaf;
}

static void BPlusTreeNodeInsertAfter(BPlusTree*      tree,
                                     BPlusTreeNode*  parent,
                                     BNPPP_Key const right_key,
                                     BPlusTreeNode*  right_node,
                                     BPlusTreeNode*  left_node) {
  BNPPP_ERROR_IF(return , !tree, "bplus_tree_node_insert_after with null tree");
  BNPPP_ERROR_IF(return , !parent, "bplus_tree_node_insert_after with null parent");
  BNPPP_ERROR_IF(return , !right_node, "bplus_tree_node_insert_after with null right_node");
  BNPPP_ERROR_IF(return , !left_node, "bplus_tree_node_insert_after with null left_node");

  L_TRACE("insert %lu=>%p after %lu=>%p to node %p (k: %lu, size: %lu, keys: %lu,%lu,%lu,%lu)",
          right_key,
          right_node,
          left_node->key,
          left_node,
          parent,
          parent->key,
          parent->size,
          parent->items[0].key,
          parent->items[1].key,
          parent->items[2].key,
          parent->items[3].key);

  /* Inserts the given right_node immediately after the position of (left_node->key, left_node) */
  size_t index = BPlusTreeLeafFindKeyData(parent, left_node->key, left_node);
  BNPPP_ERROR_IF(return ,
                 index >= parent->size,
                 "bplus_tree_node_insert_after left_node %lu=>%p, but node found at %zu and parent size is %zu",
                 left_node->key, left_node, index, parent->size);
  index++;

  BPlusTreeNodeMoveAndResizeData(parent, index + 1, index);
  parent->items[index].key       = right_key;
  parent->items[index].key  = right_key;
  parent->items[index].data = right_node;

  if (parent->size >= BPLUS_TREE_ORDER) {
    BPlusTreeNodeSplit(tree, parent);
  }
}

static void BPlusTreeNodeInsert(BPlusTree* tree, BPlusTreeNode* node, BNPPP_Key const key, void* data) {
  BNPPP_ERROR_IF(return , !tree, "bplus_tree_node_insert with null tree");
  BNPPP_ERROR_IF(assert(0);
                 return , !node, "bplus_tree_node_insert with null node");

  L_TRACE("insert %lu=>%p to node %p (k: %lu, size: %lu, keys: %lu,%lu,%lu,%lu)",
          key,
          data,
          node,
          node->key,
          node->size,
          node->items[0].key,
          node->items[1].key,
          node->items[2].key,
          node->items[3].key);

  /* Inserts the given (key, data) after the last item with a key lesser or equals to key */
  size_t const index = BPlusTreeNodeGetKeyIndexAfterLast(node, key);
  BPlusTreeNodeMoveAndResizeData(node, index + 1, index);
  node->items[index].key       = key;
  node->items[index].key  = key;
  node->items[index].data = data;

  if (node->size >= BPLUS_TREE_ORDER) {
    BPlusTreeNodeSplit(tree, node);
  }
}

static void BPlusTreeNodePropagateMinimalKey(BPlusTreeNode* node) {
  /* Update the node's children lesser key to the node's key if needed, this is used to keep the tree consistent after a removal */
  if (!BPlusTreeNodeIsLeaf(node) && (node->size > 0)) {
    node->items[0].key      = node->key;
    node->items[0].key = node->key;
    BPlusTreeNode* first_child = (BPlusTreeNode*) node->items[0].data;
    first_child->key = node->key;

    BPlusTreeNodePropagateMinimalKey(first_child);
  }
}

static void BPlusTreeNodeRemoveRange(BPlusTree*     tree,
                                     BPlusTreeNode* node,
                                     size_t const   index_from,
                                     size_t const   index_to) {
  BNPPP_ERROR_IF(return , !tree, "bplus_tree_node_remove with null tree");
  BNPPP_ERROR_IF(return , !node, "bplus_tree_node_remove with null node");
  BNPPP_ERROR_IF(return , index_to <= index_from, "bplus_tree_node_remove with index_to <= index_from");
  BNPPP_ERROR_IF(return , index_to > node->size, "bplus_tree_node_remove with index_to > node->size");

  L_TRACE("remove [%lu,%lu[ from node %p (k: %lu, size: %lu, keys: %lu,%lu,%lu,%lu)",
          index_from,
          index_to,
          node,
          node->key,
          node->size,
          node->items[0].key,
          node->items[1].key,
          node->items[2].key,
          node->items[3].key);

  /* Shrink the current' node data */
  BPlusTreeNodeMoveAndResizeData(node, index_from, index_to);
  if (index_from == 0) {
    /* If we've erased the current node first children, we need to update the lesser keys */
    BPlusTreeNodePropagateMinimalKey(node);
  }

  if (node == tree->root) {
    return;
  }

  /* If the node is empty, try to collapse the tree */
  if (node->size == 0) {
    if (node->key > 0) {
      BPlusTreeLeafRemoveKeyData(tree, node->parent, node->key, node);
      BPlusTreeNodeDestroy(tree, node);
    }

    BPlusTreeNodeCollapseRoot(tree);
  }
}

static void* BPlusTreeLeafRemoveKeyFirst(BPlusTree* tree, BPlusTreeNode* node, BNPPP_Key const key) {
  BNPPP_ERROR_IF(return NULL, !tree, "bplus_tree_leaf_remove_key_first with null tree");
  BNPPP_ERROR_IF(return NULL, !node, "bplus_tree_leaf_remove_key_first with null node");

  L_TRACE("remove first %lu from node %p (k: %lu, size: %lu, keys: %lu,%lu,%lu,%lu)",
          key,
          node,
          node->key,
          node->size,
          node->items[0].key,
          node->items[1].key,
          node->items[2].key,
          node->items[3].key);

  /* Remove the first element with the given key */
  size_t index = BPlusTreeLeafFindKey(node, key);
  void*  data  = NULL;
  if (index < node->size) {
    data = node->items[index].data;
    BPlusTreeNodeRemoveRange(tree, node, index, index + 1);
  }

  return data;
}

static void BPlusTreeLeafRemoveKeyData(BPlusTree* tree, BPlusTreeNode* node, BNPPP_Key const key, void const* data) {
  BNPPP_ERROR_IF(return , !tree, "bplus_tree_leaf_remove_key_data with null tree");
  BNPPP_ERROR_IF(return , !node, "bplus_tree_leaf_remove_key_data with null node");

  L_TRACE("remove %lu=>%p from node %p (k: %lu, size: %lu, keys: %lu,%lu,%lu,%lu)",
          key,
          data,
          node,
          node->key,
          node->size,
          node->items[0].key,
          node->items[1].key,
          node->items[2].key,
          node->items[3].key);

  /* Remove the first item with the given (key, data) */
  size_t index = BPlusTreeLeafFindKeyData(node, key, data);
  if (index < node->size) {
    BPlusTreeNodeRemoveRange(tree, node, index, index + 1);
  }
}

static void BPlusTreeNodeSplit(BPlusTree* tree, BPlusTreeNode* left_node) {
  BNPPP_ERROR_IF(return , !tree, "bplus_tree_node_split with null tree");
  BNPPP_ERROR_IF(return , !left_node, "bplus_tree_node_split with null node");

  L_TRACE("split node %p", left_node);

  size_t const total_size = left_node->size;

  /* Create the new right sibling */
  BPlusTreeNode* right_node = BPlusTreeNodeCreate(tree, BPlusTreeNodeIsLeaf(left_node), left_node);

  /* Compute the partition of the current node's data */
  BNPPP_Key const first_key = left_node->key;
  BNPPP_Key const last_key  = BPlusTreeNodeLast(left_node)->key;
  size_t          right_node_offset;
  if (first_key == last_key) {
    right_node->key   = first_key;
    right_node_offset = total_size / 2;
  } else {
    right_node->key   = first_key + (last_key - first_key) / 2;
    right_node_offset = BPlusTreeNodeGetKeyIndexAfterLast(left_node, right_node->key);
    right_node->key++;
  }

  /* Move the right part of the data partition of new sibling */
  BPlusTreeNodeCopyData(right_node, 0, left_node, right_node_offset, total_size - right_node_offset);

  /* If the new sibling is not a leaf, the sibling key has to be the minimal key of its children */
  if (!BPlusTreeNodeIsLeaf(left_node)) {
    right_node->key = right_node->items[0].key;
  }

  BPlusTreeNode* parent = left_node->parent;
  if (!parent) {
    /* If the current node does not have a parent, create a new one, it's the new tree root */
    parent            = BPlusTreeNodeCreate(tree, 0, 0);
    parent->key       = left_node->key;
    tree->root        = parent;
    left_node->parent = parent;

    parent->items[0].data = left_node;
    parent->items[0].key  = left_node->key;
    parent->items[0].key       = left_node->key;
    parent->size          = 1;
  }

  right_node->parent = parent;
  BPlusTreeNodeInsertAfter(tree, parent, right_node->key, right_node, left_node);

  /* If new sibling is full, split it again, this may seem weird, but this is required because we allow multiple entries with the same key */
  if (right_node->size >= BPLUS_TREE_ORDER) {
    BPlusTreeNodeSplit(tree, right_node);
  }
} /* BPlusTreeNodeSplit */

static void BPlusTreeNodeUpdateParent(BPlusTreeNode* node) {
  /* Update the current node children' parent link */
  if (!BPlusTreeNodeIsLeaf(node)) {
    for (size_t i = 0; i < node->size; ++i) {
      ((BPlusTreeNode*) node->items[i].data)->parent = node;
    }
  }
}

static void BPlusTreeNodeCollapseRoot(BPlusTree* tree) {
  BNPPP_ERROR_IF(return , !tree, "bplus_tree_node_collapse_root with null tree");

  /* If the root only has one child, we can collapse it, until it is also a leaf node */
  while (tree->root->size <= 1 && !BPlusTreeNodeIsLeaf(tree->root)) {
    BPlusTreeNode* node = tree->root;
    tree->root = (BPlusTreeNode*) node->items[0].data;
    BPlusTreeNodeDestroy(tree, node);
  }

  tree->root->parent = 0;
}

BPlusTree* BPlusTreeCreate() {
  BPlusTree* tree = BNPPP_MEMORY_ALLOC_ZEROED(BPlusTree);

  L_TRACE("create tree %p", tree);
  BNPPP_ERROR_IF(return 0, !tree, "create tree failed, unable to allocate memory of size %zu.", sizeof(BPlusTree));

  tree->root      = BPlusTreeNodeCreate(tree, 1, 0);
  tree->root->key = 0;

  tree->first = tree->root;
  tree->last  = tree->root;

  return tree;
}

static void BPlusTreeNodeForeachDestroy(BPlusTreeNode* node, void* arguments) {
  BPlusTreeNodeDestroy((BPlusTree*) arguments, node);
}

void BPlusTreeDestroy(BPlusTree* tree) {
  BNPPP_ERROR_IF(return , !tree, "bplus_tree_destroy with null tree");

  BPlusTreeForeachNode(tree, &BPlusTreeNodeForeachDestroy, tree);

  L_TRACE("destroy tree %p", tree);
  BNPPP_MEMORY_FREE(BPlusTree, tree);
}

void BPlusTreeDestroyEach(BPlusTree* tree,
                          void (* function)(void const* tree, BNPPP_Key key, void* data, void* argument),
                          void* argument) {
  BPlusTreeIterator* iterator = BPlusTreeFirst(tree);

  while (iterator) {
    function(tree, iterator->item->key, iterator->item->data, argument);
    iterator = BPlusTreeIteratorNext(iterator);
  }
  BPlusTreeDestroy(tree);
}

static BPlusTreeNode* BPlusTreeGetLeaf(BPlusTree const* tree, BNPPP_Key const key) {
  BNPPP_ERROR_IF(return 0, !tree, "bplus_tree_get_leaf with null tree");

  /* Returns the first leaf that may contain the given key */
  BPlusTreeNode* node = tree->root;
  while (node && !BPlusTreeNodeIsLeaf(node)) {
    size_t const index = BPlusTreeNodeGetKeyIndexFirstOrBefore(node, key);
    node = (BPlusTreeNode*) node->items[index].data;
  }

  return node;
}

static BPlusTreeNode* BPlusTreeFindLeafWithKey(BPlusTree const* tree, BNPPP_Key const key) {
  BNPPP_ERROR_IF(return 0, !tree, "bplus_tree_find_leaf_with_key with null tree");

  /* Returns the first leaf that actually contains some item with the given key */
  BPlusTreeNode* leaf = BPlusTreeLeafGetNotEmpty(BPlusTreeGetLeaf(tree, key));
  while (leaf) {
    if (leaf->items[0].key <= key) {
      if (BPlusTreeLeafFindKey(leaf, key) < leaf->size) {
        return leaf;
      }
    } else {
      break;
    }

    leaf = BPlusTreeLeafGetNotEmpty(leaf->leaf_link->next);
  }

  return 0;
}

static BPlusTreeNode* BPlusTreeFindLeafWithKeyData(BPlusTree const* tree, BNPPP_Key const key, void* data) {
  BNPPP_ERROR_IF(return 0, !tree, "bplus_tree_find_leaf_with_key with null tree");

  /* Returns the first leaf that actually contains some item with the given key and given data */
  BPlusTreeNode* leaf = BPlusTreeLeafGetNotEmpty(BPlusTreeGetLeaf(tree, key));
  while (leaf) {
    if (leaf->items[0].key <= key) {
      if (BPlusTreeLeafFindKeyData(leaf, key, data) < leaf->size) {
        return leaf;
      }
    } else {
      break;
    }

    leaf = BPlusTreeLeafGetNotEmpty(leaf->leaf_link->next);
  }

  return 0;
}

static BPlusTreeItem const* BPlusTreeLeafGetNextToLastWithKey(BPlusTree const* tree, BNPPP_Key const key) {
  BPlusTreeItem const* end      = 0;
  BPlusTreeNode const* end_leaf = BPlusTreeGetLeaf(tree, key);

  if (end_leaf) {
    size_t end_index = BPlusTreeNodeGetKeyIndexAfterLast(end_leaf, key);
    if (end_index < end_leaf->size) {
      end = end_leaf->items + end_index;
    } else {
      end_leaf = end_leaf->leaf_link->next;
      if (end_leaf) {
        end = end_leaf->items;
      }
    }
  }

  return end;
}

void BPlusTreeInsert(BPlusTree* tree, BNPPP_Key const key, void* data) {
  BNPPP_ERROR_IF(return , !tree, "bplus_tree_insert with null tree");

  BPlusTreeNodeInsert(tree, BPlusTreeGetLeaf(tree, key), key, data);
}

BPlusTreeItem const* BPlusTreeAt(BPlusTree const* tree, size_t const index) {
  BNPPP_ERROR_IF(return 0, !tree, "bplus_tree_get_at with null tree");

  /* Returns the item at the given index in the leaf linked list */
  size_t         data_index = index;
  BPlusTreeNode* leaf       = BPlusTreeLeafGetNotEmpty(tree->first);
  while (leaf && data_index >= leaf->size) {
    data_index -= leaf->size;
    leaf        = BPlusTreeLeafGetNotEmpty(leaf->leaf_link->next);
  }

  if (!leaf) {
    return 0;
  } else {
    return leaf->items + data_index;
  }
}

BPlusTreeItem const* BPlusTreeGet(BPlusTree const* tree, BNPPP_Key const key) {
  BNPPP_ERROR_IF(return 0, !tree, "bplus_tree_get_first with null tree");

  /* Try to find the first item with the given key */
  BPlusTreeNode* leaf = BPlusTreeLeafGetNotEmpty(BPlusTreeGetLeaf(tree, key));
  if (leaf) {
    size_t index = BPlusTreeLeafFindKey(leaf, key);

    if (index < leaf->size) {
      return leaf->items + index;
    }
  }

  return 0;
}

void BPlusTreeRemove(BPlusTree* tree, BNPPP_Key const key, void* data) {
  BNPPP_ERROR_IF(return , !tree, "bplus_tree_remove with null tree");

  /* Try to find the first item with the given (key, data) and remove it from the tree */
  BPlusTreeNode* leaf = BPlusTreeFindLeafWithKeyData(tree, key, data);
  if (leaf) {
    BPlusTreeLeafRemoveKeyData(tree, leaf, key, data);
  }
}

void* BPlusTreeRemoveFirst(BPlusTree* tree, BNPPP_Key const key) {
  BNPPP_ERROR_IF(return NULL, !tree, "bplus_tree_remove_first with null tree");

  /* Try to find the first item with the given key and remove it from the tree */
  BPlusTreeNode* leaf = BPlusTreeFindLeafWithKey(tree, key);
  if (leaf) {
    return BPlusTreeLeafRemoveKeyFirst(tree, leaf, key);
  }

  return NULL;
}

void BPlusTreeRemoveAll(BPlusTree* tree, BNPPP_Key const key) {
  BNPPP_ERROR_IF(return , !tree, "bplus_tree_remove_all with null tree");

  /* Try to find the all items with the given key and remove them from the tree */
  BPlusTreeNode* leaf = BPlusTreeFindLeafWithKey(tree, key);
  while (leaf && leaf->items[0].key <= key) {
    size_t index_from = BPlusTreeLeafFindKey(leaf, key);
    size_t index_to   = BPlusTreeNodeGetKeyIndexAfterLast(leaf, key);

    BPlusTreeNodeRemoveRange(tree, leaf, index_from, index_to);
    leaf = BPlusTreeLeafGetNotEmpty(leaf->leaf_link->next);
  }
}

BPlusTreeItem const* BPlusTreeIteratorGetItem(BPlusTreeIterator const* iterator) {
  return iterator->item;
}

BPlusTreeIterator* BPlusTreeFirst(BPlusTree const* tree) {
  BNPPP_ERROR_IF(return 0, !tree, "bplus_tree_first with null tree");

  BPlusTreeNode* leaf = BPlusTreeLeafGetNotEmpty(tree->first);
  if (leaf) {
    return BPlusTreeIteratorCreate(tree, leaf, leaf->items, 0);
  }

  return 0;
}

BPlusTreeIterator* BPlusTreeFirstWithKey(BPlusTree const* tree, BNPPP_Key const key) {
  BNPPP_ERROR_IF(return 0, !tree, "bplus_tree_get_first with null tree");

  BPlusTreeNode* leaf = BPlusTreeLeafGetNotEmpty(BPlusTreeGetLeaf(tree, key));
  if (leaf) {
    size_t index = BPlusTreeLeafFindKey(leaf, key);

    if (index < leaf->size) {
      return BPlusTreeIteratorCreate(tree, leaf, leaf->items + index, BPlusTreeLeafGetNextToLastWithKey(tree, key));
    }
  }

  return 0;
}

BPlusTreeIterator* BPlusTreeRangeFirst(BPlusTree const* tree,
                                       BNPPP_Key const  key_from,
                                       BNPPP_Key const  key_to) {
  BNPPP_ERROR_IF(return 0, !tree, "bplus_tree_range_first with null tree");

  BPlusTreeNode* leaf = BPlusTreeLeafGetNotEmpty(BPlusTreeGetLeaf(tree, key_from));
  if (leaf) {
    size_t index = BPlusTreeLeafFindKey(leaf, key_from);

    if (index < leaf->size) {
      return BPlusTreeIteratorCreate(tree, leaf, leaf->items + index, BPlusTreeLeafGetNextToLastWithKey(tree, key_to));
    }
  }

  return 0;
}

static BPlusTreeIterator* BPlusTreeIteratorCreate(BPlusTree const*     tree,
                                                  BPlusTreeNode const* leaf,
                                                  BPlusTreeItem const* item,
                                                  BPlusTreeItem const* end) {
  // BPlusTreeIterator * iterator = g_slice_new0(BPlusTreeIterator);
  BPlusTreeIterator* iterator = (BPlusTreeIterator*) &(tree->iterator);

  BNPPP_ERROR_IF(return 0, !iterator, "create iterator failed, unable to allocate memory of size %zu.",
                 sizeof(BPlusTreeIterator));

  if (iterator->leaf != 0) {
    L_WARN("reusing iterator that has not been ended.");
  }

  iterator->leaf     = leaf;
  iterator->leaf_end = leaf->items + leaf->size;
  iterator->item     = item;
  iterator->end      = end;

  return iterator;
}

static void BPlusTreeIteratorDestroy(BPlusTreeIterator* iterator) {
  BNPPP_ERROR_IF(return , !iterator, "bplus_tree_iterator_destroy with null iterator");

  iterator->leaf     = 0;
  iterator->leaf_end = 0;
  iterator->item     = 0;
  iterator->end      = 0;

  // g_slice_free(BPlusTreeIterator, iterator);
}

BPlusTreeIterator* BPlusTreeIteratorNext(BPlusTreeIterator* iterator) {
  BNPPP_ERROR_IF(return 0, !iterator, "bplus_tree_iterator_next with null iterator");

  iterator->item++;
  if (iterator->leaf_end <= iterator->item) {
    BPlusTreeNode* const leaf = BPlusTreeLeafGetNotEmpty(iterator->leaf->leaf_link->next);
    if (leaf) {
      iterator->item     = leaf->items;
      iterator->leaf_end = leaf->items + leaf->size;
      iterator->leaf     = leaf;
    } else {
      BPlusTreeIteratorEnd(iterator);
      return 0;
    }
  }

  if (iterator->item == iterator->end) {
    BPlusTreeIteratorEnd(iterator);
    return 0;
  }

  return iterator;
}

void BPlusTreeIteratorEnd(BPlusTreeIterator* iterator) {
  if (iterator) {
    BPlusTreeIteratorDestroy(iterator);
  }
}

static void BPlusTreeNodeForeachNode(BPlusTree const* tree,
                                     BPlusTreeNode* node,
                                     void (* function)(BPlusTreeNode* node, void* argument),
                                     void* argument) {
  BNPPP_ERROR_IF(return , !tree, "bplus_tree_node_foreach_node with null tree");
  BNPPP_ERROR_IF(return , !node, "bplus_tree_node_foreach_node with null node");

  if (!BPlusTreeNodeIsLeaf(node)) {
    for (size_t i = 0; i < node->size; ++i) {
      if (node->items[i].data) {
        BPlusTreeNodeForeachNode(tree, (BPlusTreeNode*) node->items[i].data, function, argument);
      }
    }
  }

  function(node, argument);
}

static void BPlusTreeForeachNode(BPlusTree const* tree, void (* function)(BPlusTreeNode* node,
                                                                          void*          argument), void* argument) {
  BNPPP_ERROR_IF(return , !tree, "bplus_tree_foreach_node with null tree");

  BPlusTreeNodeForeachNode(tree, tree->root, function, argument);
}

static void BPlusTreeNodeDebug(BPlusTreeNode* node, void* argument) {
  struct _IO_FILE* file = (struct _IO_FILE*) argument;

  if (BPlusTreeNodeIsLeaf(node)) {
    fprintf(file, "  node_%p [label = \"{{<p> prev|%lu|size: %zu|<n> next}|{", node, node->key, node->size);
    for (size_t i = 0; i < BPLUS_TREE_ORDER; ++i) {
      if (i > 0) {
        fprintf(file, "| ");
      }
      if (i < node->size) {
        fprintf(file, "{<k%zu> %lu|%p}", i, node->items[i].key, node->items[i].data);
      }
    }
    fprintf(file, "}}\"];\n");

    if (node->leaf_link->previous) {
      fprintf(file, "  node_%p:p -> node_%p:n;\n", node, node->leaf_link->previous);
    }

    if (node->leaf_link->next) {
      fprintf(file, "  node_%p:n -> node_%p:p;\n", node, node->leaf_link->next);
    }
  } else {
    fprintf(file, "  node_%p [label = \"{%lu|size: %zu|{", node, node->key, node->size);
    for (size_t i = 0; i < BPLUS_TREE_ORDER; ++i) {
      if (i > 0) {
        fprintf(file, "| ");
      }
      if (i < node->size) {
        fprintf(file, "<k%zu> %lu", i, node->items[i].key);
      }
    }
    fprintf(file, "}}\"];\n");

    for (size_t i = 0; i < node->size; ++i) {
      fprintf(file, "  node_%p:k%zu -> node_%p;\n", node, i, node->items[i].data);
    }
  }
}

void BPlusTreeDebug(BPlusTree const* tree, FILE* file) {
  fprintf(file, "digraph tree {\n");
  fprintf(file, "  node [shape=record];\n");
  fprintf(file, "  tree [label = \"<f> first|<r> root|<l> last\"];\n");
  fprintf(file, "  tree:f -> node_%p;\n", tree->first);
  fprintf(file, "  tree:r -> node_%p;\n", tree->root);
  fprintf(file, "  tree:l -> node_%p;\n", tree->last);

  BPlusTreeForeachNode(tree, &BPlusTreeNodeDebug, file);
  fprintf(file, "}\n");
}

static void BPlusTreeDebugToFile(BPlusTree* tree) {
  char buffer[128];

  snprintf(buffer, 128, "tree_%p.dot.tmp", tree);
  FILE* file = fopen(buffer, "w");
  BPlusTreeDebug(tree, file);
  fclose(file);

  char buffer2[128];
  snprintf(buffer2, 128, "tree_%p.dot", tree);
  rename(buffer, buffer2);
}

typedef struct _BNPPP_Map         BNPPP_Map;
typedef struct _BNPPP_MapIterator BNPPP_MapIterator;

BNPPP_Map* BNPPP_MapCreate();
void       BNPPP_MapDestroy(BNPPP_Map* map);
void       BNPPP_MapDestroyEach(BNPPP_Map* map,
                                void (* function)(void const* tree, BNPPP_Key key, void* data, void* argument),
                                void* argument);

void  BNPPP_MapInsert(BNPPP_Map* map, BNPPP_Key const key, void* data);
void  BNPPP_MapRemove(BNPPP_Map* map, BNPPP_Key const key, void* data);
void* BNPPP_MapRemoveFirst(BNPPP_Map* map, BNPPP_Key const key);
void  BNPPP_MapRemoveAll(BNPPP_Map* map, BNPPP_Key const key);

BNPPP_MapItem const* BNPPP_MapAt(BNPPP_Map const* tree, size_t const index);
BNPPP_MapItem const* BNPPP_MapGet(BNPPP_Map const* tree, BNPPP_Key const key);

BNPPP_MapIterator*   BNPPP_MapIteratorNext(BNPPP_MapIterator* iterator);
void                 BNPPP_MapIteratorEnd(BNPPP_MapIterator* iterator);
BNPPP_MapItem const* BNPPP_MapIteratorGetItem(BNPPP_MapIterator const* iterator);

BNPPP_MapIterator* BNPPP_MapFirst(BNPPP_Map const* tree);
BNPPP_MapIterator* BNPPP_MapFirstWithKey(BNPPP_Map const* tree, BNPPP_Key const key);
BNPPP_MapIterator* BNPPP_MapRangeFirst(BNPPP_Map const* tree, BNPPP_Key const key_from, BNPPP_Key const key_to);

void BNPPP_MapDebug(BNPPP_Map const* map);

BNPPP_Map* BNPPP_MapCreate() {
  return (BNPPP_Map*) BPlusTreeCreate();
}

void BNPPP_MapDestroy(BNPPP_Map* map) {
  BPlusTreeDestroy((BPlusTree*) map);
}

void BNPPP_MapDestroyEach(BNPPP_Map* map,
                          void (* function)(void const* tree, BNPPP_Key key, void* data, void* argument),
                          void* argument) {
  BPlusTreeDestroyEach((BPlusTree*) map, function, argument);
}

void BNPPP_MapInsert(BNPPP_Map* map, BNPPP_Key const key, void* data) {
  BPlusTreeInsert((BPlusTree*) map, key, data);
}

void BNPPP_MapRemove(BNPPP_Map* map, BNPPP_Key const key, void* data) {
  BPlusTreeRemove((BPlusTree*) map, key, data);
}

void* BNPPP_MapRemoveFirst(BNPPP_Map* map, BNPPP_Key const key) {
  return BPlusTreeRemoveFirst((BPlusTree*) map, key);
}

void BNPPP_MapRemoveAll(BNPPP_Map* map, BNPPP_Key const key) {
  BPlusTreeRemoveAll((BPlusTree*) map, key);
}

BNPPP_MapItem const* BNPPP_MapAt(BNPPP_Map const* map, size_t const index) {
  return BPlusTreeAt((BPlusTree*) map, index);
}

BNPPP_MapItem const* BNPPP_MapGet(BNPPP_Map const* map, BNPPP_Key const key) {
  return BPlusTreeGet((BPlusTree*) map, key);
}

BNPPP_MapIterator* BNPPP_MapIteratorNext(BNPPP_MapIterator* iterator) {
  return (BNPPP_MapIterator*) BPlusTreeIteratorNext((BPlusTreeIterator*) iterator);
}

void BNPPP_MapIteratorEnd(BNPPP_MapIterator* iterator) {
  BPlusTreeIteratorEnd((BPlusTreeIterator*) iterator);
}

BNPPP_MapItem const* BNPPP_MapIteratorGetItem(BNPPP_MapIterator const* iterator) {
  return (BNPPP_MapItem const*) BPlusTreeIteratorGetItem((BPlusTreeIterator const*) iterator);
}

BNPPP_MapIterator* BNPPP_MapFirst(BNPPP_Map const* map) {
  return (BNPPP_MapIterator*) BPlusTreeFirst((BPlusTree*) map);
}

BNPPP_MapIterator* BNPPP_MapFirstWithKey(BNPPP_Map const* map, BNPPP_Key const key) {
  return (BNPPP_MapIterator*) BPlusTreeFirstWithKey((BPlusTree*) map, key);
}

BNPPP_MapIterator* BNPPP_MapRangeFirst(BNPPP_Map const* map, BNPPP_Key const key_from, BNPPP_Key const key_to) {
  return (BNPPP_MapIterator*) BPlusTreeRangeFirst((BPlusTree*) map, key_from, key_to);
}

void BNPPP_MapDebug(BNPPP_Map const* map) {
  BPlusTreeDebug((BPlusTree const*) map, stderr);
}
