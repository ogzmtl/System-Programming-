#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"
#include <sys/wait.h>

#define FIRST_CHILD_FIFO "/tmp/firstFifo"
#define SEC_CHILD_FIFO "/tmp/secFifo"


int addValuesInInteger(int* convertedArray, int size){
    int sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += convertedArray[i];
    }
    return sum;
}

void intoStringWithDelim(int sum, char* buffer, const char delim){
    sprintf(buffer, "%d", sum);
    
    int len = 0;
    while (buffer[len] != '\0') {
        len++;
    }
        buffer[len] = delim;
    buffer[len + 1] = '\0';
}
// char* readOneByOne(const char* firstChildFifo){
    
//     int numBytesRead = 0;
//     int childBufferCounter = 0;
//     char* buffer = (char*)malloc(sizeof(char)*6);
//     // char buffer[256];
//     memset(buffer, 0, 6);

//     int fd = open(firstChildFifo, O_RDONLY);
//     if(fd == -1){
//         perror("open failed");
//         free(buffer);
//         exit(EXIT_FAILURE);
//     }
//     for(;;)
//     {
//         char c; 
//         char log[256];

//         memset(log, 0, sizeof(log));
//         fflush(stdout);
//         numBytesRead = read(fd, &c, sizeof(c));

//         if(numBytesRead == 0){
            
//             buffer[childBufferCounter] = '\0';
//             break;
//         }
//         else if(numBytesRead == -1){
//             perror("read");
//             free(buffer);
//             return NULL;
//         }
//         else{
//             if(childBufferCounter+1 % 6 == 0){
//                 buffer = (char *)realloc(buffer, sizeof(char)*(childBufferCounter*2)); 
//             }
//             buffer[childBufferCounter++] = c;
//         }
//     }
//     if(close(fd) == -1){
//         perror("close");
//         free(buffer);
//         return NULL;
//     }


//     write(STDOUT_FILENO, buffer, strlen(buffer));
//     return buffer;
// }
long int multiplyValuesInInteger(int* convertedArray, int size){
    long int result = 1;
    for (int i = 0; i < size; ++i) {
        result *= convertedArray[i];
    }
    return result;
}
int multiplyOperation(int* convertedArray, int count){


    long int result = multiplyValuesInInteger(convertedArray, count);
    return result;

}

//return value must be error code 
int writeToFirstFifo(const char* str, int fd){


    int numBytesWritten = write(fd, str, strlen(str)+1);
    if(numBytesWritten == -1 )
    {
        perror("write failed");
        return -1;
        //fail oldugunda error code ver
    }
    return 1;
}

// void parseAndCalculate(const char* input) {
//     int multiplicationResult = 1;
//     int additionResult = 0;
//     char* token;
//     char* tempInput = strdup(input); // Make a copy of the input string to tokenize

//     // Parse the input string
//     token = strtok(tempInput, ",");
//     while (token != NULL) {
//         if (strcmp(token, "multiply") == 0) {
//             // Perform multiplication
//             token = strtok(NULL, ",");
//             while (token != NULL && token[0] != '!') {
//                 printf("multiply token: %s\n",token);
//                 multiplicationResult *= atoi(token);
//                 token = strtok(NULL, ",");
//             }
//         } else {
//             // Perform addition
//             additionResult += atoi(token);
//             token = strtok(NULL, ",");
//         }
//     }

//     // Free allocated memory
//     free(tempInput);

//     // Print the results
//     printf("Multiplication result: %d\n", multiplicationResult);
//     printf("Addition result: %d\n", additionResult);
//     printf("Total result: %d\n", multiplicationResult + additionResult);
// }

int parseSecondfifo(const char* buffer, const char delim){
    char** splitted;
    int* convertedArray;
    char *endptr = NULL;
    int flag = 0;
    char* convertedArrayStr= NULL;
    int count = countHowManyElementsWillExtract(buffer, delim);
    // printf("ARRAY COUNT %d \n", count);
    splitted = (char**)malloc(sizeof(char*)*count);
    convertedArrayStr = splitStringIntoArray_Custom(buffer, splitted,convertedArrayStr);

    convertedArray = (int*)malloc(sizeof(int)*count);
    for(int i = 0; i < count-1; i++){
        if(strncmp(splitted[count-1], "multiply", 8) == 0){
            flag = 1;
        }
        // printf("ss necwcc: %d\n",strlen( splitted[i]));
        convertedArray[i] = convertSingleStringToInteger(splitted[i]);
    }
    if(flag == 0){
        free(convertedArrayStr);
        free(convertedArray);
        for(int i = 0; i < count+1; i++){
            free(splitted[i]);
        }
        free(splitted);
        return -1;
    }
    // for(int i = 0; i < count-1; i++){

    //     printf("sas necwcc: %d\n",convertedArray[i]);
    // }
    long int res =  multiplyValuesInInteger(convertedArray, count-1);
    int addRes = strtol(convertedArrayStr, &endptr, 10);
    printf("Multiplication result: %ld\n", res);
    printf("Addition result: %d\n", addRes);
    printf("Total result: %ld\n", res + addRes);

    for(int i = 0; i < count+1; i++){
        free(splitted[i]);
    }
    free(convertedArrayStr);
    free(convertedArray);
    free(splitted);
    return res+addRes;
}

