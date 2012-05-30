#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#ifndef BPLUS_TREE_ORDER
# define BPLUS_TREE_ORDER_P 5
# define BPLUS_TREE_ORDER   (1 << BPLUS_TREE_ORDER_P)
#endif /* ifndef BPLUS_TREE_ORDER */

typedef struct _BplusTree BplusTree;
typedef struct _BplusNode BplusNode;
typedef struct _BplusLeaf BplusLeaf;

BplusTree* bplus_tree_new(GEqualFunc key_equal_func);
void       bplus_tree_destroy(BplusTree* bplus_tree);

BplusTree* bplus_tree_ref(BplusTree* bplus_tree);
void       bplus_tree_unref(BplusTree* bplus_tree);

static BplusNode* bplus_node_new(BplusTree* bplus_tree);
static BplusNode* bplus_node_dup(BplusTree* bplus_tree, BplusNode* bplus_node);
static void       bplus_node_destroy(BplusTree* bplus_tree, BplusNode* bplus_node);

void bplus_node_split(BplusTree* bplus_tree, BplusNode* bplus_node);
void bplus_node_insert_at(BplusTree* bplus_tree, BplusNode* bplus_node, size_t index, gpointer key, void* value);
void bplus_node_shrink(BplusTree* bplus_tree, BplusNode* bplus_node);
void bplus_node_remove_at(BplusTree* bplus_tree, BplusNode* bplus_node, size_t index);

static BplusLeaf* bplus_leaf_new(BplusTree* bplus_tree);
static BplusLeaf* bplus_leaf_dup(BplusTree* bplus_tree, BplusLeaf* bplus_leaf);
static void       bplus_leaf_destroy(BplusTree* bplus_tree, BplusLeaf* bplus_leaf);

int bplus_tree_print(BplusTree* bplus_tree);

struct _BplusTree {
  BplusNode* root;

  BplusLeaf* first;
  BplusLeaf* last;

  GEqualFunc key_equal_func;
  gint       ref_count;
};

struct _BplusNode {
  BplusNode* parent;
  gboolean   is_leaf;

  guint    length;
  gpointer keys[BPLUS_TREE_ORDER];
  gpointer values[BPLUS_TREE_ORDER];
};

#ifndef BPLUS_TREE_SEARCH_LINEAR
# define bplus_tree_node_fill_empty(bplus_node_m)            \
  memset(bplus_node_m->keys + bplus_node_m->length, 0xFF,    \
         sizeof(bplus_node_m->keys) - bplus_node_m->length * \
         sizeof(*bplus_node_m->keys));
#else // ifndef BPLUS_TREE_SEARCH_LINEAR
# define bplus_tree_node_fill_empty(bplus_node_m) do {} while (0)
#endif // ifndef BPLUS_TREE_SEARCH_LINEAR

struct _BplusLeaf {
  BplusNode node;

  BplusLeaf* left;
  BplusLeaf* right;
};

static size_t bplus_tree_search_key_index(BplusTree*            bplus_tree,
                                          size_t const          key_count,
                                          gpointer const* const keys,
                                          gpointer              key) {
  /* Few obvious cases that need to be resolved so we can start iterating at the middle */
  if ((key_count == 0) || (bplus_tree->key_equal_func(keys[0], key) > 0)) {
    return 0;
  }

#ifdef BPLUS_TREE_SEARCH_LINEAR
  size_t index = key_count - 1;
  while (index > 0) {
    if (bplus_tree->key_equal_func(keys[index], key) <= 0) {
      break;
    }
    index--;
  }

#else // ifdef BPLUS_TREE_SEARCH_LINEAR

  if (bplus_tree->key_equal_func(keys[key_count - 1], key) <= 0) {
    return key_count - 1;
  }

  /* Jump by power of two steps, smaller and smaller, starting at the middle of the array.
   *
   * We might jump on the actual result without knowing it until we've checked the lower keys
   * so we might have to correct the result at the end.
   */
  size_t index = 0;
  size_t step  = BPLUS_TREE_ORDER >> 1;
  do {
    int result = bplus_tree->key_equal_func(keys[index], key);
    if ((index >= key_count) || (result > 0)) {
      index -= step;
    } else {
      index += step;
    }

    step >>= 1;
  } while (step > 0);

  /* Correct if we've missed the exact position */
  if (bplus_tree->key_equal_func(keys[index], key) > 0) {
    index--;
  }
#endif // ifdef BPLUS_TREE_SEARCH_LINEAR

  return index;
}

