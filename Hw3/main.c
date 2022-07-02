/*
 * test_map.c
 *
 *  Created on: 2 Jul 2022
 *      Author: max
 */

#include <stdio.h>
#include <string.h>

#include "hash_map.h"
#include "util.h"


void free_pointer(void* ptr) {
	free(ptr);
}

size_t hash_key(const void* const value) {
	return hash_string(value);
}


int main(void) {
	HashMap_t* map = HashMap_create(hash_key);

	for (int i = 0; i < 40; i++) {
		char* buffer = NULL;
		int size = i < 10 ? 2 : 3;
		buffer = calloc(sizeof(*buffer), size);
		if (buffer == NULL) {
			printf("Failed to allocate buffer for key\n");
            return 0;
		}
		sprintf(buffer, "%d", i);
		buffer[size - 1] = '\0';

		int* value = NULL;
		value = calloc(sizeof(*value), 1);
		if (value == NULL) {
			printf("Failed to allocate value\n");
            return 0;
		}
		*value = i;
		if (HashMap_add(map, buffer, value) < 0) {
			printf("Failed to put key %s and value %d to buffer\n", buffer, i);
            free(buffer);
            free(value);
            return 0;
		}
	}

	for (int i = 0; i < 20; i++) {
		char* buffer = NULL;
		int size = i < 10 ? 2 : 3;
		buffer = calloc(1, size);
		if (buffer == NULL) {
			printf("Failed to allocate buffer for key\n");
            return 0;
		}
		sprintf(buffer, "%d", i);
		buffer[size - 1] = '\0';
		int* value = NULL;

		HashMap_remove(map, buffer, (void**) &value, free_pointer);
        free(value);
        free(buffer);
	}
	
	HashMap_clear_and_free(map, free_pointer, free_pointer);
	map = NULL;
    return 0;
}
