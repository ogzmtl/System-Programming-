#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>

#define MAX_WORKER 20
#define LOG_LENGTH 256
#define FILE_NAME_LEN 64
typedef struct fileInformation {
    char src_filename[FILE_NAME_LEN];
    char dest_filename[FILE_NAME_LEN];
    int src_fd; 
    int dest_fd;
}fileInfo;

typedef struct source_destination{
    char source_filename[FILE_NAME_LEN];
    char destination_filename[FILE_NAME_LEN];
}filen;

fileInfo *buffer;

void directoryOperation(DIR* dir, filen* file_manager){

    struct dirent* entry;
    int numBytes = 0;
    char log[LOG_LENGTH];

    while((entry = readdir(dir)) != NULL)
    {
        if(strcmp(entry->d_name,".") == 0 && strcmp(entry->d_name, "..") == 0){
            continue;
        }
        // recursive cagri burada yapilir. 
        numBytes = snprintf(log, LOG_LENGTH, "dirname %s\n", entry->d_name);
        write(STDOUT_FILENO, log, numBytes);
        memset(log, 0, numBytes);
    }
    // free(entry);
}

void* manager(void* arg)
{
    filen* managerParser = (filen*) arg;
    int numBytesWrittenManager = 0;
    char logManager[LOG_LENGTH];
    DIR* dir_stream;
    struct stat st;

    if(stat(managerParser->source_filename, &st) == -1)
    {
        if(errno == ENOENT){
            numBytesWrittenManager = snprintf(logManager, LOG_LENGTH, "Source filepath doesnot exists\n");
            write(STDOUT_FILENO, logManager, numBytesWrittenManager);
            memset(logManager, 0, numBytesWrittenManager);
            perror("stat()");
            pthread_exit(NULL);
        }
    }

    if(S_ISDIR(st.st_mode))
    {
        dir_stream = opendir(managerParser->source_filename);
        if(dir_stream == NULL){
            perror("opendir()");
            return NULL;
        }

        directoryOperation(dir_stream, managerParser);
        errno = 0; //according to man page of readdir()
                    //    If the end of the directory stream is reached, NULL is returned
                    //    and errno is not changed.  If an error occurs, NULL is returned
                    //    and errno is set to indicate the error.  To distinguish end of
                    //    stream from an error, set errno to zero before calling readdir()
                    //    and then check the value of errno if NULL is returned.
        // while(entry = readdir(dir_stream))
        // {
        //     if(strcmp(entry->d_name,".") == 0 && strcmp(entry->d_name, "..") == 0){
        //         continue;
        //     }
        //     numBytesWrittenManager = snprintf(logManager, LOG_LENGTH, "dirname %s\n", entry->d_name);
        //     write(STDOUT_FILENO, logManager, numBytesWrittenManager);
        //     memset(logManager, 0, numBytesWrittenManager);
        // }

        numBytesWrittenManager = snprintf(logManager, LOG_LENGTH, "it is directory\n");
        write(STDOUT_FILENO, logManager, numBytesWrittenManager);
        memset(logManager, 0, numBytesWrittenManager);
    }    // else if (S_ISREG(st.st_mode)) // else if(S_ISFIFO(st.st_mode))
    else{
        numBytesWrittenManager = snprintf(logManager, LOG_LENGTH, "it is not directory continue copy\n");
        write(STDOUT_FILENO, logManager, numBytesWrittenManager);
        memset(logManager, 0, numBytesWrittenManager);
        return NULL;
    }
    numBytesWrittenManager = snprintf(logManager, LOG_LENGTH, "%s, %s\n", \
    managerParser->source_filename, managerParser->destination_filename);

    write(STDOUT_FILENO, logManager, numBytesWrittenManager);
    memset(logManager, 0, numBytesWrittenManager);
    closedir(dir_stream);
}

int main(int argc, char **argv)
{
    int numBytesWrittenLog = 0;
    char logBuffer[LOG_LENGTH];
    char *endptr, *strCmp= NULL;
    long int numOfWorker = 0;
    long int bufferSize = 0;
    pthread_t managerThread; 
    pthread_t worker[MAX_WORKER];
    filen filenames;

    /*START OF USAGE ERROR */
    if(argc != 5){
        numBytesWrittenLog = snprintf(logBuffer,LOG_LENGTH, "Usage %s: <buffer_size> <#of_worker> <src_filepath> <dst_filepath>\n", argv[0] );
        write(STDOUT_FILENO, logBuffer, numBytesWrittenLog);
        memset(logBuffer, 0, numBytesWrittenLog);
        exit(EXIT_FAILURE);
    }

    bufferSize = atoi(argv[1]);
    if(bufferSize == 0){
        numBytesWrittenLog = snprintf(logBuffer,LOG_LENGTH, "Invalid buffSize %s: <buffer_size> <#of_worker> <src_filepath> <dst_filepath>\n", argv[0] );
        write(STDOUT_FILENO, logBuffer, numBytesWrittenLog);
        memset(logBuffer, 0, numBytesWrittenLog);
        exit(EXIT_FAILURE);       
    }

    numOfWorker = atoi(argv[2]);
    if(numOfWorker == 0) {
        numBytesWrittenLog = snprintf(logBuffer,LOG_LENGTH, "Is not Integer value for #worker\nUsage %s: <buffer_size> <#of_worker> <src_filepath> <dst_filepath>\n", argv[0] );
        write(STDOUT_FILENO, logBuffer, numBytesWrittenLog);
        memset(logBuffer, 0, numBytesWrittenLog);     
        exit(EXIT_FAILURE);     
    }
    /*END OF USAGE ERROR */
    if(MAX_WORKER < numOfWorker){
        numBytesWrittenLog = snprintf(logBuffer,LOG_LENGTH, "Greater then worker\n");
        write(STDOUT_FILENO, logBuffer, numBytesWrittenLog);
        memset(logBuffer, 0, numBytesWrittenLog);     
        exit(EXIT_FAILURE);  
    }

    buffer =(fileInfo*) malloc(bufferSize * sizeof(fileInfo));
    strcpy(filenames.source_filename, argv[3]);
    strcpy(filenames.destination_filename, argv[4]);


    pthread_create(&managerThread, NULL, manager, &filenames);

    pthread_join(managerThread, NULL);


    free(buffer);
    return 0;
}