#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>

struct file_copy_buffer
{
    int source_file_descriptor;
    int destination_file_descriptor;
    char source_filename[512];
    char destination_filename[512];
};

struct file_copy_buffer *buffer;

struct filenames_buffer
{
    char filenames[2][512];
    int buffer_size;
};

int buffer_size = 0;
int buffer_index = 0;
int bufferCounter = 0;
int done = 0;
int signalint = 0 ; 
int num_files_copied = 0;
int num_dirs_copied = 0;
int num_bytes_copied = 0;
int num_of_fifo_copied = 0;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_produce = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_consume = PTHREAD_COND_INITIALIZER;

void sigint_handler(int signum) {
    printf("signal handled%d\n", signum);
    signalint = 1;
    free(buffer);
}

void copyFile(const char *source, const char *destination)
{
    char destination_copy[512] = "\0";
    char source_copy[512] = "\0";
    strcpy(source_copy, source);
    strcpy(destination_copy, destination);
    mode_t mode = S_IRUSR | S_IWUSR |
                  S_IRGRP | S_IWGRP |
                  S_IROTH | S_IWOTH;
    // signal

    char *token = strtok(source_copy, "/");
    char temp[512]="\0";

    while (token != NULL)
    { // printing each token
        strcpy(temp, token);
        token = strtok(NULL, "/");
    }

    strcat(destination_copy, "/");
    strcat(destination_copy, temp);
    memset(source_copy, 0, sizeof(source_copy));
    strcpy(source_copy, source);

    int fd_source = open(source_copy, O_RDONLY | O_CREAT, mode);
    if(fd_source == -1)
    {
        perror("open");
        return;

    }

    int fd_target = open(destination_copy, O_WRONLY | O_CREAT | O_TRUNC, mode);
    if (fd_target == -1)
    {
        close(fd_source);
        perror("open");
        return; 
    }

    int s = pthread_mutex_lock(&mtx);
    if (s != 0)
    {
        perror("pthread_mutex_lock");
        return;
    }

    while (bufferCounter == buffer_size)
    {
        pthread_cond_wait(&cond_produce, &mtx);
    }

    buffer[bufferCounter].destination_file_descriptor = fd_target;
    strcpy(buffer[bufferCounter].destination_filename, destination_copy);
    buffer[bufferCounter].source_file_descriptor = fd_source;
    strcpy(buffer[bufferCounter].source_filename, source_copy);
    bufferCounter++;

    s = pthread_cond_broadcast(&cond_consume);
    if (s != 0)
    {
        perror("pthread_cond_signal");
        return;
    }

    s = pthread_mutex_unlock(&mtx);
    if (s != 0)
    {
        perror("pthread_mutex_unlock");
        return;
    }

    return;
}

void createDestinationFolder(const char *source, const char *destination) {
    struct stat st;
    if (stat(source, &st) == -1) {
        perror("Failed source directory information");
        return;
    }

    mode_t mode = st.st_mode & 0777;

    if (mkdir(destination, mode) == -1 && errno != EEXIST) {
        perror("Failed create destination folder");
        return;
    }

}

void directorySearch(const char *source, char *destination)
{
    DIR *dir = opendir(source);

    if (dir)
    {
        char path[512] = "\0", *endptr = path;
        struct dirent *entry;
        strcpy(path, source);
        strcat(path, "/");

        endptr += strlen(source);
        int source_len = strlen(path);

        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }
            else
            {
                char new_temp[512] = "\0";
                strncpy(new_temp, path, source_len);
                fflush(stdout);
                memset(path + source_len, 0, sizeof(path) - source_len);
                strcpy(path, new_temp);
                strncpy(new_temp, path, source_len);

                struct stat info;
                strcat(endptr, entry->d_name);
                if (!stat(path, &info))
                {
                    if (S_ISDIR(info.st_mode))
                    {
                        char new_target[512] = "\0";
                        strcat(new_target, destination);
                        strcat(new_target, "/");
                        strcat(new_target, entry->d_name);

                        if (mkdir(new_target, info.st_mode) == -1 && errno != EEXIST)
                        {
                            perror("mkdir");
                            return;
                        }
                        num_dirs_copied++;
                        directorySearch(path, new_target);
                        strcpy(path, new_temp);
                        strcat(path, "/");
                    }
                    else if (S_ISREG(info.st_mode))
                    {
                        copyFile(path, destination);
                        num_files_copied++;
                        num_bytes_copied += info.st_size;
                    }else if (S_ISFIFO(info.st_mode)) {
                        char new_target[512] = "\0";
                        strcat(new_target, destination);
                        strcat(new_target, "/");
                        strcat(new_target, entry->d_name);
                        mkfifo(new_target, info.st_mode);
                        num_of_fifo_copied++;
                    }
                }
            }
        }
    }
    closedir(dir);
}


