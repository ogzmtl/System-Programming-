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
#include <signal.h>
#include <time.h>
#include <math.h>
#include "queue.h"

#define PORT_NUM 8080
#define LOG_BUFFER_LEN 256
#define BUFF_SIZE 1024

typedef struct {
    timer_t timerID;
    int threadID;
} TimerInfo;

typedef struct {
    int motoID;
    Pide bag[3];
    int received;
}courrier;

typedef struct{ 
    int home_locationX;
    int home_locationY;
    int cityX;
    int cityY;
    int clientCapacity;
}variables;

typedef enum {
    BURNED,
    CANCELLED,
    REQUESTED,
    PREPARED, 
    ON_DELIVERY,
    DELIVERED
}orderStatus;

// typedef struct{ 
//     int X;
//     int Y;
//     int clientCapacity;
//     int sockfd;
//     orderStatus status;
// }sehirPopulasyon;

typedef struct{ 
    int X;
    int Y;
    int clientCapacity;
    int sockfd;
    int orderID;
    int cookID;
    int motoID;
    orderStatus status;
}sehirPopulasyon;

sehirPopulasyon environmentInfo;
int timerCount = 0;
int speed = 0;
int shopX=0;
int shopY=0;
int cookSize = 0;
int delivSize= 0;
int siparisIptal = 0;
TimerInfo *tInfo;
courrier *moto;
Queue *orderQueue;
pthread_mutex_t orderMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t orderEmpty = PTHREAD_COND_INITIALIZER;
pthread_cond_t orderFull = PTHREAD_COND_INITIALIZER;

Queue *preparedQueue;
pthread_mutex_t preparedMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t preparedEmpty = PTHREAD_COND_INITIALIZER;
pthread_cond_t preparedFull = PTHREAD_COND_INITIALIZER;

int aparatus = 3;
pthread_mutex_t apparatusMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t apparatusCond = PTHREAD_COND_INITIALIZER;

Queue* ovenQueue;
pthread_mutex_t ovenMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ovenCondEmpty = PTHREAD_COND_INITIALIZER;
pthread_cond_t ovenCondFull = PTHREAD_COND_INITIALIZER;

int totalOrder =0;
pthread_mutex_t totalOrderMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t *cook, *motoThread;



#define openFlags  O_CREAT | O_RDWR | O_NONBLOCK

void handler(int signum)
{
    char logg[LOG_BUFFER_LEN];
    int written = 0;

    if(signum == SIGINT){
        sehirPopulasyon temp = {0,0,0,0,0,0,0,BURNED};
        temp.sockfd = environmentInfo.sockfd;

        written = snprintf(logg, LOG_BUFFER_LEN, "SIGINT catched, Burned Down\n");
        send(temp.sockfd, &temp, sizeof(sehirPopulasyon), 0);
        write(STDOUT_FILENO, logg, written);
        memset(logg, 0, LOG_BUFFER_LEN);
        exit(0);
    }
    else if(signum == SIGUSR1){
        written = snprintf(logg, LOG_BUFFER_LEN, "^C.. Upps quiting.. writing log file\n");
        write(STDOUT_FILENO, logg, written);
        memset(logg, 0, LOG_BUFFER_LEN);
        printf("deliv size %d, cooksize %d\n",delivSize, cookSize);
        for(int i = 0; i< cookSize; i++)
        {
            pthread_cancel(cook[i]);
        }
        for(int i = 0; i< delivSize; i++)
        {
            pthread_cancel(motoThread[i]);
        }
    }
}

int writeToLog(const char *log) {
    const char *logFile = "log.txt";
    int log_fd;
    unsigned int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    unsigned const int log_length = strlen(log);

    // Open the log file
    log_fd = open(logFile, O_CREAT | O_RDWR | O_APPEND, mode);
    if (log_fd == -1) {
        perror("open log file");
        return -1;
    }

    // Lock the file
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0; // Lock the whole file
    if (fcntl(log_fd, F_SETLK, &fl) == -1) {
        perror("File lock error");
        close(log_fd);
        return -1;
    }

    // Write to the log file
    if (write(log_fd, log, log_length) < 0) {
        perror("log file write error");
        close(log_fd);
        return -1;
    }

    // Unlock the file
    fl.l_type = F_UNLCK;
    if (fcntl(log_fd, F_SETLK, &fl) == -1) {
        perror("File unlock error");
        close(log_fd);
        return -1;
    }

    // Close the log file
    if (close(log_fd) == -1) {
        perror("close log file error");
        return -1;
    }

    return 0;
}

