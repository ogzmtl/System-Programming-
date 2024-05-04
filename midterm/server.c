#include "common.h"
#include <semaphore.h>
#include <signal.h>
#include <dirent.h>
#include <sys/wait.h>
//cocuklara kill sinyali gonder arrayde pid
//clientlara kill sinyali gonder arrayde pid 
long int clients[MAX_CLIENT];
long int child[MAX_CHILD];
sem_t *queue;
int clientCounter =0;

int find_process_and_kill(int clpid)
{
    if(clpid != -1)
    {
        for(int i = 0; i < MAX_CLIENT; i++)
        {
            if(clients[i] == clpid)
            {
                clients[i] = -1;
                return 1;
            }
        }
        return -1;
    }
    return -1;
}
int kill_fork(int clpid)
{
    if(clpid != -1)
    {
        for(int i = 0; i < MAX_CLIENT; i++)
        {
            if(child[i] == clpid)
            {
                clients[i] = -1;
                return 1;
            }
        }
        return -1;
    }
    return -1;
}
void handler(int sig)
{
    int status;
    if(sig == SIGCHLD)
    {
        pid_t child_pid = waitpid(-1, &status, 0);
        if(find_process_and_kill(child_pid) != 1)
        {
            kill_fork(child_pid);
        }
        else
        {
            clientCounter--;
            sem_post(queue);
        }
    }

    if(sig == SIGINT || sig == SIGTERM || sig == SIGKILL)
    {
        // char semaphore_one[BUFF_SIZE];
        // char semaphore_two[BUFF_SIZE];

        // snprintf(semaphore_one, CLIENT_SEM_NAME_LEN, CLIENT_SEM_TEMP, (long)getpid());
        // snprintf(semaphore_two, CLIENT_SEM_NAME_LEN, CLIENT_SEM2_TEMP, (long)getpid());

        unlink(SERVER_FIFO);
        // sem_close(semaphore_one);
        // sem_close(semaphore_two);
        // sem_unlink(semaphore_one);//error check -1 on error
        // sem_unlink(semaphore_two);
        //unlink fifos

        for(int i = 0; i < 1; i++)
        {
            kill(clients[i], SIGINT);
            kill(child[i], SIGKILL);
        }
    }
    exit(0);
}

void find_proper_location(int clpid)
{
        for(int i = 0; i < MAX_CLIENT; i++)
        {
            if(clients[i] == -1)
            {
                clients[i] = clpid;
            }
        }
}
void createTempAndDelete(int srcFd, int dstFd, int n)
{
    int nth_line_offset = 0;
    char c;
    int offset = 0;
    char buf[BUFF_SIZE];
    int bytes_read;
    int newline_count = 0;
    // int i = 0;
    while ((bytes_read = read(srcFd, &c, 1)) > 0) {
        // for (int i = 0; i < bytes_read; i++) {
        
        if (c == '\n') {
            newline_count++;
            if (newline_count == n - 1) {
                printf("offset: %d\n", offset);
                nth_line_offset = offset + 1;
                break;
            }
        }
        // }
        if (newline_count == n - 1) {
            break;
        }
        offset++;
    }

    if (lseek(srcFd, nth_line_offset, SEEK_SET) == -1) {
        perror("Error seeking to (n-1)th line");
        exit(1);
    }

    while ((bytes_read = read(srcFd, buf, BUFF_SIZE)) > 0) {
        if (write(dstFd, buf, bytes_read) == -1) {
            perror("Error writing to destination file");
            exit(1);
        }
    }

    if (ftruncate(srcFd, nth_line_offset+bytes_read) == -1) {
        perror("Error truncating source file");
        exit(1);
    }
}

void appendToFile(int srcFd, char* string)
{
    if (lseek(srcFd, 0, SEEK_END) == -1) {
        perror("Error seeking to end of file");
        exit(1);
    }

    int bytes_written = write(srcFd, string, strlen(string));
    if (bytes_written == -1) {
        perror("Error writing to file");
        exit(1);
    }

    if (bytes_written != (int) strlen(string)) {
        // fprintf(stderr, "Error: Incomplete write to file\n");
        exit(1);
    }
}

