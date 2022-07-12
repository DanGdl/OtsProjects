#include "hash_map.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define ITEM_NULL "NULL"
#define SIZE_HASH_MAP 20
#define TEMPLATE_HASHMAP_EMPTY "HashMap[]"
#define TEMPLATE_HASHMAP "HashMap[ %s ]"
#define TEMPLATE_NODE "%s -> %s"

#define RESIZE_RATIO 1.3
#define RESULT_ADD_NODE_RESIZE 1



typedef struct MapNode {
	void *key;
	void *value;
} MapNode_t;


typedef struct HashMap {
	MapNode_t **head;
	size_t (*hash_func)(const void *const key);
	int (*key_compare)(const void *const key1, const void *const key2);
	void (*key_free)(void *key);
	int64_t size;
	int64_t capacity;
} HashMap_t;




HashMap_t* HashMap_create(size_t (*hash_func)(const void *const key),
		int (*key_compare)(const void *const key1, const void *const key2),
		void (*key_free)(void *value)) {
	if (key_free == NULL || key_compare == NULL || hash_func == NULL) {
		return NULL;
	}
	HashMap_t *map;
	map = malloc(sizeof(*map));
	if (map == NULL) {
		return NULL;
	}
	map->head = calloc(sizeof(map->head), SIZE_HASH_MAP);
	if (map->head == NULL) {
		return NULL;
	}
	map->hash_func = hash_func;
	map->key_free = key_free;
	map->key_compare = key_compare;
	map->size = 0;
	map->capacity = SIZE_HASH_MAP;
	return map;
}


int64_t HashMap_size(const HashMap_t *const map) {
	if (map == NULL) {
		return 0;
	}
	return map->size;
}


int are_keys_same(
		int (*key_compare)(const void *const key1, const void *const key2),
		const void *const key1, const void *const key2) {
	return key_compare(key1, key2) == 0;
}


int add_node(MapNode_t **array, int64_t capacity, MapNode_t *node, void *pKey,
		void *pData,
		int (*key_compare)(const void *const key1, const void *const key2),
		size_t (*hash_func)(const void *const key)) {
	if (node == NULL && pKey == NULL) {
		printf("Validation error: node is NULL and key is NULL\n");
		return -1;
	}

	void *key = pKey == NULL ? node->key : pKey;
	void *data = pData == NULL ? node->value : pData;
	const size_t key_hash = hash_func(key);
	const int position = key_hash % (capacity);

	int i = position;
	do {
		int idx = i % capacity;
		MapNode_t *tmp_node = *(array + idx);
		if (tmp_node == NULL) {
			break;
		} else if (are_keys_same(key_compare, tmp_node->key, key)) {
			return RESULT_ADD_NODE_ALREDY_EXIST;
		}
		i++;
		if (i == capacity) {
			i = 0;
		}
	} while (i != position);

	if (node == NULL) {
		node = malloc(sizeof(*node));
		if (node == NULL) {
			printf("Failed to allocate memory for MapNode\n");
			return -1;
		}
		node->key = key;
		node->value = data;
	}

	i = position;
	do {
		int idx = i % capacity;
		if (*(array + idx) == NULL) {
			*(array + idx) = node;
			return 0;
		}
		i++;
		if (i == capacity) {
			i = 0;
		}
	} while (i != position);

	// free node only if it's created here
	if (pKey == NULL) {
		free(node);
	}
	return RESULT_ADD_NODE_RESIZE;
}


int resize_map(HashMap_t *map, bool forced) {
	if (!forced && map->capacity >= (RESIZE_RATIO * (map->size + 1))) {
		return 0;
	}

	MapNode_t **old_array = map->head;
	const int64_t old_cap = map->capacity;

	const uint64_t new_cap = old_cap * 2;
	MapNode_t **new_array = calloc(sizeof(map->head), new_cap);
	if (new_array == NULL) {
		return -1;
	}

	for (int i = 0; i < old_cap; i++) {
		MapNode_t *node = *(old_array + i);
		if (node == NULL) {
			continue;
		}

		if (add_node(new_array, new_cap, node, NULL, NULL, map->key_compare,
				map->hash_func) < 0) {
			free(new_array);
			printf("Resize: failed to add node\n");
			return -1;
		}
	}
	map->capacity = new_cap;
	map->head = new_array;
	free(old_array);
	return 0;
}


int HashMap_add(HashMap_t *map, void *key, void *data) {
	if (map == NULL) {
		return -1;
	}

	if (resize_map(map, false) < 0) {
		return -1;
	}
	int result;
	do {
		result = add_node(map->head, map->capacity, NULL, key, data,
				map->key_compare, map->hash_func);
		if (result == RESULT_ADD_NODE_RESIZE) {
			if (resize_map(map, true) < 0) {
				return -1;
			}
		} else if (result < 0) {
			return result;
		} else {
			break;
		}
	} while (true);
	map->size += 1;
	return 0;
}


