#ifndef HASH_H
#define HASH_H

#define HASH_SIZE 24593

typedef struct HashNode {
    char* key;
    void* data;
    struct HashNode* next;
} HashNode;

typedef struct HashMap {
    struct HashNode* nodes[HASH_SIZE];
} HashMap;

unsigned int hash(const char* str);
HashMap* hashmap_new();
int hashmap_insert(HashMap* map, const char* key, void* data);
int hashmap_delete(HashMap* map, const char* key);
HashNode* hashmap_node(HashMap* map, const char* key);
void* hashmap_value(HashMap* map, const char* key);

#endif