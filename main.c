#include <stdio.h>
#include <pthread.h>
#include <zconf.h>
#include "thread_pool.h"

void *test(void *arg) {
    int i;
    for(i=0; i<5; i++) {
        printf("tid:%ld task:%ld\n", pthread_self(), (long)arg);
        fflush(stdout);
        sleep(1);
    }
}

int main() {
    int i;
    thread_pool_t pool = thread_pool_create(3, 0);
    //sleep(5);
    for(i = 0; i < 5; i++){
        thread_pool_add_task(pool, test, (void*)i);
    }

    printf("Hello, World!\n");
    getchar();

    thread_pool_destroy(pool);
    return 0;
}