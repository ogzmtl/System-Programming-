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
#include <signal.h>

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
    int num_written  = sprintf(buffer, "%d", sum);
    buffer[num_written] = delim;
    buffer[num_written + 1] = '\0';
}

long int multiplyValuesInInteger(long int* convertedArray, int size){
    long int result = 1;
    for (int i = 0; i < size; ++i) {
        result *= convertedArray[i];
    }
    return result;
}
int multiplyOperation(long int* convertedArray, int count){

    long int result = multiplyValuesInInteger(convertedArray, count);
    return result;

}

//return value must be error code 
int writeToFirstFifo(const char* str, int fd){


    int numBytesWritten = write(fd, str, strlen(str)+1);
    if(numBytesWritten == -1 )
    {
        perror("write failed\n");
        return -1;
    }
    return 1;
}

int parseSecondfifo(const char* buffer, const char delim){
    // char** splitted;
    char **splitted;
    long int convertedArray[32];
    char *endptr = NULL;
    int flag = 0;
    char* convertedArrayStr = NULL;
    int count = countHowManyElementsWillExtract(buffer, delim);
    // printf("Count of elements: %d\n", count);
    splitted = (char**)calloc(count, sizeof(char*));
    if (splitted == NULL) {
        printf("Memory allocation failed!\n");
        free(convertedArrayStr);
        return -1;
    }

    convertedArrayStr = splitStringIntoArray_Custom(buffer, splitted);

    for(int i = 0; i < count-1; i++){
        if(strncmp(splitted[i], "multiply", 8) == 0){
            flag = 1;
        }
        convertedArray[i] = convertSingleStringToInteger(splitted[i]);
    }
    if(strncmp(splitted[count-1], "multiply", 8) == 0){
        flag = 1;
    }
    if(flag == 0){
        free(convertedArrayStr);
        // free(convertedArray);
        for(int i = 0; i < count; i++){
            free(splitted[i]);
        }
        free(splitted);
        return -1;
    }

    long int res =  multiplyValuesInInteger(convertedArray, count-1);

    int addRes = strtol(convertedArrayStr, &endptr, 10);
    // printf("HIIII5\n");
    printf("Multiplication result: %ld\n", res);
    printf("Addition result: %d\n", addRes);
    printf("Total result: %ld\n", res + addRes);

    for(int i = 0; i < count; i++){
        free(splitted[i]);
    }
    free(convertedArrayStr);
    // free(convertedArray);
    free(splitted);
    return res+addRes;
}

int parseFifo(const char* buffer, const char delim){

    char** splitted;
    int* convertedArray;
    int count = countHowManyElementsWillExtract(buffer, delim);
    splitted = (char**)calloc(count, sizeof(char*));
    if(splitted == NULL){
        return -1;
    }

    convertedArray = (int*)calloc(count,sizeof(int));
    if(convertedArray == NULL){
        return -1;
    }
    // printf("successfuly done malloc 215\n");

    splitStringIntoArray_I(buffer, delim, splitted, count);
    convertStringArrayToInteger(splitted, count, convertedArray);

    int sum = addValuesInInteger(convertedArray, count);
    
    for(int i = 0; i < count; i++){
        free(splitted[i]);
    }
    free(splitted);
    free(convertedArray);
    return sum;
}
static int spawnedProcess=2;
static int signalCnt;
static void sigchldHandler(int sign){
    int status, savedErrno;
    int childpid;

    savedErrno = errno;

    sigset_t mask, prevMask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &prevMask);
    savedErrno = errno;
    while((childpid = waitpid(-1, &status, WNOHANG)) > 0){
        printf("handler: Reaped child %ld\n", (long) childpid);
        signalCnt++;
        spawnedProcess--;
    }
    if (childpid == -1 && errno != ECHILD)
        perror("waitpid");
    if(sign == SIGINT){
        printf("SIGINT handled... Your operation will done after process done\n");
        unlink(FIRST_CHILD_FIFO);
        unlink(SEC_CHILD_FIFO);
    }
    sigprocmask(SIG_SETMASK, &prevMask, NULL);
    errno = savedErrno;
}


