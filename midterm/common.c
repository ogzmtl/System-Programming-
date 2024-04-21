#include "common.h"


#define openFlags  O_CREAT | O_RDWR | O_NONBLOCK

int writeToLog(const char*log)
{
    const char *logFile = "log.txt";
    int log_fd; 
    unsigned int mode = S_IRUSR | S_IRGRP | S_IROTH | S_IXGRP | S_IXUSR | S_IXOTH;
    unsigned const int log_length = strlen(log);

    log_fd = open(logFile, openFlags | O_APPEND, mode);
    if(log_fd == -1){
        perror("open logfile");
        return ERR;
    }
    if(write(log_fd, log, log_length) <0){
        //is it necessary to close the log file when error occured
        if(close(log_fd) == -1)
        {
            perror("close log file error");
            return ERR;
        }
        perror("log file write error");
        return ERR;
    }
    if(close(log_fd) == ERR)
    {
        perror("close log file error");
        return ERR;
    }

    return SUCCESS;
}
