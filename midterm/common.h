#ifndef __OGUZ_COMMON_H__
#define __OGUZ_COMMON_H__

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <semaphore.h>


#define ERR -1
#define SUCCESS 0
#define BUFF_SIZE 256
#define MAX_CLIENT 100
#define MAX_CHILD 100
#define MAX_ARGUMENT 4
#define SERVER_FIFO "/tmp/serverFifo"
#define CLIENT_FIFO "/tmp/clientFifo_%ld"
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO) + 20)
#define CLIENT_SEM_TEMP "sem.%ld"
#define CLIENT_SEM2_TEMP "sem2.%ld"
#define CLIENT_SEM_NAME_LEN (sizeof(CLIENT_SEM_TEMP) + 20)
#define SHM_NAME "/my_shared_mem"
#define SEM_EMPTY_NAME "/producer_semaphorex"
#define SEM_MUTEX_NAME "/consumer_semaphorex"
#define SEM_FULL_NAME "/full_semaphorex"
#define BUFFER_SIZE 4096
#define TEMP_DOWNLOAD_FIFO "/tmp/downfifo"
#define QUEUE_SEM "/queue_sem"
#define QUEUE_SEM_LEN (sizeof(CLIENT_SEM_TEMP) +1)

struct request{
    long int pid;
    long int serverPid;
    char connect[BUFF_SIZE];
};

struct response{
    char command[BUFF_SIZE];
};

enum command_level {
    INVALID_COMMAND,
    HELP, 
    LIST,
    READF,
    WRITET, 
    UPLOAD,
    DOWNLOAD, 
    ARCHIVE,
    KILL,
    QUIT,
};

int writeToLog(const char* log);
int countHowManyElementsWillExtract(const char *str, const char delim);
int splitStringIntoArray_I(const char* str, const char delim, char(*splitted)[BUFF_SIZE], int count);
int splitStringIntoArray_S(const char* str, const char delim, char(*splitted)[BUFF_SIZE]);
off_t get_file_size(const char *file_path);
void producer(const char *filename);
void consumerr(const char *filename);
long int consumerrr(const char *filename);
long int producerr(const char *filename);
#endif /* __OGUZ_COMMON_H__ */