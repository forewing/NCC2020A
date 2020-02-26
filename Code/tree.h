#ifndef TREE_H
#define TREE_H

enum { STATE_TERM, STATE_NOTERM, STATE_EMPTY };
enum { TYPE_INT, TYPE_FLOAT };
enum {
    RELOP_GT,
    RELOP_GE,
    RELOP_LT,
    RELOP_LE,
    RELOP_EQ,
    RELOP_NE,
};

typedef struct TreeNode {
    const char* name;
    int size;

    int lineno;
    int type;
    union {
        int data_int;
        float data_float;
        char* data_str;
    };
    struct TreeNode* parent;
    struct TreeNode** children;
} TreeNode;

TreeNode* tree_new(const char* name, int size);

void tree_set_children(TreeNode* node, ...);

extern TreeNode* tree_root;

#endif