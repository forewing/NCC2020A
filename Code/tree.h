#ifndef TREE_H
#define TREE_H

enum { NODE_TERM, NODE_NOTERM, NODE_EMPTY };
enum { TYPENAME_INT, TYPENAME_FLOAT };
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
    int node_type;
    int lex_type;
    union {
        long data_int;
        float data_float;
        char* data_str;
    };
    struct TreeNode* parent;
    struct TreeNode** children;
} TreeNode;

TreeNode* tree_new(const char* name, int size);

void tree_set_children(TreeNode* node, ...);

extern TreeNode* tree_root;

void print_syntax_tree();

#endif