#include <stdio.h>
#include <string.h>
#include "syntax.tab.h"
#include "tree.h"
#include "yy.h"

void tree_traverse(TreeNode* root, int tabs);

int main(int argc, char** argv) {
    if (argc > 1) {
        if (!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
    }
    yyparse();
    tree_traverse(tree_root, 0);
    return 0;
}

void tree_traverse(TreeNode* root, int tabs) {
    for (int i = 0; i < tabs; i++) {
        putchar(' ');
    }
    if (root->type == STATE_NOTERM) {
        printf("%s (%d)\n", root->name, root->lineno);
        for (int i = 0; i < root->size; i++) {
            tree_traverse(root->children[i], tabs + 2);
        }
    } else if (root->type == ID) {
        printf("ID: %s\n", root->data_str);
    } else if (root->type == TYPE) {
        printf("TYPE: %s\n", root->data_int == TYPE_FLOAT ? "float" : "int");
    } else if (root->type == INT) {
        printf("INT: %d\n", root->data_int);
    } else if (root->type == FLOAT) {
        printf("FLOAT: %f\n", root->data_float);
    } else {
        printf("%s\n", root->name);
    }
}
