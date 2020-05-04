#include "hash.h"

unsigned int hash(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash % HASH_SIZE;
}

HashMap* hashmap_new() {
    HashMap* ret = (HashMap*)malloc(sizeof(HashMap));
    for (int i = 0; i < HASH_SIZE; i++)
        ret->nodes[i] = NULL;
    ret->next = NULL;
    return ret;
}

void hashmap_free(HashMap* map) {
    // for (int i = 0; i < HASH_SIZE; i++) {
    //     HashNode* ptr = map->nodes[i];
    //     while (ptr) {
    //         HashNode* tmp = ptr;
    //         ptr = ptr->next;
    //         HASHNODE_FREE(tmp);
    //     }
    // }
    // if (map->nodes != NULL)
    // free(map->nodes);
}

int hashmap_insert(HashMap* map, const char* key, SymNode* data) {
    if (hashmap_node(map, key) != NULL)
        return -1;  // Duplicated

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

SymNode* hashmap_value(HashMap* map, const char* key) {
    HashNode* node = hashmap_node(map, key);
    if (node == NULL)
        return NULL;
    return node->data;
}

int hashmap_delete(HashMap* map, const char* key) {
    unsigned int pos = hash(key);
    HashNode* ptr = map->nodes[pos];

    HashNode* node = hashmap_node(map, key);
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

void hashmap_print(HashMap* map) {
    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode* ptr = map->nodes[i];
        while (ptr) {
            printf("%d\t%s\n", i, ptr->key);
            ptr = ptr->next;
        }
    }
}