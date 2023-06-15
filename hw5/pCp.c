#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>

#define MAX_PATH_LENGTH 256
#define MAX_BUFFER_SIZE 10

typedef struct {
    int fd_src;
    int fd_dest;
    char src_filename[MAX_PATH_LENGTH];
    char dest_filename[MAX_PATH_LENGTH];
} FileInfo;

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_empty_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_full_cond = PTHREAD_COND_INITIALIZER;

FileInfo buffer[MAX_BUFFER_SIZE];
int buffer_count = 0;
int buffer_index = 0;
int done = 0;
int num_files_copied = 0;
int num_dirs_copied = 0;


void* producer_thread(void* arg) {
    char** directories = (char**)arg;
    char* src_directory = directories[0];
    char* dest_directory = directories[1];

    DIR* dirp = opendir(src_directory);
    if (dirp == NULL) {
        perror("Failed to open source directory");
        return NULL;
    }

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char src_path[MAX_PATH_LENGTH];
        char dest_path[MAX_PATH_LENGTH];

        snprintf(src_path, sizeof(src_path), "%s/%s", src_directory, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_directory, entry->d_name);

        struct stat st;
        if (lstat(src_path, &st) == -1) {
            perror("Failed to get file information");
            continue;
        }

        FileInfo file_info;
        strncpy(file_info.src_filename, src_path, sizeof(file_info.src_filename));
        strncpy(file_info.dest_filename, dest_path, sizeof(file_info.dest_filename));

        if (S_ISDIR(st.st_mode)) {
            mkdir(dest_path, st.st_mode);
            num_dirs_copied++;
        } else if (S_ISREG(st.st_mode)) {
            file_info.fd_src = open(src_path, O_RDONLY);
            if (file_info.fd_src == -1) {
                perror("Failed to open source file");
                continue;
            }

            file_info.fd_dest = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);
            if (file_info.fd_dest == -1) {
                perror("Failed to open destination file");
                close(file_info.fd_src);
                continue;
            }

            num_files_copied++;
            total_bytes_copied += st.st_size;
        } else if (S_ISFIFO(st.st_mode)) {
            mkfifo(dest_path, st.st_mode);
            num_files_copied++;
        }

        pthread_mutex_lock(&buffer_mutex);
        while (buffer_count == MAX_BUFFER_SIZE) {
            pthread_cond_wait(&buffer_full_cond, &buffer_mutex);
        }

        buffer[buffer_index] = file_info;
        buffer_count++;
        buffer_index = (buffer_index + 1) % MAX_BUFFER_SIZE;

        pthread_cond_signal(&buffer_empty_cond);
        pthread_mutex_unlock(&buffer_mutex);
    }

    closedir(dirp);

    pthread_mutex_lock(&buffer_mutex);
    done = 1;
    pthread_cond_broadcast(&buffer_empty_cond);
    pthread_mutex_unlock(&buffer_mutex);

    return NULL;
}

void* consumer_thread(void* arg) {
    while (1) {
        pthread_mutex_lock(&buffer_mutex);
        while (buffer_count == 0 && !done) {
            pthread_cond_wait(&buffer_empty_cond, &buffer_mutex);
        }

        if (buffer_count == 0 && done) {
            pthread_mutex_unlock(&buffer_mutex);
            break;
        }

        FileInfo file_info = buffer[buffer_index];
        buffer_count--;
        buffer_index = (buffer_index + 1) % MAX_BUFFER_SIZE;

        pthread_cond_signal(&buffer_full_cond);
        pthread_mutex_unlock(&buffer_mutex);

        if (file_info.fd_src != -1) {
            off_t bytes_copied = 0;
            ssize_t bytes_read, bytes_written;
            char buffer[1024];

            while ((bytes_read = read(file_info.fd_src, buffer, sizeof(buffer))) > 0) {
                bytes_written = write(file_info.fd_dest, buffer, bytes_read);
                if (bytes_written == -1) {
                    perror("Failed to write to destination file");
                    break;
                }

                bytes_copied += bytes_written;
            }

            close(file_info.fd_src);
            close(file_info.fd_dest);

            printf("Copied: %s (%ld bytes)\n", file_info.src_filename, bytes_copied);
        } else {
            printf("Created: %s\n", file_info.src_filename);
        }
    }

    return NULL;
}
void sigint_handler(int signum) {
    signal = 1;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: %s <buffer_size> <num_consumers> <src_directory> <dest_directory>\n", argv[0]);
        return 1;
    }

    int buffer_size = atoi(argv[1]);
    int num_consumers = atoi(argv[2]);
    char** directories = &argv[3];

    pthread_t producer;
    pthread_t consumers[num_consumers];


    if (pthread_create(&producer, NULL, producer_thread, directories) != 0) {
        perror("Failed to create producer thread");
        return 1;
    }

    for (int i = 0; i < num_consumers; i++) {
        if (pthread_create(&consumers[i], NULL, consumer_thread, NULL) != 0) {
            perror("Failed to create consumer thread");
            return 1;
        }
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    pthread_join(producer, NULL);

    for (int i = 0; i < num_consumers; i++) {
        pthread_join(consumers[i], NULL);
    }

    gettimeofday(&end_time, NULL);
    double total_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

    printf("Total files copied: %d\n", num_files_copied);
    printf("Total directories copied: %d\n", num_dirs_copied);
    printf("Total bytes copied: %lld\n", (long long)total_bytes_copied);
    printf("Total time taken: %.6f seconds\n", total_time);

    return 0;
}