size_t bplus_node_search_key_index(BplusTree* bplus_tree, BplusNode* bplus_node, gpointer const key) {
  return bplus_tree_search_key_index(bplus_tree, bplus_node->length, bplus_node->keys, key);
}

BplusTree* bplus_tree_ref(BplusTree* bplus_tree) {
  g_return_val_if_fail(bplus_tree != NULL, NULL);

  g_atomic_int_inc(&bplus_tree->ref_count);

  return bplus_tree;
}

void bplus_tree_unref(BplusTree* bplus_tree) {
  g_return_if_fail(bplus_tree != NULL);

  if (g_atomic_int_dec_and_test(&bplus_tree->ref_count)) {
    if (bplus_tree->root->is_leaf) {
      bplus_leaf_destroy(bplus_tree, bplus_tree->first);
    } else {
      bplus_node_destroy(bplus_tree, bplus_tree->root);
    }

    g_slice_free(BplusTree, bplus_tree);
  }
}

BplusTree* bplus_tree_new(GEqualFunc key_equal_func) {
  BplusTree* bplus_tree = g_slice_new(BplusTree);

  bplus_tree->key_equal_func = key_equal_func;
  bplus_tree->first          = bplus_leaf_new(bplus_tree);
  bplus_tree->last           = bplus_tree->first;
  bplus_tree->root           = (BplusNode*) bplus_tree->first;

  bplus_tree->ref_count = 1;

  return bplus_tree;
}

void bplus_tree_destroy(BplusTree* bplus_tree) {
  g_return_if_fail(bplus_tree != NULL);

  // bplus_tree_remove_all(bplus_tree);
  bplus_tree_unref(bplus_tree);
}

static void bplus_node_init(BplusNode* bplus_node, gboolean is_leaf) {
  g_return_if_fail(bplus_node != NULL);

  bplus_node->parent  = NULL;
  bplus_node->is_leaf = is_leaf;

  bplus_node->length = 0;
  bplus_tree_node_fill_empty(bplus_node);
}

BplusLeaf* bplus_leaf_new(BplusTree* bplus_tree) {
  BplusLeaf* bplus_leaf = g_slice_new(BplusLeaf);

  bplus_node_init(&bplus_leaf->node, TRUE);
  bplus_leaf->left  = NULL;
  bplus_leaf->right = NULL;

  return bplus_leaf;
}

BplusNode* bplus_node_new(BplusTree* bplus_tree) {
  BplusNode* bplus_node = g_slice_new(BplusNode);

  bplus_node_init(bplus_node, FALSE);

  return bplus_node;
}

BplusNode* bplus_node_dup(BplusTree* bplus_tree, BplusNode* bplus_node) {
  return g_slice_dup(BplusNode, bplus_node);
}

BplusLeaf* bplus_leaf_dup(BplusTree* bplus_tree, BplusLeaf* bplus_leaf) {
  return g_slice_dup(BplusLeaf, bplus_leaf);
}

void bplus_leaf_destroy(BplusTree* bplus_tree, BplusLeaf* bplus_leaf) {
  g_return_if_fail(bplus_tree != NULL);
  g_return_if_fail(bplus_leaf != NULL);

  if (bplus_leaf->left != NULL) {
    bplus_leaf->left->right = bplus_leaf->right;
  }

  if (bplus_leaf->right != NULL) {
    bplus_leaf->right->left = bplus_leaf->left;
  }

  if (bplus_tree->first == bplus_leaf) {
    bplus_tree->first = bplus_leaf->right;
  }

  if (bplus_tree->last == bplus_leaf) {
    bplus_tree->last = bplus_leaf->left;
  }

  g_slice_free(BplusLeaf, bplus_leaf);
}

void bplus_node_destroy(BplusTree* bplus_tree, BplusNode* bplus_node) {
  g_return_if_fail(bplus_tree != NULL);
  g_return_if_fail(bplus_node != NULL);

  if (bplus_node->is_leaf) {
    bplus_leaf_destroy(bplus_tree, (BplusLeaf*) bplus_node);
  } else {
    for (size_t i = 0; i < bplus_node->length; ++i) {
      bplus_node_destroy(bplus_tree, (BplusNode*) bplus_node->values[i]);
    }

    g_slice_free(BplusNode, bplus_node);
  }
}