long int pseudo_inverse(int m, int n){
    sleep(4);
    return 2;
}
float customSqrt(float num) {
    float guess = num / 2.0f;
    float epsilon = 0.001f;

    if (num < 0) return -1;

    while ((guess * guess - num > epsilon) || (num - guess * guess > epsilon)) {
        guess = (guess + num / guess) / 2.0f;
    }

    return guess;
}

float calculateDistance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return customSqrt(dx * dx + dy * dy);
}

void delivery(int id, Pide bag[3], int size)
{
    char logBuf[LOG_BUFFER_LEN];
    int numBytesWritten = 0;
    memset(logBuf, 0, LOG_BUFFER_LEN);

    for (int i = 0; i < size; i++) {
        float distance;
        if (i == 0) {
            distance = calculateDistance(shopX, shopY, bag[i].loc_x, bag[i].loc_y);
            moto[id-1].received++;
        } else{
            distance = calculateDistance(bag[i - 1].loc_x, bag[i - 1].loc_y, bag[i].loc_x, bag[i].loc_y);
            moto[id-1].received++;
        }
        numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN,"Moto %d, Delivering Pide %d. Location = (%d, %d), Distance to travel = %f units\n",id, bag[i].id,bag[i].loc_x, bag[i].loc_y, distance);
        writeToLog(logBuf);
        memset(logBuf,0, numBytesWritten);

        usleep(distance / speed * 100000);

        numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN,"Moto %d, Delivered Pide id: %d\n",id, bag[i].id);
        // send(environmentInfo.sockfd, logBuf, LOG_BUFFER_LEN,0);
        writeToLog(logBuf);
        memset(logBuf,0, numBytesWritten);

    }
    float moveBack = calculateDistance(bag[size - 1].loc_x, bag[size - 1].loc_y, shopX, shopY);
    numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN, "Moto %d, moving back to Shop, Distance to travel = %f units\n", id, moveBack);
    write(STDOUT_FILENO, logBuf, numBytesWritten);
    memset(logBuf, 0,numBytesWritten); 

    usleep(moveBack / speed * 100000);

}

void timerHandler(union sigval sv) {
    TimerInfo *tInfo = (TimerInfo *)sv.sival_ptr;
    Pide sicakPide;
    sehirPopulasyon temp = {0,0,0,0,0,0,0,REQUESTED};
    char logBuf[LOG_BUFFER_LEN];
    int numBytesWritten = 0;
    memset(logBuf, 0, LOG_BUFFER_LEN);

    
    numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN,"Timer expired... cook ID: %d returns to oven\n", tInfo->threadID);
    writeToLog(logBuf);
    memset(logBuf,0, numBytesWritten);
    
    pthread_mutex_lock(&apparatusMutex);
    while(aparatus == 0){
        pthread_cond_wait(&apparatusCond, &apparatusMutex);
    }
    //aparati azaltmayacaksin. log yazabilirsin
    aparatus--;
    pthread_cond_signal(&apparatusCond);
    pthread_mutex_unlock(&apparatusMutex);
    
    numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN,"The cook: %d took the aparatus...\n", tInfo->threadID);
    writeToLog(logBuf);
    memset(logBuf,0, numBytesWritten);
    
    pthread_mutex_lock(&ovenMutex);
    while (isEmpty(ovenQueue)) {
        pthread_cond_wait(&ovenCondFull, &ovenMutex);
    }

    sicakPide = dequeue(ovenQueue);

    pthread_cond_broadcast(&ovenCondEmpty);
    pthread_mutex_unlock(&ovenMutex);

    //log icin if check continue ile
    numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN,"The cook: %d, took order id: %d from oven \n", tInfo->threadID,sicakPide.id);
    writeToLog(logBuf);
    // send(environmentInfo.sockfd, logBuf, LOG_BUFFER_LEN,0);
    memset(logBuf,0, numBytesWritten);

    //aparati continue ile check et azaltmadigin icin arttirmana gerek yok
    pthread_mutex_lock(&apparatusMutex);
    aparatus++;
    pthread_cond_signal(&apparatusCond);
    pthread_mutex_unlock(&apparatusMutex);

    temp.clientCapacity = environmentInfo.clientCapacity;
    temp.sockfd = environmentInfo.sockfd;
    temp.orderID = sicakPide.id;
    temp.cookID = tInfo->threadID;
    temp.status = PREPARED;
    send(temp.sockfd, &temp, sizeof(sehirPopulasyon),0);

    numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN,"The cook: %d left the aparatus...to take out of the oven \n", tInfo->threadID);
    writeToLog(logBuf);
    memset(logBuf,0, numBytesWritten);

    //if check continue
    pthread_mutex_lock(&preparedMutex);
    while (isFull(preparedQueue)) {
        pthread_cond_wait(&preparedEmpty, &preparedMutex);
        //break;
    }
    //if check continue
    enqueue(preparedQueue, sicakPide);

    pthread_cond_broadcast(&preparedFull);
    pthread_mutex_unlock(&preparedMutex);

    //log bastirma icin check
    numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN,"The cook: %d, sends order id: %d to manager \n", tInfo->threadID,sicakPide.id);
    writeToLog(logBuf);
    memset(logBuf,0, numBytesWritten);
}

