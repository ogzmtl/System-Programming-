
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
#include <fcntl.h>

#define MUTEX_NAME "/om_mutex"

struct file_informations{
    char filename[512];
    char modification_time[256];
    unsigned int file_modes; 
};

struct client_socket{
    
    char *client_src;
    int fd;
};

void reallocate_struct(int i);
struct file_informations* client_files;
int size_of_client_inside; 



int main(int argc, char** argv){

    if(argc != 3){
        char* err_msg = "Invalid command line arguments\n";
        write(STDOUT_FILENO,err_msg, strlen(err_msg));
        exit(EXIT_FAILURE);
    }

    int socketfd, new_client_fd;
    char buffer[512];
    struct sockaddr_in addr; 
    char *client_directory = argv[1];
    int PORT_NUM = atoi(argv[2]);
    struct client_socket clien; 

    clien.client_src =  argv[1];


    client_files = (struct file_informations *)malloc(sizeof(struct file_informations) * 5);
 

    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(socketfd == -1){
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_NUM);

    if(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) == -1){
        perror("invalid address");
        exit(EXIT_FAILURE);
    }
    // send_directory_info(client_directory);


    // Set up signal handling for SIGINT
    // signal(SIGINT, handleSignal);

    
    snprintf(buffer, sizeof(buffer), "Client is connecting on port %d...\n", PORT_NUM);
    write(STDOUT_FILENO, buffer, sizeof(buffer));
    memset(buffer, 0, sizeof(buffer));
    new_client_fd = connect(socketfd, (struct sockaddr *)&addr, sizeof(addr));
    if(new_client_fd == -1){
        perror("Failed to accept socket1");
        exit(EXIT_FAILURE);
    }
    clien.fd = socketfd;
    int cli_files = take_files(clien.client_src, 0);
    size_of_client_inside = cli_files; 
    for(int j = 0; j < size_of_client_inside; j++){
        printf("files - mod_time :%s - %s", client_files[j].filename, client_files[j].modification_time);
        printf(" - modes :%d \n", client_files[j].file_modes);
    }
    while(1)
    {
        int sizeof_server;
        struct file_informations* from_server; 
        read(socketfd, &sizeof_server, sizeof(sizeof_server));
        snprintf(buffer, sizeof(buffer), "Writing from client, Server size : %d\n", sizeof_server);
        from_server = (struct file_informations*) malloc(sizeof(struct file_informations) *( ((5-(sizeof_server%5))+sizeof_server)));
        recv(socketfd, &from_server[0], sizeof(from_server[0]),0);
        for (int j = 0; j < sizeof_server; j++)
        {
            recv(socketfd, &from_server[j], sizeof(from_server[j]),0);
        }

        
        // for(int j = 0; j < sizeof_server; j++){
        //     printf("files - mod_time :%s - %s", from_server[j].filename, from_server[j].modification_time);
        //     printf(" - modes :%d \n", from_server[j].file_modes);
        // }
        
        write(STDOUT_FILENO, buffer, sizeof(buffer));
        memset(buffer, 0, sizeof(buffer));
        snprintf(buffer, sizeof(buffer), "Hello from client\n");
        write(socketfd, buffer, sizeof(buffer));
        sleep(4);
        // snprintf(buffer, sizeof(buffer), "Hello from client another message sended\n");
        // write(socketfd, buffer, sizeof(buffer));
        sleep(4);

    }


    close(socketfd);


}

int take_files(char *path, int file_counter)
{
    char buffer[512];
    char file[512];
    int i = file_counter;
    int folder_count = 0;
    // memset(buffer, 0, sizeof(buffer));
    // snprintf(buffer, sizeof(buffer), "take_files function\n");
    // write(STDOUT_FILENO, buffer, sizeof(buffer));
    // char **filenames = (char **)malloc(sizeof(char *) * 5);

    DIR *directory;
    struct dirent *entry;
    strcat(path, "/");
    directory = opendir(path);
    //mutex koyulabilir 
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
        // printf("i : %d - fileCounter : %d\n", i, file_counter);
        
    }

    size_of_client_inside = i;

    return file_counter;
}
void reallocate_struct(int i)
{

    char **temp_filenames = (char **)malloc(sizeof(char *) * (i));
    char **temp_modification = (char **)malloc(sizeof(char *) * (i));
    unsigned int *modes = (unsigned int *)malloc(sizeof(unsigned int)*i);

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