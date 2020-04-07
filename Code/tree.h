#ifndef TREE_H
#define TREE_H

#include <stdarg.h>
#include <stdlib.h>

#include "helper.h"
#include "state.h"

enum NODE_TYPE { NODE_TERM, NODE_NOTERM, NODE_EMPTY };
enum TYPENAME { TYPENAME_INT, TYPENAME_FLOAT };
enum RELOP_TYPE {
    RELOP_GT,
    RELOP_GE,
    RELOP_LT,
    RELOP_LE,
    RELOP_EQ,
    RELOP_NE,
};

typedef struct TreeNode {
    struct TreeNode* parent;
    struct TreeNode** children;
    const char* name;
    union {
        long data_int;
        float data_float;
        char* data_str;
    };

    int size;
    int lineno;

    enum NODE_TYPE node_type;
    enum STATE state_type;
} TreeNode;

TreeNode* tree_new(const char* name, int size);

void tree_set_children(TreeNode* node, ...);

extern TreeNode* tree_root;

void print_syntax_tree();

#endif