void startTimer(TimerInfo *tInfo, int seconds) {

    struct sigevent sev;
    struct itimerspec its;

    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_value.sival_ptr = tInfo;
    sev.sigev_notify_function = timerHandler;
    sev.sigev_notify_attributes = NULL;
  
    if (timer_create(CLOCK_REALTIME, &sev, &tInfo->timerID) == -1) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }

    its.it_value.tv_sec = seconds;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    if (timer_settime(tInfo->timerID, 0, &its, NULL) == -1) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }
}

void* pideShopAsci(void* arg){
    TimerInfo *tInfo = (TimerInfo *)arg;
    char logBuff[LOG_BUFFER_LEN];
    int numBytesWritten = 0;
    int s = pthread_setcancelstate(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (s != 0){
        perror("pthread_setcancelstate");
    }
    while(1)
    {
        Pide preparePide;
        sehirPopulasyon temp = {0,0,0,0,0,0,0,REQUESTED};

        pthread_mutex_lock(&orderMutex);
        
        //if check gelebilir/coskun buraya gelmiyor dedi
        while(isEmpty(orderQueue))
        {
            printf("Asci %d order bekliyor \n", tInfo->threadID);
            pthread_cond_wait(&orderFull, &orderMutex);// hepsi kapaninca deadlock olabilir dikkat et
            //if continue break
        }
        //if icerisine al
        preparePide = dequeue(orderQueue);
        pthread_cond_broadcast(&orderFull);
        pthread_mutex_unlock(&orderMutex);

        temp.clientCapacity = environmentInfo.clientCapacity;
        temp.sockfd = environmentInfo.sockfd;
        temp.orderID = preparePide.id;
        temp.cookID = tInfo->threadID;

        numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN, "Pide id %d, preparing... thanks to cook:%d\n", preparePide.id,tInfo->threadID);
        writeToLog(logBuff);
        write(STDOUT_FILENO, logBuff, numBytesWritten);
        memset(logBuff, 0, numBytesWritten);

        int sleep_time = pseudo_inverse(30,40);//it is just sleep function, make cpu busy for a time pide prepration
        
        pthread_mutex_lock(&apparatusMutex);
        while(aparatus == 0){
            numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN, "Cook: %d waits for aparatus for (Order id: %d)\n",tInfo->threadID, preparePide.id);
            writeToLog(logBuff);
            memset(logBuff, 0, numBytesWritten);
            printf("Asci %d aparat bekliyor \n", tInfo->threadID);
            pthread_cond_wait(&apparatusCond, &apparatusMutex);
            //if check continue
        }

        //aparati azaltmaya gerek yok
        aparatus--;
        pthread_cond_broadcast(&apparatusCond);
        pthread_mutex_unlock(&apparatusMutex);
        
        numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN,"Cook took the aparatus (cook id: %d)\n", tInfo->threadID);
        writeToLog(logBuff);
        memset(logBuff, 0, numBytesWritten);

        pthread_mutex_lock(&ovenMutex);
        while (isFull(ovenQueue)) {
            numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN,"Order id: %d is waiting oven queue (cook: %d)\n", tInfo->threadID);
            writeToLog(logBuff);
            memset(logBuff, 0, numBytesWritten);
            printf("Asci %d firin sirasi bekliyor \n", tInfo->threadID);

            pthread_cond_wait(&ovenCondEmpty, &ovenMutex);
            //oven queue bekledikten sonra continue yanlissa enqueue etme ama aparati arttir.
        }
        enqueue(ovenQueue, preparePide);

        pthread_cond_broadcast(&ovenCondFull);
        pthread_mutex_unlock(&ovenMutex);

        pthread_mutex_lock(&apparatusMutex);
        aparatus++;
        pthread_cond_signal(&apparatusCond);
        pthread_mutex_unlock(&apparatusMutex);
        
        numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN,"Cook left the aparatus (cook id: %d)\n", tInfo->threadID);
        writeToLog(logBuff);
        memset(logBuff, 0, numBytesWritten);

        numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN, "Order %d placed to oven (Cook: %d)\n",preparePide.id, tInfo->threadID);
        writeToLog(logBuff);
        write(STDOUT_FILENO, logBuff, numBytesWritten);
        memset(logBuff, 0,numBytesWritten);
        startTimer(tInfo, sleep_time);

    }
}

