#ifndef HASH_H
#define HASH_H

#include "type.h"

#define HASH_SIZE 24593

typedef struct HashNode {
    char* key;
    int age;
    TypeNode* data;
    struct HashNode* next;
} HashNode;

typedef struct HashMap {
    struct HashNode* nodes[HASH_SIZE];
} HashMap;

unsigned int hash(const char* str);
HashMap* hashmap_new();
int hashmap_insert(HashMap* map, const char* key, int age, TypeNode* data);
int hashmap_delete(HashMap* map, const char* key, int age);
HashNode* hashmap_node(HashMap* map, const char* key, int age);
TypeNode* hashmap_value(HashMap* map, const char* key, int age);

#endif