void copyTempAndDelete(int srcFd, int dstFd)
{
    char buffer[BUFF_SIZE];
    ssize_t bytes_read;

    if (lseek(srcFd, 0, SEEK_END) == -1) {
        perror("Error seeking to end of source file");
        exit(1);
    }

    if (lseek(dstFd, 0, SEEK_SET) == -1) {
        perror("Error seeking to beginning of destination file");
        exit(1);
    }

    while ((bytes_read = read(dstFd, buffer, BUFF_SIZE)) > 0) {//buff_Size bir yapabilirsin
        if (write(srcFd, buffer, bytes_read) != bytes_read) {
            perror("Error writing to source file");
            exit(1);
        }
    }

    if (bytes_read == -1) {
        perror("Error reading from destination file");
        exit(1);
    }

}

int helpHelper(char* secCommand, char* hbuff)
{
    int numBytesWrittenForHelp = 0;
    if(strcmp(secCommand, "readF") == 0)
    {
        numBytesWrittenForHelp = snprintf(hbuff, BUFF_SIZE, "readF <file> <line #>\n\tdisplay the #th line of the <file>, returns with an error if <file> does not exists");
        return numBytesWrittenForHelp;
    }
    else if(strcmp(secCommand, "help") == 0)
    {
        numBytesWrittenForHelp = snprintf(hbuff, BUFF_SIZE, "help\n\tdisplay the list of possible client requests");
        return numBytesWrittenForHelp;
    }
    else if(strcmp(secCommand, "list") == 0)
    {
        numBytesWrittenForHelp = snprintf(hbuff, BUFF_SIZE, "list\n\t sends a request to display the list of files in Servers directory\n\t(also displays the list received from the Server)");
        return numBytesWrittenForHelp;
    }
    else if(strcmp(secCommand, "writeT") == 0)
    {
        numBytesWrittenForHelp = snprintf(hbuff, BUFF_SIZE, "writeT <file> <line #> <string>\n\trequest to write the content of “string” to the #th line the <file>, if the line # is not given \
writes to the end of file. If the file does not exists in Servers directory creates and edits the \
file at the same time");
        return numBytesWrittenForHelp;
    }
    else if(strcmp(secCommand, "upload") == 0)
    {
        numBytesWrittenForHelp = snprintf(hbuff, BUFF_SIZE, "upload <file>\n\tuploads the file from the current working directory of client to the Servers directory");
        return numBytesWrittenForHelp;
    }
    else if(strcmp(secCommand, "download") == 0)
    {
        numBytesWrittenForHelp = snprintf(hbuff, BUFF_SIZE, "download <file>\n\trequest to receive <file> from Servers directory to client side");
        return numBytesWrittenForHelp;
    }
    else if(strcmp(secCommand, "archServer") == 0)
    {
        numBytesWrittenForHelp = snprintf(hbuff, BUFF_SIZE, "archServer <fileName>.tar\n\tUsing fork, exec and tar utilities create a child process that will collect all the files currently available on the the Server side and store them in the <filename>.tar archive");
        return numBytesWrittenForHelp;
    }
    else if(strcmp(secCommand, "killServer") == 0)
    {
        numBytesWrittenForHelp = snprintf(hbuff, BUFF_SIZE, "killServer\n\tSends a kill request to the Server");
        return numBytesWrittenForHelp;
    }
    else if(strcmp(secCommand, "quit") == 0)
    {
        numBytesWrittenForHelp = snprintf(hbuff, BUFF_SIZE, "quit\n\tSend write request to Server side log file and quits");
        return numBytesWrittenForHelp;
    }
    else
    {
        numBytesWrittenForHelp = snprintf(hbuff, BUFF_SIZE, "Invalid command entered by user");
        return numBytesWrittenForHelp;
    }
}