void* pideShopMoto(void* arg)
{
    courrier *motokurye = (courrier *)arg;
    
    int bag_count = 0;
    int i = 0;
    int flag = 0;    
    int numBytesWritten =  0;
    char logBuff[LOG_BUFFER_LEN];
    memset(logBuff, 0, LOG_BUFFER_LEN);
    int s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (s != 0){
        perror("pthread_setcancelstate");
    }
    int p = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
    while (1) {
        Pide bag[3];
        
        while(totalOrder > 0)
        {
            if(bag_count < 3)
            {
                sehirPopulasyon temp = {0,0,0,0,0,0,0,REQUESTED};
                pthread_mutex_lock(&preparedMutex);

                while(isEmpty(preparedQueue))
                {
                    numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN, "Moto id: %d is waiting for bag to be filled\n", motokurye->motoID);
                    writeToLog(logBuff);
                    write(STDOUT_FILENO, logBuff, numBytesWritten);
                    memset(logBuff, 0,numBytesWritten);
                    printf("Motor %d bos kuyruk sirasi bekliyor \n", motokurye->motoID);
                    pthread_cond_wait(&preparedFull, &preparedMutex);
                }
                bag[i++] = dequeue(preparedQueue);
                temp.orderID = bag[i-1].id;
                temp.sockfd = environmentInfo.sockfd;
                temp.status = ON_DELIVERY;
                pthread_cond_broadcast(&preparedEmpty);
                pthread_mutex_unlock(&preparedMutex);

                send(temp.sockfd, &temp, sizeof(sehirPopulasyon), 0);

                numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN, "Moto id: %d is taking order id %d\n", motokurye->motoID, bag[i-1].id);
                writeToLog(logBuff);
                write(STDOUT_FILENO, logBuff, numBytesWritten);
                memset(logBuff, 0,numBytesWritten);

                bag_count++;
                // printf("bag_count: %d\n", bag_count);
                pthread_mutex_lock(&totalOrderMutex);
                totalOrder--;
                pthread_mutex_unlock(&totalOrderMutex);
                flag = 1;
            }
            else
            {
                break;
            }
        }

        if(flag == 1){
            delivery(motokurye->motoID, bag, bag_count);
            numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN, "Moto id: %d returned\n", motokurye->motoID);
            writeToLog(logBuff);
            write(STDOUT_FILENO, logBuff, numBytesWritten);
            memset(logBuff, 0,numBytesWritten);
            bag_count = 0;
        }
        flag = 0;

    }
}
void handleClient(int sockfd, int argumentSize) {
    variables myVariables[argumentSize];
    Pide *newOrder = (Pide*)malloc(sizeof(Pide)*argumentSize);
    memset(newOrder, 0, sizeof(Pide)*argumentSize);
    
    int id;
    char buff[256];
    memset(buff, 0, 256);
    for (int i = 0; i < argumentSize; i++) {

        if (recv(sockfd, &myVariables[i], sizeof(variables), 0) == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }

        printf("took y = %d\n", myVariables[i].home_locationY);
        printf("took x = %d\n", myVariables[i].home_locationX);
        newOrder[i].id = i+1; 
        newOrder[i].loc_x = (int) myVariables[i].home_locationX;
        newOrder[i].loc_y = (int) myVariables[i].home_locationY;
        printf("i = %d\n", newOrder[i].id);
        printf("newOrderX = %d\n", newOrder[i].loc_y);

    }

    for (int i = 0; i < argumentSize; i++) {

        printf("newOrder.locy = %d\n", newOrder[i].loc_y);
        pthread_mutex_lock(&orderMutex);
    
        while(isFull(orderQueue))
        {
            pthread_cond_wait(&orderEmpty, &orderMutex);
        }
        enqueue(orderQueue, newOrder[i]);
        
        pthread_cond_broadcast(&orderFull);
        pthread_mutex_unlock(&orderMutex);
        pthread_mutex_lock(&totalOrderMutex);
        totalOrder++;
        pthread_mutex_unlock(&totalOrderMutex);
    }

    int numBytesReaded = snprintf(buff, 256, "%d new customers.. Serving...\n", argumentSize);
    writeToLog(buff);
    write(STDOUT_FILENO, buff, numBytesReaded);
    memset(buff, 0, numBytesReaded);
    free(newOrder);
}
// void setOrder(int ID, variables clientInfo){
//     Pide newOrder; 
//     int row = 0;
//     // char splitted[2][10];          // Row index for the 2D array

