#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

struct file_informations
{
    char filename[512];
    char modification_time[256];
    unsigned int file_modes;
};

struct client_socket
{
    char *client_src;
    int fd;
};

void reallocate_struct(int i);
void compare(char src[256]);
void create_directory(const char* path);
void read_from_serverx(int fd, int flag, const char* src);
void read_from_server(const char* src);
int isThereAnyDifference(const char* src);
void write_server(const char* path, int client_index);
struct file_informations *client_files;
struct file_informations *server_files;
struct client_socket clien;
int last_update;
int size_of_client_inside;
int size_of_server_inside;

int main(int argc, char **argv)
{

    if (argc != 3)
    {
        char *err_msg = "Invalid command line arguments\n";
        write(STDOUT_FILENO, err_msg, strlen(err_msg));
        exit(EXIT_FAILURE);
    }

    int socketfd, new_client_fd;
    char buffer[512];
    struct sockaddr_in addr;
    char *client_directory = argv[1];
    int PORT_NUM = atoi(argv[2]);
    
    int counter = 0;

    clien.client_src = argv[1];

    client_files = (struct file_informations *)malloc(sizeof(struct file_informations) * 5);

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1)
    {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_NUM);

    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) == -1)
    {
        perror("invalid address");
        exit(EXIT_FAILURE);
    }

    // Set up signal handling for SIGINT
    // signal(SIGINT, handleSignal);

    snprintf(buffer, sizeof(buffer), "Client is connecting on port %d...\n", PORT_NUM);
    write(STDOUT_FILENO, buffer, sizeof(buffer));
    memset(buffer, 0, sizeof(buffer));

    new_client_fd = connect(socketfd, (struct sockaddr *)&addr, sizeof(addr));
    if (new_client_fd == -1)
    {
        perror("Failed to accept socket1");
        exit(EXIT_FAILURE);
    }
    clien.fd = socketfd;
     int cli_files;

    while (1)
    {
        cli_files = take_files(clien.client_src, 0, counter);
        
        int sizeof_server;
        read(socketfd, &sizeof_server, sizeof(sizeof_server));
        // snprintf(buffer, sizeof(buffer), "Writing from client, Server size : %d\n", sizeof_server);
        // printf("A\n");

        size_of_server_inside = sizeof_server;
        server_files = (struct file_informations *)malloc(sizeof(struct file_informations) * sizeof_server);
        // printf("Size of server : %d\n", size_of_server_inside);
        for (int j = 0; j < sizeof_server; j++)
        {
            recv(socketfd, &server_files[j], sizeof(server_files[j]), 0);
        }
        // printf("B\n");

        write(socketfd, &size_of_client_inside, sizeof(int));
        printf("C\n");
        for (int i = 0; i < size_of_client_inside; i++)
        {
            send(socketfd, &client_files[i], sizeof(client_files[i]), 0);
        }
        // printf("D\n");
        // write_to_server();
        size_of_client_inside = cli_files;
        // if( counter != 0)
        // {
        //     printf("E\n");
        //     char buff[256];
        //     strcpy(buff, "continue");
        //     while(strncmp(buff, "quit", 5)!= 0)
        //     {
        //         printf("F\n");
        //         memset(buff, 0, sizeof(buff));
        //         recv(socketfd, buff, sizeof(buff), 0);
        //         printf("BUFFFF : %s\n", buff);
        //         read_from_server(clien.client_src);
        //     }
        // }
        // else if(counter == 0)
        // {
            compare(argv[1]);
            read_from_serverx(socketfd, counter++, clien.client_src);
        // }
        
        // printf("E\n");
        // write_to_server(socketfd, counter, clien.client_src);


        // sleep(4);

    }

    close(socketfd);
}

