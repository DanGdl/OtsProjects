
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include "thread_pool.h"



typedef struct Queue {
    struct Queue* head;
    ThreadPoolTask_t* task;
} Queue_t;

typedef struct ThreadPool {
    pthread_t* threads;
    unsigned int size;
    Queue_t* queue;
    pthread_mutex_t mutex;          // control access to queue
    pthread_cond_t queue_signal;    // signal, if task added to queue
    volatile atomic_uint counter;   // counter of waiting threads
} ThreadPool_t;


void unlock_mutextes(void* parameters) {
    ThreadPool_t* pool = (ThreadPool_t*) parameters;
    pthread_mutex_unlock(&(pool -> mutex));
}

void* thread_function(void* parameters) {
    ThreadPool_t* pool = (ThreadPool_t*) parameters;
    pthread_cleanup_push(unlock_mutextes, pool);

    while(1) {
        Queue_t* node = NULL;
        if (pthread_mutex_lock(&(pool -> mutex)) == 0) {
            atomic_fetch_add(&(pool -> counter), 1);
            while (pool -> queue -> head == NULL) {
                pthread_cond_wait(&(pool -> queue_signal), &(pool -> mutex));
            }
            atomic_fetch_sub(&(pool -> counter), 1);
            if (pool -> queue -> head != NULL) {
                node = pool -> queue -> head;
                pool -> queue -> head = node -> head;
            }
            pthread_mutex_unlock(&(pool -> mutex));

            if (node != NULL) {
                ThreadPoolTask_t* task = (ThreadPoolTask_t*) node -> task;
                free(node);
                task -> job(task -> params);
                free(task);
            }
        }
    }
    pthread_cleanup_pop(0);
    return NULL;
}

ThreadPool_t* DgdPool_create(unsigned int pool_size) {
    if (pool_size <= 0) {
        printf("Size can't be less or equals zero\n");
        return NULL;
    }
    ThreadPool_t* pool;
    pool = calloc(sizeof(*pool), 1);
    if (pool == NULL) {
        printf("Failed to allocate memory for thread pool\n");
        return NULL;
    }
    pool -> threads = calloc(sizeof(*(pool -> threads)), pool_size);
    if (pool == NULL) {
        printf("Failed to allocate memory for thread array\n");
        free(pool);
        return NULL;
    }
    pool -> queue = calloc(sizeof(*(pool -> queue)), 1);
    if (pool == NULL) {
        printf("Failed to allocate memory for queue\n");
        free(pool -> threads);
        free(pool);
        return NULL;
    }

    if (pthread_mutex_init(&(pool -> mutex), NULL) != 0) {
        printf("Failed to init mutex\n");
        free(pool -> threads);
        free(pool);
        return NULL;
    }
    if (pthread_cond_init(&(pool -> queue_signal), NULL) != 0) {
        printf("Failed to init conditional variable\n");
        pthread_mutex_destroy(&(pool -> mutex));
        free(pool -> threads);
        free(pool);
        return NULL;
    }
    for (unsigned int i = 0; i < pool_size; ++i) {
        if (pthread_create(pool -> threads + i, NULL, &thread_function, pool) != 0) {
            printf("Failed to create a thread %d\n", i);

            for (unsigned int j = 0; j < i; ++j) {
                pthread_cancel(*(pool -> threads + j));
            }
            free(pool -> threads);
            pthread_cond_destroy(&(pool -> queue_signal));
            pthread_mutex_destroy(&(pool -> mutex));
            free(pool);
            return NULL;
        }
    }
    pool -> size = pool_size;
    return pool;
}

void DgdPool_await(ThreadPool_t* pool) {
    while (atomic_load(&(pool -> counter)) != pool -> size);
}


int DgdPool_free(ThreadPool_t* pool) {
    if (pool == NULL) {
        return 0;
    }
    if (pool -> queue -> head != NULL) {
        printf("Queue is not empty!\n");
        return -1;
    }
    free(pool -> queue);
    for (unsigned int i = 0; i < pool -> size; ++i) {
        pthread_cancel(*(pool -> threads + i));
    }
    free(pool -> threads);
    pthread_cond_destroy(&(pool -> queue_signal));
    pthread_mutex_destroy(&(pool -> mutex));
    free(pool);
    return 0;
}

int DgdPool_submit(ThreadPool_t* pool, ThreadPoolTask_t* task) {
    if (pool == NULL || task == NULL) {
        return - 1;
    }
    if (pthread_mutex_lock(&(pool -> mutex)) == 0) {
        Queue_t* last_node = pool -> queue;
        while (last_node -> head != NULL) {
            last_node = last_node -> head;
        }
        last_node -> head = calloc(sizeof(*last_node), 1);
        if (last_node -> head == NULL) {
            printf("Failed to allocate memory for queue node\n");
            pthread_mutex_unlock(&(pool -> mutex));
            return -1;
        }
        last_node -> head -> task = task;
        pthread_cond_signal(&(pool -> queue_signal));
        pthread_mutex_unlock(&(pool -> mutex));
        return 0;
    }
    return -1;
}
