
#ifndef HASH_MAP_H_
#define HASH_MAP_H_

#include "linked_list.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct HashMap HashMap_t;


HashMap_t* HashMap_create(
	size_t (*hash_func)(const void* const key), void (*key_free)(void* key), 
	int (*key_compare)(const void* const key1, const void* const key2)
);

int64_t HashMap_size(const HashMap_t* const map);

int HashMap_add(HashMap_t* map, void* key, void* value);

void HashMap_remove(HashMap_t* map, const void* const key, void** value);

void* HashMap_get(const HashMap_t* const map, const void* const key);

LinkedList_t* HashMap_get_values(HashMap_t* map);

void HashMap_clear_and_free(HashMap_t* map, void (*value_free)(void* item));


char* HashMap_stringify(
    const void* const p_map,
    char* (*key_stringify)(const void* const key),
    char* (*value_stringify)(const void* const value)
);

size_t HashMap_hash(
    const void* const p_map,
    size_t (*key_hash)(const void* const key),
    size_t (*value_hash)(const void* const value)
);

int HashMap_compare(
    const void* const p_map1, const void* const p_map2,
    int (*key_compare)(const void* const key1, const void* const key2),
    int (*value_compare)(const void* const value1, const void* const value2)
);

#endif
