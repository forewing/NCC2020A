#include "symtab.h"

HashMap* symtab;
HashMap* symtab_root;

SymNode* symtab_lookup(HashMap* tab, const char* key) {
    if (tab)
        return hashmap_value(tab, key);
    return NULL;
}

SymNode* symtab_lookup_root(const char* key) {
    return symtab_lookup(symtab_root, key);
}

SymNode* symtab_lookup_last(const char* key) {
    return symtab_lookup(symtab, key);
}

SymNode* symtab_lookup_all(const char* key) {
    HashMap* ptr = symtab;
    while (ptr) {
        SymNode* tmp = symtab_lookup(ptr, key);
        if (tmp)
            return tmp;
        ptr = ptr->next;
    }
    return NULL;
}

int symtab_insert(HashMap* tab, const char* key, SymNode* data) {
    return hashmap_insert(tab, key, data);
}

int symtab_insert_root(const char* key, SymNode* data) {
    return symtab_insert(symtab_root, key, data);
}

int symtab_insert_last(const char* key, SymNode* data) {
    return symtab_insert(symtab, key, data);
}
