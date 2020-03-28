#include <stdio.h>
#include <string.h>
#include "hash.h"
#include "symbol.h"
#include "syntax.tab.h"
#include "tree.h"
#include "type.h"
#include "yy.h"

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
    // print_syntax_tree();
    symtab_build();
    hashmap_print(symtab);
    printf("%d\n", typeEqual(hashmap_value(symtab, "a", -1),
                             hashmap_value(symtab, "y", -1)));
    return 0;
}
