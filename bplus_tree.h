//
// Created by Stewart Lantner on 4/22/23.
//

#ifndef CS421_PROJECT_BPLUS_TREE_H
#define CS421_PROJECT_BPLUS_TREE_H

#include "attribute_types.h"
#include "record.h"
#include "table.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct bplus_tree {
  bool is_leaf; // Is this node a leaf?
  bool is_root; // Is this node a root?
  int N;        // equal to floor(max_page_size / 12) - 1
  int min_num_key_values;
  int max_num_key_values;
  int min_num_ptrs;
  int max_num_ptrs;
  int *search_key_values;
  int num_search_keys;
  void **ptrs;
  int num_ptrs;
  struct bplus_tree *parent;
};

typedef struct bplus_tree BPlusTree;

BPlusTree *init_BPlusTree(int N, bool is_root, bool is_leaf);
void insert_into_BPlusTree(BPlusTree **bplusTree, Record *record, int value);
void remove_from_BPlusTree(BPlusTree **bPlusTree, int value, void *ptr);
Record *find_in_BPlusTree(BPlusTree **bPlusTree, int value);
void traverse_tree(BPlusTree *bPlusTree);

#endif // CS421_PROJECT_BPLUS_TREE_H
