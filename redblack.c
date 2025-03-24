#include <gmp.h>
#include <stdlib.h>
#include "gmp_redblack.h"

#include <stdio.h>

void initializeLeafNode(rb_tree *tree) {
    tree->leaf = (rb_node*)malloc(sizeof(rb_node));
    mpz_init(tree->leaf->value);            // just in case because mpz_t is a weird data type/struct or however it's built
    tree->leaf->color = BLACK;              // sentinel leaf node is always BLACK
    tree->leaf->left = tree->leaf;          // points to itself
    tree->leaf->right = tree->leaf;         // points to itself
    tree->leaf->parent = tree->leaf;        // points to itself
}

rb_node *createNode(rb_tree *tree, mpz_t value) {
    rb_node *new_node = (rb_node*)malloc(sizeof(rb_node));
    mpz_init_set(new_node->value, value);   // set the node value
    new_node->color = RED;                // all new nodes are red
    new_node->left = tree->leaf;            // new nodes children will be leafs at first
    new_node->right = tree->leaf;
    new_node->parent = tree->leaf;          // no parent yet
    return new_node;
}

void initializeTree(rb_tree *tree, mpz_t value) { // require a root node to initalize
    initializeLeafNode(tree);
    tree->root = createNode(tree, value); // root node
}


void insertNode(rb_tree *tree, mpz_t value) {
    
    rb_node *new_node = createNode(tree, value);
    rb_node *new_node_place = tree->root;
    rb_node *parent = tree->leaf; 

    while (new_node_place != tree->leaf) {
        parent = new_node_place;
        if (mpz_cmp(value, new_node_place->value) < 0) {
            new_node_place = new_node_place->left; 
        } else {
            new_node_place = new_node_place->right;
        }
    }
    new_node->parent = parent;

    if (mpz_cmp(value, parent->value) < 0) {
        parent->left = new_node;
    }
    else {
        parent->right = new_node;
    }

    insertFixup(tree, new_node);
}

void rotateRight(rb_tree *tree, rb_node *origin) {
    if (origin == tree->leaf || origin->left == tree->leaf) return;
    rb_node *tmp = origin->left; // tmp is origins left child
    origin->left = tmp->right; // origin's left child is tmp's right child

    if (tmp->right != tree->leaf) { // unless tmp' r child is leaf
        tmp->right->parent = origin; // the parent of tmp's r child is origin
    }

    tmp->parent = origin->parent; // tmp's parent is origin's parent

    if (origin->parent == tree->leaf) {//origin was root
        tree->root = tmp;
    }else if (origin == origin->parent->left) { // origin was the left child
        origin->parent->left = tmp;
    }else {// origin is the right child
        origin->parent->right = tmp;
    }

    tmp->right = origin; //origin is the child of tmp
    origin->parent = tmp;
}
void rotateLeft(rb_tree *tree, rb_node *origin) {
    if (origin == tree->leaf || origin->right == tree->leaf) return;

    rb_node *tmp = origin->right;
    origin->right = tmp->left;

    if(tmp->left != tree->leaf) {
        tmp->left->parent = origin;
    }

    tmp->parent = origin->parent;

    if (origin->parent == tree->leaf) {
        tree->root = tmp;
    }else if (origin == origin->parent->left) {
        origin->parent->left = tmp;
    }else {
        origin->parent->right = tmp;
    }

    tmp->left = origin;
    origin->parent = tmp;
}

