#include <stdio.h>
#include <sys/stat.h>   // check file info
#include <sys/mman.h>   // mmap
#include <fcntl.h>      // permission flags
#include <unistd.h>     // open/close
#include <string.h>     // memcopy
#include <stdint.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/time.h>

#include "thread_pool.h"
#include "hash_map.h"

#define STRING_COMPARE_MAX 1000 /*some big number*/
#define TYPE_URL 1
#define TYPE_REFERER 2

#define SIZE_RESULT_ARRAYS 10


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
HashMap_t* map;


typedef struct Statistics {
    char* key;
    uint8_t type;
    uint64_t counter;
} Statistics_t;


Statistics_t* stats_url[SIZE_RESULT_ARRAYS];
Statistics_t* stats_referer[SIZE_RESULT_ARRAYS];


// support for hash map
void free_pointer(void* ptr) {
	free(ptr);
}

size_t hash_key(const void* const value) {
	return hash_string(value);
}

int compare_key(const void* const key1, const void* const key2) {
	if (key1 == NULL && key2 != NULL) {
		return 1;
	} else if (key1 != NULL && key2 == NULL) {
		return -1;
	}
	return key1 == key2 || strncmp(key1, key2, STRING_COMPARE_MAX);
}
// EOF: support for hash map




// called in thread pool
void find_quotation_marks(int* quotes_indexes, uint8_t* mapping, int start_idx, int end_idx) {
    int idx = start_idx;
    int counter = 0;
    while (counter < 2 && idx < end_idx) {
        if (mapping[idx] == '\"') {
            *(quotes_indexes + counter) = idx;
            counter++;
        }
        idx++;
    }
}

long int get_traffic(uint8_t* mapping, int idx, int end_idx, int* indexes) {
    // jump to traffic's digits
    idx += 6;

    // parse traffic value
    const int traffic = idx;
    char buffer[20] = {'\0'};
    while (mapping[idx] != ' ' && idx < end_idx) {
        buffer[idx - traffic] = mapping[idx];
        idx++;
    }
    indexes[0] = traffic;
    indexes[1] = idx;
    return (strcmp("-", (char*) &buffer) == 0 || strcmp("0", (char*) &buffer) == 0) ? 0 : strtoll(buffer, NULL, 10);
}

char* get_referer(uint8_t* mapping, int start_idx, int end_idx) {
    // jump to referer's start
    start_idx += 2;
    int idx = start_idx;

    char buffer[4] = {'\0'};
    while (mapping[idx] != '\"' && idx < end_idx) {
        int diff = idx - start_idx;
        if (diff < (int)(sizeof(buffer) / sizeof(buffer[0]))) {
            buffer[diff] = mapping[idx];
        }
        idx++;
    }
    if (strcmp("-", (char*) &buffer) == 0) {
        return NULL;
    }

    char* referer = NULL;
    referer = calloc(sizeof(*referer), idx - start_idx + 1);
    if (referer == NULL) {
        printf("Failed to allocate memory for referer\n");
        return NULL;
    }
    memcpy(referer, &mapping[start_idx], idx - start_idx);
    return referer;
}


void collect_statistics(uint8_t* mapping, long int mapping_size) {
    int start_idx = 0;
    for(int i = 0; i < mapping_size; i++) {
        if (mapping[i] == '\n') {
            const int end_idx = i;
            if (end_idx - start_idx <= 0) {
                start_idx = i + 1;
                continue;
            }

            int quotes_indexes[2] = { -1 };
            find_quotation_marks((int*) &quotes_indexes, mapping, start_idx, end_idx);
            // start_idx = i + 1;

            int url_start = -1;
            int url_end = -1;
            if (quotes_indexes[0] + 1 == quotes_indexes[1]) {
            	url_start = quotes_indexes[0];
				url_end = quotes_indexes[1];
			} else if (quotes_indexes[0] + 2 == quotes_indexes[1]) {
            	url_start = quotes_indexes[0] + 1;
				url_end = quotes_indexes[1];
			} else {
				int idx = 0;
				while(quotes_indexes[0] + idx < quotes_indexes[1]) {
					if (url_start == -1 && mapping[quotes_indexes[0] + idx] == ' ') {
						url_start = quotes_indexes[0] + idx + 1;
					}
					if (url_end == -1 && mapping[quotes_indexes[1] - idx] == ' ') {
						url_end = quotes_indexes[1] - idx;
					}
					if (url_start == ' ' && url_end == ' ') {
						break;
					}
					idx++;
				}
				if (url_start == -1 && url_end == -1) {
					url_start = quotes_indexes[0] + 1;
					url_end = quotes_indexes[1] - 1;
				} else if (url_start == url_end + 1) {
					url_start = quotes_indexes[0] + 1;
				} else if (url_start != -1 && url_start == url_end) {
					url_start = quotes_indexes[0] + 1;
				}
			}
            if (url_end <= url_start) {
            	char* line = calloc(sizeof(char), end_idx - start_idx + 1);
				memcpy(line, &mapping[start_idx], end_idx - start_idx);
            	printf("%s\n", line);
            	free(line);
            	start_idx = i + 1;
            	continue;
            }
			start_idx = i + 1;

            char* url = NULL;
            url = calloc(sizeof(*url), url_end - url_start + 1);
            if (url == NULL) {
                printf("Failed to allocate memory for url\n");
                return;
            }
            if (url_end - url_start > 1) {
                memcpy(url, &mapping[url_start], url_end - url_start);
            }
            long int traffic = get_traffic(mapping, quotes_indexes[1], end_idx, (int*) &quotes_indexes);
            char* referer = get_referer(mapping, quotes_indexes[1], end_idx);
            
            if (pthread_mutex_lock(&mutex) == 0) {
                Statistics_t* value = HashMap_get(map, url);
                if (value == NULL) {
                    value = calloc(sizeof(*value), 1);
                    if (value == NULL) {
                        printf("Failed to allocate memory for url statistics\n");
                        free(url);
                        free(referer);
                        pthread_mutex_unlock(&mutex);
                        return;
                    }
                    value -> key = url;
                    value -> type = TYPE_URL;
                    value -> counter = traffic;

                    if (HashMap_add(map, url, value) != 0) {
                        printf("Failed to add value to map\n");
                    }
                } else {
                    value -> counter += traffic;
                }

                if (referer != NULL) {
                    Statistics_t* value = HashMap_get(map, referer);
                    if (value == NULL) {
                        value = calloc(sizeof(*value), 1);
                        if (value == NULL) {
                            printf("Failed to allocate memory for referer statistics\n");
                            free(referer);
                            pthread_mutex_unlock(&mutex);
                            return;
                        }
                        value -> key = referer;
                        value -> type = TYPE_REFERER;
                        value -> counter = 1;

                        if (HashMap_add(map, referer, value) != 0) {
                            printf("Failed to add value to map\n");
                        }
                    } else {
                        value -> counter += 1;
                    }
                }
                pthread_mutex_unlock(&mutex);
            }
        }
    }

}

