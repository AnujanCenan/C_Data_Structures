#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "dynamic_array.h"

#define MAX_KEYS 4
#define MIN_KEYS MAX_KEYS / 2


typedef struct B_Tree_Key B_Tree_Key;
typedef struct B_Tree_Node B_Tree_Node;
typedef struct B_Tree B_Tree;

typedef struct B_Tree_Key_Node_Pair B_Tree_Key_Node_Pair;

DECLARE_DYNAMIC_ARRAY(B_Tree_Key_Node_Pair*, B_Tree_KN_Stack);

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
    int num_nodes;
    int num_keys;
};


struct B_Tree_Key_Node_Pair
{
    B_Tree_Key* key;
    B_Tree_Node* node;
};


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

    printf("%d -- num_keys=%d\n", curr->key, node->num_keys);

    curr = node->head;
    while (curr)
    {
        print_node(curr->child, generation + 1);
        curr = curr->next;
    }
}

void print_tree(B_Tree* tree)
{
    int generation = 0;
    print_node(tree->root, generation);

}


B_Tree* b_tree_init()
{
    B_Tree* tree = malloc(sizeof(B_Tree));
    tree->num_keys = 0;
    tree->num_nodes = 0;
    tree->root = NULL;
    return tree;
}

B_Tree_Key* b_tree_key_init(int key, bool is_tail)
{
    B_Tree_Key* new_key = malloc(sizeof(B_Tree_Key));
    new_key->key = key;
    new_key->child = NULL;
    new_key->next = NULL;
    new_key->prev = NULL;
    new_key->is_tail = is_tail;
    return new_key;
}

B_Tree_Node* b_tree_node_init(B_Tree_Key* key)
{
    // Does not know if the node is a leaf node or not, does NOT set the is_leaf field
    B_Tree_Node* node = malloc(sizeof(B_Tree_Node));
    node->num_keys = 1;
    node->head = key;
    node->tail = b_tree_key_init(0, true);

    key->next = node->tail;
    key->prev = NULL;
    node->tail->prev = key;
    return node;
}

void b_tree_node_insert(B_Tree_Node* node, B_Tree_Key* new_key)
{
    // ASSUMPTION node is not NULL and node->head is not NULL

    B_Tree_Key* prev_key = NULL;
    B_Tree_Key* curr_key = node->head;

    int key = new_key->key;
    
    while (!curr_key->is_tail)
    {
        if (curr_key->key > key)
        {
            new_key->next = curr_key;
            curr_key->prev = new_key;

            if (prev_key) 
            {
                prev_key->next = new_key;
                new_key->prev = prev_key;
            } else
            {
                node->head = new_key;
                new_key->prev = NULL;
            }
            break;
        }

        prev_key = curr_key;
        curr_key = curr_key->next;
    }

    if (curr_key->is_tail)
    {
        // got to the tail of the linked list without inserting
        prev_key->next = new_key;
        new_key->prev = prev_key;

        new_key->next = curr_key;
        curr_key->prev = new_key;
    }

    node->num_keys++;
}

B_Tree_Node* b_tree_node_split(B_Tree_Node* node, B_Tree_Key* parent_key, B_Tree_Node* parent_node, B_Tree* tree)
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
    left_split->tail = b_tree_key_init(0, true);
    // left_split->greater_than_child = NULL;
    left_split->num_keys = MAX_KEYS / 2;
    
    prev->next = left_split->tail;
    left_split->tail->prev = prev;

    node->head = median->next;      // right split
    node->head->prev = NULL;

    node->num_keys = node->num_keys - left_split->num_keys - 1;

    left_split->tail->child = median->child;

    // left_split->greater_than_child = median->child;
    median->child = left_split;
    

    if (!parent_key)
    {
        B_Tree_Node* new_root = b_tree_node_init(median);
        new_root->tail->child = node;
        new_root->is_leaf = false;
        tree->root = new_root;
        return new_root;
    } else {
        median->next = parent_key;
        median->prev = parent_key->prev;

        if (parent_key->prev)
        {
            parent_key->prev->next = median;
        } else
        {
            parent_node->head = median;
        }

        parent_key->prev = median;
        parent_node->num_keys++;

        return parent_node;
    }
}

B_Tree_Key_Node_Pair* b_tree_kn_pair_init(B_Tree_Node* n, B_Tree_Key* k)
{
    B_Tree_Key_Node_Pair* pair = malloc(sizeof(B_Tree_Key_Node_Pair));
    pair->node = n;
    pair->key = k;
    return pair;
}

