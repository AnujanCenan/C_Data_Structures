#include "b-tree.h"

DECLARE_DYNAMIC_ARRAY(B_Tree_Key_Node_Pair*, B_Tree_KN_Stack);

struct B_Tree_Key_Node_Pair
{
    B_Tree_Key* key;
    B_Tree_Node* node;
};


/**
 * Creates a key for the b-tree. Requires a key value and the is_tail value. Note
 * that if is_tail == true, then the value of key is not important for this
 * implementation
 */
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

/**
 * Creates a tree node for the b-tree. Requires a key because a newly created node
 * should have at least one (non-tail) key in it. Also creates the requried tail
 * key. Parameter is_leaf determines whether the node is currently being used as 
 * leaf node or a parent node.
 */
B_Tree_Node* b_tree_node_init(B_Tree_Key* key, bool is_leaf)
{
    // Does not know if the node is a leaf node or not, does NOT set the is_leaf field
    B_Tree_Node* node = malloc(sizeof(B_Tree_Node));
    node->num_keys = 1;
    node->head = key;
    node->tail = b_tree_key_init(0, true);
    node->is_leaf = is_leaf;
    
    key->next = node->tail;
    key->prev = NULL;
    node->tail->prev = key;
    return node;
}


/**
 * Initialises an instsance of the B_Tree_Key_Node_Pair struct. For insertion,
 * these pairs are used to keep track of the traversal path (parent node and 
 * parent key) towards the leaf node where the new key will be placed.
 */
B_Tree_Key_Node_Pair* b_tree_kn_pair_init(B_Tree_Node* n, B_Tree_Key* k)
{
    B_Tree_Key_Node_Pair* pair = malloc(sizeof(B_Tree_Key_Node_Pair));
    pair->node = n;
    pair->key = k;
    return pair;
}


/**
 * Given a (new) key and a node, inserts the new key, maintaining the ascending
 * order of the node. Requires a (linear) scan of the node's linked list to place
 * the new key in the appropriate spot.
 */
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

/**
 * Respoonsible for splitting a node when an insertion operation causes a node to 
 * become too full. Specifically, the inputted node is transformed in place to become
 * the right-split node, while a new node structure is created to take the role as
 * the left-split node. As per the b-tree insertion algorithm, the median of the 
 * inputted node is pushed up to the parent node, using the parent key as an
 * indication as to where it should go. Finally, if the splitting operation causes
 * a new root to be formed, this is updated in the B_Tree structure.
 */
B_Tree_Node* b_tree_node_split(B_Tree_Node* node, B_Tree_Key* parent_key, B_Tree_Node* parent_node, B_Tree* tree)
{
    // Node can be a leaf node OR a parent node, we don't know, we are just splitting
    // Assumption is that node->num_keys == MAX_KEYS + 1
    int keys_left_side = tree->min_keys;

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

    left_split->num_keys = keys_left_side;
    
    prev->next = left_split->tail;
    left_split->tail->prev = prev;

    node->head = median->next;      // right split
    node->head->prev = NULL;

    node->num_keys = node->num_keys - left_split->num_keys - 1;

    left_split->tail->child = median->child;

    median->child = left_split;
    

    if (!parent_key)
    {
        B_Tree_Node* new_root = b_tree_node_init(median, false);
        new_root->tail->child = node;
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

void b_tree_insert(B_Tree* tree, int key)
{

    if (tree->root == NULL)
    {
        B_Tree_Key* new_key = b_tree_key_init(key, false);

        tree->root = b_tree_node_init(new_key, true);
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
    
    while (curr_node->num_keys > tree->max_keys)
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