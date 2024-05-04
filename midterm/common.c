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
int splitStringIntoArray_S(const char* str, const char delim, char(*splitted)[BUFF_SIZE]){

    int count = countHowManyElementsWillExtract(str, delim);
    if(count > 4 || count < 1){
        return -1;
    }
    // printf("count : %d\n", count);
    return splitStringIntoArray_I(str, delim, splitted, count);
}

int splitStringIntoArray_I(const char* str, const char delim, char(*splitted)[BUFF_SIZE], int count){

    char *tmp = (char*)str;
    // splitted = (char**)malloc(sizeof(char*)*count);
    int inside_counter = 0;
    char temp_element[32];
    int i = 0;

    memset(temp_element, 0, sizeof(temp_element));

    while(*tmp){
        if(*tmp == delim)
        {
            temp_element[inside_counter] = '\0';
            // splitted[i] = (char*)calloc(inside_counter, sizeof(char));
            strcpy(splitted[i], temp_element);
            i++;
            memset(temp_element, 0, sizeof(temp_element));
            inside_counter = 0;
        }
        else{
            temp_element[inside_counter++] =*tmp;
        }
        tmp++;
    }
    temp_element[inside_counter] = '\0';
    // splitted[i] = (char*)calloc(inside_counter, sizeof(char));
    strcpy(splitted[i], temp_element);
    
    return count;
}
int countHowManyElementsWillExtract(const char *str, const char delim){

    char* temp = (char *)str;
    int count = 0;

    while(*temp){

        if(delim == *temp){
            count++;
        }
        temp++;
    }
    return count+1;
}

off_t get_file_size(const char *file_path) {
    struct stat st;
    if (stat(file_path, &st) == -1) {
        perror("Error getting file size");
        exit(EXIT_FAILURE);
    }
    return st.st_size;
}

void producer(const char *filename) {
    int shm_fd;
    char *shm_ptr;
    sem_t *empty, *full, *mutex;

    // Open or create the POSIX shared memory object
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Set the size of the shared memory object
    if (ftruncate(shm_fd, BUFFER_SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }

    // Map the shared memory object into the address space of the process
    shm_ptr = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    memset(shm_ptr, 0, BUFFER_SIZE);
    // Open or create the producer semaphore
    mutex = sem_open(SEM_MUTEX_NAME, O_CREAT, 0666, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open producer");
        exit(1);
    }

    // Open or create the consumer semaphore
    full = sem_open(SEM_FULL_NAME, O_CREAT, 0666, 0);
    if (full == SEM_FAILED) {
        perror("sem_open consumer");
        exit(1);
    }

    empty = sem_open(SEM_EMPTY_NAME, O_CREAT, 0666, 1);
    if (empty == SEM_FAILED) {
        perror("sem_open consumer");
        exit(1);
    }

    // Open the input file for reading
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        exit(1);
    }

    // Read the content of the file and write it to shared memory
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while (1) {
        printf("BITIRDIM3\n");
        bytes_read = fread(buffer, 1, BUFFER_SIZE, file);

        sem_wait(empty);
        sem_wait(mutex); // Wait until the consumer is ready
        printf("BITIRDIM2\n");
        if(bytes_read < 1){
            *shm_ptr = '\0';
            sem_post(mutex);
            sem_post(full); 
        }else{
            memcpy(shm_ptr, buffer, bytes_read);
            sem_post(mutex);
            sem_post(full);
        }
 // Signal the consumer that data is available
        if(bytes_read == 0)
        {
            break;
        }
        printf("BITIRDIM1\n");

    }
    printf("BITIRDIM5\n");
    fclose(file);
    // shm_ptr[0] = '\0';
    munmap(shm_ptr, BUFFER_SIZE);
    // sem_post(mutex);
    close(shm_fd);
    sem_close(empty);
    sem_close(full);
    sem_close(mutex);
    
    return; // Exit with status code 0 (success)
}
void producerr(const char *filename) {
    int shm_fd;
    char *shm_ptr;
    sem_t *producer_sem, *consumer_sem;
    printf("naber\n");
    // Open or create the POSIX shared memory object
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return;
    }

    // Set the size of the shared memory object
    if (ftruncate(shm_fd, BUFFER_SIZE) == -1) {
        perror("ftruncate");
        return;
    }

    // Map the shared memory object into the address space of the process
    shm_ptr = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        return;
    }

    // Open or create the producer semaphore
    producer_sem = sem_open(SEM_MUTEX_NAME, O_CREAT, 0666, 1);
    if (producer_sem == SEM_FAILED) {
        perror("sem_open producer");
        return;
    }

    // Open or create the consumer semaphore
    consumer_sem = sem_open(SEM_FULL_NAME, O_CREAT, 0666, 0);
    if (consumer_sem == SEM_FAILED) {
        perror("sem_open consumer");
        return;
    }

    // Open the input file for reading
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    // Read the content of the file and write it to shared memory
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    // while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        bytes_read = fread(buffer, 1, BUFFER_SIZE, file);
        sem_wait(producer_sem); // Wait until the consumer is ready
        memcpy(shm_ptr, buffer, bytes_read);
        sem_post(consumer_sem); // Signal the consumer that data is available
    // }