//     // Use strtok to tokenize the string by spaces
//     // char* token = strtok(buffer, " ");
//     // while (token != NULL && row < 2) {
//     //     strncpy(splitted[row], token, sizeof(splitted[row]) - 1);
//     //     splitted[row][sizeof(splitted[row]) - 1] = '\0';  // Ensure null termination
//     //     row++;
//     //     token = strtok(NULL, " ");
//     // }
//     newOrder.id = ID; 
//     // newOrder.loc_x = atof(splitted[0]);
//     // newOrder.loc_y = atof(splitted[1]);

//     newOrder.loc_x = clientInfo.home_locationX;
//     newOrder.loc_y = clientInfo.home_locationY;

//     pthread_mutex_lock(&orderMutex);
    
//     while(isFull(orderQueue))
//     {
//         printf("Manager siparisi tahtaya yazmayi bekliyor.(order ID: %d)\n", newOrder.id);
//         pthread_cond_wait(&orderEmpty, &orderMutex);
//     }
//     printf("Manager siparisi tahtaya yazdi.(order ID: %d)\n", newOrder.id);
//     enqueue(orderQueue, newOrder);
    
//     pthread_cond_broadcast(&orderFull);
//     pthread_mutex_unlock(&orderMutex);


//     // printf("%d\n", newOrder.id);
//     // printf("%f\n", newOrder.loc_x);
//     // printf("%f\n", newOrder.loc_y);
//     printf("----------\n");

// }

