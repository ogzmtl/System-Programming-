#include "fifo.h"
#include <stdio.h>
#include <semaphore.h>
#include <signal.h>
#include <dirent.h>
#include <stdlib.h>

sem_t* req_semaphore;
sem_t* req1_semaphore;
sem_t* req2_semaphore;
#define BUFF_SIZE 256
int responseSend(int clientFileDescriptor, struct response* req);
int requestTake(int clientFileDescriptor, struct request* resp);// vakit kalirsa degisken adini degistir
void handleRequests(struct request* req, struct response* resp, char* dirname);
// void signalHandler(int signal, int& clientNumber);
void quit(struct response* resp);
void list(struct response* resp, char* dirname);
char **split_string(char *str, char delimiter);


int main(int argc, char *argv[]){
    
    int serverFileDescriptor, dummyFileDescriptor, clientFileDescriptor; 
    int oldPid, count = 0; 
    char clientFIFO_read[CLIENT_FIFO_TEMP_LEN];
    char clientFIFO_write[CLIENT_FIFO_TEMP_LEN];
    struct request req;
    struct response resp;
    struct requestMainFifo reqMainFifo;
    struct responseMainFifo respMainFifo;
    int seqNum = 0;
    char dirname[100];
    pid_t pid; 
    int max_clients; 
    int clientNumber= 0; 
    char stringBuffer[512];
    req_semaphore = sem_open("/req_semaphore", O_CREAT, 0);
    req1_semaphore = sem_open("/req1_semaphore", O_CREAT,0);
    req2_semaphore = sem_open("/req2_semaphore", O_CREAT,0);
    
    if(argc != 3){
        // error message
        return 1; 
    }

    max_clients = atoi(argv[2]);

    strcpy(dirname, argv[1]);

    if(mkdir(dirname, 0777) == -1 && errno != EEXIST){
        //error message
        return 1;
    }


    int sizeofString = sprintf(stringBuffer, ">>Server started at PID %d... \n", getpid());
    write(STDOUT_FILENO, stringBuffer, sizeofString);

    sizeofString = sprintf(stringBuffer, CLIENT_FIFO_TEMP, getpid());
    write(STDOUT_FILENO, stringBuffer, sizeofString);


    reqMainFifo.pid = getpid();
    umask(0);

    if(mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST){
        perror("mkfifo");
    }


    int bytesReaded;
    sizeofString = sprintf(stringBuffer, "\n>>waiting for clients... \n");
    write(STDOUT_FILENO, stringBuffer, sizeofString);
    serverFileDescriptor = open(SERVER_FIFO, O_RDONLY);
    if(serverFileDescriptor == -1){
        perror("open");
    }
    dummyFileDescriptor = open(SERVER_FIFO, O_WRONLY);
    if(dummyFileDescriptor == -1){
        perror("open");
    }
    while(1)
    {
        bytesReaded = read(serverFileDescriptor, &reqMainFifo, sizeof(struct requestMainFifo));
        sizeofString = sprintf(stringBuffer, ">> \n");
        write(STDOUT_FILENO, stringBuffer, sizeofString);
        if (clientNumber == 0){
            oldPid = 0;
        }
        if(clientNumber <= max_clients ){
            snprintf(clientFIFO_read, CLIENT_FIFO_TEMP_LEN, CLIENT_FIFO_TEMP_WRITE, (long) reqMainFifo.pid);
            snprintf(clientFIFO_write, CLIENT_FIFO_TEMP_LEN, CLIENT_FIFO_TEMP_READ, (long) reqMainFifo.pid);
            //clientNumber < max_clients && 
            if(oldPid != reqMainFifo.pid && reqMainFifo.inp_pid == getpid()){

                oldPid = reqMainFifo.pid;
                clientNumber++;
                sizeofString = sprintf(stringBuffer, ">>Client PID %d connected as client%2d \n", reqMainFifo.pid,clientNumber);
                write(STDOUT_FILENO, stringBuffer, sizeofString);

                pid = fork();
                switch (pid)
                {
                case -1: 
                    perror("fork");
                    return 1;
                case 0:
                    // sizeofString = sprintf(stringBuffer, "YENDIM", getpid());
                    // write(STDOUT_FILENO, stringBuffer, sizeofString);
                    
                    if(close(serverFileDescriptor) == -1){
                        perror("close");
                        return 1;
                    }
                    if(close(dummyFileDescriptor) == -1){
                        perror("close");
                        return 1;
                    }
                    clientFileDescriptor = open(clientFIFO_read, O_RDONLY);
                    if(clientFileDescriptor == -1){
                        perror("open");
                        return 1;
                    }
                    // sizeofString = sprintf(stringBuffer, "YENDIM", getpid());
                    // write(STDOUT_FILENO, stringBuffer, sizeofString);
                    // snprintf(clientFIFO, CLIENT_FIFO_TEMP_LEN, CLIENT_FIFO_TEMP, (long) req.pid);
                    int clientFileDescriptor_w = open(clientFIFO_write, O_WRONLY);
                    if(clientFileDescriptor_w == -1){
                        perror("open");
                        return 1;
                    }
                    while(1)
                    {
                        // sizeofString = sprintf(stringBuffer, "NABER");
                        // write(STDOUT_FILENO, stringBuffer, sizeofString);
                        if(requestTake(clientFileDescriptor, &req) == sizeof(struct request)){
                            // sizeofString = sprintf(stringBuffer, req.data);
                            // write(STDOUT_FILENO, stringBuffer, sizeofString);
                            handleRequests(&req, &resp, argv[1]);
                            responseSend(clientFileDescriptor_w, &resp);
                            //     exit(-1);
                            // }

                            // sizeofString = sprintf(stringBuffer, "response sending\n");
                            // write(STDOUT_FILENO, stringBuffer, sizeofString);
                        }
                    }
                    return 0;
                    break;
                
                default:
                        // sizeofString = sprintf(stringBuffer, "XXXNABERZZZ");
                        // write(STDOUT_FILENO, stringBuffer, sizeofString);
    
                    break;
                }
                if(clientNumber > max_clients){
                    sizeofString = sprintf(stringBuffer, ">>\Connection request PID %d... Que FULL \n", req.pid);
                    write(STDOUT_FILENO, stringBuffer, sizeofString);
                    wait(NULL);
                }
            }
        }
        else{
            sizeofString = sprintf(stringBuffer, ">>\Connection request PID %d... Que FULL \n", req.pid);
            write(STDOUT_FILENO, stringBuffer, sizeofString);
            wait(NULL);
        }
    }
    unlink(CLIENT_FIFO_TEMP_READ);
    unlink(CLIENT_FIFO_TEMP_WRITE);
    unlink(SERVER_FIFO);
    close(serverFileDescriptor);
    

    return 0;
}

