//
// Created by yanxi on 6/24/17.
//

#include <stdlib.h>
#include <pthread.h>
// #include <zconf.h>
#include "thread_pool.h"

// static void cleanup(pthread_mutex_t* lock) {
//     pthread_mutex_unlock(lock);
// }
static void* consumer(thread_pool_t pool){
    task_t t;
    sleep(60);
    printf("%ld is waiting!\n", pthread_self());
    while(1){
        pthread_mutex_lock(&pool->mutex);
        //pthread_cleanup_push((void(*)(void*))cleanup, &pool->mutex);
        while(empty(pool->buffers)){
            pthread_cond_wait(&pool->queue_ready, &pool->mutex);
        }
        printf("%ld is serving!\n", pthread_self());
        t = (task_t)front(pool->buffers);
        dequeue(pool->buffers);
       //pthread_cleanup_pop(0);
        pthread_mutex_unlock(&pool->mutex);
        t->function(t->args);
        free(t);
    }
    return NULL;
}

thread_pool_t thread_pool_create(unsigned int size, int flag, int wtime){
    unsigned int i;
    thread_pool_t pool = (thread_pool_t)malloc(sizeof(struct thread_pool));
    pool->size = size;
    pool->wait_time = wtime;
    pool->flag=flag;
    pool->buffers = create_queue();
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->queue_ready, NULL);

    pool->consumers = (pthread_t*)malloc(sizeof(pthread_t) * size);
    for(i = 0; i < size; i++){
        pthread_t* tmp = pool->consumers[i];
        pthread_create(pool->consumers + i, NULL, (void* (*)(void*))consumer, pool);
    }
    return pool;
}
void thread_pool_add_task(thread_pool_t pool, void* (*function)(void*), void* arg){
    pthread_mutex_lock(&pool->mutex);
    task_t t = (task_t)malloc(sizeof(struct task));
    t->function = function;
    t->args = arg;
   // t->function(t->args);
    if(pool->flag==0)
        enqueue(pool->buffers, t);
    else
        priority_enqueue(pool->buffers, t);

    pthread_cond_signal(&pool->queue_ready);
    pthread_mutex_unlock(&pool->mutex);
    //free(t);
}
void thread_pool_destroy(thread_pool_t pool){
    unsigned int i;
    for(i = 0; i < pool->size; i++){
        pthread_cancel(pool->consumers + i);
    }
    for(i = 0; i < pool->size; i++){
        pthread_join(pool->consumers + i, NULL);
    }
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->queue_ready);
    destroy(pool->buffers);
    free(pool);
}