void bplus_node_propagate_key(BplusTree* bplus_tree, BplusNode* bplus_node, gpointer key) {
  g_return_if_fail(bplus_tree != NULL);
  g_return_if_fail(bplus_node != NULL);
  g_return_if_fail(bplus_node->parent != NULL || bplus_tree->root == bplus_node);

  do {
    g_return_if_fail(bplus_node->length > 0);
    if (!bplus_tree->key_equal_func(key, bplus_node->keys[0])) {
      break;
    }

    bplus_node->keys[0] = key;
    bplus_node          = bplus_node->parent;
  } while (bplus_node != NULL);
}

size_t bplus_node_get_index_in_parent(BplusTree* bplus_tree, BplusNode* bplus_node) {
  g_return_val_if_fail(bplus_tree != NULL, 0);
  g_return_val_if_fail(bplus_node != NULL, 0);
  g_return_val_if_fail(bplus_node->parent != NULL, 0);
  g_return_val_if_fail(bplus_node->parent != NULL || bplus_tree->root == bplus_node, 0);

  size_t index = bplus_node_search_key_index(bplus_tree, bplus_node->parent, bplus_node->keys[0]);
  while (bplus_node->parent->values[index] != bplus_node) {
    if(index == 0)
      return index;
    index--;
  }

  return index;
}

BplusNode* bplus_node_add_right_node(BplusTree* bplus_tree, BplusNode* bplus_node) {
  g_return_val_if_fail(bplus_tree != NULL, NULL);
  g_return_val_if_fail(bplus_node != NULL, NULL);
  g_return_val_if_fail(bplus_node->parent != NULL || bplus_tree->root == bplus_node, NULL);

  BplusNode* bplus_node_right = NULL;
  if (bplus_node->is_leaf) {
    BplusLeaf* bplus_leaf       = (BplusLeaf*) bplus_node;
    BplusLeaf* bplus_leaf_right = bplus_leaf_new(bplus_tree);

    bplus_leaf_right->left  = bplus_leaf;
    bplus_leaf_right->right = bplus_leaf->right;
    bplus_leaf->right       = bplus_leaf_right;

    if (bplus_leaf_right->right != NULL) {
      bplus_leaf_right->right->left = bplus_leaf_right;
    }
    if (bplus_tree->last == bplus_leaf) {
      bplus_tree->last = bplus_leaf_right;
    }

    bplus_node_right = (BplusNode*) bplus_leaf_right;
  } else {
    bplus_node_right = bplus_node_new(bplus_tree);
  }

  /* Insert right node into parent, and create a new parent if required */
  if (bplus_node->parent == NULL) {
    bplus_tree->root = bplus_node_new(bplus_tree);

    bplus_tree->root->length    = 1;
    bplus_tree->root->keys[0]   = bplus_node->keys[0];
    bplus_tree->root->values[0] = bplus_node;

    bplus_node->parent = bplus_tree->root;
  }
  bplus_node_right->parent = bplus_node->parent;

  return bplus_node_right;
} /* bplus_node_add_right_node */

void bplus_node_split(BplusTree* bplus_tree, BplusNode* bplus_node) {
  g_return_if_fail(bplus_tree != NULL);
  g_return_if_fail(bplus_node != NULL);
  g_return_if_fail(bplus_node->parent != NULL || bplus_tree->root == bplus_node);
  g_return_if_fail(bplus_node->length == BPLUS_TREE_ORDER);

  BplusNode* bplus_node_right = bplus_node_add_right_node(bplus_tree, bplus_node);
  g_return_if_fail(bplus_node_right != NULL);

  /* Find where to split */
  size_t split_index = BPLUS_TREE_ORDER >> 1;

  // gpointer const first_key   = bplus_node->keys[0];
  // gpointer const last_key    = bplus_node->keys[BPLUS_TREE_ORDER - 1];
  // if (first_key != last_key) {
  // gpointer const middle_key = first_key + (last_key - first_key) / 2;
  // split_index = bplus_node_search_key_index(bplus_tree, bplus_node, middle_key) + 1;
  // }

  /* Update parent pointer for nodes to be copied to right */
  if (!bplus_node->is_leaf) {
    for (size_t i = split_index; i < bplus_node->length; ++i) {
      ((BplusNode*) bplus_node->values[i])->parent = bplus_node_right;
    }
  }

  /* Move half keys and values to right node */
  bplus_node_right->length = bplus_node->length - split_index;
  memcpy(bplus_node_right->keys, bplus_node->keys + split_index, bplus_node_right->length *
         sizeof(*bplus_node_right->keys));
  memcpy(bplus_node_right->values, bplus_node->values + split_index, bplus_node_right->length *
         sizeof(*bplus_node_right->values));

  bplus_node->length = split_index;
  bplus_tree_node_fill_empty(bplus_node);

  size_t const insert_index = bplus_node_get_index_in_parent(bplus_tree, bplus_node) + 1;
  bplus_node_insert_at(bplus_tree, bplus_node->parent, insert_index, bplus_node_right->keys[0], bplus_node_right);
} /* bplus_node_split */

