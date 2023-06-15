#include "fifo.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <semaphore.h>

sem_t* req_semaphore;
sem_t* req1_semaphore;
sem_t* req2_semaphore;

static char clientFIFO_r[CLIENT_FIFO_TEMP_LEN];
static char clientFIFO_w[CLIENT_FIFO_TEMP_LEN];

int findNewLine(char* command)
{
    int counter = 0;
    while(command[counter] != '\n'){
        counter++;
    }

    return counter;

}
int requestSend(int clientFileDescriptor, struct request* req);
int responseTake(int clientFileDescriptor, struct response* resp);
char* arrange_file_path(char* directory);
void sendResponse(int client_id, const char* request, int max_client, struct response* resp);

int main(int argc, char *argv[])
{

    int serverFd, clientFd_r;
    struct request req;
    struct response resp;
    struct requestMainFifo reqMain;
    char* connect; 
    int server_pid; 

    if (argc != 3)
    {
        return 1;
    }
    int LEN = strlen(argv[1]) + CLIENT_FIFO_TEMP_LEN;

    server_pid = atoi(argv[2]);
    umask(0);

    req_semaphore = sem_open("/req_semaphore", O_CREAT, 0644, 0);
    req1_semaphore = sem_open("/req1_semaphore", O_CREAT, 0644, 0);
    req2_semaphore = sem_open("/req2_semaphore", O_CREAT,0644,1);

    snprintf(clientFIFO_r, CLIENT_FIFO_TEMP_LEN, CLIENT_FIFO_TEMP_READ, (long) getpid());

    if(mkfifo(clientFIFO_r, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST){
        perror("Error creating client fifo");
        return 1;
    }


    snprintf(clientFIFO_w, CLIENT_FIFO_TEMP_LEN, CLIENT_FIFO_TEMP_WRITE, (long) getpid());
    if(mkfifo(clientFIFO_w, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST){
        perror("Error creating client fifo");
        return 1;
    }

    serverFd = open(SERVER_FIFO,O_WRONLY);
    if(serverFd == -1){
        perror("Error opening server fifso");
        return 1;
    }
  
    req.pid = getpid();
    reqMain.inp_pid = atoi(argv[2]);
    reqMain.pid = getpid();

    strncpy(req.data, argv[1], strlen(argv[1])); 
    // strncpy(reqMain.data, argv[1], strlen(argv[1]));

    requestSend(serverFd, &reqMain);
    char stringBuffer[256];
    // int sizeofString = sprintf(stringBuffer, "adsads\n");
    // write(STDOUT_FILENO, stringBuffer, sizeofString);
    int clientFd_w = open(clientFIFO_w,O_WRONLY);
    if(clientFd_w == -1){
        perror("Error opening client fifo");
        return 1;
    }
    //     sizeofString = sprintf(stringBuffer, "xxxx\n");
    // write(STDOUT_FILENO, stringBuffer, sizeofString);
    clientFd_r = open(clientFIFO_r,O_RDONLY);
    if(clientFd_r == -1){
        perror("Error opening client fifo");
        return 1;
    }


    if(close(serverFd) == -1){
        perror("Error closing server fifo");
        return 1;
    }
    // sizeofString = sprintf(stringBuffer, "bbbb\n");
    // write(STDOUT_FILENO, stringBuffer, sizeofString);
    printf(">>Waiting for Que... Connection established.\n");
    fflush(stdout);
    // sizeofString = sprintf(stringBuffer, "ccccc\n");
    // write(STDOUT_FILENO, stringBuffer, sizeofString);
    while (1)
    {

        printf(">>Enter command:");
        fflush(stdout);
        char command[256]; 
        // memset(command, "\0", sizeof(command));
        fgets(command, sizeof(command), stdin);

        command[strcspn(command, "\n")] = '\0';
        strcpy(req.data, command);
        // req.data = command;
        req.pid = (long) getpid();



        requestSend(serverFd, &req);
        // sem_post(req_semaphore);
        // sem_wait(req1_semaphore);
        if(responseTake(clientFd_r, &resp) == sizeof(struct response))
        {              
            char stringBuffer[256]; 
            int sizeofString = sprintf(stringBuffer, resp.data);
            // write(STDOUT_FILENO, stringBuffer, sizeofString);
            sizeofString = sprintf(stringBuffer, resp.data);
            write(STDOUT_FILENO, stringBuffer, sizeofString);
        }

    }
    close(serverFd);
    close(clientFd_r);
    unlink(clientFIFO_r);
    unlink(clientFIFO_w);
    exit(EXIT_SUCCESS);
    return 0;
}
int requestSend(int clientFileDescriptor, struct request* req){
    char stringBuffer[256]; 
    // int sizeofString = sprintf(stringBuffer, "naber1212");
    // write(STDOUT_FILENO, stringBuffer, sizeofString);


    write(clientFileDescriptor, req, sizeof(struct request));

}

int responseTake(int clientFileDescriptor, struct response* resp){
    
    return read(clientFileDescriptor, resp, sizeof(struct response));
}
char* arrange_file_path(char* directory){
    char path[256]; 

    strncpy(path, directory, strlen(directory)-1);
    path[strlen(path)-1] = '/'; 
    
    strcat(path,CLIENT_FIFO_TEMP );
    return path; 
}
