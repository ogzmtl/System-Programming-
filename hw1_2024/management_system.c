#include <errno.h>
#include <stdio.h> //for perror
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h> // for string manipulations
#include <stdlib.h>
#include <sys/wait.h>
#define openFlags  O_CREAT | O_RDWR | O_NONBLOCK
#define SUCCESS 0
#define COMMAND_ERR -1

//enum olusturulacak r, w, o, r-w 

void write_to_log(const char * log)
{
    const char *logFile = "log.txt";
    int log_fd; 
    unsigned int mode = S_IRUSR | S_IRGRP | S_IROTH | S_IXGRP | S_IXUSR | S_IXOTH;
    unsigned const int log_length = strlen(log);

    log_fd = open(logFile, openFlags | O_APPEND, mode);
    if(log_fd == -1){
        perror("open logfile");
    }
    if(write(log_fd, log, log_length) <0){
        //is it necessary to close the log file when error occured
        if(close(log_fd) == -1)
        {
            perror("close log file error");
        }
        perror("log file write error");
    }
    if(close(log_fd) == -1)
    {
        perror("close log file error");
    }
}

int choose_syscall(const char** commands){
    //TODO : implement enum with syscall function by parsing command
    return 1;
}

void parse_command(const char *command, char** parsed_commands) {

    // implement your own functions 
    // TO DO : parse each char 
  int i = 0;
  int j = 0;
  int k = 0;

  // Komut satırını " karakterlerinden ayırma
//   char *token = strtok(command, "\"");
  char temp_token[24];
  memset(temp_token, 0, sizeof(temp_token));
    while(command[i] != '\0')
    {
        
        if(command[i] != ' ' && command[i] != '\"' && command[i+1] != '\0')
        {
         
            temp_token[j++] = command[i];

        }
        else
        {
            if(temp_token[0] != '\0')
            {
                strcpy(parsed_commands[k++], temp_token);
            }
            memset(temp_token, 0, sizeof(temp_token));
            j=0;
        }
        i++;
    }
}

int validateCommand(const char* buffer, int bufferLen, char** command)
{
    const char* gtuStudentGrade = "gtuStudentGrade";
    const char* addStudentGrade = "addStudentGrade";
    const char* searchStudent = "searchStudent ";
    const char* sortAll = "sortAll";
    const char* showAll = "showAll";
    const char* listGrades = "listGrades";
    const char* listSome = "listSome";
    int counter = 0;

    /*
        The command of the our management system 
    */
    if(strlen(buffer) <= 0){
        write_to_log("Empty buffer for user input..Failed...Exiting..!!!");
        return COMMAND_ERR;
    }
    //parse with the delimeter of " \" " and space character.
    parse_command(buffer, command);
    //for counting the number of parsed words
    while(command[counter][0] != '\0') {
            counter++;
    }
    // printf("%d", counter);
    if(strncmp(command[0], gtuStudentGrade, sizeof(gtuStudentGrade)) == 0)
    {
        //after first command check then we have to check it is in proper format
        if(counter == 1 || counter == 2)
            return SUCCESS;
        // write(STDOUT_FILENO, "1", 1);
        
        return COMMAND_ERR;

    }
        
    else if(strncmp(command[0], addStudentGrade, strlen(addStudentGrade)) == 0)
    {
        if(counter == 2 || counter == 3) // maybe without surname is valid ??
            return SUCCESS;
        // write(STDOUT_FILENO, "2", 1);
        return COMMAND_ERR;
    }
        
    else if(strncmp(command[0], searchStudent, strlen(searchStudent)) == 0)
    {
        if(counter == 2 || counter == 3) // maybe without surname is valid ??
            return SUCCESS;
        // write(STDOUT_FILENO, "3", 1);
        return COMMAND_ERR;
    }
    
    else if(strncmp(command[0], sortAll, strlen(sortAll)) == 0)
    {
        if(counter == 2)
            return SUCCESS;
        //may you add ascending - descending or name- grade order
        // write(STDOUT_FILENO, "4", 1);
        return COMMAND_ERR;
    }

    else if(strncmp(command[0], showAll, strlen(showAll)) == 0)
    {
        if(counter == 2)
            return SUCCESS;
        // write(STDOUT_FILENO, "5", 1);
        return COMMAND_ERR;
    }
    
    else if(strncmp(command[0], listGrades, strlen(listGrades)) == 0)
    {
        if(counter == 2)
            return SUCCESS;
        // write(STDOUT_FILENO, "6", 1);
        return COMMAND_ERR;
    }
    
    else if(strncmp(command[0], listSome, strlen(listSome)) == 0)
    {
        if(counter == 4)
            return SUCCESS;
        // write(STDOUT_FILENO, "7", 1);
        return COMMAND_ERR;
    }

    else
        return COMMAND_ERR;
}

