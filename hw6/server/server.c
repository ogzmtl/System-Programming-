#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

struct client_info
{
    char *file;
    char *modification_time;
};

struct socket_server
{
    char *server_src;
    int fd;
};

struct server_inside
{
    char filename[512];
    char modification_time[256];
    unsigned int file_modes;
};

int size_of_server_insides;
int size_of_client_insides;
// int file_counter = 0;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_produce = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_consume = PTHREAD_COND_INITIALIZER;

struct server_inside *server_files_information;
struct server_inside *client_files_information;
struct server_inside *difference;
struct socket_server srv;
void *synchronize_server_side(void *arg);
int take_files(char *path, int file_counter);
void reallocate_struct(int i);
void compare();
void create_directory(const char* temp1);

int main(int argc, char **argv)
{

    if (argc != 4)
    {
        char *err_msg = "Invalid command line arguments\n";
        write(STDOUT_FILENO, err_msg, strlen(err_msg));
        exit(EXIT_FAILURE);
    }

    int socketfd, new_client_fd;
    int bind_res, listen_res;
    char buffer[512];
    struct sockaddr_in addr;
    // char *server_directory = argv[1];
    int thread_pool_size = atoi(argv[2]);
    int PORT_NUM = atoi(argv[3]);
    pthread_t threads[thread_pool_size];
    
    srv.server_src = argv[1];

    server_files_information = (struct server_inside *)malloc(sizeof(struct server_inside) * 5);

    // for (int j = 0; j < 5; j++)
    // {
    //     server_files_information[j].filename = malloc(sizeof(char) * 512);
    //     server_files_information[j].modification_time = malloc(sizeof(char) * 256);
    //     // filenames[j] = (char *)malloc(sizeof(char) * 512);
    // }

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1)
    {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT_NUM);

    bind_res = bind(socketfd, (struct sockaddr *)&addr, sizeof(addr));
    if (bind_res == -1)
    {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    listen_res = listen(socketfd, thread_pool_size);
    if (listen_res == -1)
    {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    // Create a thread pool for handling client connections

    // Set up signal handling for SIGINT
    // signal(SIGINT, handleSignal);

    snprintf(buffer, sizeof(buffer), "Server is listening on port %d...\n", PORT_NUM);
    write(STDOUT_FILENO, buffer, sizeof(buffer));

    socklen_t addrlen = sizeof(addr);
    int i = 0;
    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t cliend_addr_len = sizeof(client_addr);
        memset(buffer, 0, sizeof(buffer));
        // snprintf(buffer, sizeof(buffer), "New client waiting\n");
        // write(STDOUT_FILENO, buffer, sizeof(buffer));
        new_client_fd = accept(socketfd, (struct sockaddr *)&client_addr, &cliend_addr_len);

        srv.fd = new_client_fd;

        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, sizeof(buffer), "Client accepted\n");
        write(STDOUT_FILENO, buffer, sizeof(buffer));
        if (new_client_fd == -1)
        {
            perror("Failed to accept socket2");
            exit(EXIT_FAILURE);
        }
        int s = pthread_create(&threads[i], NULL, synchronize_server_side, &srv);
        if (s != 0)
        {
            perror("pthread_create");
            return 1;
        }
        i = (i + 1) % thread_pool_size;
        // read(new_client_fd, buffer, sizeof(buffer));
        // write(STDOUT_FILENO, buffer, strlen(buffer));
    }

    close(new_client_fd);
    close(socketfd);
    free(server_files_information);
}

void *synchronize_server_side(void *arg)
{
    struct socket_server *communication = (struct socket_server *)arg;
    char buffer[512];
    int file_counter = take_files(communication->server_src, 0);
    // printf("Total files counted on the server : %d\n", file_counter);
    // for (int j = 0; j < file_counter; j++)
    // {
    //     printf("files - mod_time :%s - %s", server_files_information[j].filename, server_files_information[j].modification_time);
    //     printf(" - modes :%d \n", server_files_information[j].file_modes);
    // }
    // printf("total files : %d\n", size_of_server_insides);

    while (1)
    {
        int sizeof_client;
        memset(buffer, 0, sizeof(buffer));
        write(communication->fd, &size_of_server_insides, sizeof(int));
        // send(communication->server_src, communication, sizeof(communication), 0);

        for (int j = 0; j < size_of_server_insides; j++)
        {
            send(communication->fd, &server_files_information[j], sizeof(server_files_information[j]), 0);
        }

        read(communication->fd, &sizeof_client, sizeof(sizeof_client));
        // mutex
        size_of_client_insides = sizeof_client;
        printf("client_insides : %d\n", sizeof_client);
        client_files_information = (struct server_inside *)malloc(sizeof(struct server_inside) * sizeof_client);
        for (int j = 0; j < sizeof_client; j++)
        {
            recv(communication->fd, &client_files_information[j], sizeof(client_files_information[j]), 0);
        }
        compare();
        // for(int j = 0; j < sizeof_client; j++){
        //     printf("files - mod_time :%s - %s", client_files_information[j].filename, client_files_information[j].modification_time);
        //     printf(" - modes :%d \n", client_files_information[j].file_modes);
        // }

        // compare files and modification times
        // compare();

        // mutex_unlock
        //  read(communication->fd, buffer, sizeof(buffer));
        //  write(STDOUT_FILENO, buffer, sizeof(buffer));
        //  memset(buffer, 0, sizeof(buffer));
        //  snprintf(buffer, sizeof(buffer),"%d", i++);
        //  write(STDOUT_FILENO, buffer, strlen(buffer));
        //  take_files(communication->server_src);
    }
}

