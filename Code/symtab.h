#ifndef SYMTAB_H
#define SYMTAB_H

#include "hash.h"

extern HashMap* symtab;
extern HashMap* symtab_root;

SymNode* symtab_lookup(HashMap* tab, const char* key);
SymNode* symtab_lookup_root(const char* key);
SymNode* symtab_lookup_last(const char* key);
SymNode* symtab_lookup_all(const char* key);

int symtab_insert(HashMap* tab, const char* key, SymNode* data);
int symtab_insert_root(const char* key, SymNode* data);
int symtab_insert_last(const char* key, SymNode* data);

#endif