void readFHelper(char* filename, int lineNum, char* line)
{
        printf("filename: %s %d\n", filename, lineNum);
        int fd = open(filename, O_RDONLY);
        if (fd == -1) {
            return;
            // return "File could not opened.";
        }

        // char line[BUFF_SIZE];
        char c; 
        ssize_t read_bytes;
        int line_number = 1;
        int counter = 0;
        read_bytes = read(fd, &c, 1);
        while (read_bytes > 0) {
            if (c != '\n') {
                line[counter++] = c;
            }
            else{
                if(line_number == lineNum)
                {
                    line[counter]='\0';
                    close(fd);
                    return;
                }
                else{
                    memset(line,0, counter);
                    counter = 0;
                    line_number++;
                    printf("%d\n", line_number);
                }
            }
            read_bytes = read(fd, &c, 1);
        }

        close(fd);
        // return "Line not found.";
}

int addLastLineWithZero(char* filename, int string_len, char* string)
{
    int fd;
    fd = open(filename, O_WRONLY);
    if(fd == -1){
        perror("open");
        return -1;
    }
    lseek(fd, 0, SEEK_END);
    // for(int i = 0; i < string_len; i++){
        write(fd, &string, string_len);
    // }
    close(fd);
    return 1;
}

void writeTMiddle(char *filename, int lineNum, char *string){

    // char c; 
    // ssize_t read_bytes;
    char tempFile[32] = "temp";
    // char line[BUFF_SIZE];

    int tmpFd= open(tempFile, O_RDWR| O_CREAT| O_TRUNC, 0666);
    if(tmpFd == -1){
        perror("open1");
        return;
    }
    int fd = open(filename, O_RDWR|O_APPEND|O_CREAT, 0666);
    if(fd == -1)
    {
        perror("open2");
        return ;
    }
    createTempAndDelete(fd, tmpFd, lineNum);
    appendToFile(fd, string);
    copyTempAndDelete(fd, tmpFd);

    if(close(fd) == -1){
        perror("close");
        return;
    }
    if(close(tmpFd) == -1){
        perror("close");
        return;
    }
    if (unlink("temp") == -1) {
        perror("Error removing destination file");
        exit(1);
    }
}

void writeTHelper(char* filename, int lineNum, char* string, char* buffR)
{
    int fd; 
    // char temp_buffer[BUFF_SIZE];
    int string_len = snprintf(buffR, BUFF_SIZE, "%s\n", string);
    int numBystesWritten = 0;
    printf("buffr: %s\n", buffR);
    //file doesn't exist
    printf("xx : %d\n", string_len);
    string[string_len-1] = '\n';
    string[string_len]= '\0';
    // buffR[string_len] = '\0';
    DIR *dir = opendir(filename);
    if (dir == NULL) {
        // File does not exist, create it
        int fd = open(filename, O_CREAT | O_WRONLY, 0644);
        if (fd == -1) {
            
            perror("Error creating file");
            return;
        }
    }
    if(lineNum == -1)
    {
        memset(buffR,0,BUFF_SIZE);
        fd = open(filename, O_WRONLY|O_APPEND|O_CREAT, 0666);
        if(fd == -1){
            perror("open");
            return ;
        }
        
        write(fd, string, string_len);
    // }
        close(fd);
            // addLastLineWithZero(filename, string_len,string);
        numBystesWritten = snprintf(buffR, BUFF_SIZE, "String \"%s\" Added at the end of file \n", string);
        buffR[numBystesWritten] = '\0';
    }
    else{
        printf("burdayim\n");
        writeTMiddle(filename, lineNum, string);
        numBystesWritten = snprintf(buffR, BUFF_SIZE, "String \"%s\" Added at the %d line of file \n", string,lineNum);
        buffR[numBystesWritten] = '\0';
    }

}

