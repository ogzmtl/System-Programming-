#include "common.h"
#include <semaphore.h>
static char clientFifo[CLIENT_FIFO_NAME_LEN];
int main(int argc, char **argv){

    char log[BUFF_SIZE];
    char buffer[BUFF_SIZE];
    int numBytesWrittenLog, numBytesReadFifo,numBytesWriteFifo;
    int clientFdW,clientFdR, clientPid, serverPid, serverFd;
    struct response resp;
    struct request req;
    char clientSem[256];
    sem_t *sem_temp;
    char clientSem2[256];
    sem_t *sem_temp2;

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

    serverFd = open(SERVER_FIFO, O_WRONLY);
    if(serverFd == -1){
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
    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO,(long) clientPid);
    if(mkfifo(clientFifo, 0666) == -1)
    {
        perror("mkfifo error\n");
        close(serverFd);
        unlink(clientFifo);
        return ERR;
    }

    
    req.pid = clientPid;
    req.serverPid = serverPid;

    if(write(serverFd, &req, sizeof(struct request)) != sizeof(struct request)){
        perror("write");
        return ERR;
    }

    if(close(serverFd) == -1){
        //write_to_log
        perror("close");
        return ERR;
    }

    clientFdW = open(clientFifo, O_WRONLY);
    if(clientFdW == -1){
        //write_to_log
        perror("open");
        return ERR;
    }
    clientFdR = open(clientFifo, O_RDONLY);
    if(clientFdR == -1){
        //write_to_log
        perror("open");
        return ERR;
    }
    snprintf(clientSem, CLIENT_SEM_NAME_LEN, CLIENT_SEM_TEMP, (long)getpid());
    sem_temp = sem_open(clientSem, 0);

    snprintf(clientSem2, CLIENT_SEM_NAME_LEN, CLIENT_SEM2_TEMP, (long)getpid());
    sem_temp2 = sem_open(clientSem2, 0);
    // for(;;)
    // {
        memset(buffer, 0, BUFF_SIZE);
        // snprintf(buffer, BUFF_SIZE, ">>Enter command : ");
        // write(STDOUT_FILENO, buffer, BUFF_SIZE);
        // memset(buffer, 0, BUFF_SIZE);
        // read(STDIN_FILENO, buffer, BUFF_SIZE);
        // write(STDOUT_FILENO, buffer, BUFF_SIZE);
        fprintf(stdout, "Enter Command:\n");
        fgets(resp.command,BUFF_SIZE,stdin);
        resp.command[strlen(resp.command)] = '\0';
        if (write(clientFdW, &resp, sizeof(struct response)) != sizeof(struct response)){
            perror("write");
            //may be you can send invalid response signal to server
            // continue;
        }
        
        sem_wait(sem_temp);
        if (read(clientFdR, &resp, sizeof(struct response)) != sizeof(struct response)){
            perror("read");
            //may be you can send invalid response signal to server
            // continue;
        }
        sem_post(sem_temp2);

        fprintf(stdout, "%s\n", resp.command);

        
    // }


    
}