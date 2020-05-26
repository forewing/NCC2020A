#include <stdio.h>
#include <string.h>

#include "ir.h"
#include "mips.h"
#include "state.h"
#include "syntax.tab.h"
#include "tree.h"
#include "yy.h"

int bug_number = 0;
int total_lines = 0;

int main(int argc, char** argv) {
    if (argc > 1) {
        if (!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
    } else {
        return 1;
    }
    yyparse();
    // print_syntax_tree();
    symtab_build();

    if (bug_number > 0) {
        fprintf(stderr, "Unable to translate\n");
        return -1;
    }

    // ircode_opt(ircode_list);

    if (argc > 2) {
        FILE* fp = fopen(argv[2], "w");
        if (!fp) {
            perror(argv[2]);
            return 1;
        }
        // IrCode_print(fp, ircode_list);
        mips_print(fp, ircode_list);
    } else {
        // IrCode_print(stdout, ircode_list);
        mips_print(stdout, ircode_list);
    }

    return 0;
}
