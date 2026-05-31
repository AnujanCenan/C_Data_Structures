#include "b-tree.h"

int main()
{

    B_Tree* b_tree = b_tree_init(4);        // setting max_keys to be 4

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

    b_tree_print(b_tree);


    b_tree_free(b_tree);
    
    return 0;
}