int main(){

    //flock usage ?? 
    //what if file exists O_TRUNC or O_APPEND in use 
    //what if fork failed when should log file used with fork
    //hata handle edilsin sinyal mi ?
    int buffer_len = 1024;
    char buffer[buffer_len];
    int log_fd, grades_fd;
    const char *logFile = "log.txt";
    unsigned int mode = S_IRWXU | S_IRWXG | S_IRWXO;
    char stringBufferForWrite[256];



    /*
    System initialization variables and log file creation will make in here
    */

    //when initialization log file is created as empty file
    log_fd = open(logFile, O_CREAT | O_RDWR | O_NONBLOCK | O_TRUNC, mode);
    if(log_fd == -1)
    {
        perror("log file creation");
    }

    if(close(log_fd) == -1)
    {
        perror("close");
    }

    /*
    User input like grades, list etc will be wait with read syscall but should we be forked read syscall 
    1. Create a file with open grades.txt -> gtuStudentGrades “grades.txt” use fork in this syscall
        1.a determine open flags
        1.b When multiple user reach to txt file what should be happen ?
        1.c after file creation is done close the file
        1.d do not forget un/successfull log
        1.e in each fork you should close fd of log file (prevent conflict)
    */

    while(1)
    {
        int counter = 0;
        char **command = (char **)malloc(5 * sizeof(char *));
        for (int i = 0; i < 5; i++) {
            command[i] = (char *)malloc(24 * sizeof(char));
            memset(command[i], 0, 24 * sizeof(char));
        }
        memset(buffer, 0, sizeof(buffer));

        if(read(STDIN_FILENO, buffer, buffer_len) == -1)
        {
            write_to_log("read() error from user input...failed...exiting!!\n");
            perror("read");
        }
        write_to_log("read from command line successfull\n");

        if (validateCommand(buffer, buffer_len, command) != SUCCESS)
        {
            write_to_log("Invalid command entered Check argument helper for entered command... failed...exiting!!\n");
            for (int i = 0; i < 5; i++) {
                free(command[i]);
            }
            free(command);
            return -1;
        }
        while(command[counter][0] != '\0') {
            int sizeofString = sprintf(stringBufferForWrite, "Command %d : %s \n", counter, command[counter++]);
            write(STDOUT_FILENO, stringBufferForWrite, sizeofString);
            memset(stringBufferForWrite, 0, sizeofString);
        }
        int sizeofString = sprintf(stringBufferForWrite, "Valid command entered, executing... command >> %s, \n", command[0]);
        write_to_log(stringBufferForWrite);
        memset(stringBufferForWrite, 0, sizeofString);

        int syscall_num = choose_syscall(command);

        switch(syscall_num) {


            case OPEN:
                int childpid = fork();
                if(childpid == -1 )
                {
                    //write to log file
                    perror("fork");
                }
                else if(childpid == 0)
                {
                    int gradesfd = open(command[1], O_CREAT | O_RDWR | O_NONBLOCK | O_TRUNC, mode );
                    if(grades_fd == -1){
                        //write to log file
                        perror("open");
                    }

                    if(close(grades_fd)== -1){
                        //write log file
                        perror("close");
                    }
                    //must return own pid or something else ??
                    //how to kill child process ????
                }
                else{
                    if(wait(&childpid)== -1){
                        //write to log file
                        perror("wait");
                    }
                }
            break; 

            case READ:
            break; 

            case WRITE:
            break; 

            case READ_WRITE:
            break; 
        }


        //deallocate memory 
        for (int i = 0; i < 5; i++) {
            free(command[i]);
        }
        free(command);

    }

    return 0;
}