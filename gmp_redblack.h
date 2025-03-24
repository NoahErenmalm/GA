#ifndef GMP_REDBLACK_H
#define GMP_REDBLACK_H

#include <gmp.h>

#define RED 1
#define BLACK 0

typedef struct rb_node_struct{
    mpz_t value;
    int color;
    struct rb_node_struct *parent, *left, *right;
}rb_node;

typedef struct {
    rb_node *root; //root
    rb_node *leaf; //sentinel null pointer, för att jag hörde att det skulle vara enklare för programmet?
}rb_tree;

// funciton prototypes
void initializeLeafNode(rb_tree *tree);
void initializeTree(rb_tree *tree, mpz_t value);
rb_node *createNode(rb_tree *tree, mpz_t value);
void insertNode(rb_tree *tree, mpz_t value);
void rotateRight(rb_tree *tree, rb_node *origin);
void rotateLeft(rb_tree *tree, rb_node *origin);
void insertFixup(rb_tree *tree, rb_node *current_node);
rb_node *searchNode(rb_tree *tree, mpz_t value);
void deleteMinimum(rb_tree *tree);
void deleteMinFixup(rb_tree *tree, rb_node *node);
void CleanupTree(rb_tree *tree);



#endif