
#ifndef __FIFO_H
#define __FIFO_H

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define SERVER_FIFO "/tmp/bibofifo"
#define CLIENT_FIFO_TEMP "/tmp/bibofifo_cl.%ld"
#define CLIENT_FIFO_TEMP_READ "/tmp/bibofifo_rcl.%ld"
#define CLIENT_FIFO_TEMP_WRITE "/tmp/bibofifo_wcl.%ld"
#define CLIENT_FIFO_TEMP_LEN (sizeof(CLIENT_FIFO_TEMP_WRITE) + 20)

struct request
{
    pid_t pid;
    char data[1024];
};

struct response
{
    pid_t pid;
    char data[1024];
};

struct requestMainFifo
{
    pid_t pid;
    pid_t inp_pid;
    char data[1024];
};

struct responseMainFifo
{
    pid_t pid;
    pid_t inp_pid;
    char data[1024];
};

#endif