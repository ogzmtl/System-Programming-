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

#define FIRST_CHILD_FIFO "/tmp/firstFifo"
#define SEC_CHILD_FIFO "/tmp/secFifo"

struct secChildParam{
    int* random;
    char* command;
};

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

void parseFifo(const char* buffer, const char delim){


}


int main(int argc, char **argv)
{
    char buffer[256];
    int bufferLen = 256;
    char log_buffer[256];
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
        char err[128] = "Argument must be greater than 0.\n";
        write(STDOUT_FILENO, err, strlen(err)); 
        memset(err, 0, sizeof(err));
        exit(EXIT_FAILURE);
    }

    // fflush(STDOUT_FILENO);

    //EEXIST: eger zaten boyle bir dosya ismi var ise
    if(mkfifo(FIRST_CHILD_FIFO, 0666) == -1){
        char err[256]= "mkfifo failed\n";
        write(STDOUT_FILENO, err, strlen(err));
        memset(err, 0, sizeof(err));
        perror("mkfifo failed");
        return -1;
    }

    int pid = fork();
    switch(pid)
    {
        case 0 :
            int childBufferCounter = 0;
            int childPid = getpid();
            int log_written = sprintf(log_buffer, "Entered Child Process 1 : %d\n", childPid);
            char* childBuf;
            write(STDOUT_FILENO, &log_buffer, log_written);
            memset(log_buffer, 0, sizeof(log_written));
            // firstFifoFd = open(FIRST_CHILD_FIFO, O_RDONLY);
            // if(firstFifoFd == -1){
            //     perror("open failed");
            //     exit(EXIT_FAILURE);
            // }
            // for(;;){
            //     char c; 
            //     char log[256];
            //     int written = sprintf(log, "HELLO FROM FUNCTION \n");
            //     write(STDOUT_FILENO, log, written);
            //     int numBytesRead = read(firstFifoFd, &c, sizeof(c));
            //     if(numBytesRead == 0){
            //         buffer[childBufferCounter] = '\0';
            //         break;
            //     }
            //     else if(numBytesRead == -1){
            //         perror("read\n");

            //     }
            //     else{
            //         buffer[childBufferCounter++] = c;
            //     }
            // }
            childBuf = readOneByOne(FIRST_CHILD_FIFO);
            // if(close(firstFifoFd) == -1){
            //     perror("close failed");
            //     exit(EXIT_FAILURE);
            // }
            



            // ssize_t numBytesRead = read(firstFifoFd, buffer, sizeof(buffer));
            // parseFifo(buffer, ',');
            int numBytes = write(STDOUT_FILENO, childBuf, strlen(childBuf));
            if(numBytes == -1){
                perror("write failed");
                memset(childBuf, 0, sizeof(childBuf));
                free(childBuf);
                exit(EXIT_FAILURE);
            }
            memset(childBuf, 0, sizeof(childBuf));
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

            for(int i = 0; i < argument; i++){
                printf("%d,", randomIntegerArray[i]);
            }
            char* str = convertIntegerToString(randomIntegerArray, ',', argument);
            printf("aaaaaaaaaa: %s\n", str);

            //randomu burda uretmek gerek en basta uretirsen child erisebilir 
            int firstFifoFd = open(FIRST_CHILD_FIFO, O_WRONLY);
            if(firstFifoFd == -1 && errno == ENXIO){
                perror("open failed");
                //fail oldugunda error code ver
            }
            if(writeToFirstFifo(str, firstFifoFd))
            {
                int written_size = sprintf(buffer, "Array successfully written to first fifo\n");
                write(STDOUT_FILENO, buffer, written_size);
                memset(buffer, 0, written_size);
            }


            memset(buffer, 0, bufferLen);
            // close(firstFifoFd);
            if(close(firstFifoFd) == -1){
                perror("close failed");
                return -1;
                //fail oldugunda error code ver
            }
            wait(NULL);
            printf("child exited\n");
            free(str);

        break;
    }
    unlink(FIRST_CHILD_FIFO);
    unlink(SEC_CHILD_FIFO);

    return 0;
}