int main(int argc, char **argv)
{
    struct sigaction sa;
    sigset_t prevMask, blockMask;
    // int sig;
    int bufferLen = 256;
    char buffer[bufferLen];
    char log_buffer[bufferLen];

    int logSize = 0;

    
    if(argc != 2){
        char err[256] = "Invalid argument number, must be 2.\n"; 
        write(STDOUT_FILENO, err, strlen(err));
        memset(err, 0, sizeof(err));
        exit(EXIT_FAILURE);
    }

    int argument = atoi(argv[1]);

    if(argument <= 0){
        char err[256] = "Argument must be greater than 0.\n";
        write(STDOUT_FILENO, err, strlen(err)); 
        memset(err, 0, sizeof(err));
        exit(EXIT_FAILURE);
    }

    
    sigemptyset(&blockMask);
    sigaddset(&blockMask, SIGINT);


    if(sigprocmask(SIG_BLOCK, &blockMask, &prevMask) == -1){
        exit(EXIT_FAILURE);
    }
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigchldHandler;
    
    //EEXIST: eger zaten boyle bir dosya ismi var ise
    if(sigaction(SIGCHLD, &sa, NULL) == -1 ){
        exit(EXIT_FAILURE);
    }

    if(mkfifo(FIRST_CHILD_FIFO, 0666) == -1){
        perror("mkfifo failed\n");
        unlink(FIRST_CHILD_FIFO);
        return -1;
    }

    if(mkfifo(SEC_CHILD_FIFO, 0666) == -1){
        perror("second mkfifo failed");
        unlink(FIRST_CHILD_FIFO);
        unlink(SEC_CHILD_FIFO);
        return -1;
    }



    int pid = fork();
    switch(pid)
    {
        case 0 :
        spawnedProcess++;
            sleep(10);
            int childPid = getpid();//error check
            int log_written = sprintf(log_buffer, "Entered Child Process 1. (pid : %d)\n", childPid);
            char* childBuf;
            write(STDOUT_FILENO, &log_buffer, log_written);
            memset(log_buffer, 0, log_written);

            childBuf = readOneByOne(FIRST_CHILD_FIFO);
            int sum = parseFifo(childBuf, ',');

            // int numBytes = write(STDOUT_FILENO, childBuf, strlen(childBuf));
            int numBytes;
  
            memset(childBuf, 0, strlen(childBuf));
            intoStringWithDelim(sum, childBuf, '!');
            numBytes = sprintf(log_buffer, "\nSum (exlamation ended): %s", childBuf);
            write(STDOUT_FILENO, log_buffer, numBytes);
            memset(log_buffer, 0, strlen(log_buffer));
            // free(childBuf);

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
            numBytes = sprintf(log_buffer, "Successfully write to first child to second fifo file\n");
            write(STDOUT_FILENO, log_buffer, numBytes);
            memset(log_buffer, 0, strlen(log_buffer));

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
                r = rand() % 10;
                randomIntegerArray[i] = r;
            }

            char* str = convertIntegerToString(randomIntegerArray, ',', argument); //you can change
            char logBuffer[bufferLen]; 
            int logLen = sprintf(logBuffer, "Random values generated (ends with comma): %s\n",str);
            write(STDOUT_FILENO, logBuffer, logLen);//error
            memset(logBuffer, 0, logLen);

            int firstFifoFd = open(FIRST_CHILD_FIFO, O_WRONLY);
            if(firstFifoFd == -1 && errno == ENXIO){
                perror("open failed");
                free(str);
                break;//not sure

            }
            if(writeToFirstFifo(str, firstFifoFd) == -1)
            {
                perror("write \n");
                free(str);
                break;
            }

            int written_size = sprintf(buffer, "Array successfully written to first fifo ");
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

            }
            // //fork
            int secPid = fork();
            switch(secPid){
                case 0:
                //child2
                spawnedProcess++;
                    sleep(10);
                    // char c; 
                    char* childBuff = NULL; 
                    int pid2= getpid();
                    int log_written = sprintf(log_buffer, "\nEntered Child Process 2. (pid : %d)\n", pid2);
                    write(STDOUT_FILENO, &log_buffer, log_written);
                    memset(log_buffer, 0, log_written);

                    childBuff = readOneByOne_Custom(SEC_CHILD_FIFO, ',', '!');

                    if(parseSecondfifo(childBuff, ',')== -1){
                        free(childBuff);
                        exit(EXIT_FAILURE);
                    }
                    
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

            sigemptyset(&prevMask);
            for(;;)
            {
                logSize = sprintf(log_buffer, "Proceeding...\n");
                write(STDOUT_FILENO, log_buffer, logSize);
                memset(log_buffer, 0, logSize);
                if(spawnedProcess==0){
                    break;
                }
                if (sigsuspend(&prevMask) == -1 && errno != EINTR)
                    perror("sigsuspend");
                sleep(2);
                if(spawnedProcess == signalCnt){
                    break;
                }
            }

            printf("All childs are dead... Terminating...Bye\n");
            free(str);
            
            // if(close(sec_fifo_fd) == -1){
            //     perror("close fifo failed\n");
            //     free(str);
            //     break;
            // }

        break;
    }
    unlink(FIRST_CHILD_FIFO);
    unlink(SEC_CHILD_FIFO);

    return 0;
}

