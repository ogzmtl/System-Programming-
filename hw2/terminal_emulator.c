#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>


#define BUFF_SIZE 1024
#define COMMAND 20 

#define MAX_TOKENS 10
#define MAX_TOKEN_LENGTH 20

void tokenize_command(char command[], char tokens[][MAX_TOKEN_LENGTH], int* command_size) {
    int token_count = 0;
    char *token = strtok(command, "|");
    while (token != NULL) {
        strcpy(tokens[token_count], token);
        token_count++;
        token = strtok(NULL, "|");

    }
    *command_size = token_count;
}


void handler()
{
    printf("ctrl+c is not working\n");
}

int main ()
{
  char buff[BUFF_SIZE];
  char tokens[MAX_TOKENS][MAX_TOKEN_LENGTH] = {0};
  int command_num = 0;
  int pfd[20][2]; 
  struct sigaction newact;

  memset(&newact, 0, sizeof(newact));
  newact.sa_handler = &handler;

  if((sigemptyset(&newact.sa_mask)== -1) || (sigaction(SIGINT, &newact, NULL) == -1))
    perror("Failed to install SIGINT signal handler");


//   fd = open("file1.txt", O_CREAT | O_WRONLY, 0777);
//   dup2(fd, STDOUT_FILENO);

  while(1){//sigint sinyali koy q ile kapanacak
    

    int num_of_bytes_read = read(STDIN_FILENO, buff, BUFF_SIZE);
    // tokenize_command(buff,parsed_tokens, &command_num);
    buff[num_of_bytes_read] = '\0';
    if(strncmp(buff, ":q",2) == 0){
        return 0;
    }
    tokenize_command(buff, tokens, &command_num);
    int pipe_num = command_num - 1;
    for (int i = 0; i < pipe_num; i++) {
        if (pipe(pfd[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }
    for (int i = 0; i < command_num; i++) {
        // if (strncmp(tokens[i], "|", 2) == 0) {
        //     printf("output\n");
        //     for(int j = 0; j < i; j++) {
        //         printf("%s\n", tokens[j]);
        //     }
        // }
        pid_t pid = fork();
        switch(pid){

            case -1:
                perror("fork");
                exit(-1);
                break;
            case 0:
                if(command_num  == 1){
                    execl("/bin/sh","sh", "-c", tokens[i], (char*)NULL);
                    exit(0);
                }
                else{
                    if(i == 0)/*if(command_num- i == command_num)*/
                    {   
                        if(close(pfd[i][0]) == -1)
                        {
                            perror("close");
                        }                       
                        // if(pfd[i][1] != STDOUT_FILENO)
                        // {
                            if(dup2(pfd[i][1], STDOUT_FILENO) == -1){
                                perror("dup2");
                            }
                        // }

                        if(close(pfd[i][1]) == -1){
                            perror("close");
                        }
                        for(int k = i+1; k < pipe_num; k++){
                            if(close(pfd[k][0]) == -1){
                                perror("close");
                            }
                            if(close(pfd[k][1]) == -1){
                                perror("close");
                            }
                        }
                        if(execl("/bin/sh", "/sh", "-c", tokens[i], (char*)NULL) == -1){
                            perror("execl");
                        }

                    }
                    else if(i == command_num-1){

                        if(close(pfd[i-1][1] == -1)){
                            perror("close");
                        }
                        // if(pfd[i-1][0] != STDIN_FILENO){
                            if(dup2(pfd[i-1][0], STDIN_FILENO))
                            {
                                perror("dup2");
                            }                         
                        // }
     

                        if(close(pfd[i-1][0]) == -1){
                            perror("close");
                        }

                        for(int k = 0; k < i-1; k++){
                            if(close(pfd[k][0]) == -1){
                                perror("close");
                            }
                            if(close(pfd[k][1]) == -1){
                                perror("close");
                            }
                        }
                        if(execl("/bin/sh","sh", "-c", tokens[i], (char*)NULL) == -1){
                            perror("execl");
                        }
                    }
                    else{
                        for(int k = 0; k < i-1; k++){
                            if(close(pfd[k][0]) == -1){
                                perror("close");
                            }
                            if(close(pfd[k][1]) == -1){
                                perror("close");
                            }
                        }
                        if(close(pfd[i-1][1])== -1){
                            perror("close");
                        }
                        
                        if(close(pfd[i][0]) == -1){
                            perror("close");
                        }
                        // if(pfd[i-1][0] != STDIN_FILENO)
                        // {
                            if(dup2(pfd[i-1][0], STDIN_FILENO)){
                                    perror("dup2");
                            }                       
                        // }
                        // if(pfd[i][1] != STDOUT_FILENO){
                            if(dup2(pfd[i][1], STDOUT_FILENO)){
                                perror("dup2");
                            }
                        // }
                        
                        if(close(pfd[i-1][0]) == -1)
                        {
                            perror("close");
                        }
                        
                        if(close(pfd[i][1]) == -1)
                        {
                            perror("close");
                        }
                        
                        for(int k = i+1; k < pipe_num; k++){
                            if(close(pfd[k][0]) == -1){
                                perror("close");
                            }
                            if(close(pfd[k][1]) == -1){
                                perror("close");
                            }
                        }
                        // if(pfd[i][0] != STDIN_FILENO){
                        //     if(dup2(pfd[i][0], STDIN_FILENO)){
                        //         perror("dup2");
                        //     }
                        //     if(close(pfd[i][0])){
                        //         perror("close");
                        //     }
                        // }

                        if(execl("/bin/sh", "/sh", "-c", tokens[i], (char*)NULL) == -1){
                            perror("execl");
                        }
                    }
                    exit(0);  
                                
                }
                break;

            default:

            break;
        } 

        for(int j = 0; j < i ; j++){
            if(close(pfd[j][0]) == -1){
                perror("close");
            }
            if(close(pfd[j][1]) == -1){
                perror("close");
            }
        }

        int statusx;
        for (int j = 0; j < i; j++) {
            pid_t wpid = wait(&statusx);
            if (wpid == -1) {
                perror("wait");
            }
        }
        // int fd; 
        // fd = open("log.txt", O_WRONLY | O_CREAT | O_APPEND, 0777);
        // if(fd)
        // {
        //     perror("open");
        // }

        // if(write(fd, tokens[i], strlen(tokens[i])) == -1){
        //     perror("write");
        // }
        // if(write(fd, " ", 2) == -1){
        //     perror("write");
        // }
        // close(fd);



    }


    memset(tokens, 0, sizeof(tokens));

    }
  return 0;
}