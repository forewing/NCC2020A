#include "tree.h"
#include <stdarg.h>
#include <stdlib.h>
#include "helper.h"
#include "syntax.tab.h"

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

static void _print_syntax_tree(TreeNode* root, int tabs) {
    if (root->node_type == NODE_EMPTY) {
        // Just pass
        return;
    }
    for (int i = 0; i < tabs; i++) {
        putchar(' ');
    }
    if (root->node_type == NODE_NOTERM) {
        printf("%s (%d)\n", root->name, root->lineno);
        for (int i = 0; i < root->size; i++) {
            _print_syntax_tree(root->children[i], tabs + 2);
        }
    } else if (root->lex_type == ID) {
        printf("ID: %s\n", root->data_str);
    } else if (root->lex_type == TYPE) {
        printf("TYPE: %s\n",
               root->data_int == TYPENAME_FLOAT ? "float" : "int");
    } else if (root->lex_type == INT) {
        printf("INT: %ld\n", root->data_int);
    } else if (root->lex_type == FLOAT) {
        printf("FLOAT: %f\n", root->data_float);
    } else {
        printf("%s\n", root->name);
    }
}

void print_syntax_tree() {
    if (bug_number == 0) {
        if (tree_root->children[0]->size == 0) {
            tree_root->lineno = total_lines;
        }
        _print_syntax_tree(tree_root, 0);
    }
}