int responseSend(int clientFileDescriptor, struct response* req){
    
    int bytesReaded = write(clientFileDescriptor, req, sizeof(struct response));
    // sem_post(req_semaphore); 
    return bytesReaded;
}

int requestTake(int clientFileDescriptor, struct request* req){
    char stringBuffer[256]; 

    int bytesReaded = read(clientFileDescriptor, req, sizeof(struct request));
    // int sizeofString = sprintf(stringBuffer, "%ld",sizeof(struct request));
    // write(STDOUT_FILENO, stringBuffer, sizeofString);
    
    return bytesReaded;
}

void handleRequests(struct request* req, struct response* resp, char* dirname){
    int sizeofString = 0;
    char stringBuffer[256];
    char *tokens[100];
    int i = 0;
    char cpy[256];

    strcpy(cpy, req->data);
    sizeofString = sprintf(stringBuffer, cpy);
    write(STDOUT_FILENO, stringBuffer, sizeofString);
    tokens[i] = strtok(cpy, " ");
    while( tokens[i] != NULL ) { 
        tokens[++i] = strtok(NULL, " ");
    }
                // sizeofString = sprintf(stringBuffer, "a%s", tokens[1]);
        //  write(STDOUT_FILENO, stringBuffer, sizeofString);
    sizeofString = sprintf(stringBuffer, "%d", i);
    write(STDOUT_FILENO, stringBuffer, sizeofString);
    if(i == 1)
    {
        if(strncmp(req->data, "quit", 5) == 0){
            quit(resp);
            // sizeofString = sprintf(stringBuffer, ">Bye");
            // write(STDOUT_FILENO, stringBuffer, sizeofString);
            strcpy(resp->data, "quit");
        }

        if(strncmp(req->data, "list", 5) == 0)
        {
            // write(STDOUT_FILENO, stringBuffer, sizeofString);
            list(resp, dirname);
        }
        if(strncmp(req->data, "help", 5) == 0)
        {
            // write(STDOUT_FILENO, stringBuffer, sizeofString);
            int bytesWritten;
            char* help = "\tAvailable comments are:\nhelp, list, readF, writeT, upload, download, quit, killServer";
            snprintf(resp->data, BUFF_SIZE, "%s\n", help);
             sizeofString = sprintf(stringBuffer, resp->data);
        }
    }
    if(i == 2)
    {
            // sizeofString = sprintf(stringBuffer, "%s", req->data);
        //  write(STDOUT_FILENO, stringBuffer, sizeofString);
        if(strncmp(req->data, "help readF", 11) == 0)
        {
            // printf("asdasd");
            // fflush(stdout);
            int bytesWrittes;
            char* helpReadF = "readF <file <line#>\n\tdisplay the #th line of the <file>, returns with an\n\t error if <file> does not exists";
            snprintf(resp->data, BUFF_SIZE, "%s\n", helpReadF);
             sizeofString = sprintf(stringBuffer, resp->data);
         write(STDOUT_FILENO, stringBuffer, sizeofString);
        }
        if(strncmp(req->data, "help writeT", 11) == 0)
        {
            // printf("asdasd");
            // fflush(stdout);
            int bytesWrittes;
            char* helpWriteT = "writeT <file <line#> <string>\n\tdequest to write the content of “string” to the #th line the <file>, if the line # is not given\n\t writes to the end of file. If the file does not exists in Servers directory creates and edits the\
file at the same time";
            snprintf(resp->data, BUFF_SIZE, "%s\n", helpWriteT);
             sizeofString = sprintf(stringBuffer, resp->data);
            write(STDOUT_FILENO, stringBuffer, sizeofString);
        }
    }
    
}

void quit(struct response* resp)
{
    int sizeofString = 0;
    char stringBuffer[256];
}
void list(struct response* resp, char* dirname)
{    
    int sizeofString = 0;
    char stringBuffer[256];
    strcat(dirname, "/" );
    DIR *dir = opendir(dirname);

    struct dirent *entry;
    int buffer_size = 256;
    int bytes_written = 0;
            // sizeofString = sprintf(stringBuffer, dirname);
            // write(STDOUT_FILENO, stringBuffer, sizeofString);
    while ((entry = readdir(dir)) != NULL) {
        // Write the file name to the buffer.
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            bytes_written += snprintf(resp->data + bytes_written, buffer_size, "%s\n", entry->d_name);
    }
}

char **split_string(char *str, char delimiter) {
  int i = 0;
  char **tokens = malloc(sizeof(char *) * strlen(str) + 1);
  // Split the string on the delimiter.
  while ((tokens[i] = strtok(str, delimiter)) != NULL) {
    i++;
  }

  return tokens;
}