void send_response(char (*splitted_command)[BUFF_SIZE],int clientWriteFd, enum command_level com, char* foldername, int size, pid_t clpid)
{
    int numBytesWrittenToFd = 0;
    struct response resp;
    char copiedBuff[BUFF_SIZE];
    char temp_buffer[BUFF_SIZE];
    char file_path[256] ="";
    // char donotUser[MAX_ARGUMENT][BUFF_SIZE];
    char up[BUFF_SIZE] = "upload ";
    char down[BUFF_SIZE] = "download ";
    switch(com)
    {
        case HELP:
            if(splitted_command[1][0] == '\0')
            {
                numBytesWrittenToFd = snprintf(copiedBuff,BUFF_SIZE, "Available comments are :\nhelp, list, readF, writeT, upload, download, archServer,quit, killServer\n");
                strcpy(resp.command, copiedBuff);
                resp.command[numBytesWrittenToFd] = '\0';
                write(clientWriteFd, &resp, sizeof(struct response));
            }
            else
            {
                numBytesWrittenToFd = helpHelper(splitted_command[1], copiedBuff);
                strcpy(resp.command, copiedBuff);
                resp.command[numBytesWrittenToFd] = '\0';
                memset(copiedBuff, 0, sizeof(struct response));
                write(clientWriteFd, &resp, sizeof(struct response));
            }
        break; 

        case LIST:
            DIR *dp; 
            struct dirent *entry;
            char temp_file[BUFF_SIZE] = ""; 
            dp = opendir(foldername);
            if(dp != NULL)
            {
                while (entry = readdir (dp))
                {
                    strcat(temp_file, entry->d_name);
                    strcat(temp_file, "\n");
                }
                closedir (dp);

                strcpy(resp.command, temp_file);
                write(clientWriteFd, &resp, sizeof(struct response));
            }
            else
                perror ("Couldn't open the directory");
        break;

        case READF:
            memset(temp_buffer, 0, BUFF_SIZE);
            memset(file_path, 0, BUFF_SIZE);
            strcpy(file_path, foldername);
            strcat(file_path, "/");
            strcat(file_path, splitted_command[1]);
            
            int line_num = atoi(splitted_command[2]);//if null check
            printf("line number: %d", line_num);
            if(line_num > 0){
                //file lock in reading 
                readFHelper(file_path, -1, temp_buffer);
                memset(resp.command, 0, BUFF_SIZE);
                strcpy(resp.command, temp_buffer);
                write(clientWriteFd, &resp, sizeof(struct response));
            }
            // else
            // {

            // }
            


        break;

        case WRITET:
            memset(temp_buffer, 0, BUFF_SIZE);
            memset(file_path, 0, BUFF_SIZE);
            strcpy(file_path, foldername);
            strcat(file_path, "/");
            strcat(file_path, splitted_command[1]);
            if(size == 3){
                // file lock in writing
                writeTHelper(file_path, -1, splitted_command[2],temp_buffer);
                memset(resp.command, 0, BUFF_SIZE);
                strcpy(resp.command, temp_buffer);
                write(clientWriteFd, &resp, sizeof(struct response));
            }
            if(size == 4){
                int ln = atoi(splitted_command[2]);
                writeTHelper(file_path, ln, splitted_command[3], temp_buffer);
                memset(resp.command, 0, BUFF_SIZE);
                strcpy(resp.command, temp_buffer);
                write(clientWriteFd, &resp, sizeof(struct response));
            }
        break; 

        case UPLOAD:
            memset(temp_buffer, 0, BUFF_SIZE);
            memset(file_path, 0, BUFF_SIZE);
            strcpy(file_path, foldername);
            strcat(file_path, "/");
            strcat(file_path, splitted_command[1]);
            strcat(up, file_path);
            strcpy(resp.command, up);
            write(clientWriteFd, &resp, sizeof(struct response));
            
        break;

        case DOWNLOAD:
            memset(temp_buffer, 0, BUFF_SIZE);
            memset(file_path, 0, BUFF_SIZE);
            strcpy(file_path, foldername);
            strcat(file_path, "/");
            strcat(file_path, splitted_command[1]);
            strcat(down, file_path);
            strcpy(resp.command, down);
            write(clientWriteFd, &resp, sizeof(struct response));
        break; 

        // case ARCHIVE:
        // break;

        case KILL:
            kill(getpid(), SIGKILL);
        break;

        case QUIT:
            //write_to_log
            if(find_process_and_kill(clpid) == 1)
            {
                clientCounter--;
                kill(clpid, SIGTERM);
                sem_post(queue);
            }

        break;

        default:
        break;
    }
}