// 
    fclose(file);
    munmap(shm_ptr, BUFFER_SIZE);
    close(shm_fd);
    sem_close(producer_sem);
    sem_close(consumer_sem);
    return; // Exit with status code 0 (success)
}
void consumerrr(const char *filename){
    int shm_fd;
    char *shm_ptr;
    sem_t *producer_sem, *consumer_sem;
    sleep(4);
    // Open the POSIX shared memory object
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return;
    }

    // Map the shared memory object into the address space of the process
    shm_ptr = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        return;
    }

    // Open the producer semaphore
    producer_sem = sem_open(SEM_MUTEX_NAME, 0);
    if (producer_sem == SEM_FAILED) {
        perror("sem_open producer");
        return;
    }

    // Open the consumer semaphore
    consumer_sem = sem_open(SEM_FULL_NAME, 0);
    if (consumer_sem == SEM_FAILED) {
        perror("sem_open consumer");
    }

    // Open the file for writing
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    // Consume the data from shared memory and write it to the file
    // int done = 0;
    // while (!done) {
        sem_wait(consumer_sem); // Wait until the producer writes data
        // if (strlen(shm_ptr) > 0) {
            fprintf(file, "%s", shm_ptr);
            fflush(file);
            printf("Consumer received and wrote to file: %s\n", shm_ptr);
        // } else {
            // done = 1; // No more data, exit loop
        // }
        sem_post(producer_sem); // Signal the producer that data has been consumed
    // }

    // Clean up
    fclose(file);
    munmap(shm_ptr, BUFFER_SIZE);
    close(shm_fd);
    sem_close(producer_sem);
    return; // Exit with status code 0 (success)
}


void consumerr(const char *filename) {
    int shm_fd;
    char *shm_ptr;
    sem_t *mutex, *full,*empty;

    // Open the POSIX shared memory object
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Map the shared memory object into the address space of the process
    shm_ptr = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Open the producer semaphore
    mutex = sem_open(SEM_MUTEX_NAME, 0);
    if (mutex == SEM_FAILED) {
        perror("sem_open producer");
        exit(1);
    }

    // Open the consumer semaphore
    full = sem_open(SEM_FULL_NAME, 0);
    if (full == SEM_FAILED) {
        perror("sem_open consumer");
        exit(1);
    }

    empty = sem_open(SEM_EMPTY_NAME, 0);
    if (empty == SEM_FAILED) {
        perror("sem_open consumer");
        exit(1);
    }

    // Open the file for writing
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("fopen");
        exit(1);
    }

    // Consume the data from shared memory and write it to the file
    int done = 0;
    while (!done) {
        printf("BURDAYIM1\n");
        sem_wait(full);
        sem_wait(mutex); // Wait until the producer writes data
        if (strlen(shm_ptr) < 1) 
            break;
        // if(strlen(shm_ptr) < 1) {
        //     sem_post(mutex); // Signal the producer that data has been consumed
        //     sem_post(empty);
        //     break;
        // }
            fprintf(file, "%s", shm_ptr);
            fflush(file);
            printf("Consumer received and wrote to file: %s\n", shm_ptr);
            memset(shm_ptr, 0, BUFFER_SIZE);
        // } else {
        //     done = 1; // No more data, exit loop
        // }
        sem_post(mutex); // Signal the producer that data has been consumed
        sem_post(empty);
        if(strlen(shm_ptr) < 1)
        {
            break;
        }
        printf("BURDAYIM2\n");
    }

    // Clean up
    fclose(file);
    munmap(shm_ptr, BUFFER_SIZE);
    close(shm_fd);
    sem_close(full);
    sem_close(empty);
    sem_close(mutex);
    return; // Exit with status code 0 (success)
}

// void producer_fifo(const char *filename) {

//     sem_t *empty, *full, *mutex;
//     int file = open(filename, O_RDONLY);
//     if (file == -1) {
//         perror("fopen");
//         exit(EXIT_FAILURE);
//     }

//     int fifo_fd = open(TEMP_DOWNLOAD_FIFO, O_WRONLY);
//     if (fifo_fd == -1) {
//         perror("open");
//         exit(EXIT_FAILURE);
//     }
//     mutex = sem_open(SEM_MUTEX_NAME, 0);
//     if (mutex == SEM_FAILED) {
//         perror("sem_open producer");
//         exit(1);
//     }

//     // Open the consumer semaphore
//     full = sem_open(SEM_FULL_NAME, 0);
//     if (full == SEM_FAILED) {
//         perror("sem_open consumer");
//         exit(1);
//     }

//     empty = sem_open(SEM_EMPTY_NAME, 0);
//     if (empty == SEM_FAILED) {
//         perror("sem_open consumer");
//         exit(1);
//     }
//     char buffer[4096];
//     ssize_t bytes_read;
//     while ((bytes_read = read(file, buffer,sizeof(buffer))) > 0) {
        
//         if (write(fifo_fd, buffer, bytes_read) == -1) {
//             perror("write");
//             exit(EXIT_FAILURE);
//         }
//     }

//     fclose(file);
//     close(fifo_fd);
// }