
#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

size_t hash_string(const char* const value) {
    if (value == NULL) {
        return 0;
    }
    unsigned long long hash = 0;
    char* k = (char*) value;
    while (k != NULL && *k != '\0') {
        hash += *k;
        hash += (hash << 10);
        hash ^= (hash >> 6);
        k++;
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}


size_t hash_number_iteraction(size_t hash) {
    hash += (hash << 10);
    hash ^= (hash >> 6);
    return hash;
}

size_t hash_number_end(size_t hash) {
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

size_t hash_number(const void* const value, size_t size) {
    if (value == NULL) {
            return 0;
    }

    size_t hash = 0;
    const uint8_t* byte = (const uint8_t*) value;
    for (size_t i = 0; i < size; ++i) {
        hash += *byte++;
        hash = hash_number_iteraction(hash);
    }

    return hash_number_end(hash);
}



char* ptr_stringify(const void* const ptr) {
    size_t value = (size_t) ptr;
    int digits = 0;
    while (value != 0) {
            value = value / 16;
            digits++;
    }
    char* str = calloc(sizeof(char), digits * 2 + 3);
    sprintf(str, "%p", ptr);
    return str;
}
