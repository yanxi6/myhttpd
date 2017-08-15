//
// Created by yanxi on 6/24/17.
//

#ifndef MYTHREADPOOL_THREAD_POOL_H
#define MYTHREADPOOL_THREAD_POOL_H

#include <pthread.h>
#include "queue.h"
#include "type.h"

typedef struct thread_pool{
    unsigned int size;
    queue_t buffers;
    // pthread_t* consumers;
    pthread_t* consumers;
    pthread_mutex_t mutex;
    pthread_cond_t queue_ready;
    int wait_time;
    int flag;
}*thread_pool_t;

thread_pool_t thread_pool_create(unsigned int size, int flag, int wtime);
void thread_pool_add_task(thread_pool_t threadpool, void* (*function)(void*), void* arg);
void thread_pool_destroy(thread_pool_t threadpool);
static void* consumer(thread_pool_t pool);

#endif //MYTHREADPOOL_THREAD_POOL_H