void HashMap_remove(HashMap_t *map, const void *const key, void **data) {
	if (map == NULL) {
		*data = NULL;
		return;
	}

	size_t key_hash = map->hash_func(key);
	int position = key_hash % (map->capacity);

	int i = position;
	do {
		int idx = i % map->capacity;

		MapNode_t *node = *(map->head + idx);
		if (node == NULL) {
			break;
		} else if (are_keys_same(map->key_compare, node->key, key)) {
			*data = node->value;
			map->size -= 1;
			if (node->key != NULL) {
				map->key_free(node->key);
				node->key = NULL;
			}
			free(node);
			*(map->head + i) = NULL;
			break;
		}
		i++;
		if (i == map->capacity) {
			i = 0;
		}
	} while (i != position);

	i = position;
	do {
		int idx = i % map->capacity;

		MapNode_t *node = *(map->head + idx);
		if (node != NULL) {
			key_hash = map->hash_func(node->key);
			int expected_position = key_hash % (map->capacity);
			if (expected_position < idx) {
				*(map->head + idx) = NULL;
				int result = add_node(map->head, map->capacity, node, NULL,
						NULL, map->key_compare, map->hash_func);
				if (result != 0) {
					*(map->head + idx) = node;
				}
			}
		}
		i++;
		if (i == map->capacity) {
			i = 0;
		}
	} while (i != position);
}


void* HashMap_get(const HashMap_t *const map, const void *const key) {
	if (map == NULL) {
		return NULL;
	}
	if (key == NULL) {
		printf("Validation error: key is NULL\n");
		return NULL;
	}

	const size_t key_hash = map->hash_func(key);
	const int position = key_hash % (map->capacity);
	int i = position;
	do {
		int idx = i % map->capacity;

		MapNode_t *node = *(map->head + idx);
		if (node == NULL) {
			break;
		} else if (are_keys_same(map->key_compare, node->key, key)) {
			return node->value;
		}
		i++;
		if (i == map->capacity) {
			i = 0;
		}
	} while (i != position);
	return NULL;
}


int HashMap_clear_and_free(HashMap_t *map, void (*value_free)(void *item)) {
	if (map == NULL) {
		return 0;
	}

	if (value_free == NULL) {
		printf("Validation error: value_free pointer is NULL\n");
		return -1;
	}

	for (int j = 0; j < map->capacity; j++) {
		MapNode_t *node = *(map->head + j);
		if (node == NULL) {
			continue;
		}
		if (node->key != NULL) {
			map->key_free(node->key);
			node->key = NULL;
		}
		if (node->value != NULL) {
			value_free(node->value);
			node->value = NULL;
		}
		map->size--;
		free(node);
		*(map->head + j) = NULL;
	}
	free(map->head);
	free(map);
	return 0;
}


void HashMap_print(const HashMap_t *const p_map,
		void (*print_key)(const void *const key),
		void (*print_value)(const void *const value)) {
	if (p_map == NULL) {
		return;
	} else if (HashMap_size(p_map) == 0) {
		printf(TEMPLATE_HASHMAP_EMPTY);
	} else {
		printf("HashMap[\n");
		for (int i = 0; i < p_map->capacity; i++) {
			MapNode_t *node = *(p_map->head + i);
			if (node == NULL) {
				continue;
			}
			printf("\t ");
			print_key(node->key);
			printf(" -> ");
			print_value(node->value);
			printf("\n");
		}
		printf("]\n");
	}
}


char* HashMapNode_stringify(const void *const p_node,
		char* (*key_stringify)(const void *const key),
		char* (*item_stringify)(const void *const item)) {
	if (p_node == NULL) {
		return NULL;
	}

	const MapNode_t *const node = (MapNode_t*) p_node;
	char *item_str = NULL;
	char *key_str = NULL;
	char *value_str = NULL;
	if (key_stringify == NULL) {
		key_str = ptr_stringify(node->key);
	} else {
		key_str = key_stringify(node->key);
	}

	if (key_str == NULL) {
		key_str = calloc(sizeof(char), strlen(ITEM_NULL) + 1);
		memcpy(key_str, ITEM_NULL, sizeof(char) * strlen(ITEM_NULL));
	}

	if (item_stringify == NULL) {
		value_str = ptr_stringify(node->key);
	} else {
		value_str = item_stringify(node->value);
	}
	if (value_str == NULL) {
		value_str = calloc(sizeof(char), strlen(ITEM_NULL) + 1);
		memcpy(value_str, ITEM_NULL, sizeof(char) * strlen(ITEM_NULL));
	}
	item_str = calloc(sizeof(char),
			strlen(TEMPLATE_NODE) - 2 + strlen(key_str) + strlen(value_str)
					+ 1);
	sprintf(item_str, TEMPLATE_NODE, key_str, value_str);
	free(key_str);
	free(value_str);
	return item_str;
}


