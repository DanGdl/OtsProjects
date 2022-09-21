
#ifndef HASH_MAP_H_
#define HASH_MAP_H_

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#define RESULT_ADD_NODE_ALREDY_EXIST 1

typedef struct HashMap HashMap_t;
typedef size_t (*map_hasher)(const void* const key);
typedef size_t (*map_key_comparator)(const void* const key);

HashMap_t* HashMap_create(
	size_t (*map_hasher)(const void* const key),
	size_t (*map_key_comparator)(const void* const key),
	void (*key_free)(void* key)
);

int64_t HashMap_size(const HashMap_t* const map);

int HashMap_add(HashMap_t* map, void* key, void* value);

void HashMap_remove(HashMap_t* map, const void* const key, void** value);

void* HashMap_get(const HashMap_t* const map, const void* const key);

void HashMap_iterate(const HashMap_t* const map, void (*func_iteration)(void* key, void* value));

int HashMap_clear_and_free(HashMap_t* map, void (*value_free)(void* item));

void HashMap_print(
	const HashMap_t* const p_map,
	void (*print_key)(const void* const key),
	void (*print_value)(const void* const value)
);

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
