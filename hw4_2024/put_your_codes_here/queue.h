#ifndef __MY_QUEUE_H
#define __MY_QUEUE_H


#define MAX_FILENAME_LEN 256 
typedef struct my_queue {
    char src_filename[MAX_FILENAME_LEN];
    char dest_filename[MAX_FILENAME_LEN];
    int src_fd; 
    int dest_fd;
    int empty;
}Queue;

extern Queue* buffer;
extern int counter;
extern int last;
extern int size;

void init(int size);
int enqueue(Queue item);
Queue* dequeue();
int findAbsolute(int n);
int isEmpty();
int isFull();
void destroy();
int currentSize();


#endif