void analyze_logs(void* params) {
    char* path = (char*) params;
    printf("Analyze file %s\n", path);

    struct timeval start, stop;
    gettimeofday(&start, NULL);

    struct stat stats;
    if (stat(path, &stats) != 0) {
        printf("Can't get stats for path %s\n", path);
    } else if (!S_ISREG(stats.st_mode)) {
        printf("File is not a regular file %s\n", path);
    } else {
        int fd = open(path, O_RDONLY);
        if (fd < 0) {
            printf("Failed to open file %s\n", path);
            free(path);
            return;
        }
        uint8_t* mapped = mmap(NULL, stats.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (mapped == MAP_FAILED) {
            printf("Failed to map file to memory\n");
            close(fd);
            free(path);
            return;
        }
        collect_statistics(mapped, stats.st_size);

        if (munmap(mapped, stats.st_size) != 0) {
            printf("Could not munmap\n");
        }
        close(fd);
    }
    gettimeofday(&stop, NULL);
    double seconds = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
    printf("Analyze done file %s, time: %f seconds\n", path, seconds);
    free(path);
}
// EOF: called in thread pool




void filter_statistics(void* key, void* value) {
    Statistics_t* val = (Statistics_t*) value;

    Statistics_t** array = NULL;
    if (val -> type == TYPE_URL) {
        array = stats_url;
    } else if (val -> type == TYPE_REFERER) {
        array = stats_referer;
    } else {
        return;
    }
    for (int i = 0; i < SIZE_RESULT_ARRAYS; ++i) {
        if (array[i] == NULL) {
            array[i] = val;
            break;
        }

        if (array[i] -> counter < val -> counter) {
            Statistics_t* tmp = array[i];
            array[i] = val;
            val = tmp;
        }
    }
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Please add a path to logs directory to parameters\n");
        return 0;
    }
    else if (argc < 3) {
        printf("Please add a amount of threads to parameters\n");
        return 0;
    }
    const char* path = argv[1];
    const int count = atoi(argv[2]);
    if (count <= 0) {
        printf("Amount of threads must be bigger then zero\n");
        return 0;
    }

    struct stat stats;
    if (stat(path, &stats) != 0) {
        printf("Can't get state for path %s\n", path);
        return 0;
    } else if (!S_ISDIR(stats.st_mode)) {
        printf("Path %s is not a directory\n", path);
        return 0;
    }

    DIR* dir = opendir(path);
	if (dir == NULL) {
		printf("Directory cannot be opened %s\n", path);
		return 0;
	}

	map = HashMap_create(hash_key, compare_key, free_pointer);
    if (map == NULL) {
        closedir(dir);
        printf("Failed to create hash map\n");
		return 0;
    }
	ThreadPool_t* pool = DgdPool_create(count);
    if (pool == NULL) {
        closedir(dir);
        // we can provide NULLs, because map is still not used
        HashMap_clear_and_free(map, NULL);
        printf("Failed to create thread pool\n");
		return 0;
    }
	struct dirent* file;
	while ((file = readdir(dir)) != NULL) {
		const char* file_name = file -> d_name;
        if (strcmp(file_name, ".") == 0 || strcmp(file_name, "..") == 0) {
            continue;
        }
        char* file_path = NULL;
        file_path = calloc(sizeof(*file_path), strlen(path) + strlen(file_name) + 2);
        sprintf(file_path, "%s/%s", path, file_name);
        ThreadPoolTask_t* task = NULL;
        task = calloc(sizeof(*task), 1);
        task -> params = file_path;
        task -> job = &analyze_logs;

        DgdPool_submit(pool, task);
	}
	closedir(dir);

    // sleep(1);
    DgdPool_await(pool);
    DgdPool_free(pool);

    HashMap_iterate(map, &filter_statistics);

    printf("\nMost heavy responses:\n");
    for (int i = 0; i < SIZE_RESULT_ARRAYS; ++i) {
        if (stats_url[i] == NULL) {
            continue;
        }
        printf("Size %ld, url %s\n", stats_url[i] -> counter, stats_url[i] -> key);
    }
    printf("\nMost frequent referers:\n");
    for (int i = 0; i < SIZE_RESULT_ARRAYS; ++i) {
        if (stats_referer[i] == NULL) {
            continue;
        }
        printf("Count %ld, referer %s\n", stats_referer[i] -> counter, stats_referer[i] -> key);
    }

    HashMap_clear_and_free(map, free_pointer);
    return 0;
}