void bplus_node_insert_at(BplusTree* bplus_tree, BplusNode* bplus_node, size_t index, gpointer key, void* value) {
  g_return_if_fail(bplus_tree != NULL);
  g_return_if_fail(bplus_node != NULL);
  g_return_if_fail(bplus_node->parent != NULL || bplus_tree->root == bplus_node);
  g_return_if_fail(index <= bplus_node->length);
  g_return_if_fail(bplus_tree->key_equal_func(key, bplus_node->keys[index]) <= 0);

  size_t move_length = bplus_node->length - index;
  if (move_length > 0) {
    memmove(bplus_node->keys + index + 1, bplus_node->keys + index, move_length * sizeof(*bplus_node->keys));
    memmove(bplus_node->values + index + 1, bplus_node->values + index, move_length * sizeof(*bplus_node->values));
  }

  bplus_node->length++;

  /* If we modify keys[0], we may have to update parents lesser key as well to keep consistency */
  if (index == 0) {
    bplus_node_propagate_key(bplus_tree, bplus_node, key);
  }

  bplus_node->keys[index]   = key;
  bplus_node->values[index] = value;

  if (bplus_node->length == BPLUS_TREE_ORDER) {
    // g_print("-------------------------------------------\n");
    // g_print("Tree split, before:\n");
    // bplus_tree_print(bplus_tree);

    bplus_node_split(bplus_tree, bplus_node);

    // g_print("Tree split, after:\n");
    // bplus_tree_print(bplus_tree);
    // g_print("-------------------------------------------\n");
  }
}

void bplus_node_shrink(BplusTree* bplus_tree, BplusNode* bplus_node) {
  g_return_if_fail(bplus_tree != NULL);
  g_return_if_fail(bplus_node != NULL);
  g_return_if_fail(bplus_node->parent != NULL || bplus_tree->root == bplus_node);
  g_return_if_fail(bplus_node->length == 1);

  if (bplus_node->parent != NULL) {
    size_t const remove_index = bplus_node_get_index_in_parent(bplus_tree, bplus_node);
    bplus_node_remove_at(bplus_tree, bplus_node->parent, remove_index);
  }

  if (bplus_node->parent == NULL) {
    if (bplus_node->is_leaf) {
      bplus_tree->root  = bplus_node;
      bplus_tree->first = (BplusLeaf*) bplus_node;
      bplus_tree->last  = (BplusLeaf*) bplus_node;

    } else {
      BplusNode* bplus_node_child = (BplusNode*) bplus_node->values[0];
      bplus_tree->root         = bplus_node_child;
      bplus_node_child->parent = NULL;

      bplus_node_destroy(bplus_tree, bplus_node);
    }

  } else {
    bplus_node_destroy(bplus_tree, bplus_node);
  }
} /* bplus_node_shrink */

void bplus_node_remove_at(BplusTree* bplus_tree, BplusNode* bplus_node, size_t index) {
  g_return_if_fail(bplus_tree != NULL);
  g_return_if_fail(bplus_node != NULL);
  g_return_if_fail(bplus_node->parent != NULL || bplus_tree->root == bplus_node);
  g_return_if_fail(index < bplus_node->length);

  if (bplus_node->length == 1) {
    bplus_node_shrink(bplus_tree, bplus_node);
  }

  bplus_node->length--;
  size_t move_length = bplus_node->length - index;
  if (move_length > 0) {
    memmove(bplus_node->keys + index, bplus_node->keys + index + 1, move_length * sizeof(*bplus_node->keys));
    memmove(bplus_node->values + index, bplus_node->values + index + 1, move_length * sizeof(*bplus_node->values));
  }
}