int main(int argc, char **argv){
    int socketfd; 
    struct sockaddr_in addr; //Structures for handling internet addresses (netinet/in.h)
    socklen_t addrLen = sizeof(addr);
    int ipAddr;
    // int cookThreadPool, delivThreadPool;
    int newSocket; 
    char buffer[BUFF_SIZE];
    int orderID=0;

    variables clientInfos;
    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags = 0; // or SA_RESTART
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    
    const char *logFile = "log.txt";
    
    int log_fd = open(logFile, O_CREAT | O_RDWR | O_TRUNC, 0666);
    close(log_fd);

    /*System Log Initialization Start*/
    char log[LOG_BUFFER_LEN]; //Use STDOUT and file buffer as same
    int numBytesReadLog = 0;
    /*System Log Initializtion End*/

    /* USAGE_ERR_START*/
    if(argc != 5){
        numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "Usage %s: [PortNumber] [CookthreadPoolSize] [DeliveryPoolSize] [k]\n", argv[1]);
        write(STDOUT_FILENO, log, numBytesReadLog);
        writeToLog(log);
        memset(log, 0, numBytesReadLog);
        exit(EXIT_FAILURE);
    }
    /*USAGE_ERR_END */





    /*INPUT PREPARATION START*/
    ipAddr = atoi(argv[1]);
    cookSize = atoi(argv[2]);
    delivSize = atoi(argv[3]);
    speed = atoi(argv[4]);
    // pthread_t cook[cookThreadPool], motoThread[delivThreadPool];

    cook = (pthread_t*)malloc(sizeof(pthread_t)*cookSize);
    motoThread = (pthread_t*)malloc(sizeof(pthread_t)*delivSize);
    tInfo = (TimerInfo*) malloc(sizeof(TimerInfo) * cookSize);
    moto = (courrier*) malloc(sizeof(courrier) * delivSize);
    timerCount = cookSize;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_NUM);
    if(inet_pton(AF_INET, argv[1], &addr.sin_addr) <= 0){
        numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "Invalid Address or not supported\n");
        write(STDOUT_FILENO, log, numBytesReadLog);
        writeToLog(log);
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

    if(bind(socketfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind failed\n");
        exit(EXIT_FAILURE);
    }

    numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "> PideShop active waiting for connection...\n\n");
    write(STDOUT_FILENO, log, numBytesReadLog);
    writeToLog(log);
    memset(log, 0, numBytesReadLog);

    if(listen(socketfd, 100) == -1){
        perror("listen failed\n");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i<cookSize; i++){
        tInfo[i].threadID = i + 1;
        pthread_create(&cook[i], NULL, pideShopAsci, &tInfo[i]);
    }
    for(int i = 0; i<delivSize; i++){
        moto[i].motoID = i + 1;
        pthread_create(&motoThread[i], NULL, pideShopMoto, &moto[i]);
    }


    while((newSocket = accept(socketfd, (struct sockaddr *)&addr, &addrLen)))
    {
        orderQueue = createQueue(50);
        ovenQueue = createQueue(6);
        preparedQueue = createQueue(50);
        // newSocket = accept(socketfd, (struct sockaddr *)&addr, &addrLen);
        numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, ">Connection Established\n");
        write(STDOUT_FILENO, log, numBytesReadLog);
        writeToLog(log);
        memset(log, 0, numBytesReadLog);

        recv(newSocket, &environmentInfo, sizeof(sehirPopulasyon), 0);

        shopX = environmentInfo.X/2;
        shopY = environmentInfo.Y/2;
        printf("newSocket: %d\n", newSocket);
        environmentInfo.sockfd = newSocket;
        handleClient(environmentInfo.sockfd, environmentInfo.clientCapacity);

        while(1){
            recv(newSocket, &environmentInfo, sizeof(sehirPopulasyon), 0);
            printf("newSocket: %d\n", environmentInfo.status);
            if(environmentInfo.status == CANCELLED)
            {
                printf("Signal Handled in server\n");
                // numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "^C.. Upps quiting.. writing log file");
                // writeToLog(log);
                // write(STDOUT_FILENO, log, numBytesReadLog);
                // memset(log,0, numBytesReadLog);

                raise(SIGUSR1);
                
                destroy(orderQueue);
                destroy(ovenQueue);
                destory(preparedQueue);
                break;
            }
        }
    }
    
    for(int i = 0; i < cookSize; i++){
        pthread_join(cook[i], NULL);
    }
    for(int i = 0; i < delivSize; i++){
        pthread_join(cook[i], NULL);
    }
    close(socketfd);
    close(newSocket);
    free(tInfo);
    free(ovenQueue);
    free(orderQueue);
    free(cook);
    free(motoThread);
    free(moto);
    return 0;
}