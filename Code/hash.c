#include "hash.h"
#include <stdlib.h>
#include <string.h>
#include "helper.h"

unsigned int hash(const char* str) {
    unsigned int hash = 5381;
    int c;
    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash % HASH_SIZE;
}

HashMap* hashmap_new() {
    HashMap* ret = (HashMap*)malloc(sizeof(HashMap));
    for (int i = 0; i < HASH_SIZE; i++)
        ret->nodes[i] = NULL;
    return ret;
}

int hashmap_insert(HashMap* map, const char* key, void* data) {
    unsigned int pos = hash(key);

    HashNode* newNode = (HashNode*)malloc(sizeof(HashNode));
    newNode->data = data;
    newNode->key = strdup2(key);
    newNode->next = map->nodes[pos];
    map->nodes[pos] = newNode;

    return 0;
}

HashNode* hashmap_node(HashMap* map, const char* key) {
    unsigned int pos = hash(key);
    HashNode* ptr = map->nodes[pos];
    while (ptr) {
        if (!strcmp(key, ptr->key))
            return ptr;
        ptr = ptr->next;
    }
    return NULL;
}

void* hashmap_value(HashMap* map, const char* key) {
    HashNode* node = hashmap_node(map, key);
    if (node == NULL)
        return NULL;
    return node->data;
}

#define HASHNODE_FREE(__NODE__) \
    {                           \
        FREE(__NODE__->key);    \
        FREE(__NODE__->data);   \
    }

int hashmap_delete(HashMap* map, const char* key) {
    unsigned int pos = hash(key);
    HashNode* ptr = map->nodes[pos];

    if (ptr == NULL)
        return -1;

    if (!strcmp(key, ptr->key)) {
        map->nodes[pos] = ptr->next;
        HASHNODE_FREE(ptr);
        return 0;
    }

    while (ptr->next) {
        if (!strcmp(key, ptr->next->key)) {
            HashNode* tmp = ptr->next;
            ptr->next = tmp->next;
            HASHNODE_FREE(tmp);
            return 0;
        }
        ptr = ptr->next;
    }

    return -1;
}
