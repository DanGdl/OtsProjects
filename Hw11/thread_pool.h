#ifndef DGD_THREAD_POOL
#define DGD_THREAD_POOL


typedef struct ThreadPool ThreadPool_t;

typedef struct PoolTask {
    void* params;
    void (*job)(void*);
} ThreadPoolTask_t;


ThreadPool_t* DgdPool_create(unsigned int pool_size);
void DgdPool_await(ThreadPool_t* pool);
int DgdPool_free(ThreadPool_t* pool);

int DgdPool_submit(ThreadPool_t* pool, ThreadPoolTask_t* task);

#endif