void b_tree_insert(B_Tree* tree, int key)
{

    if (tree->root == NULL)
    {
        B_Tree_Key* new_key = b_tree_key_init(key, false);

        tree->root = b_tree_node_init(new_key);
        tree->root->is_leaf = true;

        tree->num_nodes = 1;
        tree->num_keys = 1;
        return;
    }

    B_Tree_Node* curr_node = tree->root;

    B_Tree_KN_Stack* stack = B_Tree_KN_Stack_init(10);

    while (!curr_node->is_leaf)
    {
        B_Tree_Key* curr_key = curr_node->head;
        while (!curr_key->is_tail)
        {
            if (curr_key->key == key) return;
            if (curr_key->key > key)
            {
                B_Tree_Key_Node_Pair* pair = b_tree_kn_pair_init(curr_node, curr_key);
                
                B_Tree_KN_Stack_append(stack, pair);
                curr_node = curr_key->child;
                break;
            }
            curr_key = curr_key->next;
        }

        if (curr_key->is_tail) 
        {
            B_Tree_Key_Node_Pair* pair = b_tree_kn_pair_init(curr_node, curr_key);
            curr_node = curr_key->child;
            B_Tree_KN_Stack_append(stack, pair);
        }
    }
    B_Tree_Key* new_key = b_tree_key_init(key, false);
    b_tree_node_insert(curr_node, new_key);
    
    while (curr_node->num_keys > MAX_KEYS)
    { 
        B_Tree_Key_Node_Pair* parent_pair = stack->size > 0 ? stack->data[stack->size - 1] : NULL;
        B_Tree_Node* parent_node = NULL;
        B_Tree_Key* parent_key = NULL;
        if (parent_pair)
        {
            parent_node = parent_pair->node;
            parent_key = parent_pair->key;
        }

        parent_node = b_tree_node_split(curr_node, parent_key, parent_node, tree);
        free(parent_pair);
        B_Tree_KN_Stack_pop(stack);
        curr_node = parent_node;
    }

    // freeing the remainder of the K-N stack
    while (stack->size > 0)
    {
        free(stack->data[stack->size - 1]);
        B_Tree_KN_Stack_pop(stack);
    }

    B_Tree_KN_Stack_free(stack);
}



void move_key_to_head(B_Tree_Node* node, B_Tree_Key* key)
{
    key->next = node->head;
    node->head->prev = key;

    key->prev = NULL;
    node->head = key;
    node->num_keys++;
}

void move_key_to_end(B_Tree_Node* node, B_Tree_Key* key)
{
    B_Tree_Key* before_tail = node->tail->prev;
    if (before_tail)
    {
        before_tail->next = key;
        key->prev = before_tail;
        key->next = node->tail;
        node->tail->prev = key;
    } else
    {
        key->prev = NULL;
        key->next = node->tail;
        node->tail->prev = key;
        node->head = key;
    }
    node->num_keys++;
}

void insert_key_before_key(B_Tree_Node* node, B_Tree_Key* key_to_insert, B_Tree_Key* insert_behind)
{
    B_Tree_Key* prev = insert_behind->prev;
    if (prev)
    {
        prev->next = key_to_insert->next;
        key_to_insert->prev = prev;
        key_to_insert->next = insert_behind;
        insert_behind->prev = key_to_insert;
    } else
    {
        key_to_insert->next = insert_behind;
        insert_behind->prev = key_to_insert;
        key_to_insert->prev = NULL;
        node->head = key_to_insert;
    }

    node->num_keys++;
}

void remove_key(B_Tree_Node* node, B_Tree_Key* key)
{
    B_Tree_Key* prev = key->prev;
    if (prev)
    {
        prev->next = key->next;
        key->next->prev = prev;
    } else
    {
        key->next->prev = NULL;
        node->head = key->next;
    }
    node->num_keys--;
}

B_Tree_Node* concantenate_node_to_key_to_node(B_Tree_Node* node1, B_Tree_Key* key, B_Tree_Node* node2)
{
    B_Tree_Key* node1_end = node1->tail->prev;
    free(node1->tail);
    node1_end->next = key;
    key->prev = node1_end;

    key->next = node2->head;
    node2->head->prev = key;

    node1->num_keys += (1 + node2->num_keys);
    free(node2);

    return node1;
}

