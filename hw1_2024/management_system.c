#include <errno.h>
#include <stdio.h> //for perror
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h> // for string manipulations
#include <stdlib.h>
#include <sys/wait.h>
#define openFlags  O_CREAT | O_RDWR | O_NONBLOCK

#define COMMAND_ERR -1

//enum olusturulacak
typedef enum terminalCommand
{
    GTUSTUDENTGRADE,
    GTUSTUDENTGRADE_CREATE,
    ADDSTUDENT, 
    SEARCHSTUDENT,
    SORTALL,
    SHOWALL, 
    LISTGRADES, 
    LISTSOME,
}terminalCommand;

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

void display(char** command, int counter, int display_flag)
{

    int buffer_len = 1024;
    char buffer[buffer_len];
    char stringBufferForLog[buffer_len];
    int bytes_read = 0;
    int file_read = 0;
    char c;
    char student[256];
    int index = 0;
    memset(buffer, 0, buffer_len);
    memset(stringBufferForLog, 0, buffer_len);
    memset(student, 0, 256);

    int sizeofString = sprintf(stringBufferForLog, "\nShowing student\n----------\n");
    write_to_log(stringBufferForLog);
    write(STDOUT_FILENO, stringBufferForLog, sizeofString);
    memset(stringBufferForLog, 0, sizeofString);

    int search_fd = open(command[counter-1], O_CREAT | O_RDWR | O_NONBLOCK | O_APPEND,S_IRWXU | S_IRWXG | S_IRWXO);
    if(search_fd == -1)
    {
        int sizeofString = sprintf(stringBufferForLog, "File Open failed in child...exiting\n");
        write_to_log(stringBufferForLog);
        memset(stringBufferForLog, 0, sizeofString);
        perror("open");
    }
    while(file_read == 0 && display_flag != 0)
    {
        bytes_read = read(search_fd, &c, 1);
        if(bytes_read == -1){
            file_read = 1;
            perror("read");
        }
        else if(bytes_read == 0){
            file_read = 1;
        }
        if(c != '\n')
        {
            student[index++] = c;
        }
        else{
            student[index++] = '\n';
            student[index] = '\0';
            write(STDOUT_FILENO, student, sizeof(student));
            memset(student, 0, sizeof(student));
            index = 0;
            if(display_flag > 0)
            {
                display_flag--;
            }
        }
        
    }
    if(close(search_fd)== -1){
        sizeofString = sprintf(stringBufferForLog, "Close syscall error when searching from to file\n");
        write_to_log(stringBufferForLog);
        memset(stringBufferForLog, 0, sizeofString);
        perror("close");
    }
    sizeofString = sprintf(stringBufferForLog, "Display operation is successfull\n");
    write_to_log(stringBufferForLog);
    memset(stringBufferForLog, 0, sizeofString);
}

