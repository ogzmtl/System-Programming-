#include "final_common.h"


sehirPopulasyon cityInfo;
int sockfd;

void handler(int signum)
{
    int written = 0;
    char log[LOG_BUFFER_LEN];

    if (signum == SIGINT) {
        sehirPopulasyon temp = {0,0,0,0,0,0,0,0,CANCELLED};

        send(sockfd, &temp,sizeof(sehirPopulasyon) , 0);
        close(sockfd);
        written = snprintf(log, LOG_BUFFER_LEN, "^C signal .. cancelling orders.. editing log..\n");
        write(STDOUT_FILENO, log, written);
        memset(log, 0, written);
        //writeTLog
        exit(EXIT_SUCCESS);
    }
}
void initializeVariables(variables* var) {
    for (int i = 0; i < cityInfo.clientCapacity; i++) {
        var[i].home_locationX = rand() % cityInfo.X;
        var[i].home_locationY = rand() % cityInfo.Y;
        // printf("loc_x= %d\n", var[i].home_locationX);
    }
}

void sendVariables(int socket, variables* var, int count) {
    for (int i = 0; i < count; i++) {
        // printf("%d ", var[i].home_locationX);
        // printf("%d\n", var[i].home_locationY);
        if (send(socket, &var[i], sizeof(variables), 0) == -1) {
            perror("send");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char **argv){

    int socketfd;
    struct sockaddr_in addr;
    // socklen_t addrLen = sizeof(addr);
    // int ipAddr;
    int sizex, sizey;
    int numClients;
    // int newSocket; 
    // char socketBuffer[256];
    srand(time(NULL));

    /*System Log Initialization Start*/
    char log[LOG_BUFFER_LEN];
    int numBytesReadLog = 0;
    /*System Log Initializtion End*/
    const char *logFile = "log.txt";
    
    int log_fd = open(logFile, O_CREAT | O_RDWR | O_APPEND, 0666);
    close(log_fd);

    /* USAGE_ERR_START*/
    if(argc != 6){
        numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "Usage %s: [portnumber] [ipAddress] [numberOfClients] [p] [q]\n", argv[0]);
        write(STDOUT_FILENO, log, numBytesReadLog);
        memset(log, 0, numBytesReadLog);
        exit(EXIT_FAILURE);
    }
    /*USAGE_ERR_END */

    /*INPUT PREPARATION START*/
    int flag = 0;
    int portNum = atoi(argv[1]);
    numClients = atoi(argv[3]);
    sizex = atoi(argv[4]);
    sizey = atoi(argv[5]);
    variables inputClient[numClients];
    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags = 0; // or SA_RESTART
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    long int pidClient = getpid();
    numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, ">> PID %ld\n", pidClient);
    write(STDOUT_FILENO, log, numBytesReadLog);
    memset(log, 0, numBytesReadLog);

    cityInfo.X = sizex;
    cityInfo.Y = sizey;
    cityInfo.clientCapacity = numClients;
    cityInfo.status = REQUESTED;
    cityInfo.pid = pidClient;
    initializeVariables(inputClient);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(portNum);
    if(inet_pton(AF_INET, argv[2], &addr.sin_addr) <= 0){
        numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "Invalid Address or not supported\n");
        write(STDOUT_FILENO, log, numBytesReadLog);
        memset(log, 0, numBytesReadLog);
        exit(EXIT_FAILURE);
    }
    /*INPUT PREPARATION END*/

    /* SOCKET OPERAIONS START*/
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    sockfd = socketfd;
    if(socketfd == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if(connect(socketfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect failed\n");
        exit(EXIT_FAILURE);
    }


    numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, ">> Connected to server...\n");
    write(STDOUT_FILENO, log, numBytesReadLog);
    memset(log, 0, numBytesReadLog);

    send(socketfd, &cityInfo, sizeof(sehirPopulasyon), 0);
    sendVariables(socketfd, inputClient, numClients);


    /*SOCKET OPERATIONS END*/
    while(1){
        recv(socketfd, &cityInfo, sizeof(sehirPopulasyon),0);
        
        switch(cityInfo.status){
            case BURNED: 
                numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, ">Shop has been burned down, don't wait orders\n");
                send(socketfd, &cityInfo, sizeof(sehirPopulasyon), 0);
                write(STDOUT_FILENO, log, numBytesReadLog);
                memset(log, 0, LOG_BUFFER_LEN);
                
                numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "(client) Shop has been burned down, don't wait orders\n");
                writeToLog(log);
                memset(log, 0, LOG_BUFFER_LEN);
                close(socketfd);
                exit(EXIT_FAILURE);
            break;

            case PREPARED:
                numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, ">pide id :%d is prepared\n", cityInfo.orderID);
                //log_file
                write(STDOUT_FILENO, log, numBytesReadLog);
                memset(log, 0, LOG_BUFFER_LEN);
            break;

            case ON_DELIVERY:
                numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, ">pide id :%d on delivery (Moto id: %d)\n", cityInfo.orderID, cityInfo.motoID);
                //log_file
                write(STDOUT_FILENO, log, numBytesReadLog);
                memset(log, 0, LOG_BUFFER_LEN);
            break; 

            case DELIVERED:
                numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, ">All customers served \n>Log file written ..\n");
                //log_file
                flag = 1;
                write(STDOUT_FILENO, log, numBytesReadLog);
                memset(log, 0, LOG_BUFFER_LEN);

                numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "(client) All customers served \n>Log file written ..\n");
                // writeToLog(log);
                memset(log, 0, LOG_BUFFER_LEN);
            break;

            default: 
                numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, ">Unknown Error occured\n");
                //log_file
                write(STDOUT_FILENO, log, numBytesReadLog);
                memset(log, 0, LOG_BUFFER_LEN);
            break;
        }
        if(flag == 1){
            cityInfo.status = DELIVERED;
            send(socketfd, &cityInfo, sizeof(sehirPopulasyon), 0);
            break;
        }
    }

    close(socketfd);    
    return 0;
}