char* HashMap_resize_items_str(char *items, const char *const item_str,
		int extra) {
	if (items == NULL) {
		const int new_length = strlen(item_str) + extra;
		items = calloc(sizeof(char), new_length);
	} else {
		const int new_length = strlen(items) + strlen(item_str) + extra;
		items = realloc(items, sizeof(char) * new_length);
	}
	return items;
}


char* HashMap_stringify(const void *const p_map,
		char* (*key_stringify)(const void *const key),
		char* (*item_stringify)(const void *const item)) {
	if (p_map == NULL) {
		return NULL;
	}

	char *items = NULL;
	const HashMap_t *const map = (HashMap_t*) p_map;
	for (int i = 0; i < map->capacity; i++) {
		const MapNode_t *const node = *(map->head + i);
		if (node == NULL) {
			continue;
		}
		char *item_str = HashMapNode_stringify(node, key_stringify,
				item_stringify);
		const int old_length = items == NULL ? 0 : strlen(items);
		if (old_length == 0) {
			items = HashMap_resize_items_str(items, item_str, 1);
			sprintf(items + old_length, "%s", item_str);
		} else {
			items = HashMap_resize_items_str(items, item_str, 3);
			sprintf(items + old_length, ", %s", item_str);
		}
		free(item_str);
	}

	char *result = NULL;
	if (items == NULL) {
		result = calloc(sizeof(char), strlen(TEMPLATE_HASHMAP_EMPTY) + 1);
		sprintf(result, TEMPLATE_HASHMAP_EMPTY);
	} else {
		result = calloc(sizeof(char),
				strlen(TEMPLATE_HASHMAP) - 2 + strlen(items) + 1);
		sprintf(result, TEMPLATE_HASHMAP, items);
		free(items);
	}
	return result;
}


size_t HashMap_hash(const void *const p_map,
		size_t (*key_hash)(const void *const key),
		size_t (*item_hash)(const void *const item)) {
	if (p_map == NULL) {
		return 0;
	}
	size_t hash = 0;
	const HashMap_t *const map = (HashMap_t*) p_map;
	for (int i = 0; i < map->capacity; ++i) {
		const MapNode_t *const node = *(map->head + i);
		if (node == NULL) {
			continue;
		}
		size_t node_hash = 0;
		if (key_hash == NULL) {
			node_hash = hash_number(node->key, sizeof(node->key));
		} else {
			node_hash = key_hash(node->key);

		}
		if (item_hash == NULL) {
			node_hash = hash_number_iteraction(
					node_hash + hash_number(node->value, sizeof(node->value)));
		} else {
			node_hash = hash_number_iteraction(
					node_hash + item_hash(node->value));
		}
		hash = hash_number_iteraction(hash + node_hash);
	}
	return hash_number_end(hash);
}


int HashMap_node_compare(const void *const p_node1, const void *const p_node2,
		int (*key_compare)(const void *const key1, const void *const key2),
		int (*item_compare)(const void *const item1, const void *const item2)) {
	if (p_node1 == NULL && p_node2 != NULL) {
		return 1;
	} else if (p_node1 != NULL && p_node2 == NULL) {
		return -1;
	}

	const MapNode_t *const node1 = (MapNode_t*) p_node1;
	const MapNode_t *const node2 = (MapNode_t*) p_node2;
	if (key_compare == NULL) {
		if (node1->key > node2->key) {
			return 1;
		} else if (node1->key < node2->key) {
			return -1;
		}
	} else {
		int res = key_compare(node1->key, node2->key);
		if (res != 0) {
			return res;
		}
	}

	if (item_compare == NULL) {
		if (node1->value > node2->value) {
			return 1;
		} else if (node1->value < node2->value) {
			return -1;
		}
	} else {
        int res = item_compare(node1->value, node2->value);
		if (res != 0) {
			return res;
		}
	}
	return 0;
}


int HashMap_compare(const void *const p_map1, const void *const p_map2,
		int (*key_compare)(const void *const key1, const void *const key2),
		int (*item_compare)(const void *const item1, const void *const item2)) {
	if (p_map1 == NULL && p_map2 != NULL) {
		return 1;
	} else if (p_map1 != NULL && p_map2 == NULL) {
		return -1;
	}

	if (p_map1 == p_map2) {
		return 0;
	}
	const HashMap_t *const map1 = (HashMap_t*) p_map1;
	const HashMap_t *const map2 = (HashMap_t*) p_map2;

	if (map1->size != map2->size) {
		return map1->size > map2->size ? 1 : -1;
	}

	for (int i = 0; i < map1->capacity; ++i) {
		const MapNode_t *const node1 = *(map1->head + i);
		const MapNode_t *const node2 = *(map2->head + i);
		int res = HashMap_node_compare(node1, node2, key_compare, item_compare);
		if (res != 0) {
			return res;
		}
	}
	return 0;
}
