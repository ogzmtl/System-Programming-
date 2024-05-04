#include "common.h"
#include <semaphore.h>
#include <signal.h>
static char clientFifo[CLIENT_FIFO_NAME_LEN];
//consume-> download writeT, writeT uicin yeni bir temp olustur karsi tarafta fork
//produce -> upload karsi tarafta fork 

void handler(int sig)
{
    char semaphore_one[BUFF_SIZE];
    char semaphore_two[BUFF_SIZE];

    snprintf(semaphore_one, CLIENT_SEM_NAME_LEN, CLIENT_SEM_TEMP, (long)getpid());
    snprintf(semaphore_two, CLIENT_SEM_NAME_LEN, CLIENT_SEM2_TEMP, (long)getpid());

    
    if(sig == SIGINT || sig == SIGTERM)
    {
        unlink(clientFifo);
        sem_unlink(semaphore_one);
        sem_unlink(semaphore_two);

    }
    printf("Handler %d you should use quit for proper termiantion\n");
    // exit(EXIT_SUCCESS);
}
int main(int argc, char **argv){

    char log[BUFF_SIZE];
    char buffer[BUFF_SIZE];
    char splitted_command[MAX_ARGUMENT][BUFF_SIZE];
    int numBytesWrittenLog,numBytesReadStdin;
    int clientFdW,clientFdR, clientPid, serverPid, serverFd;
    struct response resp;
    struct request req;
    char clientSem[CLIENT_SEM_NAME_LEN];
    sem_t *sem_temp;
    char clientSem2[CLIENT_SEM_NAME_LEN];
    sem_t *sem_temp2;
    struct sigaction new;
    new.sa_handler = handler;
    sigemptyset(&new.sa_mask);
    new.sa_flags = 0;
    if(sigaction (SIGINT, &new, NULL) == -1){
        perror("sigaction");
        return ERR;
    };

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
        if(write(STDOUT_FILENO, log, numBytesWrittenLog) == -1){
            perror("write error\n");
            return ERR;
        }
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
    memset(clientSem, 0, CLIENT_SEM_NAME_LEN);
    memset(clientSem2, 0, CLIENT_SEM_NAME_LEN);
    snprintf(clientSem, CLIENT_SEM_NAME_LEN, CLIENT_SEM_TEMP, (long)getpid());
    sem_temp = sem_open(clientSem, O_CREAT, 0666, 0);
    if(sem_temp == SEM_FAILED){
        perror("sem_open error\n");
        sem_unlink(clientSem);
    }

    snprintf(clientSem2, CLIENT_SEM_NAME_LEN, CLIENT_SEM2_TEMP, (long)getpid());
    sem_temp2 = sem_open(clientSem2, O_CREAT, 0666, 0);
    if(sem_temp2 == SEM_FAILED){
        perror("sem_open error\n");
        sem_unlink(clientSem2);
    }
    // char logChild[BUFF_SIZE];
    // int numBytesWrittenLogChld = snprintf(logChild, BUFF_SIZE, "client fifo name is %s\n",clientFifo);
    // write(STDOUT_FILENO, logChild,numBytesWrittenLogChld);
    // memset(logChild, 0, numBytesWrittenLogChld);

    if(write(serverFd, &req, sizeof(struct request)) != sizeof(struct request)){
        perror("write");
        close(serverFd);
        unlink(clientFifo);
        //semaphore close
        return ERR;
    }
    sem_t* queue = sem_open(QUEUE_SEM, 0);
    if (queue == SEM_FAILED) {
        perror("sem_open producer");
        exit(1);
    }

    if(close(serverFd) == -1){
        //write_to_log
        perror("close");
        close(serverFd);
        unlink(clientFifo);
         //semaphore close
        return ERR;
    }
    sem_wait(queue);
    numBytesReadStdin = snprintf(buffer, BUFF_SIZE, ">> Waiting for Que.. Connection Established ");
    write(STDOUT_FILENO, buffer, numBytesReadStdin);
    memset(buffer, 0, BUFF_SIZE);
    clientFdW = open(clientFifo, O_WRONLY);
    if(clientFdW == -1){
        //write_to_log
        perror("open");
        close(serverFd);
        unlink(clientFifo);
         //semaphore close
        return ERR;
    }
    clientFdR = open(clientFifo, O_RDONLY);
    if(clientFdR == -1){
        //write_to_log
        perror("open");
        close(serverFd);
        unlink(clientFifo);
         //semaphore close
        return ERR;
    }
    for(;;)
    {
        memset(buffer, 0, BUFF_SIZE);
        memset(resp.command, 0, BUFF_SIZE);
        numBytesReadStdin = snprintf(buffer, BUFF_SIZE, ">>Enter command : ");
        write(STDOUT_FILENO, buffer, numBytesReadStdin);
        memset(buffer, 0, BUFF_SIZE);
        numBytesReadStdin = read(STDIN_FILENO, &buffer, BUFF_SIZE);
        buffer[numBytesReadStdin] = '\0';
        // resp.command[strlen(resp.command)] = '\0';

        // fprintf(stdout, "Enter Command:\n");
        // fgets(buffer,BUFF_SIZE,stdin);
        // resp.command[strlen(resp.command)] = '\0';
        // strcpy(resp.command, buffer);
                // memset(resp.command, 0, BUFF_SIZE);
        strcpy(resp.command, buffer);
        resp.command[strlen(resp.command)-1]= '\0';
        // numBytesReadStdin = snprintf(buffer, BUFF_SIZE, ">>Enter command2 : %s \n",resp.command);
        // write(STDOUT_FILENO, buffer, numBytesReadStdin);
        
        if (write(clientFdW, &resp, sizeof(struct response)) != sizeof(struct response)){
            perror("write");
            //may be you can send invalid response signal to server
            continue;
        }
        splitStringIntoArray_S(resp.command, ' ', splitted_command);
        // memset(resp.command, 0, BUFF_SIZE);
        // sem_wait(sem_temp);
        // numBytesReadStdin = snprintf(buffer, BUFF_SIZE, ">>Enter command3 : ");
        // write(STDOUT_FILENO, buffer, numBytesReadStdin);
        // memset(buffer, 0, BUFF_SIZE);
        sem_post(sem_temp);
        // numBytesReadStdin = snprintf(buffer, BUFF_SIZE, ">>Enter command4 : ");
        // write(STDOUT_FILENO, buffer, numBytesReadStdin);
        // memset(buffer, 0, BUFF_SIZE);
        sem_wait(sem_temp2);
        // sem_post(sem_temp2);
        if (read(clientFdR, &resp, sizeof(struct response)) != sizeof(struct response)){
            perror("read");
            //may be you can send invalid response signal to server
            continue;
        }
        resp.command[strlen(resp.command)] = '\0';

        fprintf(stdout, "%s\n", resp.command);

        if(strncmp(resp.command, "upload", 6) == 0 && strcmp(splitted_command[0], "help") != 0){
            
            producerr(splitted_command[1]);
        }
        else if(strncmp(resp.command, "download", 8)==0 && strcmp(splitted_command[0], "help") != 0)
        {
            consumerrr(splitted_command[1]);
        }

    }


    
}