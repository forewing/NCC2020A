#ifndef HASH_H
#define HASH_H

#include <stdlib.h>
#include <string.h>

#include "helper.h"
#include "symbol.h"

#define HASH_SIZE 24593

typedef struct HashNode {
    char* key;
    SymNode* data;
    struct HashNode* next;
} HashNode;

typedef struct HashMap {
    HashNode* nodes[HASH_SIZE];
    struct HashMap* next;
} HashMap;

unsigned int hash(const char* str);
HashMap* hashmap_new();
void hashmap_free(HashMap* map);
int hashmap_insert(HashMap* map, const char* key, SymNode* data);
int hashmap_delete(HashMap* map, const char* key);
HashNode* hashmap_node(HashMap* map, const char* key);
SymNode* hashmap_value(HashMap* map, const char* key);
void hashmap_print(HashMap* map);

#define HASHNODE_FREE(__NODE__)    \
    {                              \
        FREE(__NODE__->key);       \
        type_free(__NODE__->data); \
        FREE(__NODE__);            \
    }

#endif