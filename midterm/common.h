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

#define ERR -1
#define SUCCESS 0
#define BUFF_SIZE 256
#define SERVER_FIFO "/tmp/serverFifo"
#define CLIENT_FIFO "/tmp/clientFifo_%ld"
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO) + 20)
#define CLIENT_SEM_TEMP "sem.%ld"
#define CLIENT_SEM2_TEMP "sem2.%ld"
#define CLIENT_SEM_NAME_LEN (sizeof(CLIENT_SEM_TEMP) + 20)

struct request{
    pid_t pid;
    pid_t serverPid;
};

struct response{
    char command[256];
};

int writeToLog(const char* log);

#endif /* __OGUZ_COMMON_H__ */