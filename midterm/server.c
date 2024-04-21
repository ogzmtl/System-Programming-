#include "common.h"


int main(int argc, char **argv){

    char log[BUFF_SIZE];
    char buffer[BUFF_SIZE];
    char clientFifo[CLIENT_FIFO_NAME_LEN];
    int numBytesWrittenLog, numBytesReadFifo;
    int pid, serverFd, dummyFd; 
    struct stat st;
    struct request req;
    struct response resp;
    
    //log dosyasina bak hata oldugunda open hatasi aliyorsun
    if(argc != 3){
        numBytesWrittenLog = sprintf(log, "Invalid size of argument(s) %d\nExample command : ./neHosServer Here #ofClients\nExiting...\n", argc);
        if(writeToLog(log) == ERR){
            memset(log, 0, sizeof(numBytesWrittenLog));
            return ERR;
        }

        if(write(STDOUT_FILENO, log, numBytesWrittenLog) == -1)
        {
            perror("Write fail");
            memset(log, 0, sizeof(numBytesWrittenLog));
            return ERR;
        }
        memset(log, 0, sizeof(numBytesWrittenLog));
        return ERR;
    }
    
    int numClients = atoi(argv[2]);
    if(numClients <= 0){
        numBytesWrittenLog = sprintf(log, "#of Clients must be greater than 0! You entered : %d\nExiting...\n",numClients );
        if(writeToLog(log) == ERR){
            memset(log, 0, sizeof(numBytesWrittenLog));
            return ERR;
        }

        if(write(STDOUT_FILENO, log, numBytesWrittenLog) == -1)
        {
            perror("Write fail");
            memset(log, 0, sizeof(numBytesWrittenLog));
            return ERR;
        }   
        memset(log, 0, sizeof(numBytesWrittenLog));
        return ERR;
    }

    if(stat(argv[1], &st) == -1)
    {
        if(mkdir(argv[1], 0770) == -1){
            numBytesWrittenLog = sprintf(log, "mkdir syscall error\n");
            if(writeToLog(log) == ERR){
                memset(log, 0, sizeof(numBytesWrittenLog));
                return ERR;
            }

            if(write(STDOUT_FILENO, log, numBytesWrittenLog) == -1){
                perror("write syscall error\n");
                memset(log, 0, sizeof(numBytesWrittenLog));
                return ERR;
            }
            memset(log, 0, sizeof(numBytesWrittenLog));
            return ERR;
        }
    }
    if(mkfifo(SERVER_FIFO,0666) == -1 && errno == EEXIST)
    {
        numBytesWrittenLog = sprintf(log, "mkfifo %s\n", SERVER_FIFO);
        if(write(STDOUT_FILENO, log, numBytesWrittenLog) == -1){
            perror("write syscall error\n");
            memset(log, 0, sizeof(numBytesWrittenLog));
            return ERR;
        }
        memset(log, 0, sizeof(numBytesWrittenLog));
        perror("mkfifo error");
    }

    pid = getpid(); // according to the man page: These functions are always successful.

    numBytesWrittenLog = snprintf(log,BUFF_SIZE, ">> Server started PID %d...\n>>waiting for clients...\n",pid);
    if(write(STDOUT_FILENO, log, numBytesWrittenLog) == -1){
        perror("write error\n");
        if(unlink(SERVER_FIFO) == -1){
            perror("unlink");
        }
        return ERR;
    }
    serverFd = open(SERVER_FIFO, O_RDONLY);
    if(serverFd == -1)
    {
        numBytesWrittenLog = snprintf(log, BUFF_SIZE, ">>Server fifo open failed %s\n",SERVER_FIFO);
        if(write(STDOUT_FILENO, log, numBytesWrittenLog) == -1){
            perror("write error\n");
            if(unlink(SERVER_FIFO) == -1){
                perror("unlink");
            }
            memset(log, 0, BUFF_SIZE);
            return ERR;
        }
    }
    printf("Gelmemesi lazim \n");
    dummyFd = open(SERVER_FIFO, O_WRONLY);
    if (dummyFd == -1){
        perror("open\n");
    }

    for(;;)
    {
        if(read(serverFd, &req, sizeof(struct request)) != sizeof(struct request)){
            perror("read \n");
            return ERR;
        }
    }
    
}