enum command_level checkCommand(char* command, int size)
{
    if(strcmp(command, "help") == 0)
    {
        if(size == 1 || size == 2)
            return HELP;
        return INVALID_COMMAND;
    }   

    else if(strcmp(command, "list") == 0)
    {
        if(size != 1)
            return INVALID_COMMAND;
        return LIST;
    }   

    else if(strcmp(command, "readF") == 0)
    {
        if(size == 3 || size == 2)
            return READF;
        return INVALID_COMMAND;
    }   

    else if(strcmp(command, "writeT") == 0)
    {
        if(size == 3 || size == 4)
            return WRITET;
        return INVALID_COMMAND;
    }   
    else if(strcmp(command, "upload") == 0)
    {
        if(size != 2)
            return INVALID_COMMAND;
        return UPLOAD;
    }   
    else if(strcmp(command, "download") == 0)
    {
        if(size != 2)
            return INVALID_COMMAND;
        return DOWNLOAD;
    }
    else if(strcmp(command, "arcServer") == 0)
    {
        if(size != 2)
            return INVALID_COMMAND;
        return ARCHIVE;
    }
    else if(strcmp(command, "killServer") == 0)
    {
        if(size != 1)
            return INVALID_COMMAND;
        return KILL;
    }
    else if(strcmp(command, "quit") == 0)
    {
        if(size != 1)
            return INVALID_COMMAND;
        return QUIT;
    }
    else{
        return INVALID_COMMAND;
    }
    
}

enum command_level handle_client_request(const char* command, char(*splitted_command)[BUFF_SIZE]){
    // int log_bytes = 0;
    // char log_function[BUFF_SIZE];
    // char response_buffer[BUFF_SIZE];
    // char splitted_command[MAX_ARGUMENT][BUFF_SIZE];
    // enum command_level lev; 

    int size = splitStringIntoArray_S(command, ' ', splitted_command);
    if(size == -1)
    {
        //write_to_log
        return INVALID_COMMAND;
    }
    return checkCommand(splitted_command[0], size);

    
    // for(int i = 0; i < size; i++){
    //     printf("splitted command : %s\n", splitted_command[i]);
    // }


}