int take_files(char *path, int file_counter)
{
    char buffer[512];
    char file[512];
    int i = file_counter;
    int folder_count = 0;
    memset(buffer, 0, sizeof(buffer));
    // snprintf(buffer, sizeof(buffer), "take_files function\n");
    // write(STDOUT_FILENO, buffer, sizeof(buffer));
    // char **filenames = (char **)malloc(sizeof(char *) * 5);

    DIR *directory;
    struct dirent *entry;
    strcat(path, "/");
    directory = opendir(path);
    // mutex koyulabilir
    if (directory)
    {
        while ((entry = readdir(directory)) != NULL)
        {
            struct stat info;
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }
            else
            {
                memset(file, 0, sizeof(file));
                snprintf(file, sizeof(file), "%s%s", path, entry->d_name);

                if (!stat(file, &info))
                {
                    if (S_ISDIR(info.st_mode))
                    {
                        // printf("FOLDER SIDE : %s - i : %d - fileCounter : %d\n", file, i, file_counter);
                        file_counter = take_files(file, file_counter);
                        folder_count++;
                    }
                    else
                    {
                        i = file_counter;
                        strcpy(server_files_information[i].filename, file);
                        // printf("STRUCT FILES : %s \n", server_files_information[i].filename);
                        char time[12];
                        snprintf(time, sizeof(time), "%ld", (info.st_mtime));
                        strcpy(server_files_information[i].modification_time, time);
                        server_files_information[i].file_modes = (info.st_mode);
                        file_counter++;
                        i++;
                        memset(time, 0, sizeof(time));
                        if (i % 5 == 0 && i != 0)
                        {
                            reallocate_struct(i);
                        }
                    }
                }
            }
        }

        closedir(directory);
        // printf("i : %d - fileCounter : %d\n", i, file_counter);
    }

    size_of_server_insides = i;

    return file_counter;
}

void reallocate_struct(int i)
{

    char **temp_filenames = (char **)malloc(sizeof(char *) * (i));
    char **temp_modification = (char **)malloc(sizeof(char *) * (i));
    unsigned int *modes = (unsigned int *)malloc(sizeof(unsigned int) * i);

    for (int j = 0; j < i; j++)
    {
        temp_filenames[j] = (char *)malloc(sizeof(char) * 512);
        temp_modification[j] = (char *)malloc(sizeof(char) * 256);
        strcpy(temp_filenames[j], server_files_information[j].filename);
        strcpy(temp_modification[j], server_files_information[j].modification_time);
        modes[j] = server_files_information[j].file_modes;
    }
    server_files_information = (struct server_inside *)malloc(sizeof(struct server_inside) * (i + 5));

    for (int j = 0; j < i + 5; j++)
    {
        // filenames[j] = (char *)malloc(sizeof(char) * 512);
        // server_files_information[j].filename = malloc(sizeof(char) * 512);
        // server_files_information[j].modification_time = malloc(sizeof(char) * 256);
        // printf("server_inside3\n");
        if (j < i)
        {
            strcpy(server_files_information[j].filename, temp_filenames[j]);
            strcpy(server_files_information[j].modification_time, temp_modification[j]);
            server_files_information[j].file_modes = modes[j];
        }
    }

    for (int j = 0; j < i; j++)
    {
        free(temp_filenames[j]);
        free(temp_modification[j]);
    }
    free(modes);
    free(temp_modification);
    free(temp_filenames);
}

void compare()
{
    // printf("NABER\n");
    char created_filename[256]; 
    for (int i = 0; i < size_of_client_insides; i++)
    {
        memset(created_filename, 0, sizeof(created_filename));
        char *delimeter_client = strchr(client_files_information[i].filename, '/');
        int flag = 0;
        if (delimeter_client != NULL)
        {
            delimeter_client = delimeter_client + 1;
            // printf("Remaining: %s\n", delimeter_srv);
        }
        for (int j = 0; j < size_of_server_insides; j++)
        {
            char *delimeter_srv = strchr(server_files_information[j].filename, '/');
            if (delimeter_srv != NULL)
            {
                delimeter_srv = delimeter_srv + 1;
                // printf("Remaining: %s\n", delimeter_client);
            }
            if (strcmp(delimeter_srv, delimeter_client) == 0
                // && strcmp(server_files_information[j].file_modes, client_files_information[i].file_modes) == 0
            )
            {
                printf("Remaining: %s\n", delimeter_srv);
                flag = 1;
                break;
            }
        }

        if (flag == 0)
        {
            char temp[256];
            memset(temp, 0, sizeof(temp));
            strcat(temp, srv.server_src);
            // strcat(temp, "/");
            strcat(temp,delimeter_client);
            // printf("Added: %s\n", temp);
            
            //parse and create folder then send temp to open syscall
            create_directory(temp);
            int fd = open(temp, O_CREAT | O_RDWR | O_TRUNC,client_files_information[i].file_modes);
            if(fd == -1){
                perror("Open");
                continue;
            }
            close(fd);
        }
    }
}

void create_directory(const char* path) {
    const char delimiter[] = "/";
    char temp[256];
    strcpy(temp, path);

    char* token = strtok(temp, delimiter);
    char* lastToken = NULL;
    char temp_old[256] = "";  // Separate variable to hold modified string

    printf("Creating directories\n");

    while (token != NULL) {
        lastToken = token;
        token = strtok(NULL, delimiter);

        if (token != NULL) {
            strcat(temp_old, lastToken);
            strcat(temp_old, delimiter);

            if (mkdir(temp_old, 0777) != 0) {
                if (errno != EEXIST) {
                    perror("mkdir");
                    return;
                }
            }
            // printf("temp_old: %s\n", temp_old);
        }
    }
}

            
