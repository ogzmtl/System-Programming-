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
    if(isFull()){
        return 0;
    }
    memset(buffer[last].src_filename, 0, MAX_FILENAME_LEN);
    memset(buffer[last].dest_filename, 0, MAX_FILENAME_LEN);

    strcpy(buffer[last].src_filename, element.src_filename);
    strcpy(buffer[last].dest_filename, element.dest_filename);

    buffer[last].src_fd= element.src_fd;
    buffer[last].dest_fd= element.dest_fd;
    buffer[last].empty = 0;

    if(last == size-1) last = 0;
    else last++;

    printf("lasttt %d\n",last);
    return 1;
}

int isEmpty(int last)
{
    if(buffer[last].empty == 0){
        return 0;
    }
    else{
        return 1;
    }
}

Queue* dequeue()
{
    int temp = counter;
    // if(counter == last){
    //     return NULL;
    // }
    if(counter == size-1){
        counter = 0;
    }
    else counter++;

    buffer[temp].empty = 1;
    return &buffer[temp];
}

int isFull(){
    if(last < counter){
        if(findAbsolute(counter - last) == 1) return 1;
        else return 0;
    }
    else{
        if(findAbsolute(counter - last) == size-1) return 1;
        else return 0;
    }

}
void destroy(){
    free(buffer);
}

int currentSize()
{
    if(counter < last)
    {
        return (last - counter) + 1;
    }else{
        if(findAbsolute(last - counter) == size-1) return 1;
        if(findAbsolute(last - counter) == 1) return size; 

        return size - counter + last;
    }
}
