
#ifndef HASH_MAP_H_
#define HASH_MAP_H_

#include "linked_list.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct HashMap HashMap_t;


HashMap_t* HashMap_create(size_t (*hash_func)(const void* const key));

int64_t HashMap_size(const HashMap_t* const map);

int HashMap_add(HashMap_t* map, void* key, void* data);

void HashMap_remove(
    HashMap_t* map, const void* const key, void** data, void (*key_free)(void* key)
);

void* HashMap_get(
    const HashMap_t* const map, const void* const key,
    int (*key_compare)(const void* const key1, const void* const key2)
);

LinkedList_t* HashMap_get_values(HashMap_t* map);

void HashMap_clear_and_free(HashMap_t* map,	void (*key_free)(void* key), void (*value_free)(void* item));


char* HashMap_stringify(
    const void* const p_map,
    char* (*key_stringify)(const void* const key),
    char* (*item_stringify)(const void* const item)
);

size_t HashMap_hash(
    const void* const p_map,
    size_t (*key_hash)(const void* const key),
    size_t (*item_hash)(const void* const item)
);

int HashMap_compare(
    const void* const p_map1, const void* const p_map2,
    int (*key_compare)(const void* const key1, const void* const key2),
    int (*item_compare)(const void* const item1, const void* const item2)
);

#endif