void b_tree_delete_fixup(
    B_Tree_Node* deletion_node,
    B_Tree_Key* del_location,
    B_Tree_Node* parent_node,
    B_Tree_Key* parent_key,
    B_Tree_Node* child_node,

    B_Tree* tree
)
{
    if (deletion_node->is_leaf)
    {
        if (deletion_node == tree->root)
        {
            if (tree->root->num_keys == 0)
            {
                free(tree->root);
                tree->root = NULL;
            }
            return;
        }

        B_Tree_Node* sibling;
        B_Tree_Key* sibling_key;
        B_Tree_Key* separator;

        if (parent_key->is_tail)
        {
            // left sibling
            sibling = parent_key->prev->child;
            sibling_key = sibling->tail->prev;
            
            separator = parent_key->prev;
        } else
        {
            // right sibling
            sibling = parent_key->next->child;
            sibling_key = sibling->head;
            separator = parent_key;
        }


        if (sibling->num_keys >= MIN_KEYS + 1)
        {
            remove_key(sibling, sibling_key);

            if (parent_key->is_tail)
            {
                // left sibling
                insert_key_before_key(parent_node, sibling_key, separator);
                move_key_to_head(deletion_node, separator);
            } else
            {
                // right sibling
                insert_key_before_key(parent_node, sibling_key, separator->next);
                move_key_to_end(deletion_node, separator);
            }
            separator->child = NULL;
            sibling_key->child = deletion_node;
        } else
        {
            B_Tree_Node* merged_node;
            B_Tree_Key* after_separator = separator->next;
            remove_key(parent_node, separator);
            if (parent_key->is_tail)
            {
                // left sibling
                merged_node = concantenate_node_to_key_to_node(sibling, separator, deletion_node);
            } else
            {
                merged_node = concantenate_node_to_key_to_node(deletion_node, separator, sibling);
            }
            // after_separator->child = merged_node;
            separator->child = NULL;

            if (parent_node == tree->root && parent_node->num_keys == 0)
            {
                free(tree->root);
                tree->root = merged_node;
                return;
            }

            if (parent_node != tree->root && parent_node->num_keys < MIN_KEYS)
            {
                b_tree_delete_fixup(parent_node, after_separator, NULL, NULL, merged_node, tree);
            } else
            {
                after_separator->child = merged_node;
            }
        }
    } else
    {
        B_Tree_Node* left_child = child_node;
        B_Tree_Node* right_child = del_location->child;
        if (left_child->num_keys >= right_child->num_keys)
        {
            B_Tree_Key* new_parent = left_child->tail->prev;
            B_Tree_Node* new_parent_old_child = new_parent->child;
            remove_key(left_child, new_parent);
            insert_key_before_key(deletion_node, new_parent, del_location);
            new_parent->child = left_child;

            if (left_child->num_keys < MIN_KEYS)
                b_tree_delete_fixup(left_child, left_child->tail, deletion_node, new_parent, new_parent_old_child, tree);
        } else
        {
            B_Tree_Key* new_parent = right_child->head;
            B_Tree_Node* new_parent_old_child = new_parent->child;
            remove_key(right_child, new_parent);
            insert_key_before_key(deletion_node, new_parent, del_location);
            new_parent->child = left_child;

            if (right_child->num_keys < MIN_KEYS)
                b_tree_delete_fixup(right_child, right_child->head, deletion_node, new_parent->next, new_parent_old_child, tree);
        }

    }
}

void b_tree_delete(B_Tree* tree, int key)
{
    // traverse the tree
    // find the key
    // delete the key and store the parent node, the parent key and the right-adjacent key to the one we deleted

    if (!tree || !tree->root || !tree->root->head) return;
    B_Tree_Node* parent_node = NULL;
    B_Tree_Key* parent_key = NULL;
    
    B_Tree_Node* curr_node = tree->root;
    B_Tree_Key* curr_key = tree->root->head;

    while (curr_node)
    {
        if (curr_key->is_tail || curr_key->key > key) 
        {
            parent_node = curr_node;
            parent_key = curr_key;

            curr_node = curr_key->child;
            if (curr_node) curr_key = curr_node->head;
        } else if (curr_key->key == key) break;
        else
        {
            curr_key = curr_key->next;
        }
    }

    if (!curr_node) return;

    B_Tree_Key* del_location = curr_key->next;
    B_Tree_Node* child_node = curr_key->child;

    if (curr_key->prev)
    {
        curr_key->prev->next = curr_key->next;
        curr_key->next->prev = curr_key->prev;
    } else
    {
        curr_node->head = curr_key->next;
        curr_node->head->prev = NULL;
    }

    free(curr_key);
    curr_node->num_keys--;

    if (curr_node->num_keys < MIN_KEYS)
        b_tree_delete_fixup(curr_node, del_location, parent_node, parent_key, child_node, tree);
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

    free(node);
}

void b_tree_free(B_Tree* tree)
{
    b_tree_node_free(tree->root);
    free(tree);
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

    b_tree_delete(b_tree, 26);

    b_tree_delete(b_tree, 14);


    b_tree_insert(b_tree, 7);
    b_tree_insert(b_tree, 31);
    b_tree_insert(b_tree, 1);
    b_tree_insert(b_tree, 10);

    b_tree_insert(b_tree, 22);
    b_tree_insert(b_tree, 23);


    b_tree_delete(b_tree, 2);
    b_tree_insert(b_tree, 24);

    b_tree_insert(b_tree, 8);
    b_tree_insert(b_tree, 12);

    b_tree_insert(b_tree, 13);

    print_tree(b_tree);


    b_tree_free(b_tree);
    
    return 0;
}