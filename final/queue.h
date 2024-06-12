#ifndef __FINAL_QUEUE_H__
#define __FINAL_QUEUE_H__

// #include <limits.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct pide {
    int id;
    float loc_x, loc_y;
    long int mod_time;
    // int cookid;
} Pide;

typedef struct Queue {
    Pide* data;
    int front;
    int rear;
    int size;
    int capacity;
} Queue;

Queue* createQueue(int capacity);
int isEmpty(Queue* queue);
int isFull(Queue* queue);
void enqueue(Queue* queue, Pide item);
Pide dequeue(Queue* queue);
Pide front(Queue* queue);
Pide rear(Queue* queue);
int size(Queue* queue);
void destroy(Queue* queue);
#endif