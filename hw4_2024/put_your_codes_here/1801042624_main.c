#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include "queue.h"

#define MAX_WORKER 20
#define LOG_LENGTH 256
#define FILE_NAME_LEN 128
#define MAX_LOG 1024
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

fileInfo *buffers;
int buffer_count = 0;
int directory_count = 0;
int regular_file_count = 0;
int fifo_count = 0;
pthread_mutex_t buffer_mutex;
pthread_cond_t full;
pthread_cond_t empty; 
int MAX_BUFF_SIZE = 0;
long int totalBytesCopied = 0;
Queue *myQueueBuffer;
/* run flags */
int running_status = 1;
int done = 0;

/*external initialization*/
Queue* buffer;
int counter = 0;
int last = 0;
int size = 0;

int buffer_size_counter =0;

void handler(int signum){
    if(signum == SIGINT)
    {
        running_status = 0;
    }
}

void copyAll(char* src, char* dst)
{
    DIR* dir_stream;
    struct stat st; 
    struct dirent* entry; 
    char log_for_copying[LOG_LENGTH];
    int num_log_written = 0;
    char relative_filepath_src[1024];
    char relative_filepath_dst[1024];
    

    if(stat(src, &st) == -1)
    {
        if(errno == ENOENT){
            num_log_written = snprintf(log_for_copying, LOG_LENGTH, "Source filepath doesnot exists\n");
            write(STDOUT_FILENO, log_for_copying, num_log_written);
            memset(log_for_copying, 0, num_log_written);
            perror("stat()");
            return; // may return false value 
        }
    }
    dir_stream = opendir(src);
    if(dir_stream == NULL){
            num_log_written = snprintf(log_for_copying, LOG_LENGTH, "Source filepath doesnot exists\n");
            write(STDOUT_FILENO, log_for_copying, num_log_written);
            memset(log_for_copying, 0, num_log_written);
            perror("opendir()");
            return; // may return false value 
    }
    errno = 0;      //according to man page of readdir()
                    //    If the end of the directory stream is reached, NULL is returned
                    //    and errno is not changed.  If an error occurs, NULL is returned
                    //    and errno is set to indicate the error.  To distinguish end of
                    //    stream from an error, set errno to zero before calling readdir()
                    //    and then check the value of errno if NULL is returned.
    while(((entry =readdir(dir_stream)) != NULL) && running_status == 1)
    {
        if(strcmp(entry->d_name,".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            
            if(entry->d_type == DT_DIR)
            {
                directory_count++;
                // DIR* recursiveDirectory;
                // filen* recursiveFileManager;

                sprintf(relative_filepath_src, "%s/%s", src, entry->d_name);
                sprintf(relative_filepath_dst, "%s/%s", dst, entry->d_name);

                mkdir(relative_filepath_dst, st.st_mode);
                //perror ekle
                   
                copyAll(relative_filepath_src, relative_filepath_dst);        
            }
            else if(entry->d_type == DT_REG)
            {
                Queue temp_queue;
                struct stat st_file;
                regular_file_count++;
                // pthread_mutex_lock()
                //mutex lock
                pthread_mutex_lock(&buffer_mutex);

                while(buffer_size_counter == MAX_BUFF_SIZE)
                {
                    pthread_cond_wait(&empty, &buffer_mutex);
                }
                sprintf(relative_filepath_src, "%s/%s", src, entry->d_name);
                sprintf(relative_filepath_dst, "%s/%s", dst, entry->d_name);
                stat(relative_filepath_src, &st_file);
                // printf("%s\n", relative_filepath_dst);
                
                int src_fd = open(relative_filepath_src, O_RDONLY);
                if(src_fd == -1){
                    perror("src open()\n");
                    return;
                }

                int dst_fd = open(relative_filepath_dst, O_WRONLY | O_CREAT | O_TRUNC, 0777);
                if(dst_fd == -1)
                {
                    perror("dst open()\n");
                    close(src_fd);
                    return;
                }
                // printf("%d---%d---isfull:%d \n", dst_fd, src_fd, buffer_size_counter);
                memset(temp_queue.src_filename, 0, MAX_FILENAME_LEN);
                memset(temp_queue.dest_filename, 0, MAX_FILENAME_LEN);
                
                strcpy(temp_queue.src_filename, relative_filepath_src);
                strcpy(temp_queue.dest_filename, relative_filepath_dst);
                temp_queue.src_fd = src_fd;
                temp_queue.dest_fd = dst_fd;

                if(enqueue(temp_queue)){
                    buffer_size_counter++;
                } // may be if check

                
                // num_log_written = sprintf(log_for_copying, "%d: %d, %d\n",buffer_size_counter, counter, last);
                write(STDOUT_FILENO, log_for_copying, num_log_written);
                memset(log_for_copying, 0, num_log_written);
                // sleep(2);
                
                pthread_cond_broadcast(&full);
                pthread_mutex_unlock(&buffer_mutex);
            }
        }

    }
    closedir(dir_stream);
}


void* manager(void* arg)
{
    // sleep(1);
    filen* managerParser = (filen*) arg;

    copyAll(managerParser->source_filename, managerParser->destination_filename);
    // printf(" Number of Directories: %d\n", directory_count);
    pthread_mutex_lock(&buffer_mutex);
    done = 1;
    pthread_cond_broadcast(&full); // Wake up all waiting consumers
    pthread_mutex_unlock(&buffer_mutex);
    // pthread_exit(NULL);

}

void* consumer(void* arg)
{
    int bytes_read = 0;
    int bytes_write = 0;
    char log[MAX_LOG];
    char log_consumer[MAX_LOG];
    int num_log = 0;

    // sleep(10);
    while( running_status && (!done || (buffer_size_counter != 0)))
    {
        pthread_mutex_lock(&buffer_mutex);

        while(buffer_size_counter == 0)
        {
            if(done){
                pthread_cond_broadcast(&empty);
                pthread_mutex_unlock(&buffer_mutex);
                return NULL; 
            }
            pthread_cond_wait(&full, &buffer_mutex);
        }
        
        // num_log = sprintf(log_consumer, "CONSUMER %d: %d, %d\n",buffer_size_counter, counter, last);
        write(STDOUT_FILENO, log_consumer, num_log);
        memset(log_consumer, 0, num_log);
        Queue item = dequeue();
        // if(item != NULL){
            buffer_size_counter--;
            // num_log = sprintf(log_consumer, "CONSUMER z%d: %d, %d\n",buffer_size_counter, counter, last);
            write(STDOUT_FILENO, log_consumer, num_log);
            memset(log_consumer, 0, num_log);
            pthread_cond_broadcast(&empty);
            pthread_mutex_unlock(&buffer_mutex);
            
            // num_log = sprintf(log_consumer, "CONSUMER z%d: %d, %d\n",item.src_fd, item.dest_fd, last);
            write(STDOUT_FILENO, log_consumer, num_log);
            memset(log_consumer, 0, num_log);
            
            while((bytes_read = read(item.src_fd, log, MAX_LOG)) > 0){
                totalBytesCopied += bytes_read;
                bytes_write = write(item.dest_fd, log, bytes_read);
                memset(log, 0, MAX_LOG);
                if(bytes_write < bytes_read){
                    perror("write()");
                }
            }

            close(item.src_fd);
            close(item.dest_fd);           
        // }
        // else{
        //     pthread_cond_broadcast(&empty);
        //     pthread_mutex_unlock(&buffer_mutex);
        // }
    }
}

int main(int argc, char **argv)
{
    int numBytesWrittenLog = 0;
    char logBuffer[LOG_LENGTH];
    // char *endptr, *strCmp= NULL;
    long int numOfWorker = 0;
    long int bufferSize = 0;
    pthread_t managerThread; 
    pthread_t* worker;
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

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handler;
    sigaction(SIGINT, &sa, NULL);

    buffers =(fileInfo*) malloc(bufferSize * sizeof(fileInfo));
    worker = (pthread_t *) malloc(numOfWorker * sizeof(pthread_t)); 
    MAX_BUFF_SIZE = bufferSize;
    init(bufferSize);
    strcpy(filenames.source_filename, argv[3]);
    strcpy(filenames.destination_filename, argv[4]);
    pthread_mutex_init(&buffer_mutex, NULL);
    pthread_cond_init(&empty, NULL);
    pthread_cond_init(&full, NULL);

    pthread_create(&managerThread, NULL, manager, &filenames);
    for(int i = 0; i < numOfWorker; i++){
        pthread_create(&worker[i], NULL, consumer, NULL);
    }

    struct timeval initTime;

    if (gettimeofday(&initTime, NULL) != 0)
    {
        fprintf(stderr, "gettimeofday error\n");
        return 1;
    }

    pthread_join(managerThread, NULL);
    long int initmsec = initTime.tv_sec * 1000 + initTime.tv_usec / 1000;
    for(int i = 0; i < numOfWorker; i++){
        pthread_join(worker[i], NULL);
    }

    struct timeval lastTime;

    if (gettimeofday(&lastTime, NULL) != 0)
    {
        fprintf(stderr, "gettimeofday error\n");
        return 1;
    }
    long int lastmsec = lastTime.tv_sec * 1000 + lastTime.tv_usec / 1000;
    numBytesWrittenLog = snprintf(logBuffer,LOG_LENGTH, "---------------STATS---------------\n");
    write(STDOUT_FILENO, logBuffer, numBytesWrittenLog);
    memset(logBuffer, 0, numBytesWrittenLog);

    numBytesWrittenLog = snprintf(logBuffer,LOG_LENGTH, "Worker: %ld, Buffer Size %ld\n", numOfWorker, bufferSize);
    write(STDOUT_FILENO, logBuffer, numBytesWrittenLog);
    memset(logBuffer, 0, numBytesWrittenLog);

    numBytesWrittenLog = snprintf(logBuffer,LOG_LENGTH, "Total File: %d\nTotal Directory: %d\n",regular_file_count,directory_count);
    write(STDOUT_FILENO, logBuffer, numBytesWrittenLog);
    memset(logBuffer, 0, numBytesWrittenLog);

    numBytesWrittenLog = snprintf(logBuffer,LOG_LENGTH, "Total Bytes Copied: %ld\n",totalBytesCopied);
    write(STDOUT_FILENO, logBuffer, numBytesWrittenLog);
    memset(logBuffer, 0, numBytesWrittenLog);
    
    long int totalmsec = lastmsec - initmsec;

    int minutes = totalmsec / (1000 * 60);
    totalmsec %= (1000 * 60);
    int seconds = totalmsec / 1000;
    int milliseconds_part = totalmsec % 1000;


    numBytesWrittenLog = snprintf(logBuffer,LOG_LENGTH, "Total Time : %02d:%02d.%03d(min:sec:msec)\n", minutes, seconds, milliseconds_part);
    write(STDOUT_FILENO, logBuffer, numBytesWrittenLog);
    memset(logBuffer, 0, numBytesWrittenLog);
    
    
    pthread_cond_destroy(&empty);
    pthread_cond_destroy(&full);
    pthread_mutex_destroy(&buffer_mutex);

    destroy();
    
    free(worker);
    free(buffers);
    return 0;
}