void display_some(char** command, int counter)
{
    int buffer_len = 1024;
    char buffer[buffer_len];
    char stringBufferForLog[buffer_len];
    int bytes_read = 0;
    int file_read = 0;
    char c;
    char student[256];
    int index = 0;
    int temp_index = 0;
    memset(buffer, 0, buffer_len);
    memset(stringBufferForLog, 0, buffer_len);
    memset(student, 0, 256);
    int listSome_1 = command[1][0] - '0';
    int listSome_2 = command[2][0] - '0';

    int sizeofString = sprintf(stringBufferForLog, "%d - %d \n", listSome_1, listSome_2);
    write_to_log(stringBufferForLog);
    write(STDOUT_FILENO, stringBufferForLog, sizeofString);
    memset(stringBufferForLog, 0, sizeofString);

    int search_fd = open(command[counter-1], O_CREAT | O_RDWR | O_NONBLOCK | O_APPEND,S_IRWXU | S_IRWXG | S_IRWXO);
    if(search_fd == -1)
    {
        int sizeofString = sprintf(stringBufferForLog, "File Open failed in child...exiting\n");
        write_to_log(stringBufferForLog);
        memset(stringBufferForLog, 0, sizeofString);
        perror("open");
    }
    if(listSome_1 < 0 || listSome_2 < 0){
        int sizeofString = sprintf(stringBufferForLog, "page number is not lower than 0\n");
        write_to_log(stringBufferForLog);
        memset(stringBufferForLog, 0, sizeofString);
    }
    temp_index = listSome_1*(listSome_2-1);
    while(file_read == 0 && listSome_1 > 0)
    {
        bytes_read = read(search_fd, &c, 1);
        if(bytes_read == -1){
            file_read = 1;
            perror("read");
        }
        else if(bytes_read == 0){
            file_read = 1;
        }
        if(c != '\n')
        {
            student[index++] = c;
            counter++;
        }
        else{
            student[index++] = '\n';
            student[index] = '\0';
            if(temp_index != 0){
                temp_index--;
            }
            else{
                write(STDOUT_FILENO, student, sizeof(student));
                listSome_1--;
            }
            memset(student, 0, sizeof(student));
            index = 0;
        }
        
    }
    if(close(search_fd)== -1){
        sizeofString = sprintf(stringBufferForLog, "Close syscall error when searching from to file\n");
        write_to_log(stringBufferForLog);
        memset(stringBufferForLog, 0, sizeofString);
        perror("close");
    }
    sizeofString = sprintf(stringBufferForLog, "Display operation is successfull\n");
    write_to_log(stringBufferForLog);
    memset(stringBufferForLog, 0, sizeofString);
}

void sortAll(char**command , int counter)
{
    return;
}

char* searchStudent(const char* name, int fd, char* row )
{
    int index = 0;
    char c;
    int read_bytes; 
    int file_read=0;
    // char row[256];
    char stringBufferForLog[512]; 

    memset(row, 0, sizeof(row));
    memset(stringBufferForLog, 0, sizeof(stringBufferForLog));
    while (file_read == 0) 
    {
        read_bytes = read(fd, &c, 1);
        if (read_bytes == -1) 
           perror("read while: -1");
        if (read_bytes == 0) 
            file_read = 1;
        
        if(c != ',')
            row[index++]=c;
        else{
            if(strncmp(row, name, strlen(row)) == 0)
            {
                row[index++]=c;
                while(c != '\n')
                {
                    read_bytes = read(fd, &c, 1);
                    if (read_bytes == -1) 
                        perror("read while: -1");
                    if (read_bytes == 0) 
                        file_read = 1;

                    row[index++]=c;
                }
                row[index] = '\0';
                int sizeofString = sprintf(stringBufferForLog, "Found entered student name in the file: %s\n", row);
                write(STDOUT_FILENO, stringBufferForLog, sizeofString);
                write_to_log(stringBufferForLog);
                return row;
            }
            else{
                while(c != '\n')
                {
                    read_bytes = read(fd, &c, 1);
                    if (read_bytes == -1) 
                        perror("read while: -1");
                    if (read_bytes == 0) 
                        file_read = 1;
                }
                memset(row, 0, 256);
                index = 0;
            }
        }
    }
    int sizeofString = sprintf(stringBufferForLog, "Student is not found in the file: %s\n", row);
    write(STDOUT_FILENO, stringBufferForLog, sizeofString);
    memset(row, 0, 256);
    return NULL;
}

