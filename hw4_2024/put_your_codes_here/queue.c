#include "queue.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int findAbsolute(int n) 
{   
    if (n < 0)  
    { 
        n = (-1) * n; 
    } 
    return n;
} 

void init(int len){
    buffer = (Queue*) malloc(sizeof(Queue)*len);
    size = len;
}

int enqueue(Queue element)
{
    if(buffer_size_counter == size){
        return 0;
    }
    memset(buffer[last].src_filename, 0, MAX_FILENAME_LEN);
    memset(buffer[last].dest_filename, 0, MAX_FILENAME_LEN);

    strcpy(buffer[last].src_filename, element.src_filename);
    strcpy(buffer[last].dest_filename, element.dest_filename);

    buffer[last].src_fd= element.src_fd;
    buffer[last].dest_fd= element.dest_fd;
    buffer[last].empty = 0;

    last = (last +1) % size;
    // if(last == size-1) last = 0;
    // else last++;

    return 1;
}

Queue dequeue()
{
    // if(buffer_size_counter == 0){
    //     return;
    // }
    int temp = counter;
    // if(counter == last){
    //     return NULL;
    // }
    counter = (counter +1) % size;
    // if(counter == size-1){
    //     counter = 0;
    // }
    // else counter++;

    // buffer[temp].empty = 1;
    return buffer[temp];
}

void destroy(){
    free(buffer);
    buffer = NULL;
}
