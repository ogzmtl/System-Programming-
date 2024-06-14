#include "queue.h"
 
Queue* createQueue(int capacity) {
    Queue* queue = (Queue*)malloc(sizeof(Queue));

    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;

    queue->data = (Pide*)malloc(queue->capacity * sizeof(Pide));
    if (!queue->data) {
        printf("Memory allocation error\n");
        free(queue);
        return NULL;
    }
    return queue;
}

int isEmpty(Queue* queue) {
    return queue->size == 0;
}

int isFull(Queue* queue) {
    return queue->size == queue->capacity;
}

void enqueue(Queue* queue, Pide item) {
    if (isFull(queue)) {
        return;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->data[queue->rear] = item;
    queue->size++;
}

Pide dequeue(Queue* queue) {
    if (isEmpty(queue)) {
        Pide empty = {0, 0, 0, 0};
        return empty;
    }
    Pide item = queue->data[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return item;
}

Pide front(Queue* queue) {
    if (isEmpty(queue)) {
        Pide empty = {0, 0, 0, 0};
        return empty;
    }
    return queue->data[queue->front];
}

Pide rear(Queue* queue) {
    if (isEmpty(queue)) {
        Pide empty = {0, 0, 0, 0};
        return empty;
    }
    return queue->data[queue->rear];
}

int size(Queue* queue) {
    return queue->size;
}

void destroy(Queue* queue) {
    free(queue->data);
    free(queue);
}