BplusNode* bplus_node_find_next_for_key(BplusTree* bplus_tree, BplusNode* bplus_node, gpointer const key) {
  g_return_val_if_fail(bplus_node->parent != NULL || bplus_tree->root == bplus_node, NULL);

  return (BplusNode*) bplus_node->values[bplus_node_search_key_index(bplus_tree, bplus_node, key)];
}

BplusLeaf* bplus_tree_find_leaf_for_key(BplusTree* bplus_tree, gpointer key) {
  BplusNode* bplus_node = bplus_tree->root;

  while (!bplus_node->is_leaf) {
    g_return_val_if_fail(bplus_node != NULL || bplus_tree_print(bplus_tree), NULL);

    // g_return_val_if_fail(bplus_node->parent != NULL || bplus_tree_print(bplus_tree), NULL);
    // g_return_val_if_fail(bplus_tree->root == bplus_node || bplus_tree_print(bplus_tree), NULL);
    bplus_node = bplus_node_find_next_for_key(bplus_tree, bplus_node, key);
  }

  return (BplusLeaf*) bplus_node;
}

void bplus_tree_insert(BplusTree* bplus_tree, gpointer key, void* value) {
  BplusNode* bplus_node = (BplusNode*) bplus_tree_find_leaf_for_key(bplus_tree, key);

  g_return_if_fail(bplus_node->parent != NULL || bplus_tree->root == bplus_node);

  size_t insert_index = bplus_node_search_key_index(bplus_tree, bplus_node, key);
  if ((bplus_node->length > 0) && (bplus_tree->key_equal_func(bplus_node->keys[insert_index], key) <= 0)) {
    insert_index++;
  }

  bplus_node_insert_at(bplus_tree, bplus_node, insert_index, key, value);
}

void bplus_tree_remove(BplusTree* bplus_tree, gpointer key) {}

void bplus_tree_remove_value(BplusTree* bplus_tree, gpointer key, void* value) {
  // BplusNode* bplus_node   = (BplusNode*) bplus_tree_find_leaf_for_key(bplus_tree, key);
  // size_t     remove_index = bplus_node_search_key_index(bplus_node, key);

  // if (bplus_node->keys[remove_index] == key) {
  // bplus_node_remove_at(bplus_tree, bplus_node, remove_index);
  // }
}

void bplus_tree_get(BplusTree* bplus_tree, gpointer key) {
  // BplusNode* bplus_node   = (BplusNode*) bplus_tree_find_leaf_for_key(bplus_tree, key);
  // size_t     remove_index = bplus_node_search_key_index(bplus_node, key);

  // if (bplus_node->keys[remove_index] == key) {
  // bplus_node_remove_at(bplus_tree, bplus_node, remove_index);
  // }
}

void bplus_value_print(gpointer key, void* value, int depth) {
  static char const* indent = "                                                                   ";

  g_print("%*.s [%lu] = %p\n", depth, indent, key, value);
}

void bplus_node_print(gpointer key, BplusNode* bplus_node, int depth) {
  static char const* indent = "                                                                   ";

  g_print("%*.s [%lu] %p (p: %p) {\n", depth, indent, key, bplus_node, bplus_node->parent);
  for (size_t i = 0; i < bplus_node->length; ++i) {
    if (bplus_node->is_leaf) {
      bplus_value_print(bplus_node->keys[i], bplus_node->values[i], depth + 2);
    } else {
      bplus_node_print(bplus_node->keys[i], (BplusNode*) bplus_node->values[i], depth + 2);
    }
  }
  g_print("%*.s }\n", depth, indent);
}

int bplus_tree_print(BplusTree* bplus_tree) {
  g_print("\ntree %p {\n", bplus_tree->root);

  BplusNode* bplus_node = bplus_tree->root;
  for (size_t i = 0; i < bplus_node->length; ++i) {
    if (bplus_node->is_leaf) {
      bplus_value_print(bplus_node->keys[i], bplus_node->values[i], 2);
    } else {
      bplus_node_print(bplus_node->keys[i], (BplusNode*) bplus_node->values[i], 2);
    }
  }

  g_print("}\n");

  return 0;
}
