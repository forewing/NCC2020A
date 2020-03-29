#include "hash.h"
#include <stdlib.h>
#include <string.h>
#include "helper.h"
#include "type.h"

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

int hashmap_insert(HashMap* map, const char* key, int age, TypeNode* data) {
    if (age < 0)
        return -1;  // Only allow insert when age >= 0

    if (hashmap_node(map, key, age) != NULL)
        return -1;  // Duplicated

    unsigned int pos = hash(key);

    HashNode* newNode = (HashNode*)malloc(sizeof(HashNode));
    newNode->data = data;
    // newNode->data = (TypeNode*)malloc(sizeof(TypeNode));
    // memcpy(newNode->data, data, sizeof(TypeNode));
    newNode->key = strdup2(key);
    newNode->age = age;
    newNode->next = map->nodes[pos];
    map->nodes[pos] = newNode;

    return 0;
}

HashNode* hashmap_node(HashMap* map, const char* key, int age) {
    unsigned int pos = hash(key);
    HashNode* ptr = map->nodes[pos];

    if (age == -1) {
        int age_max = -1;
        HashNode* ret = NULL;
        while (ptr) {
            if (!strcmp(key, ptr->key) && (ptr->age > age_max)) {
                age_max = ptr->age;
                ret = ptr;
            }
            ptr = ptr->next;
        }
        return ret;
    } else {
        while (ptr) {
            if (!strcmp(key, ptr->key) && ptr->age == age)
                return ptr;
            ptr = ptr->next;
        }
    }

    return NULL;
}

TypeNode* hashmap_value(HashMap* map, const char* key, int age) {
    HashNode* node = hashmap_node(map, key, age);
    if (node == NULL)
        return NULL;
    return node->data;
}

#define HASHNODE_FREE(__NODE__)    \
    {                              \
        FREE(__NODE__->key);       \
        type_free(__NODE__->data); \
        FREE(__NODE__);            \
    }

int hashmap_delete(HashMap* map, const char* key, int age) {
    if (age < 0)
        return -1;  // Only allow delete with age

    unsigned int pos = hash(key);
    HashNode* ptr = map->nodes[pos];

    HashNode* node = hashmap_node(map, key, age);
    if (node == NULL)
        return -1;

    if (node == ptr) {
        map->nodes[pos] = ptr->next;
        HASHNODE_FREE(node);
        return 0;
    }

    while (ptr->next) {
        if (ptr->next == node) {
            ptr->next = node->next;
            HASHNODE_FREE(node);
            return 0;
        }
        ptr = ptr->next;
    }

    return -1;
}

int hashmap_delete_age(HashMap* map, int age) {
    if (age < 0)
        return -1;
    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode* ptr = map->nodes[i];
        while (ptr) {
            if (ptr->age == age) {
                HashNode* tmp = ptr->next;
                hashmap_delete(map, ptr->key, age);
                ptr = tmp;
            } else {
                ptr = ptr->next;
            }
        }
    }
    return 0;
}

void hashmap_print(HashMap* map) {
    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode* ptr = map->nodes[i];
        while (ptr) {
            printf("%s\t%d\n", ptr->key, ptr->age);
            ptr = ptr->next;
        }
    }
}