int parseFifo(const char* buffer, const char delim){

    char** splitted;
    int* convertedArray;
    int count = countHowManyElementsWillExtract(buffer, delim);
    splitted = (char**)malloc(sizeof(char*)*count);
    convertedArray = (int*)malloc(sizeof(int)*count);

    splitStringIntoArray_I(buffer, delim, splitted, count);
    convertStringArrayToInteger(splitted, count, convertedArray);
    // for(int i = 0; i < count; i++){

    //     printf("hg int : %d\n", convertedArray[i]);
    // }

    int sum = addValuesInInteger(convertedArray, count);
    
    for(int i = 0; i < count; i++){
        free(splitted[i]);
    }
    free(splitted);
    free(convertedArray);
    return sum;
}


int main(int argc, char **argv)
{
    int bufferLen = 256;
    char buffer[bufferLen];
    char log_buffer[bufferLen];
    // char err[256]; 
    
    if(argc != 2){
        char err[256] = "Invalid argument number, must be 2.\n"; 
        write(STDOUT_FILENO, err, strlen(err));
        memset(err, 0, sizeof(err));
        exit(EXIT_FAILURE);
    }

    int argument = atoi(argv[1]);
    int sizeofprint = snprintf(buffer, sizeof(buffer), "%d", argument);
    write(STDOUT_FILENO, buffer, sizeofprint);

    if(argument < 0){
        char err[256] = "Argument must be greater than 0.\n";
        write(STDOUT_FILENO, err, strlen(err)); 
        memset(err, 0, sizeof(err));
        exit(EXIT_FAILURE);
    }

    // fflush(STDOUT_FILENO);

    //EEXIST: eger zaten boyle bir dosya ismi var ise
    if(mkfifo(FIRST_CHILD_FIFO, 0666) == -1){
        perror("mkfifo failed\n");
        unlink(FIRST_CHILD_FIFO);
        return -1;
    }

    if(mkfifo(SEC_CHILD_FIFO, 0666) == -1){
        // char err[256]= "mkfifo failed\n";
        // write(STDOUT_FILENO, err, strlen(err));
        // memset(err, 0, sizeof(err));
        perror("second mkfifo failed");
        unlink(FIRST_CHILD_FIFO);
        unlink(SEC_CHILD_FIFO);
        return -1;
    }

    int pid = fork();
    switch(pid)
    {
        case 0 :
            sleep(1);
            // int childBufferCounter = 0;
            int childPid = getpid();//error check
            int log_written = sprintf(log_buffer, "Entered Child Process 1 : %d\n", childPid);
            char* childBuf;
            write(STDOUT_FILENO, &log_buffer, log_written);
            memset(log_buffer, 0, log_written);

            childBuf = readOneByOne(FIRST_CHILD_FIFO);
            int sum = parseFifo(childBuf, ',');

            int numBytes = write(STDOUT_FILENO, childBuf, strlen(childBuf));
            if(numBytes == -1){
                perror("write failed\n");
                memset(childBuf, 0, strlen(childBuf));
                free(childBuf);
                exit(EXIT_FAILURE);
            }
            memset(childBuf, 0, strlen(childBuf));
            intoStringWithDelim(sum, childBuf, '!');
            // free(childBuf);

            //open wronly fifo2
            int fifo2 = open(SEC_CHILD_FIFO, O_WRONLY);
            if(fifo2 == -1){
                perror("open failed\n");
                memset(childBuf, 0, strlen(childBuf));
                free(childBuf);
                exit(EXIT_FAILURE);
            }

            numBytes = write(fifo2, childBuf, strlen(childBuf));
            if(numBytes == -1){
                perror("write failed\n");
                close(fifo2);
                memset(childBuf, 0, strlen(childBuf));
                free(childBuf);
                exit(EXIT_FAILURE);
            }

            if(close(fifo2) == -1){
                perror("close failed");
                memset(childBuf, 0, strlen(childBuf));
                free(childBuf);
                exit(EXIT_FAILURE);
            }
           
            memset(childBuf, 0, strlen(childBuf));
            free(childBuf);
            exit(EXIT_SUCCESS);
        break; 

        case -1:
            perror("fork failed");
        break;

        default :
        //According to mkfifo man page, file open in both ends, so open is in blocking mode.
            srand(time(NULL));
            int r; 
            int randomIntegerArray[argument];
            for(int i = 0; i < argument; i++){
                r = rand() % 100;
                randomIntegerArray[i] = r;
            }

            // for(int i = 0; i < argument; i++){
            //     printf("%d,", randomIntegerArray[i]);
            // }
            char* str = convertIntegerToString(randomIntegerArray, ',', argument); //you can change
            char logBuffer[bufferLen]; 
            int logLen = sprintf(logBuffer, "Random values(ends with comma): %s\n",str);
            write(STDOUT_FILENO, logBuffer, logLen);//error
            memset(logBuffer, 0, logLen);
            //randomu burda uretmek gerek en basta uretirsen child erisebilir 
            int firstFifoFd = open(FIRST_CHILD_FIFO, O_WRONLY);
            if(firstFifoFd == -1 && errno == ENXIO){
                perror("open failed");
                free(str);
                break;//not sure
                //fail oldugunda error code ver
            }
            if(writeToFirstFifo(str, firstFifoFd) == -1)
            {
                perror("write \n");
                free(str);
                break;
            }

            int written_size = sprintf(buffer, "Array successfully written to first fifo\n");
            write(STDOUT_FILENO, buffer, written_size);
            memset(buffer, 0, written_size);

            if(close(firstFifoFd) == -1){
                perror("close failed");
                free(str);
                if(unlink(FIRST_CHILD_FIFO) == -1){
                    perror("unlink error");
                    exit(EXIT_FAILURE);
                }
                // exit(EXIT_FAILURE);
                break;
                //fail oldugunda error code ver
            }
            //fork
            int secPid = fork();
            switch(secPid){
                case 0:
                //child2
                    sleep(1);
                    // char c; 
                    char* childBuff = NULL; 
                    int fifo = open(SEC_CHILD_FIFO, O_RDONLY);
                    if(fifo == -1){
                        perror("open failed");
                        free(childBuff);
                        exit(EXIT_FAILURE);
                    }

                    childBuff = readOneByOne(SEC_CHILD_FIFO);
                    printf("I Read From Fifo : \n");
                    int numBytesSecChild = write(STDOUT_FILENO, childBuff, strlen(childBuff));
                    if(numBytesSecChild == -1){
                        perror("write failed (parent to child2)\n");
                        free(childBuff);
                        close(fifo);
                        exit(EXIT_FAILURE);
                    }

                    if(close(fifo) == -1){
                        perror("close failed in fifo 2\n");
                        free(childBuff);
                        exit(EXIT_FAILURE);
                    }

                    parseSecondfifo(childBuff, ',');
                    
                    free(childBuff);
                    exit(EXIT_SUCCESS);
                break; 

                case -1:
                    perror("second fork failed\n");
                break;

                default:
                    str = (char *)realloc(str, sizeof(char)*(strlen(str)+10));
                    char multip[10] = "multiply,";
                    int sizeBuffer = strlen(str);
                    
                    for(int i =0; i < 9; i++){
                        str[sizeBuffer+i] = multip[i];
                    }
                    str[sizeBuffer+9] = '\0';

                    printf("I Write to fifo %s\n", str);
                    int fdSecFifo = open(SEC_CHILD_FIFO, O_WRONLY);
                    if(fdSecFifo == -1){
                        perror("open failed in (fifo2) \n");
                        free(str);
                        break;
                    }
                    int numBytesWrittenSec = write(fdSecFifo, str,strlen(str));
                    if(numBytesWrittenSec == -1){
                        perror("write to second fifo failed\n");
                        free(str);
                        break;
                    }

                    if(close(fdSecFifo) == -1){
                        perror("close fifo failed\n");
                        free(str);
                        break;
                    }
                    // free(str);              
                break;
            }
            // for(;;){
            //     sleep(2);
            //     //write()
            // }
            //sleep 
            //waitpid koy
            //wronly fifo2
            wait(NULL);
            printf("child exited\n");
            free(str);

        break;
    }
    unlink(FIRST_CHILD_FIFO);
    unlink(SEC_CHILD_FIFO);

    return 0;
}

