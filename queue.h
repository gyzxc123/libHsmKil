#ifndef QUEUE_H_HWLAYER
#define QUEUE_H_HWLAYER

#include <stdio.h>
#include <stdlib.h>

#include "CommonInclude.h"

typedef struct node {
    int item;
    struct node* next;
} node;

/**
 * The Queue struct, contains the pointers that
 * point to first node and last node, the size of the Queue,
 * and the function pointers.
 */
typedef struct queue {
    node* head;
    node* tail;

    void (*push) (struct queue*, int); // add item to tail
    // get item from head and remove it from queue
    int (*pop) (struct queue*);
    // get item from head but keep it in queue
    int (*peek) (struct queue*);
    // display all element in queue
    void (*display) (struct queue*);
    // size of this queue
    int size;
} queue;



void push(queue* queue, int item);
int pop(queue* queue);
int peek(queue* queue);
void display(queue* queue);
queue createQueue ();


#endif /*QUEUE_H_HWLAYER*/