void insertFixup(rb_tree *tree, rb_node *current_node) {
    rb_node *parent, *uncle, *grandparent;

    while (current_node != tree->root && current_node->parent->color == RED) {
        parent = current_node->parent;
        grandparent = parent->parent;

        if (parent == grandparent->left) { // parent is left child
            uncle = grandparent->right;

            if (uncle->color == RED && uncle != tree->leaf) { // CASE 1: Uncle is red
                parent->color = BLACK;
                uncle->color = BLACK;
                grandparent->color = RED;
                current_node = grandparent;
            } 
            else {
                if (current_node == parent->right) { // CASE 2: Triangle formation
                    current_node = parent;
                    rotateLeft(tree, current_node);
                }
                
                // CASE 3: Line formation
                parent->color = BLACK; 
                grandparent->color = RED;
                rotateRight(tree, grandparent);
            }
        }
        else { // parent is right child of grandparent
            uncle = grandparent->left;

            if (uncle->color == RED && uncle != tree->leaf) { //CASE 1: Uncle is red
                parent->color = BLACK;
                uncle->color = BLACK;
                grandparent->color = RED;
                current_node = grandparent;
            }
            else {
                if(current_node == parent->left) { //CASE 2: Triangle formation
                    current_node = parent;
                    rotateRight(tree, current_node);
                }
                
                //CASE 3: line formation
                parent->color = BLACK;
                grandparent->color = RED;
                rotateLeft(tree, grandparent);
            }
        }
    }
    tree->root->color = BLACK;
}

rb_node *searchNode(rb_tree *tree, mpz_t value) {
    rb_node *current = tree->root;

    while(current != tree->leaf) {
        int comparison = mpz_cmp(value, current->value);

        if (comparison == 0) {
            return current;
        }
        else if (comparison < 0) {
            current = current->left;
        }
        else {
            current = current->right;
        }
    }
    return NULL;
}

void deleteMinimum(rb_tree *tree) {
    
    rb_node *minNode = tree->root;
    while (minNode->left != tree->leaf) {
        minNode = minNode->left;
    }

    rb_node *child = minNode->right; // possible right child or just leaf node
    rb_node *parent = minNode->parent;
    int originalColor = minNode->color;

    if (parent == tree->leaf) {
        tree->root = child; // was root
    }
    else {
        parent->left = child;
    }
    
    if (child != tree->leaf) { // don't change leaf node's parent
        child->parent = parent;
    }

    mpz_clear(minNode->value);
    free(minNode);

    if (originalColor == BLACK && child != tree->leaf) {
        deleteMinFixup(tree, child);
    }    
}

void deleteMinFixup(rb_tree *tree, rb_node *node) {
    rb_node *sibling = node->parent->right;

    while (node != tree->root && node->color == BLACK) {
        // sibling is red
        if (sibling->color == RED) {
            sibling->color = BLACK;
            node->parent->color = RED;
            rotateLeft(tree, node->parent);
            sibling = node->parent->right;
        }

        // sibling is black, and sibling children are black
        if (sibling->left->color == BLACK && sibling->right->color == BLACK) {
            sibling->color == RED;
            node = node->parent;
        } 
        else {
            // sibling right child is black and left child is red
            if (sibling->right->color == BLACK) {
                sibling->left->color == BLACK;
                sibling->color = RED;
                rotateRight(tree, sibling);
                sibling = node->parent->right;
            }

            // sibling right child is red
            sibling->color = node->parent->color;
            node->parent->color = BLACK;
            sibling->right->color = BLACK;
            rotateLeft(tree, node->parent);
            node = tree->root;

        }
    }
    node->color = BLACK;
}

void CleanupTree(rb_tree *tree) {
    if (tree->root == tree->leaf)
        return;

    rb_node **stack = malloc(1000 * sizeof(rb_node*)); // stack size
    int stack_capacity = 1000;
    int stack_size = 0;

    //node onto stack
    stack[stack_size++] = tree->root;

    while (stack_size > 0) {
        // node from stack
        rb_node *node = stack[--stack_size];

        //push child node onto stack if not leaf
        if (node->left != tree->leaf) {
            if (stack_size >= stack_capacity) {
                stack_capacity *= 2;
                stack = realloc(stack, stack_capacity * sizeof(rb_node*));
            }
            stack[stack_size++] = node->left;
        }
        if(node->right != tree->leaf) {
            if (stack_size >= stack_capacity) {
                stack_capacity *= 2;
                stack = realloc(stack, stack_capacity * sizeof(rb_node*));
            }
            stack[stack_size++] = node->right;
        }

        mpz_clear(node->value);
        free(node);
    }
    free(stack);
    free(tree->leaf);
    tree->root = tree->leaf = NULL;
} // glöm inte tree struct sen