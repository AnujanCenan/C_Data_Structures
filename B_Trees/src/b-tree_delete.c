#include "b-tree.h"


/**
 * Given an unowned key, positions the key to become the head of the given node
 * Updates the num_keys of the node accordingly.
 */
void move_key_to_head(B_Tree_Node* node, B_Tree_Key* key)
{
    key->next = node->head;
    node->head->prev = key;

    key->prev = NULL;
    node->head = key;
    node->num_keys++;
}

/**
 * Given an unowned key, positions the key to the end of a node. If this happens to
 * make this key the front of the node, the node head ptr is adjusted accordingly.
 * 
 */
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


/**
 * General insertion function for an unowned key. Positions a key (key_to_insert)
 * such that it is right before the insert_behind key. Assumes that insert_behind
 * is a legitimate key (tail or non-tail) found in node. Increases the num_keys of
 * node accordingly.
 */
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

/**
 * Removes the given key from the given node. Assumes the key is located in the
 * given node. Adjusts node's num_keys accordingly. Does not provide the key with
 * a new owner node. (Use insert_key_before_key to provide the key with a new owner)
 */
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

/**
 * Highly specific function for the delete operation. In the context where two 
 * sibling leaf nodes need to be concatenated together due to a deletion operation,
 * this function is capable of doing just that. Note that the input key should be
 * the parent separator key that separates the two siblings.
 */
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

/**
 * The fixup function which ensures the properties of the b-tree are maintained
 * after a key is deleted. Should be called whenever a non-root node's num_keys 
 * falls below the tree->min_keys value.
 */
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


        if (sibling->num_keys >= tree->min_keys + 1)
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

            if (parent_node != tree->root && parent_node->num_keys < tree->min_keys)
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

            if (left_child->num_keys < tree->min_keys)
                b_tree_delete_fixup(left_child, left_child->tail, deletion_node, new_parent, new_parent_old_child, tree);
        } else
        {
            B_Tree_Key* new_parent = right_child->head;
            B_Tree_Node* new_parent_old_child = new_parent->child;
            remove_key(right_child, new_parent);
            insert_key_before_key(deletion_node, new_parent, del_location);
            new_parent->child = left_child;

            if (right_child->num_keys < tree->min_keys)
                b_tree_delete_fixup(right_child, right_child->head, deletion_node, new_parent->next, new_parent_old_child, tree);
        }
    }
}

/**
 * Delets a key from the tree
 */
void b_tree_delete(B_Tree* tree, int key)
{
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

    if (curr_node->num_keys < tree->min_keys)
        b_tree_delete_fixup(curr_node, del_location, parent_node, parent_key, child_node, tree);
}

