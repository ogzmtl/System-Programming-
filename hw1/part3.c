#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int dup(int oldfd){

    int is_file_opened = fcntl(oldfd, F_GETFL);

    if (-1 == is_file_opened) {
        return -1;
    }

    int new_file_descriptor = fcntl(oldfd, F_DUPFD);
    if (-1 == new_file_descriptor) {
        return -1;
    }

    return new_file_descriptor; 
}

int main(){

    mode_t mode = S_IRUSR | S_IWUSR |
                  S_IRGRP | S_IWGRP ;

    int flags = O_CREAT | O_RDWR;
    int fd_original, fd_duplicated;
    int wr_orgnl, wr_dup;
    int close_orgnl, close_duplicated;

    fd_original = open("part3.txt", flags, mode); 

    if(-1 == fd_original){
        perror("open");
        return -1; 
    }

    fd_duplicated = dup(fd_original);
    if(-1 == fd_duplicated){
        perror("duplicate");
        return -1; 
    }

    char* original_buffer = "Hello from original file descriptor.\n";
    wr_orgnl = write(fd_original,original_buffer, strlen(original_buffer));

    char* duplicated_buffer = "Hello from duplicated file descriptor.\n";
    wr_dup = write(fd_duplicated,duplicated_buffer, strlen(duplicated_buffer));

    long int offset_fd = lseek(fd_original, 0, SEEK_CUR);
    long int offset_fd_duplicated = lseek(fd_duplicated, 0, SEEK_CUR);

    printf("original file descriptor offset: %ld\nduplicated file descriptor offset: %ld\n", offset_fd, offset_fd_duplicated);

    if(-1 == close(fd_original)){
        perror("close");
        return -1; 
    }
    if(-1 ==close(fd_duplicated)){
        perror("close");
        return -1;
    }
}
