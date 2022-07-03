/*
 * test_map.c
 *
 *  Created on: 2 Jul 2022
 *      Author: max
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>   // check file info
#include <sys/mman.h>   // mmap
#include <fcntl.h>      // permission flags
#include <unistd.h>     // open/close
#include <string.h>
#include <ctype.h>

#include "hash_map.h"
#include "util.h"

#define STRING_COMPARE_MAX 500 /*some big number*/



void free_pointer(void* ptr) {
	free(ptr);
}

size_t hash_key(const void* const value) {
	return hash_string(value);
}

int compare_key(const void* const key1, const void* const key2) {
	return strncmp(key1, key2, STRING_COMPARE_MAX);
}

char* stringify_key(const void* const key) {
    char* buffer = NULL;
    int size = strlen(key);
    buffer = calloc(sizeof(*buffer), size);
    if (buffer == NULL) {
        printf("Failed to allocate buffer for value\n");
        return NULL;
    }
    memcpy(buffer, key, sizeof(char) * size);
	return buffer;
}

char* stringify_value(const void* const value) {
    const int* number = (int*) value;
	char* buffer = NULL;
    int size = *number < 10 ? 2 : 3;
    buffer = calloc(sizeof(*buffer), size);
    if (buffer == NULL) {
        printf("Failed to allocate buffer for value\n");
        return NULL;
    }
    sprintf(buffer, "%d", *number);
    buffer[size - 1] = '\0';
    return buffer;
}

void free_map(HashMap_t* map) {
    HashMap_clear_and_free(map, free_pointer);
}




int count_words(uint8_t* mapping, struct stat* stats) {
    int start_idx = 0;
    HashMap_t* map = HashMap_create(hash_key, free_pointer, compare_key);
    
    for(int i = 0; i < stats -> st_size; i++) {
        if (mapping[i] == ' ' || mapping[i] == '\t'  || mapping[i] == '\n' || !isalpha(mapping[i])) {
            const int end_idx = i;
            if (end_idx - start_idx <= 0) {
                start_idx = i + 1;
                continue;
            }
            char* word = NULL;
            word = calloc(sizeof(*word), end_idx - start_idx);
            if (word == NULL) {
                printf("Failed to allocate memory for word\n");
                free_map(map);
                return -1;
            }
            memcpy(word, &mapping[start_idx], sizeof(char) * (end_idx - start_idx));
            start_idx = i + 1;
            
            
            int* counter = HashMap_get(map, word);
            if (counter == NULL) {
                int* value = NULL;
                value = calloc(sizeof(*value), 1);
                if (value == NULL) {
                    printf("Failed to allocate memory for value\n");
                    free_map(map);
                    return -1;
                }
                *value = 1;
                if (HashMap_add(map, word, value) < 0) {
                    printf("Failed to put key %s and value %d to map\n", word, *value);
                    free(word);
                    free(value);
                    free_map(map);
                    return -1;
                }
            } else {
                (*counter) += 1;
            }

        }
    }
    char* string = HashMap_stringify(map, stringify_key, stringify_value);
    printf("%s\n", string);
    free(string);
	
	free_map(map);
	map = NULL;
    return 0;
}

int main(int argc, char* argv[]) {
    char* path = NULL;
    if (argc == 2) {
        path = argv[1];
    } else {
        printf("Please add a path to file to parameters\n");
        return 0;
    }

    struct stat stats;
    if (stat(path, &stats) != 0) {
        printf("Can't get state for path %s\n", path);
    } else if (S_ISDIR(stats.st_mode)) {
        printf("File is directory, path %s\n", path);
    } else if (!S_ISREG(stats.st_mode)) {
        printf("File is not regular file, path %s\n", path);
    } else {
        int fd = open(path, O_RDONLY);
        if (fd < 0) {
            printf("Failed to open file %s\n", path);
            return 0;
        }
        uint8_t* mapped = mmap(NULL, stats.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (mapped == MAP_FAILED) {
            printf("Failed to map file to memory\n");
            close(fd);
            return 0;
        }
        count_words(mapped, &stats);

        if (munmap(mapped, stats.st_size) != 0) {
            printf("Could not munmap\n");
        }
        close(fd);
    }
    return 0;
}
