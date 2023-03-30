#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int dup(int oldfd);
int dup2(int oldfd, int newfd);


int main(){

    int fd, fd_dup, fd_dup2; 
    int wr_orgnl, wr_dup, wr_dup2; 
    int close_orgnl, close_duplicated, close_duplicated2;
    int fd_original, fd_duplicated,fd_duplicated2; 
    mode_t mode = S_IRUSR | S_IWUSR |
                  S_IRGRP | S_IWGRP ;
    int flags = O_CREAT | O_RDWR; 

    // fd_dup2 = dup2(15, 5);
    // printf("fd_dup2 : %d\n", fd_dup2);
    
    fd = open("part2.txt", flags, mode);
    

    fd_dup = dup(fd);
    char* original_buffer = "Hello from original file descriptor.\n";
    wr_orgnl = write(fd,original_buffer, strlen(original_buffer));

    char* duplicated_buffer = "Hello from dup file descriptor.\n";
    wr_dup = write(fd_dup,duplicated_buffer, strlen(duplicated_buffer));


    fd_dup2 = dup2(fd, fd_dup);
    char* dup2_buffer = "Hello from dup2 file descriptor.\n";
    wr_dup2 = write(fd_dup2,dup2_buffer, strlen(dup2_buffer));

    if(-1 == close(fd)){
        perror("close");
        return -1; 
    }

    if(fd_dup2 != fd_dup){
        if(-1 ==close(fd_dup)){
            perror("close");
            return -1;
        }
    }

    if(-1 ==close(fd_dup2)){
        perror("close");
        return -1;
    }
    return 0; 
}

int dup(int oldfd){

    int is_file_opened = fcntl(oldfd, F_GETFL);

    if (-1 == is_file_opened) {
        errno = EBADF;
        return -1;
    }

    int new_file_descriptor = fcntl(oldfd, F_DUPFD);
    if (-1 == new_file_descriptor) {
        return -1;
    }

    return new_file_descriptor; 
}

int dup2(int oldfd, int newfd){

    int is_file_opened = fcntl(oldfd, F_GETFL);

    if (-1 == is_file_opened) {
        errno = EBADF;
        return -1;
    }
    if(oldfd == newfd){
        return newfd; 
    }
    else{
        if(-1 != fcntl(newfd, F_GETFD)){/*According to the manual page of dup2() = the close is performed silently (i.e., any*/
            close(newfd);               /*errors during the close are not reported by dup2()).*/
        }
        
        if(-1 == fcntl(oldfd, F_DUPFD, newfd)){
            return -1; 
        }
    }
    return newfd;
}

