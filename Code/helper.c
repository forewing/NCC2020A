#include "helper.h"

char* strdup2(const char* str) {
    char* ret = (char*)malloc(strlen(str) + 1);
    strcpy(ret, str);
    return ret;
}

void __free(void* n) {}