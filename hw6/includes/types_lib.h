#ifndef __TYPES_LIB_H
#define __TYPES_LIB_H

#include <pthread.h>


struct files_inside
{
    char filename[512];
    char modification_time[256];
    unsigned int file_modes; 
};

struct socket_server
{
    char *server_src;
    int fd;
};



#endif