int take_files(char *path, int file_counter, int flag)
{
    char buffer[512];
    char file[512];
    int i = file_counter;
    int folder_count = 0;

    DIR *directory;
    struct dirent *entry;
    if(flag == 0) strcat(path, "/");
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
                        file_counter = take_files(file, file_counter, 0);
                        folder_count++;
                    }
                    else
                    {
                        i = file_counter;
                        strcpy(client_files[i].filename, file);
                        // printf("STRUCT FILES : %s \n", server_files_information[i].filename);
                        char time[12];
                        snprintf(time, sizeof(time), "%ld", (info.st_mtime));
                        strcpy(client_files[i].modification_time, time);
                        client_files[i].file_modes = (info.st_mode);
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
    }
    size_of_client_inside = i;

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
        strcpy(temp_filenames[j], client_files[j].filename);
        strcpy(temp_modification[j], client_files[j].modification_time);
        modes[j] = client_files[j].file_modes;
    }
    client_files = (struct file_informations *)malloc(sizeof(struct file_informations) * (i + 5));

    for (int j = 0; j < i + 5; j++)
    {
        // filenames[j] = (char *)malloc(sizeof(char) * 512);
        // client_files[j].filename = malloc(sizeof(char) * 512);
        // client_files[j].modification_time = malloc(sizeof(char) * 256);
        // printf("server_inside3\n");
        if (j < i)
        {
            strcpy(client_files[j].filename, temp_filenames[j]);
            strcpy(client_files[j].modification_time, temp_modification[j]);
            client_files[j].file_modes = modes[j];
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

void compare(char src[256])
{
    // printf("server_files %s\n", server_files[0].filename);
    char created_filename[256]; 
    for (int i = 0; i < size_of_server_inside; i++)
    {
        memset(created_filename, 0, sizeof(created_filename));
        char *delimeter_srv = strchr(server_files[i].filename, '/');
        int flag = 0;
        if (delimeter_srv != NULL)
        {
            delimeter_srv = delimeter_srv + 1;
        }
        // printf("server_files %d\n", size_of_client_inside);
        for (int j = 0; j < size_of_client_inside; j++)
        {
            char *delimeter_client = strchr(client_files[j].filename, '/');
            if (delimeter_client != NULL)
            {
                delimeter_client = delimeter_client + 1;
            }
            if (strcmp(delimeter_srv, delimeter_client) == 0
                // && strcmp(server_files_information[j].file_modes, client_files_information[i].file_modes) == 0
            )
            {
                flag = 1;
                break;
            }
        }

        if (flag == 0)
        {
            char temp[256];
            memset(temp, 0, sizeof(temp));
            strcat(temp, src);

            strcat(temp,delimeter_srv);

            if(size_of_client_inside % 5 == 0 && size_of_client_inside != 0)
            {   
                reallocate_struct(size_of_client_inside);
            }
            strcpy(client_files[size_of_client_inside++].filename, temp);

            create_directory(temp);
            // printf("Created files %s\n", temp);
            int fd = open(temp, O_CREAT | O_RDWR | O_TRUNC,server_files[i].file_modes);
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
    char temp_old[256] = "";


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
        }
    }
}

int isThereAnyDifference(const char* src)
{
    char created_filename[256];
    for (int i = 0; i < size_of_client_inside; i++)
    {
        char *delimeter_client = strchr(client_files[i].filename, '/');
        int flag = 0;
        if (delimeter_client != NULL)
        {
            delimeter_client = delimeter_client + 1;
        }

        for (int j = 0; j < size_of_server_inside; j++)
        {
            char *delimeter_srv = strchr(server_files[j].filename, '/');
            if (delimeter_srv != NULL)
            {
                delimeter_srv = delimeter_srv + 1;
            }
            if (strcmp(delimeter_client, delimeter_srv) == 0)
            {
                // if (strcmp(client_files[i].modification_time, server_files[j].modification_time) > 0 && last_update < client_files[i].modification_time )
                // {
                //     printf("File %s has different modification time.\n", client_files[i].filename);
                    printf("Writing to a server ");
                    write_server(delimeter_client, i);
                    last_update = (unsigned long)time(NULL);
                    // Perform necessary actions for different modification time
                // }
            }
        }
    }
    // for (int i = 0; i < size_of_client_inside; i++)
    // {
    //     memset(created_filename, 0, sizeof(created_filename));
    //     char *delimeter_client = strchr(client_files[i].filename, '/');
    //     int flag = 0;
    //     if (delimeter_client != NULL)
    //     {
    //         delimeter_client = delimeter_client + 1;
    //     }
    //     // printf("server_files %d\n", size_of_client_inside);
    //     for (int j = 0; j < size_of_server_inside; j++)
    //     {
    //         char *delimeter_srv = strchr(server_files[j].filename, '/');
    //         if (delimeter_srv != NULL)
    //         {
    //             delimeter_srv = delimeter_srv + 1;
    //         }
    //         if (strcmp(delimeter_client, delimeter_srv) == 0
    //            // && strcmp(server_files[j].file_modes, client_files[i].file_modes) == 0
    //         )
    //         {
    //             flag = 1;
    //             break;
    //         }
    //     }

    //     if (flag == 0)
    //     {
    //         char temp[256];
    //         memset(temp, 0, sizeof(temp));
    //         strcat(temp, src);

    //         strcat(temp,delimeter_client);

    //         if(size_of_client_inside % 5 == 0 && size_of_client_inside != 0)
    //         {   
    //             reallocate_struct(size_of_client_inside);
    //         }
    //         strcpy(client_files[size_of_client_inside++].filename, temp);

    //         create_directory(temp);
    //         // printf("Created files %s\n", temp);
    //         int fd = open(temp, O_CREAT | O_RDWR | O_TRUNC,server_files[i].file_modes);
    //         if(fd == -1){
    //             perror("Open");
    //             continue;
    //         }
    //         close(fd);
    //     }
    // }
    //unix zamanini al modifiye edildigi zaman 
    //burada farkliliga bak eger farklilik varsa yazmaya gonder
}

void read_from_serverx(int fd, int flag, const char* src) {
    int bytesRead;
    int BUFFER_SIZE = 1;
    char buffer[BUFFER_SIZE];
    if (flag == 0) {
        for (int i = 0; i < size_of_client_inside; i++) {
            char delimiter_server[256];
            recv(fd, delimiter_server, 256, 0);
            char temp[256];
            memset(temp, 0, sizeof(temp));
            strcpy(temp, src);
            strcat(temp, delimiter_server);
            // printf("%s\n", temp);
            int file_fd = open(temp, O_WRONLY | O_TRUNC);
            if (file_fd == -1) {
                perror("Failed to open file");
                exit(EXIT_FAILURE);
            }
            
            long int size; 
            read(fd,&size , sizeof(long int));
            printf("SIZE = %ld\n", size);

            for(int j = 0; j < size; j++)
            {
                recv(fd, buffer, 1, 0);
                // printf("Readed: %s\n", buffer);
                if (bytesRead == -1) {
                    perror("Receive failed");
                    exit(EXIT_FAILURE);
                }
                if (write(file_fd, buffer, 1) == -1) {
                    perror("Write to file failed");
                    exit(EXIT_FAILURE);
                }
            }          
            close(file_fd);
        }
    }
}


void write_to_server(int fd, int flag) {
    int bytesRead;
    int BUFFER_SIZE = 1;
    char buffer[BUFFER_SIZE];
    if (flag == 0) 
    {
        // printf("Writing to client\n");
        for (int i = 0; i < size_of_server_inside; i++) {

            // printf("aaaa %s : \n", server_files_information[i].filename);
            int file_fd = open(server_files[i].filename, O_RDONLY);
            if (file_fd == -1) {
                perror("Failed to open file");
                exit(EXIT_FAILURE);
            }
            char* delimiter_server = strchr(server_files[i].filename, '/');
            if (delimiter_server != NULL) {
                delimiter_server = delimiter_server + 1;
                // printf("Remaining: %s\n", delimiter_server);
            }
            printf("Sending: %s\n", delimiter_server);
            send(fd, delimiter_server, 256, 0);
            struct stat st; 
            stat(server_files[i].filename, &st);

            write(fd, &st.st_size, sizeof(long int));

            for (int j = 0; j < st.st_size; j++) {
                bytesRead = read(file_fd, buffer, 1);
                // printf("Readed: %s\n", buffer);
                if (bytesRead == -1) {
                    perror("Read failed");
                    exit(EXIT_FAILURE);
                }
                if (send(fd, buffer, 1, 0) == -1) {
                    perror("Send failed");
                    exit(EXIT_FAILURE);
                }
            }
            close(file_fd);
        }
    }
}

void write_server(const char* path, int client_index)
{
    char buffer[128];
    int bytesCounter = 0;
    int file_fd = open(client_files[client_index].filename, O_RDONLY);
    if (file_fd == -1) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    send(clien.fd , path, 256, 0);
    struct stat st; 
    stat(client_files[client_index].filename, &st);

    write(clien.fd, &st.st_size, sizeof(long int));

    for (int j = 0; j < st.st_size; j++) {
        int bytesRead = read(file_fd, buffer, 1);
        bytesCounter += bytesRead;
        // printf("Readed: %s\n", buffer);
        if (bytesRead == -1) {
            perror("Read failed");
            exit(EXIT_FAILURE);
        }
        if (send(clien.fd, buffer, 1, 0) == -1) {
            perror("Send failed");
            exit(EXIT_FAILURE);
        }
    }
    close(file_fd);
    printf("Writing %d bytes\n from file %s to server\n", bytesCounter, path);
}

void read_from_server(const char* src) {
    int bytesRead;
    int BUFFER_SIZE = 1;
    char buffer[BUFFER_SIZE];
    char filen[256];
    int bytes; 
    int bytesCounter =0;
    recv(clien.fd, filen, sizeof(filen), 0);
    char temp[256];


    strcat(temp, src);
    strcat(temp, filen);
    int file_fd = open(temp, O_WRONLY);
    if (file_fd == -1) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    read(clien.fd, &bytes, sizeof(bytes));
    struct stat st; 
    for (int j = 0; j < st.st_size; j++) {
        int bytesRead = recv(clien.fd, buffer, 1, 0);
        bytesCounter += bytesRead;
        // printf("Readed: %s\n", buffer);
        if (bytesRead == -1) {
            perror("Read failed");
            exit(EXIT_FAILURE);
        }
        if (write(file_fd, buffer, 1) == -1) {
            perror("Send failed");
            exit(EXIT_FAILURE);
        }
    }
    printf("Writing %d bytes\n from file %s to server\n", bytesCounter, temp);
}