int main(int argc, char **argv){

    char log[BUFF_SIZE];
    char buffer[BUFF_SIZE];
    char clientFifo[CLIENT_FIFO_NAME_LEN];
    int numBytesWrittenLog;
    int pid, serverFd, dummyFd; 
    char filen[BUFF_SIZE];
    struct stat st;
    struct request req;
    struct response resp;
    char clientSem[CLIENT_SEM_NAME_LEN];
    sem_t *sem_temp;
    char clientSem2[CLIENT_SEM_NAME_LEN];
    sem_t *sem_temp2;
    int fork_count=0;
    struct sigaction new;
    new.sa_handler = handler;
    sigemptyset(&new.sa_mask);
    new.sa_flags = 0;
    if(sigaction (SIGINT, &new, NULL) == -1){
        perror("sigaction");
        return ERR;
    };


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
    for(int i = 0; i < MAX_CLIENT; i++)
    {
        clients[i] = -1;
    }
    queue = sem_open(QUEUE_SEM, O_CREAT, 0666, numClients);
    if (queue == SEM_FAILED) {
        perror("sem_open producer");
        exit(1);
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
    if(mkfifo(SERVER_FIFO,0666) == -1)
    {
        if(errno == EEXIST){
            unlink(SERVER_FIFO);
        }
        numBytesWrittenLog = sprintf(log, "mkfifo %s\n", SERVER_FIFO);
        if(write(STDOUT_FILENO, log, numBytesWrittenLog) == -1){
            perror("write syscall error\n");
            memset(log, 0, sizeof(numBytesWrittenLog));
            return ERR;
        }
        memset(log, 0, sizeof(numBytesWrittenLog));
        // unlink(SERVER_FIFO);
        perror("mkfifo error");
        return ERR;
    }
    if(mkfifo(TEMP_DOWNLOAD_FIFO,0666) == -1)
    {
        if(errno == EEXIST){
            unlink(SERVER_FIFO);
            unlink(TEMP_DOWNLOAD_FIFO);
        }
        numBytesWrittenLog = sprintf(log, "mkfifo %s\n", SERVER_FIFO);
        if(write(STDOUT_FILENO, log, numBytesWrittenLog) == -1){
            perror("write syscall error\n");
            memset(log, 0, sizeof(numBytesWrittenLog));
            return ERR;
        }
        memset(log, 0, sizeof(numBytesWrittenLog));
        perror("mkfifo error");
        return ERR;
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
        if(unlink(SERVER_FIFO) == -1){
            perror("unlink");
        } 
        return ERR;
    }
    for(;;)
    {
        if(read(serverFd, &req, sizeof(struct request)) != sizeof(struct request)){
            perror("read \n");
            unlink(SERVER_FIFO);
            return ERR;
        }
        if(req.serverPid != pid)
        {
            numBytesWrittenLog = snprintf(log, BUFF_SIZE, "Wrong server pid %ld\n", req.serverPid);
            write(STDOUT_FILENO, log, numBytesWrittenLog);
            memset(log, 0, numBytesWrittenLog);
            //return error to given client fifo
        }
        if(clientCounter > numClients && strcmp(req.connect,"tryConnect") == 0)
        {
            kill(req.pid, SIGTERM);
        }
        else
        {
            sem_wait(queue);
            find_proper_location(req.pid);
            // clients[clientCounter]= req.pid;
            clientCounter++;
            numBytesWrittenLog = snprintf(buffer,BUFF_SIZE,">>Client PID %ld connected as client%2d \n", req.pid,clientCounter);
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
                    char splitted_command[MAX_ARGUMENT][BUFF_SIZE];
                    enum command_level lev;
                    snprintf(clientSem, CLIENT_SEM_NAME_LEN, CLIENT_SEM_TEMP, (long)req.pid);
                    sem_temp = sem_open(clientSem, 0);
                    if(sem_temp == SEM_FAILED){
                        perror("sem_open error\n");
                    }
                    snprintf(clientSem2, CLIENT_SEM_NAME_LEN, CLIENT_SEM2_TEMP, (long)req.pid);
                    sem_temp2 = sem_open(clientSem2,0);

                    if(sem_temp2 == SEM_FAILED){
                        perror("sem_open error\n");
                    }

                    char logChild[BUFF_SIZE];
                    int childBufferWritten = 0;
                    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN,CLIENT_FIFO,(long) req.pid);

                    // int numBytesWrittenLogChld = snprintf(logChild, BUFF_SIZE, "client fifo name is %s\n",clientFifo);
                    // write(STDOUT_FILENO, logChild,numBytesWrittenLogChld);
                    // memset(logChild, 0, numBytesWrittenLogChld);

                    int clientReadFd = open(clientFifo, O_RDONLY);
                    int clientWriteFd  = open(clientFifo, O_WRONLY);
                    // char buffChld[BUFF_SIZE];
                    // char doNotUse[MAX_ARGUMENT][BUFF_SIZE];
                    // int numBytesWrittenLogChld = snprintf(buffChld,BUFF_SIZE,">>Clint fifo started\n");
                    // write(STDOUT_FILENO, buffChld, numBytesWrittenLogChld);
                    // memset(buffChld, 0, numBytesWrittenLogChld);
                    for(;;)
                    {
                        memset(logChild, 0, BUFF_SIZE);
                        sem_wait(sem_temp);
                        // numBytesWrittenLogChld = snprintf(buffChld,BUFF_SIZE,">>Clint2 fifo started\n");
                        // write(STDOUT_FILENO, buffChld, numBytesWrittenLogChld);
                        // memset(buffChld, 0, numBytesWrittenLogChld);
                        if(read(clientReadFd, &resp, sizeof(struct response)) != sizeof(struct response))
                        {
                            perror("read");
                            continue;
                        }

                        lev = handle_client_request(resp.command,splitted_command);
                        if( lev == INVALID_COMMAND){
                            childBufferWritten = snprintf(logChild, BUFF_SIZE, "Invalid command number entered : %s\n", resp.command);
                            memset(resp.command,0, BUFF_SIZE);
                            strcpy(resp.command, logChild);
                            resp.command[childBufferWritten] = '\0';
                            write(clientWriteFd, &resp, sizeof(struct response));
                        }
                        else{
                            int sizeOfCommand = splitStringIntoArray_S(resp.command, ' ', splitted_command);
                            send_response(splitted_command,clientWriteFd, lev, argv[1],sizeOfCommand,req.pid);
                            //bir tane daha fork gerekebilir mi ?
                        }

                        if(strncmp(resp.command, "upload", 6) == 0){
                            int lastpid = fork();
                            if(lastpid == -1){
                                perror("fork");
                                exit(EXIT_FAILURE);
                            }
                            else if(lastpid == 0){
                                close(clientWriteFd);
                                close(clientReadFd);
                                // sem_t *mutex, *full,*empty;
                                strcpy(filen, argv[1]);
                                strcat(filen, "/");
                                strcat(filen, splitted_command[1]);
 
                                consumerrr(filen);
                                exit(EXIT_SUCCESS);
                            }
                        }
                        else if(strncmp(resp.command, "download", 8) == 0)
                        {
                            int lastpid = fork();
                            if(lastpid == -1){
                                perror("fork");
                                exit(EXIT_FAILURE);
                            }
                            else if(lastpid == 0){
                                close(clientWriteFd);
                                close(clientReadFd);
                                strcpy(filen, argv[1]);
                                strcat(filen, "/");
                                strcat(filen, splitted_command[1]);
                                printf("filen %s \n", filen);
                                producerr(filen);
                                exit(EXIT_SUCCESS);
                            }
                        }
                        // strcpy(resp.command, logChild);
                        // resp.command[childBufferWritten] = '\0';
                        // memset(logChild, 0, BUFF_SIZE);
                        // sem_wait(sem_temp2);
                        // write(clientWriteFd, &resp, sizeof(struct response));
                        sem_post(sem_temp2);
                        for(int i = 0; i < MAX_ARGUMENT; i++){
                            memset(splitted_command[i], 0, BUFF_SIZE);
                        }
                        wait(NULL);

                    // }
                        // if(close(clientReadFd) == -1){
                        //     perror("close");
                        //     exit(EXIT_FAILURE);
                        // }
                    // if(close(clientWriteFd) == -1){
                    //     perror("close");
                    //     exit(EXIT_FAILURE);
                    }
                    // unlink(clientFifo);
                    // exit(EXIT_SUCCESS);

                break; 


                case -1:
                    perror("fork");
                    unlink(SERVER_FIFO);
                    close(serverFd);
                    close(dummyFd);
                break;

                default: 
                    child[fork_count++] = forkPid;
                    // clients[clientCounter-1] = req.pid;
                break;
            }
        }
        // else{
            //return invalid pid
        // }
        
    }
    unlink(clientFifo);
    
}