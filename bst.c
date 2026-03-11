#include <stdlib.h>
#include "bst.h"
BST* createBST(int (*cmp)(void*, void*), void (*print)(void*), void (*freeData)(void*)) {
    //creates a pointer to the first Node in the tree.
    BST* head = malloc(sizeof(BST));
    head->root = NULL;
    head->compare = cmp;
    head->print = print;
    head->freeData = freeData;
    return head;
}

void bstInsert(BSTNode** root, void* data, int (*cmp)(void*, void*)) {
    //adding a new node to the tree.
    if (!(*root)) {
        *root = malloc(sizeof(BSTNode));
        (*root)->data = data;
        (*root)->left = NULL;
        (*root)->right = NULL;
        return;
    }
    if ((*cmp)(data, (*root)->data) < 0) {
        bstInsert(&(*root)->left, data, cmp);
    }
    if ((*cmp)(data, (*root)->data) > 0) {
        bstInsert(&(*root)->right, data, cmp);
    }
}

void* bstFind(BSTNode* root, void* data, int (*cmp)(void*, void*)) {
    //find specific node by his data.
    if (!root)
        return NULL;
    if ((*cmp)(root->data, data) == 0) {
        return root->data;
    }
    if ((*cmp)(data, root->data) < 0) {
        return bstFind(root->left, data, cmp);
    }
    if ((*cmp)(data, root->data) > 0) {
        return bstFind(root->right, data, cmp);
    }
    return NULL;
}

void bstFree(BSTNode* root, void (*freeData)(void*)) {
    //free all the nodes recursively using post-order method.
    if (root->left) {
        bstFree(root->left, freeData);
    }
    if (root->right) {
        bstFree(root->right, freeData);
    }
    freeData(root->data);
    free(root);
}

void bstPreorder(BSTNode* root, void (*print)(void*)) {
    //Node first and then the left and the right children.
    if (!root)
        return;
    print(root->data);
    if (root->left) {
        bstPreorder(root->left, print);
    }
    if (root->right) {
        bstPreorder(root->right, print);
    }
}

void bstInorder(BSTNode* root, void (*print)(void*)) {
    //first the left child. then the root and the right child in the last.
    if (!root)
        return;
    if (root->left) {
        bstInorder(root->left, print);
    }
    print(root->data);
    if (root->right) {
        bstInorder(root->right, print);
    }
}

void bstPostorder(BSTNode* root, void (*print)(void*)) {
    //the left and right children first and then the root.
    if (!root)
        return;
    if (root->left) {
        bstPostorder(root->left, print);
    }
    if (root->right) {
        bstPostorder(root->right, print);
    }
    print(root->data);
}