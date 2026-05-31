#include "b-tree.h"

/**
 * Initialises the tree. Requires a max_keys to know the capacity of each
 * node. Note that the minimum number of keys a node can have is equal to 
 * floor( max_keys / 2 )
 */
B_Tree* b_tree_init(int max_keys)
{
    B_Tree* tree = malloc(sizeof(B_Tree));
    tree->root = NULL;
    tree->max_keys = max_keys;
    tree->min_keys = max_keys / 2;
    return tree;
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

/**
 * For file-directory style printing of the tree. Based on given generation integer
 * the size of the indent is decided.
 */
void print_indent(int generation)
{
    for (int i = 0; i < generation; ++i)
    {
        printf(" ");
    }
    printf("Gen %d: ", generation);
}

/**
 * Prints the subtree with given node as the root. Recursive nature allows for
 * an entire tree to be printed starting at root.
 */
void b_tree_node_print(B_Tree_Node* node, int generation)
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
        b_tree_node_print(curr->child, generation + 1);
        curr = curr->next;
    }
}


/**
 * Prints the tree in file-directory style. Uses indents and generation number.
 */
void b_tree_print(B_Tree* tree)
{
    int generation = 0;
    b_tree_node_print(tree->root, generation);

}