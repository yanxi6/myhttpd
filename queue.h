//
// Created by yanxi on 6/24/17.
//

#ifndef MYTHREADPOOL_QUEUE_H
#define MYTHREADPOOL_QUEUE_H


#include "type.h"

typedef struct node{
    void* element;
    struct node* next;
} mynode;

typedef struct queue{
    struct node* front;
    struct node* tail;
}*queue_t;

queue_t create_queue();
int empty(queue_t q);
void enqueue(queue_t q, void* element);
void priority_enqueue(queue_t q, void* element);
void* front(queue_t q);
void dequeue(queue_t q);
void destroy(queue_t q);


#endif //MYTHREADPOOL_QUEUE_H
