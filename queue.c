//
// Created by yanxi on 6/24/17.
//


#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
#include "thread_pool.h"
#include "type.h"

queue_t create_queue(){
    queue_t q = (queue_t)malloc(sizeof(queue_t));
    if(!q){
        printf("memory allocation failed!");
        return NULL;
    }
    q->front = (struct node*)malloc(sizeof(struct node));
    if(!q->front) return NULL;
    q->front->element = NULL;
    q->front->next = NULL;
    q->tail = q->front;
    return q;
}

int empty(queue_t pool){
    return pool->front == pool->tail;
}
void enqueue(queue_t q, void* element){
    struct node* new = (struct node*)malloc(sizeof(struct node));
    new->element = element;
    new->next = NULL;
    q->tail->next = new;
    q->tail = q->tail->next;
}

void priority_enqueue(queue_t q, void* element){
    struct node* new = (struct node*)malloc(sizeof(struct node));
    struct node* tmp = q->front;
    request_t request = (request_t)element;
    int size = request->file_size;
    task_t t = (task_t)element;
    new->element = element;
    new->next = NULL;
    while(tmp->next){
        request_t next = (request_t)tmp->next->element;
        if(size < next->file_size){
            new->next = tmp->next;
            tmp->next = new;
            break;
        }
        tmp = tmp->next;
    }
    if(!tmp->next){
        tmp->next = new;
        q->tail = new;
    }
}
void* front(queue_t q){
    if(!q || empty(q)) return NULL;
    return q->front->next->element;
}
void dequeue(queue_t q){
    if(!q || empty(q)) return;
    struct node* tmp = q->front->next;
    //void* re = tmp->element;
   // task_t t = (task_t)re;
    q->front->next = tmp->next;
    if(!q->front->next){
        q->tail = q->front;
    }
    free(tmp);
}

void display(queue_t q){
    struct node* tmp = q->front->next;
    while(tmp){
        request_t re = (request_t)tmp->element;
        printf("%d,",re->file_size);
        tmp = tmp->next;
    }
}
void destroy(queue_t q){
    struct node* p = q->front->next;
    struct node* tmp;


    while(p){
        tmp = p;
        p = p->next;
        free(tmp);
    }
    free(q);
}
