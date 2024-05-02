#include "common.h"


#define openFlags  O_CREAT | O_RDWR | O_NONBLOCK

int writeToLog(const char*log)
{
    const char *logFile = "log.txt";
    int log_fd; 
    unsigned int mode = S_IRUSR | S_IRGRP | S_IROTH | S_IXGRP | S_IXUSR | S_IXOTH;
    unsigned const int log_length = strlen(log);

    log_fd = open(logFile, openFlags | O_APPEND, mode);
    if(log_fd == -1){
        perror("open logfile");
        return ERR;
    }
    if(write(log_fd, log, log_length) <0){
        //is it necessary to close the log file when error occured
        if(close(log_fd) == -1)
        {
            perror("close log file error");
            return ERR;
        }
        perror("log file write error");
        return ERR;
    }
    if(close(log_fd) == ERR)
    {
        perror("close log file error");
        return ERR;
    }

    return SUCCESS;
}
int splitStringIntoArray_S(const char* str, const char delim, char(*splitted)[BUFF_SIZE]){

    int count = countHowManyElementsWillExtract(str, delim);
    if(count > 4 || count < 1){
        return -1;
    }
    // printf("count : %d\n", count);
    return splitStringIntoArray_I(str, delim, splitted, count);
}

int splitStringIntoArray_I(const char* str, const char delim, char(*splitted)[BUFF_SIZE], int count){

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
            // splitted[i] = (char*)calloc(inside_counter, sizeof(char));
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
    temp_element[inside_counter] = '\0';
    // splitted[i] = (char*)calloc(inside_counter, sizeof(char));
    strcpy(splitted[i], temp_element);
    
    return count;
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
    return count+1;
}

off_t get_file_size(const char *file_path) {
    struct stat st;
    if (stat(file_path, &st) == -1) {
        perror("Error getting file size");
        exit(EXIT_FAILURE);
    }
    return st.st_size;
}