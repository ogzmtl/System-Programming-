#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char* convertIntegerToString(const int* arr, const char delim, const int arr_size){

    int total_len=0; 

    for(int i = 0; i < arr_size; i++){
        
        //snprintf yazilacak toplam lengthi ifade etmektedir +1 eklenmesinin sebebi de delimeter icindir
        int num_bytes_written = snprintf(NULL, 0,"%d",arr[i]);
        total_len += num_bytes_written+1;
    }

    total_len+=2;

    char* str = (char*)malloc(total_len*sizeof(char));
    int point = 0;


    for(int i = 0; i < arr_size; i++){
        
        //snprintf yazilacak toplam lengthi ifade etmektedir +1 eklenmesinin sebebi de delimeter icindir
        int num_bytes_written = snprintf(str+point,total_len-point,"%d",arr[i]);
        if(num_bytes_written < 0 || num_bytes_written > total_len - point)
        {
            exit(EXIT_FAILURE);
        }
        point += num_bytes_written;
        if (i < arr_size - 1) {
            str[point] = delim;
            point++;
        }
    }

    str[point++] = delim;
    str[point] = '\0';
    return str;
}

int* convertStringArrayToInteger(char** str){

    int array_size = sizeof(str);
    int *arr = (int*)malloc(sizeof(int)*array_size);

    for(int i = 0; i < array_size; i++){
        int val = convertSingleStringToInteger(str[i]);
        if(val < 0){
            exit(EXIT_FAILURE);
        }
        arr[i] = val;
    }

    return arr;
}

int convertSingleStringToInteger(char* str){

    char *endptr;
    int val = strtol(str, &endptr, 10);
    int errno = 0;

    if(errno != 0){
        perror("strtol");
    }
    
    if (endptr == str) {
        char err[256]; 
        snprintf(err, 256, "No digits were found\n");
        write(STDOUT_FILENO, err, strlen(err));
        return -1;
    }

    return val;

}

//returnden sonra splitted stringini free yapmadan cikma
void splitStringIntoArray_S(const char* str, const char delim, char** splitted){

    int count = countHowManyElementsWillExtract(str, delim);
    printf("count : %d\n", count);
    splitStringIntoArray_I(str, delim, splitted, count);
}

void splitStringIntoArray_I(const char* str, const char delim, char** splitted, int count){

    char *tmp = (char*)str;
    splitted = (char**)malloc(sizeof(char*)*count);
    int inside_counter = 0;
    char temp_element[32];
    int i = 0;

    memset(temp_element, 0, sizeof(temp_element));

    while(*tmp){
        if(*tmp == delim)
        {
            temp_element[inside_counter] = '\0';
            splitted[i] = (char*)malloc(sizeof(char)*inside_counter);
            strcpy(splitted[i], temp_element);
            i++;
            memset(temp_element, 0, sizeof(temp_element));
            inside_counter = 0;
        }
        else{
            temp_element[inside_counter++] =*tmp;
            tmp++;
        }
    }

}

int countHowManyElementsWillExtract(const char *str, const char delim){

    char* temp = (char *)str;
    int count = 0;

    while(*temp){

        if(delim == *temp){
            count++;
        }
        temp++;
    }
    return count;
}

char* readOneByOne(const char* firstChildFifo){

    
    int numBytesRead = 0;
    int childBufferCounter = 0;
    char* buffer = (char*)malloc(sizeof(char)*6);
    // char buffer[256];
    memset(buffer, 0, 6);

    int firstFifoFd = open(firstChildFifo, O_RDONLY);
    if(firstFifoFd == -1){
        perror("open failed");
        free(buffer);
        exit(EXIT_FAILURE);
    }
    for(;;)
    {
        char c; 
        char log[256];

        memset(log, 0, sizeof(log));
        fflush(stdout);
        numBytesRead = read(firstFifoFd, &c, sizeof(c));
        if(numBytesRead == 0){
            
            buffer[childBufferCounter] = '\0';
            break;
        }
        else if(numBytesRead == -1){
            perror("read");
            free(buffer);
            return NULL;
        }
        else{
            if(childBufferCounter+1 % 6 == 0){
                buffer = (char *)realloc(buffer, sizeof(char)*(childBufferCounter*2)); 
            }
            buffer[childBufferCounter++] = c;
        }
    }
    if(close(firstFifoFd) == -1){
        perror("close");
        free(buffer);
        return NULL;
    }


    write(STDOUT_FILENO, buffer, strlen(buffer));
    return buffer;
}