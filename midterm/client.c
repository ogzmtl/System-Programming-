#include "common.h"

int main(int argc, char **argv){

    char log[BUFF_SIZE];
    char buffer[BUFF_SIZE];
    int numBytesWrittenLog, numBytesReadFifo;
    int clientFd, clientPid, serverPid;
    struct response resp;
    struct request req;

    if(argc != 3){
        numBytesWrittenLog = sprintf(log, "Invalid size of argument(s) %d\nExample command : ./neHosClient <connect/tryConnect> serverPID\nExiting...\n", argc);
        if(writeToLog(log) == ERR)
            return ERR;
        if(write(STDOUT_FILENO, log, numBytesWrittenLog) == -1)
        {
            perror("Write fail");
            return ERR;
        }
        return ERR;
    }
    
    if(strcmp(argv[1],"connect") != SUCCESS && strcmp(argv[1], "tryConnect") != SUCCESS){

        numBytesWrittenLog = sprintf(log, "Valid command is \"connect\" or \"tryConnect\" you entered :%s\nExample command : ./neHosClient <connect/tryConnect> serverPID\nExiting...\n", argv[1]);
        if(writeToLog(log) == ERR)
            return ERR;

        if(write(STDOUT_FILENO, log, numBytesWrittenLog) == -1){
            perror("write");
            return ERR;
        }
        return ERR;
    }
    serverPid = atoi(argv[2]);
    if(serverPid <= 1){
        return ERR;
    }

    clientFd = open(SERVER_FIFO, O_WRONLY);
    if(clientFd == -1){
        numBytesWrittenLog = snprintf(log,BUFF_SIZE, ">> Server fifo open failed...\n");
        // if(write(STDOUT_FILENO, log, numBytesWrittenLog) == -1){
        //     perror("write error\n");
        //     return ERR;
        // }
        perror("open");
        memset(log, 0, BUFF_SIZE);
        return ERR;
    }
    clientPid = getpid(); // according to the man page: These functions are always successful.

    
    req.pid = clientFd;
    req.serverPid = serverPid;

    if(write(clientFd, &req, sizeof(struct request)) != sizeof(struct request)){
        perror("write");
        return ERR;
    }



    
}