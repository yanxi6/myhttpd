#ifndef MYTHREADPOOL_TYPE_H
#define MYTHREADPOOL_TYPE_H

typedef struct task{
    void* (*function)(void*);
    void* args;
}*task_t;




typedef struct request
{
    int socketfd;
    char *file_name;
    unsigned int ip;
    char time_arrival[128];
    char buffer[256];
    int file_size;
}*request_t;
#endif