void addingStudent(char** command, int counter)
{
    char name[24]; 
    char grade[4];
    int buffer_len = 1024;
    char buffer[buffer_len];
    char stringBufferForLog[buffer_len];
    char student[256];
    memset(buffer, 0, buffer_len);
    memset(stringBufferForLog, 0, buffer_len);
    memset(student, 0, sizeof(student));

    int studentAdd_fd = open(command[counter-1], O_CREAT | O_RDWR | O_NONBLOCK | O_APPEND,S_IRWXU | S_IRWXG | S_IRWXO);
    if(studentAdd_fd == -1)
    {
        int sizeofString = sprintf(stringBufferForLog, "File Open failed in child...exiting\n");
        write_to_log(stringBufferForLog);
        memset(stringBufferForLog, 0, sizeofString);
        perror("open");
    }
    for(int i = 1; i < counter-1; i++)
    {
        if(i == 1)
            strcpy(name, command[i]);
        else if(i != counter -2){
            strcat(name, " ");
            strcat(name, command[i]);
        }
        else {
            strcat(name, ",");
            strcpy(grade, command[i]);
            strcat(grade, "\n");
        }
    }
    searchStudent(name, studentAdd_fd, student);
    int sizeofString = sprintf(stringBufferForLog, "Student %s\n", grade);
    write(STDOUT_FILENO, stringBufferForLog, sizeofString);
    memset(stringBufferForLog, 0, sizeofString);
    if(student != NULL )
    {
        char logbuff[1024];
        snprintf(logbuff, 1024, "Entered student name was found \n");
        write_to_log(logbuff);
        if(close(studentAdd_fd)== -1){
            int sizeofString = sprintf(stringBufferForLog, "Close syscall error when adding to file\n");
            write_to_log(stringBufferForLog);
            memset(stringBufferForLog, 0, sizeofString);
            perror("close");
        }
        return;
    }
    // else
    // {
        // printf("name is %s\n", name);
        // printf("grade is %s\n",grade);
        sizeofString = sprintf(stringBufferForLog, "name %s\n", name);
        write(STDOUT_FILENO, stringBufferForLog, sizeofString);
        strcat(name, grade);
        strcpy(buffer, name);
        
        int offset = lseek(studentAdd_fd, 0, SEEK_END);
        if(offset == -1){
            int sizeofString = sprintf(stringBufferForLog, "lseek syscall error when adding to file\n");
            write_to_log(stringBufferForLog);
            memset(stringBufferForLog, 0, sizeofString);
            perror("lseek");
        }

        int bytes_written = write(studentAdd_fd, &buffer, strlen(buffer));
        if(bytes_written == -1){
            int sizeofString = sprintf(stringBufferForLog, "Write syscall error when adding to file\n");
            write_to_log(stringBufferForLog);
            memset(stringBufferForLog, 0, sizeofString);
            perror("write");
        }

        if(close(studentAdd_fd)== -1){
            int sizeofString = sprintf(stringBufferForLog, "Close syscall error when adding to file\n");
            write_to_log(stringBufferForLog);
            memset(stringBufferForLog, 0, sizeofString);
            perror("close");
        }
        sizeofString = sprintf(stringBufferForLog, "Student succesfully added\n");
        write_to_log(stringBufferForLog);
        memset(stringBufferForLog, 0, sizeofString);
    // }
    
}

