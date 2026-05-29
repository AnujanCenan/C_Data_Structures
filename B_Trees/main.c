#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "dynamic_array.h"

#define MAX_KEYS 4

typedef struct B_Tree_Key B_Tree_Key;
typedef struct B_Tree_Node B_Tree_Node;
typedef struct B_Tree B_Tree;

DECLARE_DYNAMIC_ARRAY(B_Tree_Node*, B_Node_Stack);

struct B_Tree_Key
{
    B_Tree_Key* next;
    B_Tree_Node* child;
    int key;

};

struct B_Tree_Node
{
    B_Tree_Key* head;
    B_Tree_Node* greater_than_child;
    int num_keys;
    bool is_leaf;
};

struct B_Tree
{
    B_Tree_Node* root;
    int num_nodes;
    int num_keys;
};


B_Tree* b_tree_init()
{
    B_Tree* tree = malloc(sizeof(B_Tree));
    tree->num_keys = 0;
    tree->num_nodes = 0;
    tree->root = NULL;
    return tree;
}

B_Tree_Key* b_tree_key_init(int key)
{
    B_Tree_Key* new_key = malloc(sizeof(B_Tree_Key));
    new_key->key = key;
    new_key->child = NULL;
    new_key->next = NULL;
    return new_key;
}

void b_tree_node_insert(B_Tree_Node* node, B_Tree_Key* new_key)
{
    // ASSUMPTION node is not NULL

    if (node->head == NULL)
    {
        node->head = new_key;
        new_key->next = NULL;
        node->num_keys = 1;
        return;
    }

    B_Tree_Key* prev_key = NULL;
    B_Tree_Key* curr_key = node->head;

    int key = new_key->key;
    
    while (curr_key)
    {
        if (curr_key->key > key)
        {
            new_key->next = curr_key;

            if (prev_key) 
            {
                prev_key->next = new_key;
            } else
            {
                node->head = new_key;
            }
            break;
        }

        prev_key = curr_key;
        curr_key = curr_key->next;
    }

    if (!curr_key)
    {
        // got to the end of the linked list without inserting
        prev_key->next = new_key;
        new_key->next = NULL;
    }

    node->num_keys++;
}

B_Tree_Node* b_tree_node_split(B_Tree_Node* node, B_Tree_Node* parent_node)
{
    // Node can be a leaf node OR a parent node, we don't know, we are just splitting

    // Assumption is that node->num_keys == MAX_KEYS + 1
    int keys_left_side = MAX_KEYS / 2;

    B_Tree_Key* median = node->head;
    B_Tree_Key* prev = NULL;

    
    for (int count = 0; count < keys_left_side; ++count)
    {
        prev = median;
        median = median->next;
    }

    B_Tree_Node* left_split = malloc(sizeof(B_Tree_Node));
    left_split->head = node->head;
    left_split->is_leaf = true;
    left_split->greater_than_child = NULL;
    left_split->num_keys = MAX_KEYS / 2;
    prev->next = NULL;

    node->head = median->next;      // right split
    node->num_keys = node->num_keys - left_split->num_keys - 1;

    left_split->greater_than_child = median->child;
    median->child = left_split;
    

    if (!parent_node)
    {        
        parent_node = malloc(sizeof(B_Tree_Node));
        parent_node->greater_than_child = node;
        parent_node->head = NULL;
        parent_node->is_leaf = false;
        parent_node->num_keys = 0;
    }
    
    b_tree_node_insert(parent_node, median);

    return parent_node;
  
}


B_Tree_Node* b_tree_node_init(B_Tree_Key* key)
{
    B_Tree_Node* node = malloc(sizeof(B_Tree_Node));
    node->num_keys = 1;
    node->head = key;
    key->next = NULL;
    return node;
}

void b_tree_insert(B_Tree* tree, int key)
{

    if (tree->root == NULL)
    {
        B_Tree_Key* new_key = b_tree_key_init(key);
        tree->root = b_tree_node_init(new_key);
        tree->root->is_leaf = true;
        tree->root->greater_than_child = NULL;

        tree->num_nodes = 1;
        tree->num_keys = 1;
        return;
    }
    // Find the leaf node
    B_Tree_Node* curr_node = tree->root;

    B_Node_Stack* stack = B_Node_Stack_init(10);

    while (!curr_node->is_leaf)
    {
        B_Node_Stack_append(stack, curr_node);
        B_Tree_Key* curr_key = curr_node->head;
        while (curr_key)
        {
            if (curr_key->key == key) return;
            if (curr_key->key > key)
            {
                curr_node = curr_key->child;
                break;
            }
            curr_key = curr_key->next;
        }

        if (curr_key == NULL) curr_node = curr_node->greater_than_child;
    }
    B_Tree_Key* new_key = b_tree_key_init(key);
    b_tree_node_insert(curr_node, new_key);

    while (curr_node->num_keys > MAX_KEYS)
    { 
        B_Tree_Node* parent = stack->size > 0 ? stack->data[stack->size - 1] : NULL;
        B_Tree_Node* popped_parent = parent;
        parent = b_tree_node_split(curr_node, parent);
        if (popped_parent == NULL)
        {
            tree->root = parent;
        }
        B_Node_Stack_pop(stack);
        curr_node = parent;
    }

    B_Node_Stack_free(stack);
}

void b_tree_node_free(B_Tree_Node* node)
{
    if (!node) return;

    B_Tree_Key* curr = node->head;
    while (curr)
    {
        b_tree_node_free(curr->child);
        B_Tree_Key* prev = curr;
        curr = curr->next;
        free(prev);
    }

    b_tree_node_free(node->greater_than_child);
    free(node);
}

void b_tree_free(B_Tree* tree)
{
    b_tree_node_free(tree->root);
    free(tree);
}


void print_indent(int generation)
{
    for (int i = 0; i < generation; ++i)
    {
        printf(" ");
    }
    printf("Gen %d: ", generation);
}

void print_node(B_Tree_Node* node, int generation)
{
    if (!node || !node->head) return;
    B_Tree_Key* curr = node->head;

    print_indent(generation);

    while (curr->next)
    {
        printf("%d --> ", curr->key);
        curr = curr->next;
    }

    printf("%d\n", curr->key);

    curr = node->head;
    while (curr)
    {
        print_node(curr->child, generation + 1);
        curr = curr->next;
    }

    print_node(node->greater_than_child, generation + 1);
}

void print_tree(B_Tree* tree)
{
    int generation = 0;
    print_node(tree->root, generation);

}

int main()
{

    B_Tree* b_tree = b_tree_init();

    b_tree_insert(b_tree, 9);
    b_tree_insert(b_tree, 15);
    b_tree_insert(b_tree, 26);
    b_tree_insert(b_tree, 2);
    b_tree_insert(b_tree, 14);
    b_tree_insert(b_tree, 103);
    b_tree_insert(b_tree, 21);
    b_tree_insert(b_tree, 7);
    b_tree_insert(b_tree, 31);
    b_tree_insert(b_tree, 1);
    b_tree_insert(b_tree, 10);

    b_tree_insert(b_tree, 22);
    b_tree_insert(b_tree, 23);
    b_tree_insert(b_tree, 24);

    b_tree_insert(b_tree, 8);
    b_tree_insert(b_tree, 12);

    b_tree_insert(b_tree, 13);

    print_tree(b_tree);


    b_tree_free(b_tree);
    
    return 0;
}