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

void convertStringArrayToInteger(char** str, int count, int*arr){

    for(int i = 0; i < count; i++){
        int val = convertSingleStringToInteger(str[i]);
        if(val < 0){
            free(str);
            free(arr);
            exit(EXIT_FAILURE);
        }
        arr[i] = val;
    }
}

int convertSingleStringToInteger(const char* str){

    char *endptr=NULL;
    int val = strtol(str, &endptr, 10);
    int errno = 0;

    if(errno != 0){
        perror("strtol");
        perror("convertStringArrayToInteger");
    }
    
    if (endptr == str) {
        char err[256]; 
        snprintf(err, 256, "No digits were found\n");
        write(STDOUT_FILENO, err, strlen(err));
        return 1;
    }

    return val;

}

//returnden sonra splitted stringini free yapmadan cikma
int splitStringIntoArray_S(const char* str, const char delim, char** splitted){

    int count = countHowManyElementsWillExtract(str, delim);
    printf("count : %d\n", count);
    return splitStringIntoArray_I(str, delim, splitted, count);
}

int splitStringIntoArray_I(const char* str, const char delim, char** splitted, int count){

    char *tmp = (char*)str;
    // splitted = (char**)malloc(sizeof(char*)*count);
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
        }
        tmp++;
    }
    
    return count;

}

char* splitStringIntoArray_Custom(const char* str, char** splitted, char* compared){

    char *tmp = (char*)str;
    // splitted = (char**)malloc(sizeof(char*)*count);
    int inside_counter = 0;
    char temp_element[32];
    int i = 0;

    memset(temp_element, 0, sizeof(temp_element));
    memset(compared,0,sizeof(char));

    while(*tmp){
        if(*tmp == ',' || *tmp == '!')
        {
            temp_element[inside_counter] = '\0';
            if(*tmp == '!'){
                
                compared = (char*)malloc(sizeof(char) * inside_counter);
                printf("temp element: %s\n", temp_element);
                strcpy(compared, temp_element);
            }
            else{
                splitted[i] = (char*)malloc(sizeof(char)*inside_counter);
                strcpy(splitted[i], temp_element);
                i++;
            }
            // temp_element[inside_counter] = '\0';
            // splitted[i] = (char*)malloc(sizeof(char)*inside_counter);
            // strcpy(splitted[i], temp_element);
            // i++;
            memset(temp_element, 0, sizeof(temp_element));
            inside_counter = 0;
        }
        else{
            temp_element[inside_counter++] =*tmp;
        }
        tmp++;
    }
    
    free(tmp);
    return compared;

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

    int fd = open(firstChildFifo, O_RDONLY);
    if(fd == -1){
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
        numBytesRead = read(fd, &c, sizeof(c));
        if(numBytesRead == 0){
            
            buffer[childBufferCounter] = '\0';
            break;
        }
        else if(numBytesRead == -1){
            perror("read");
            free(buffer);
            close(fd);
            return NULL;
        }
        else{
            if(childBufferCounter+1 % 6 == 0){
                buffer = (char *)realloc(buffer, sizeof(char)*(childBufferCounter*2)); 
            }
            buffer[childBufferCounter++] = c;
        }
    }
    if(close(fd) == -1){
        perror("close");
        free(buffer);
        return NULL;
    }


    write(STDOUT_FILENO, buffer, strlen(buffer));
    return buffer;
}

int check_command(const char** splitted, const char* command, int count){

    int flag=0;
    for(int i = 0; i < count; i++){
        // printf("compare %s: %d\n", splitted[i], strlen(splitted[i]));
        if(strncmp(splitted[i], command, strlen(command)) == 0)
        {
            flag = 1;
        }
    }

    return flag;
}