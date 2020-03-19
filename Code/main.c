#include <stdio.h>
#include <string.h>
#include "syntax.tab.h"
#include "tree.h"
#include "yy.h"

void print_syntax_tree(TreeNode* root, int tabs);

int bug_number = 0;
int total_lines = 0;

int main(int argc, char** argv) {
    if (argc > 1) {
        if (!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
    }
    yyparse();
    if (bug_number == 0) {
        if (tree_root->children[0]->size == 0) {
            tree_root->lineno = total_lines;
        }
        print_syntax_tree(tree_root, 0);
    }
    return 0;
}

void print_syntax_tree(TreeNode* root, int tabs) {
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
            print_syntax_tree(root->children[i], tabs + 2);
        }
    } else if (root->lex_type == ID) {
        printf("ID: %s\n", root->data_str);
    } else if (root->lex_type == TYPE) {
        printf("TYPE: %s\n", root->data_int == TYPE_FLOAT ? "float" : "int");
    } else if (root->lex_type == INT) {
        printf("INT: %ld\n", root->data_int);
    } else if (root->lex_type == FLOAT) {
        printf("FLOAT: %f\n", root->data_float);
    } else {
        printf("%s\n", root->name);
    }
}
