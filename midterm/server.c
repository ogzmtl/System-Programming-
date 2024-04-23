#include "common.h"
#include <semaphore.h>


int main(int argc, char **argv){

    char log[BUFF_SIZE];
    char buffer[BUFF_SIZE];
    char clientFifo[CLIENT_FIFO_NAME_LEN];
    int numBytesWrittenLog, numBytesReadFifo;
    int pid, serverFd, dummyFd, clientCounter=0; 
    struct stat st;
    struct request req;
    struct response resp;
    char clientSem[256];
    sem_t *sem_temp;
    char clientSem2[256];
    sem_t *sem_temp2;

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
        unlink(SERVER_FIFO);
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
        if(req.serverPid != pid)
        {
            numBytesWrittenLog = snprintf(log, BUFF_SIZE, "Wrong server pid %d\n", req.serverPid);
            write(STDOUT_FILENO, log, numBytesWrittenLog);
            memset(log, 0, numBytesWrittenLog);
            //return error to given client fifo
        }
        if(clientCounter < numClients)
        {
            snprintf(clientSem, CLIENT_SEM_NAME_LEN, CLIENT_SEM_TEMP, (long)req.pid);
            sem_temp = sem_open(clientSem, 0);

            snprintf(clientSem2, CLIENT_SEM_NAME_LEN, CLIENT_SEM2_TEMP, (long)req.pid);
            sem_temp2 = sem_open(clientSem2, 0);
            clientCounter++;
            numBytesWrittenLog = snprintf(buffer,BUFF_SIZE,">>Client PID %d connected as client%2d \n", req.pid,clientCounter);
            write(STDOUT_FILENO, buffer, numBytesWrittenLog);
            memset(buffer, 0, numBytesWrittenLog);

            int forkPid = fork();
            switch(forkPid)
            {
                case 0:
                    if(close(serverFd) == -1){
                        perror("close");
                        exit(EXIT_FAILURE);
                    }
                    if(close(dummyFd) == -1){
                        perror("close");
                        exit(EXIT_FAILURE);
                    }

                    char logChild[BUFF_SIZE];
                    int childBufferWritten = 0;
                    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN,CLIENT_FIFO,(long) req.pid);
                    int clientReadFd = open(clientFifo, O_RDONLY);
                    int clientWriteFd  = open(clientFifo, O_WRONLY);

                    // for(;;)
                    // {
                        if(read(clientReadFd, &resp, sizeof(struct response)) != sizeof(struct response))
                        {
                            perror("read");
                            continue;
                        }
                        childBufferWritten = snprintf(logChild, BUFF_SIZE, "Hello from server child response :\n");
                        logChild[childBufferWritten] = '\0';
                        write(STDOUT_FILENO, logChild, childBufferWritten);
                        fflush(stdout);
                        write(STDOUT_FILENO, resp.command, BUFF_SIZE);
                        strcpy(resp.command, logChild);
                        sem_post(sem_temp);
                        write(clientWriteFd, &resp, sizeof(struct response));
                        sem_wait(sem_temp2);
                    // }
                    if(close(clientReadFd) == -1){
                        perror("close");
                        exit(EXIT_FAILURE);
                    }
                    // if(close(clientWriteFd) == -1){
                    //     perror("close");
                    //     exit(EXIT_FAILURE);
                    // }
                    unlink(clientFifo);
                    exit(EXIT_SUCCESS);

                break; 


                case -1:
                    perror("fork");
                break;

                default: 
                break;
            }
        }
        
    }
    unlink(clientFifo);
    
}