void help_command()
{
    char stringBufferForWrite[100];
    int sizeofString = sprintf(stringBufferForWrite, "gtuStudentsGrade \"filename.txt\"\n");
    write(STDOUT_FILENO, stringBufferForWrite, sizeofString);
    memset(stringBufferForWrite, 0, sizeofString);
    
    sizeofString = sprintf(stringBufferForWrite, "addStudentsGrade \"Name Surname\" \"GRADE\"\"filename.txt\"\n");
    write(STDOUT_FILENO, stringBufferForWrite, sizeofString);
    memset(stringBufferForWrite, 0, sizeofString);
    
    sizeofString = sprintf(stringBufferForWrite, "searchStudent \"Name Surname\" \"filename.txt\"\n");
    write(STDOUT_FILENO, stringBufferForWrite, sizeofString);
    memset(stringBufferForWrite, 0, sizeofString);
    
    sizeofString = sprintf(stringBufferForWrite, "sortAll \"filename.txt\"\n");
    write(STDOUT_FILENO, stringBufferForWrite, sizeofString);
    memset(stringBufferForWrite, 0, sizeofString);

    sizeofString = sprintf(stringBufferForWrite, "showAll \"filename.txt\"\n");
    write(STDOUT_FILENO, stringBufferForWrite, sizeofString);
    memset(stringBufferForWrite, 0, sizeofString);

    sizeofString = sprintf(stringBufferForWrite, "listGrades \"filename.txt\"\n");
    write(STDOUT_FILENO, stringBufferForWrite, sizeofString);
    memset(stringBufferForWrite, 0, sizeofString);

    sizeofString = sprintf(stringBufferForWrite, "listSome \"count\" \"page_num\" \"filename.txt\"\n");
    write(STDOUT_FILENO, stringBufferForWrite, sizeofString);
    memset(stringBufferForWrite, 0, sizeofString);
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
            //" " oldugunda j+1 e ekle
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

terminalCommand validateCommand(const char* buffer, int bufferLen, char** command)
{
    const char* gtuStudentGrade = "gtuStudentGrade";
    const char* addStudentGrade = "addStudentGrade";
    const char* searchStudent = "searchStudent";
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
    if(strncmp(command[0], gtuStudentGrade, sizeof(gtuStudentGrade)) == 0)
    {
        //after first command check then we have to check it is in proper format
        if(counter == 1 )
            return GTUSTUDENTGRADE;
        else if(counter == 2)
            return GTUSTUDENTGRADE_CREATE;
        // write(STDOUT_FILENO, "1", 1);
        
        return COMMAND_ERR;

    }
        
    else if(strncmp(command[0], addStudentGrade, strlen(addStudentGrade)) == 0)
    {
        if(counter == 4 || counter == 5) // maybe without surname is valid ??
            return ADDSTUDENT;
        // write(STDOUT_FILENO, "2", 1);
        return COMMAND_ERR;
    }
        
    else if(strncmp(command[0], searchStudent, strlen(searchStudent)) == 0)
    {
        write(STDOUT_FILENO, "3", 1);
        if(counter == 4 || counter == 5) // maybe without surname is valid ??
            return SEARCHSTUDENT;
        
        return COMMAND_ERR;
    }
    
    else if(strncmp(command[0], sortAll, strlen(sortAll)) == 0)
    {
        if(counter == 2)
            return SORTALL;
        //may you add ascending - descending or name- grade order
        // write(STDOUT_FILENO, "4", 1);
        return COMMAND_ERR;
    }

    else if(strncmp(command[0], showAll, strlen(showAll)) == 0)
    {
        if(counter == 2)
            return SHOWALL;
        // write(STDOUT_FILENO, "5", 1);
        return COMMAND_ERR;
    }
    
    else if(strncmp(command[0], listGrades, strlen(listGrades)) == 0)
    {
        if(counter == 2)
            return LISTGRADES;
        // write(STDOUT_FILENO, "6", 1);
        return COMMAND_ERR;
    }
    
    else if(strncmp(command[0], listSome, strlen(listSome)) == 0)
    {
        if(counter == 4)
            return LISTSOME;
        // write(STDOUT_FILENO, "7", 1);
        return COMMAND_ERR;
    }

    else
        return COMMAND_ERR;
}

int main(){

    int buffer_len = 1024;
    char buffer[buffer_len];
    int log_fd, grades_fd;
    const char *logFile = "log.txt";
    unsigned int mode = S_IRWXU | S_IRWXG | S_IRWXO;
    char stringBufferForWrite[512];



    /*
    System initialization variables and log file creation will make in here
    */

    //when initialization log file is created as empty file
    log_fd = open(logFile, O_CREAT | O_RDWR | O_TRUNC, mode);
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
        int childpid;
        char **command = (char **)malloc(8 * sizeof(char *));
        for (int i = 0; i < 8; i++) {
            command[i] = (char *)malloc(24 * sizeof(char));
            memset(command[i], 0, 24 * sizeof(char));
        }
        memset(buffer, 0, sizeof(buffer));

        if(read(STDIN_FILENO, buffer, buffer_len) == -1)
        {
            write_to_log("read() error from user input...failed...exiting!!\n");
            perror("read");
        }
        int sizeofString = sprintf(stringBufferForWrite, "read from command line successfull. Command: %s \n", buffer);
        write_to_log(stringBufferForWrite);
        memset(stringBufferForWrite, 0, sizeofString);
        
        terminalCommand terminal_t = validateCommand(buffer, buffer_len, command);
        if (terminal_t == COMMAND_ERR)
        {
            write_to_log("Invalid command entered Check argument helper for entered command... failed...exiting!!\n");
            for (int i = 0; i < 8; i++) {
                free(command[i]);
            }
            free(command);
            return -1;
        }
        while(command[counter][0] != '\0') {
            sizeofString = sprintf(stringBufferForWrite, "Command %d : %s \n", counter, command[counter++]);
            write(STDOUT_FILENO, &stringBufferForWrite, sizeofString);
            memset(stringBufferForWrite, 0, sizeofString);
            // counter++;
        }
        sizeofString = sprintf(stringBufferForWrite, "Valid command entered, executing... command >> %s, \n", command[0]);
        write_to_log(stringBufferForWrite);
        memset(stringBufferForWrite, 0, sizeofString);
        
        switch(terminal_t) {

            case GTUSTUDENTGRADE:
                childpid = fork();
                if(childpid == -1 )
                {
                    sizeofString = sprintf(stringBufferForWrite, "Fork error when displaying commands... %s\n", command[0]);
                    write_to_log(stringBufferForWrite);
                    memset(stringBufferForWrite, 0, sizeofString);
                    perror("fork");
                }
                else if(childpid == 0)
                {
                    help_command();
                    sizeofString = sprintf(stringBufferForWrite, "Displaying all available commands in terminal...\n");
                    write_to_log(stringBufferForWrite);
                    memset(stringBufferForWrite, 0, sizeofString);
                    write_to_log(stringBufferForWrite);
                    exit(EXIT_SUCCESS);
                }
            break; 

            case GTUSTUDENTGRADE_CREATE:
                childpid = fork();
                if(childpid == -1 )
                {
                    sizeofString = sprintf(stringBufferForWrite, "Fork error when creating file %s\n", command[1]);
                    write_to_log(stringBufferForWrite);
                    memset(stringBufferForWrite, 0, sizeofString);
                    perror("fork");
                }
                else if(childpid == 0)
                {
                    int gradesfd = open(command[1], O_CREAT | O_RDWR | O_NONBLOCK | O_TRUNC, mode );
                    if(grades_fd == -1)
                    {
                        sizeofString = sprintf(stringBufferForWrite, "%s File Open syscall failed in child...exiting\n",command[1]);
                        write_to_log(stringBufferForWrite);
                        memset(stringBufferForWrite, 0, sizeofString);
                        perror("open");
                    }

                    if(close(grades_fd)== -1){
                        sizeofString = sprintf(stringBufferForWrite, "%s File Close syscall failed in child...exiting\n", command[1]);
                        write_to_log(stringBufferForWrite);
                        memset(stringBufferForWrite, 0, sizeofString);
                        perror("close");
                    }
                    sizeofString = sprintf(stringBufferForWrite, "%s File Creation is Successfull\n", command[1]);
                    write_to_log(stringBufferForWrite);
                    memset(stringBufferForWrite, 0, sizeofString);
                    exit(EXIT_SUCCESS);
                }
            break; 

            case ADDSTUDENT:
                childpid = fork();
                if(childpid == -1 )
                {
                    sizeofString = sprintf(stringBufferForWrite, "Fork error when creating file %s\n", command[1]);
                    write_to_log(stringBufferForWrite);
                    memset(stringBufferForWrite, 0, sizeofString);
                    perror("fork");
                }
                else if(childpid == 0)
                {
                    addingStudent(command, counter);

                    sizeofString = sprintf(stringBufferForWrite, "Adding Student is Successfull\n");
                    write_to_log(stringBufferForWrite);
                    memset(stringBufferForWrite, 0, sizeofString);
                    exit(EXIT_SUCCESS);
                }
            break; 

            case SEARCHSTUDENT:
                childpid = fork();
                if(childpid == -1 )
                {
                    sizeofString = sprintf(stringBufferForWrite, "Fork error when searching file %s\n", command[1]);
                    write_to_log(stringBufferForWrite);
                    memset(stringBufferForWrite, 0, sizeofString);
                    perror("fork");
                }
                else if(childpid == 0)
                {
                    int search_fd = open(command[counter-1], O_CREAT | O_RDWR | O_NONBLOCK | O_APPEND, mode );
                    if(search_fd == -1)
                    {
                        sizeofString = sprintf(stringBufferForWrite, "File Open failed in child...exiting\n");
                        write_to_log(stringBufferForWrite);
                        memset(stringBufferForWrite, 0, sizeofString);
                        perror("open");
                    }
                    char studentName[256];
                    char row[256];
                    memset(studentName, 0, sizeof(studentName));
                    memset(row, 0, sizeof(row));
                    strcpy(studentName, command[1]);
                    strcat(studentName, " ");
                    strcat(studentName, command[2]);

                    searchStudent(studentName, search_fd, row);
                    sizeofString = sprintf(stringBufferForWrite, "student %s\n", row);
                    write(STDOUT_FILENO, stringBufferForWrite, sizeofString);
                    memset(stringBufferForWrite, 0, sizeofString);
                    if(close(search_fd)== -1){
                        sizeofString = sprintf(stringBufferForWrite, "File close failed in child...exiting\n");
                        write_to_log(stringBufferForWrite);
                        memset(stringBufferForWrite, 0, sizeofString);
                        perror("close");
                    }
                    //take student name and grade
                    sizeofString = sprintf(stringBufferForWrite, "Searching Student is Successfull\n");
                    write_to_log(stringBufferForWrite);
                    memset(stringBufferForWrite, 0, sizeofString);
                    memset(row, 0, sizeofString);

                    exit(EXIT_SUCCESS);
                }
            break; 

            case SORTALL:
            break; 

            case SHOWALL:
                childpid = fork();
                if(childpid == -1 )
                {
                    sizeofString = sprintf(stringBufferForWrite, "Fork error when displaying all %s\n", command[0]);
                    write_to_log(stringBufferForWrite);
                    memset(stringBufferForWrite, 0, sizeofString);
                    perror("fork");
                }
                else if(childpid == 0)
                {
                    //maybe return error code or success code
                    display(command, counter, -1);

                    int sizeofString = sprintf(stringBufferForWrite, "Searching text file is Successfull\n");
                    write_to_log(stringBufferForWrite);
                    memset(stringBufferForWrite, 0, sizeofString);
                    exit(EXIT_SUCCESS);
                }
            break; 

            case LISTGRADES:
                childpid = fork();
                if(childpid == -1 )
                {
                    sizeofString = sprintf(stringBufferForWrite, "Fork error when displaying grades %s\n", command[0]);
                    write_to_log(stringBufferForWrite);
                    memset(stringBufferForWrite, 0, sizeofString);
                    perror("fork");
                }
                else if(childpid == 0)
                {
                    display(command, counter, 5);

                    int sizeofString = sprintf(stringBufferForWrite, "Searching text file is Successfull\n");
                    write_to_log(stringBufferForWrite);
                    memset(stringBufferForWrite, 0, sizeofString);
                    exit(EXIT_SUCCESS);
                }
            break; 

            case LISTSOME:
                childpid = fork();
                if(childpid == -1 )
                {
                    sizeofString = sprintf(stringBufferForWrite, "Fork error when listing some grades.\n");
                    write_to_log(stringBufferForWrite);
                    memset(stringBufferForWrite, 0, sizeofString);
                    perror("fork");
                }
                else if(childpid == 0)
                {
                    display_some(command, counter);

                    int sizeofString = sprintf(stringBufferForWrite, "Searching text file is Successfull\n");
                    write_to_log(stringBufferForWrite);
                    memset(stringBufferForWrite, 0, sizeofString);
                    exit(EXIT_SUCCESS);
                }
            break; 

            default:
            break;
        }
        wait(NULL);
        //deallocate memory 
        for (int i = 0; i < 8; i++) {
            free(command[i]);
        }
        free(command);
        
    }
    return 0;
}