#ifndef B_TREE_H
#define B_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "dynamic_array.h"

typedef struct B_Tree_Key B_Tree_Key;
typedef struct B_Tree_Node B_Tree_Node;
typedef struct B_Tree B_Tree;

typedef struct B_Tree_Key_Node_Pair B_Tree_Key_Node_Pair;


struct B_Tree_Key
{
    B_Tree_Key* next;
    B_Tree_Key* prev;
    B_Tree_Node* child;
    int key;
    bool is_tail;
};

struct B_Tree_Node
{
    B_Tree_Key* head;
    B_Tree_Key* tail;
    int num_keys;
    bool is_leaf;
};

struct B_Tree
{
    B_Tree_Node* root;
    int max_keys;
    int min_keys;
};


B_Tree* b_tree_init(int max_keys);
void b_tree_insert(B_Tree* tree, int key);
void b_tree_delete(B_Tree* tree, int key);
void b_tree_free(B_Tree* tree);
void b_tree_print(B_Tree* tree);

#endif