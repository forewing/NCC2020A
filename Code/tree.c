#include "tree.h"
#include <stdarg.h>
#include <stdlib.h>

TreeNode* tree_new(const char* name, int size) {
    TreeNode* ret = (TreeNode*)malloc(sizeof(TreeNode));

    ret->name = name;

    if (size > 0) {
        ret->size = size;
        ret->children = (TreeNode**)malloc(sizeof(TreeNode) * size);
    } else {
        ret->size = 0;
        ret->children = NULL;
    }

    return ret;
}

void tree_set_children(TreeNode* node, ...) {
    va_list children;
    va_start(children, node);
    for (int i = 0; i < node->size; i++) {
        TreeNode* child = va_arg(children, TreeNode*);
        node->children[i] = child;
        child->parent = node;
    }
    va_end(children);
}