void *producer(void *arg)
{
    char filenames[2][512] = {{"\0"}};
    struct filenames_buffer *tempStruct = (struct filenames_buffer *)arg;
    strcpy(filenames[0], tempStruct->filenames[0]);
    strcpy(filenames[1], tempStruct->filenames[1]);
    
    directorySearch(filenames[0], filenames[1]);
    done = 1;
    pthread_exit(NULL);
}

void consume()
{
    char file_buffer[4096] = "\0";
    int BUFF_SIZE = 4096;
    int num_of_bytes_readed = 1;
    int num_of_copied = 0;
    struct file_copy_buffer file_info = buffer[bufferCounter];

    while (num_of_bytes_readed > 0)
    {
        memset(file_buffer, 0, BUFF_SIZE);
        num_of_bytes_readed = read(file_info.source_file_descriptor, file_buffer, BUFF_SIZE);
        if (num_of_bytes_readed == -1)
        {
            perror("read");
            return;
        }
        int num_of_written = write(file_info.destination_file_descriptor, file_buffer, num_of_bytes_readed);
        if (num_of_written == -1)
        {
            perror("write");
            return;
        }
        num_of_copied += num_of_written;
        
    }
    printf("Copied: %s, total bytes copies : %d bytes)\n", file_info.source_filename, num_of_copied);
    fflush(stdout);

    close(file_info.destination_file_descriptor);
    close(file_info.source_file_descriptor);    

}

void *consumer(void *arg)
{
    while(1)
    {
        int s = pthread_mutex_lock(&mtx);
        if (s != 0)
        {
            perror("pthread_mutex_lock");
            return (void*)1;
        }

        while (bufferCounter == 0 && !done)
        {
            pthread_cond_wait(&cond_consume, &mtx);
        }

        if((bufferCounter == 0 || done) && signalint != 1){
            pthread_mutex_unlock(&mtx);
            break;
        }

        bufferCounter--;
        consume();
        sleep(1);
        
        buffer_index = (buffer_index + 1)%buffer_size;
        // printf("file %s, copied\n", buffer[bufferCounter+1].source_filename);

        s = pthread_cond_broadcast(&cond_produce);
        if (s != 0)
        {
            perror("pthread_cond_signal");
            pthread_mutex_unlock(&mtx);
            return (void *)1;
        }

        s = pthread_mutex_unlock(&mtx);
        sleep(1);
        if (s != 0)
        {
            perror("pthread_mutex_unlock");
            return (void*)1;
        }
    }

    pthread_exit(NULL);
}


int main(int argc, char **argv)
{
    struct timeval start_time, end_time;

    if (argc != 5)
    {
        return -1;
    }

    pthread_t thread[atoi(argv[2]) +1];
    char filenames[2][512];

    buffer_size = atoi(argv[1]);
    buffer = (struct file_copy_buffer *)malloc(atoi(argv[1]) * sizeof(struct file_copy_buffer));

    strcpy(filenames[0], argv[3]);
    strcpy(filenames[1], argv[4]);

    createDestinationFolder(filenames[0], filenames[1]);
    gettimeofday(&start_time, NULL);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigint_handler;
    sigaction(SIGINT, &sa, NULL);
    int s = pthread_create(&thread[0], NULL, producer, (void *)filenames);
    if (s != 0)
    {
        perror("pthread_create");
        return 1;
    }

    for (int i = 1; i < atoi(argv[2]) + 1; i++)
    {
        s = pthread_create(&thread[i], NULL, consumer, NULL);
        if (s != 0)
        {
            perror("pthread_create");
            return 1;
        }
    }

    s = pthread_join(thread[0], NULL);
    if (s != 0)
    {
        perror("pthread_join");
        return 1;
    }

    for (int i = 1; i < atoi(argv[2] + 1); i++)
    {
        pthread_join(thread[i], NULL);
    }
    gettimeofday(&end_time, NULL);

    double time = end_time.tv_sec - start_time.tv_sec + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

    printf("Total time : %lf\n", time);
    printf("Total files copied: %d\n", num_files_copied);
    printf("Total directories copied: %d\n", num_dirs_copied);
    printf("Total fifo copied: %d \n", num_of_fifo_copied);
    printf("Total bytes copied: %lld\n", (long long)num_bytes_copied);
    printf("Total time taken: %lf seconds\n", time);
    
    fflush(stdout);

    free(buffer);
    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&cond_produce);
    pthread_cond_destroy(&cond_produce);

    exit(EXIT_SUCCESS);
}