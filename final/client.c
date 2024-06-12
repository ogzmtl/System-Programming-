#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT_NUM 8080
#define LOG_BUFFER_LEN 256

typedef struct{ 
    int home_locationX;
    int home_locationY;
    int cityX;
    int cityY;
    int clientCapacity;
}variables;

typedef struct{ 
    int cityX;
    int cityY;
    int clientCapacity;
}sehirPopulasyon;

void initializeVariables(variables* var, sehirPopulasyon c) {
    for (int i = 0; i < c.clientCapacity; i++) {
        var[i].home_locationX = rand() % c.cityX;
        var[i].home_locationY = rand() % c.cityY;
    }
}

void sendVariables(int socket, variables* var, int count) {
    for (int i = 0; i < count; i++) {
        if (send(socket, &var[i], sizeof(variables), 0) == -1) {
            perror("send");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char **argv){

    int socketfd;
    struct sockaddr_in addr; //Structures for handling internet addresses (netinet/in.h)
    socklen_t addrLen = sizeof(addr);
    int ipAddr;
    int sizex, sizey;
    int numClients;
    int newSocket; 
    char socketBuffer[1024];
    srand(time(NULL));

    /*System Log Initialization Start*/
    char log[LOG_BUFFER_LEN]; //Use STDOUT and file buffer as same
    int numBytesReadLog = 0;
    /*System Log Initializtion End*/

    /* USAGE_ERR_START*/
    if(argc != 5){
        numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "Usage %s: [portnumber] [numberOfClients] [p] [q]\n", argv[0]);
        write(STDOUT_FILENO, log, numBytesReadLog);
        memset(log, 0, numBytesReadLog);
        exit(EXIT_FAILURE);
    }
    /*USAGE_ERR_END */



    /*INPUT PREPARATION START*/
    ipAddr = atoi(argv[1]);
    numClients = atoi(argv[2]);
    sizex = atoi(argv[3]);
    sizey = atoi(argv[4]);
    variables inputClient[numClients];
    sehirPopulasyon cityInfo;

    cityInfo.cityX = sizex;
    cityInfo.cityY = sizey;
    cityInfo.clientCapacity = numClients;
    initializeVariables(inputClient, cityInfo);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_NUM);
    if(inet_pton(AF_INET, argv[1], &addr.sin_addr) <= 0){
        numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "Invalid Address or not supported\n");
        write(STDOUT_FILENO, log, numBytesReadLog);
        memset(log, 0, numBytesReadLog);
        exit(EXIT_FAILURE);
    }
    /*INPUT PREPARATION END*/

    /* SOCKET OPERAIONS START*/
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(socketfd == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if(connect(socketfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind failed\n");
        exit(EXIT_FAILURE);
    }
    // snprintf(socketBuffer, 1024, "%d %d", sizex, sizey);
    // send(socketfd, socketBuffer, 1024, 0);
    send(socketfd, &cityInfo, sizeof(cityInfo), 0);
    sendVariables(socketfd, inputClient, numClients);
    numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "> Connected to server...\n");
    write(STDOUT_FILENO, log, numBytesReadLog);
    memset(log, 0, numBytesReadLog);

    /*SOCKET OPERATIONS END*/

    printf("Hello World\n");
    return 0;
}