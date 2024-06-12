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

typedef struct{ 
    int cityX;
    int cityY;
    int clientCapacity;
}sehirPopulasyon;

int timerCount = 0;
int speed = 0;
int shopX=0;
int shopY=0;
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


//oven queue for each thread
//one ovenqueue variable




long int pseudo_inverse(int m, int n){
    //long int = rand()  % 1000000  return int
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
    for (int i = 0; i < size; i++) {
        float distance;
        if (i == 0) {
            distance = calculateDistance(shopX, shopY, bag[i].loc_x, bag[i].loc_y);
            moto[id-1].received++;
        } else{
            distance = calculateDistance(bag[i - 1].loc_x, bag[i - 1].loc_y, bag[i].loc_x, bag[i].loc_y);
            moto[id-1].received++;
        }
        printf("Moto %d, Delivering Pide %d: Distance to travel = %f units\n",id, bag[i].id, distance);
        usleep(distance / speed * 1000000);

    }
    float moveBack = calculateDistance(bag[size - 1].loc_x, bag[size - 1].loc_y, shopX, shopY);
    printf("Moto %d, moving back to Shop, Distance to travel = %f units\n",id, moveBack);
    usleep(moveBack / speed * 1000000);

}

void timerHandler(union sigval sv) {
    TimerInfo *tInfo = (TimerInfo *)sv.sival_ptr;
    Pide sicakPide; 
    printf("Timer expired in thread ID: %d (thread: %ld)\n", tInfo->threadID, pthread_self());
    pthread_mutex_lock(&apparatusMutex);
    while(aparatus == 0){
        pthread_cond_wait(&apparatusCond, &apparatusMutex);
    }
    aparatus--;
    pthread_cond_signal(&apparatusCond);
    pthread_mutex_unlock(&apparatusMutex);

    pthread_mutex_lock(&ovenMutex);
    while (isEmpty(ovenQueue)) {
        pthread_cond_wait(&ovenCondFull, &ovenMutex);
    }

    sicakPide = dequeue(ovenQueue); //dequeue yap manager bufferina gonder

    pthread_cond_broadcast(&ovenCondEmpty);
    pthread_mutex_unlock(&ovenMutex);

    pthread_mutex_lock(&apparatusMutex);
    aparatus++;
    pthread_cond_signal(&apparatusCond);
    pthread_mutex_unlock(&apparatusMutex);


    pthread_mutex_lock(&preparedMutex);
    while (isFull(preparedQueue)) {
        pthread_cond_wait(&preparedEmpty, &preparedMutex);
    }

    enqueue(preparedQueue, sicakPide); //dequeue yap manager bufferina gonder

    pthread_cond_broadcast(&preparedFull);
    pthread_mutex_unlock(&preparedMutex);
                    pthread_mutex_lock(&totalOrderMutex);
                totalOrder++;
                pthread_mutex_unlock(&totalOrderMutex);
    printf("Order ID %d, sended to manager\n", sicakPide.id);
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

    // Start a timer that expires after 5 seconds
    
// hep calismasi lazim bossa da doluysa da donmesi gerekiyor 
// if check icerisinde kontrol et buyuklugunu eger 0 sa dondurme
// 0 dan buyukse komple consume edilmesi gerekmekte butun arrayi lockla ve biten surede hepsini bosalt
// bir timer sure sonunda bekleyen pideler kismina gonder

    while(1)
    {
        Pide preparePide;
        pthread_mutex_lock(&orderMutex);
        
        while(isEmpty(orderQueue))
        {
            pthread_cond_wait(&orderFull, &orderMutex);// hepsi kapaninca deadlock olabilir dikkat et
        }
        
        preparePide = dequeue(orderQueue);
        printf("Pide id %d, hazirlanmaya basladi %d\n", preparePide.id,tInfo->threadID);
        pthread_cond_broadcast(&orderFull);
        pthread_mutex_unlock(&orderMutex);

        int sleep_time = pseudo_inverse(30,40);//it is just sleep function, make cpu busy for a time pide prepration
        
        //timer baslat ve baslattigin timer cond icerisinde olsun waite kal eger bir sonraki donus daha kisa olursa
        // TimerInfo *tInfo = (TimerInfo *)arg;
        pthread_mutex_lock(&apparatusMutex);
        while(aparatus == 0){
            printf("Aparat bekkeniyor (Asci: %d)\n", tInfo->threadID);
            pthread_cond_wait(&apparatusCond, &apparatusMutex);
        }
        printf("Aparat alindi (Asci: %d)\n", tInfo->threadID);
        aparatus--;
        pthread_cond_broadcast(&apparatusCond);
        pthread_mutex_unlock(&apparatusMutex);
        
        pthread_mutex_lock(&ovenMutex);
        while (isFull(ovenQueue)) {
            printf("Order id %d icin firin sirasi bekleniyor. (Asci: %d)\n",preparePide.id, tInfo->threadID);
            pthread_cond_wait(&ovenCondEmpty, &ovenMutex);
        }
        printf("Order %d firina konuldu (Asci: %d)\n",preparePide.id, tInfo->threadID);
        enqueue(ovenQueue, preparePide);

        pthread_cond_broadcast(&ovenCondFull);
        pthread_mutex_unlock(&ovenMutex);

        pthread_mutex_lock(&apparatusMutex);
        printf("Aparat birakildi (Asci: %d)\n", tInfo->threadID);
        aparatus++;
        pthread_cond_signal(&apparatusCond);
        pthread_mutex_unlock(&apparatusMutex);
        printf("Order id %d icin firin zamanlayicisi baslatildi (Asci: %d)\n",preparePide.id, tInfo->threadID);
        startTimer(tInfo, sleep_time);
        

        
        // sendToManager(preparePide);
        //oven buffer
        

        printf("%d Asci consumed orderid %d\n",tInfo->threadID, preparePide.id);
    }
}

void* pideShopMoto(void* arg)
{
    courrier *motokurye = (courrier *)arg;
    
    int bag_count = 0;
    int i = 0;
    int flag = 0;
    while (1) {
        Pide bag[3];
        while(totalOrder > 0)
        {
            if(bag_count <= 3)
            {
                pthread_mutex_lock(&preparedMutex);

                while(isEmpty(preparedQueue))
                {
                    pthread_cond_wait(&preparedFull, &preparedMutex);
                }
                bag[i++] = dequeue(preparedQueue);
                pthread_cond_broadcast(&preparedEmpty);
                pthread_mutex_unlock(&preparedMutex);
                bag_count++;
                pthread_mutex_lock(&totalOrderMutex);
                totalOrder--;
                pthread_mutex_unlock(&totalOrderMutex);
                flag = 1;
            }
        }
        //yola cik
        if(flag == 1){
            delivery(motokurye->motoID, bag, bag_count);
            printf("Moto %d dukkana geri dondu\n", motokurye->motoID);
        }
        flag = 0;
        // if (size(orderQueue) && bag_count == 0) {
        // }

        // // Process the items in the bag
        // for (i = 0; i < bag_count; i++) {
        //     bag[i] = dequeue(preparedQueue);
        //     // Modify as needed for your application
        //     bag[i].id += i;
        //     bag[i].loc_x += i * 0.1;
        //     bag[i].loc_y += i * 0.1;
        //     bag[i].mod_time += i * 1000;
        //     bag[i].cookid += i;
        // }

        // // Example of how to use the bag array
        // for (i = 0; i < bag_count; i++) {
        //     printf("Pide %d: id=%d, loc_x=%f, loc_y=%f, mod_time=%ld, cookid=%d\n",
        //            i, bag[i].id, bag[i].loc_x, bag[i].loc_y,
        //            bag[i].mod_time, bag[i].cookid);
        // }

        // // Reset bag count and unlock mutex
        // bag_count = 0;
    }
}
void handleClient(int client_sock, int argumenSize) {
    variables myVariables[argumenSize];
    Pide newOrder;
    int id;
    
    // Receive the structures
    for (int i = 0; i < argumenSize; i++) {
        if (recv(client_sock, &myVariables[i], sizeof(variables), 0) == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }
    }

    // newOrder.id = ID; 
    // newOrder.loc_x = atof(splitted[0]);
    // newOrder.loc_y = atof(splitted[1]);

    // Process the received structures (for demonstration, we just print them)
    for (int i = 0; i < argumenSize; i++) {
        printf("Structure %d: home_locationX=%d, home_locationY=%d\n",
               i, myVariables[i].home_locationX, myVariables[i].home_locationY);
        newOrder.id = i+1; 
        newOrder.loc_x = myVariables[i].home_locationX;
        newOrder.loc_y = myVariables[i].home_locationY;
        pthread_mutex_lock(&orderMutex);
    
        while(isFull(orderQueue))
        {
            printf("Manager siparisi tahtaya yazmayi bekliyor.(order ID: %d)\n", newOrder.id);
            pthread_cond_wait(&orderEmpty, &orderMutex);
        }
        printf("Manager siparisi tahtaya yazdi.(order ID: %d)\n", newOrder.id);
        enqueue(orderQueue, newOrder);
        
        pthread_cond_broadcast(&orderFull);
        pthread_mutex_unlock(&orderMutex);
    }
}
void setOrder(int ID, variables clientInfo){
    Pide newOrder; 
    int row = 0;
    // char splitted[2][10];          // Row index for the 2D array

    // Use strtok to tokenize the string by spaces
    // char* token = strtok(buffer, " ");
    // while (token != NULL && row < 2) {
    //     strncpy(splitted[row], token, sizeof(splitted[row]) - 1);
    //     splitted[row][sizeof(splitted[row]) - 1] = '\0';  // Ensure null termination
    //     row++;
    //     token = strtok(NULL, " ");
    // }
    newOrder.id = ID; 
    // newOrder.loc_x = atof(splitted[0]);
    // newOrder.loc_y = atof(splitted[1]);

    newOrder.loc_x = clientInfo.home_locationX;
    newOrder.loc_y = clientInfo.home_locationY;

    pthread_mutex_lock(&orderMutex);
    
    while(isFull(orderQueue))
    {
        printf("Manager siparisi tahtaya yazmayi bekliyor.(order ID: %d)\n", newOrder.id);
        pthread_cond_wait(&orderEmpty, &orderMutex);
    }
    printf("Manager siparisi tahtaya yazdi.(order ID: %d)\n", newOrder.id);
    enqueue(orderQueue, newOrder);
    
    pthread_cond_broadcast(&orderFull);
    pthread_mutex_unlock(&orderMutex);


    // printf("%d\n", newOrder.id);
    // printf("%f\n", newOrder.loc_x);
    // printf("%f\n", newOrder.loc_y);
    printf("----------\n");

}

int main(int argc, char **argv){
    int socketfd; 
    struct sockaddr_in addr; //Structures for handling internet addresses (netinet/in.h)
    socklen_t addrLen = sizeof(addr);
    int ipAddr;
    int cookThreadPool, delivThreadPool;
    int newSocket; 
    char buffer[BUFF_SIZE];
    int orderID=0;
    orderQueue = createQueue(50);
    ovenQueue = createQueue(6);
    preparedQueue = createQueue(50);
    variables clientInfos;
    sehirPopulasyon environmentInfo;


    /*System Log Initialization Start*/
    char log[LOG_BUFFER_LEN]; //Use STDOUT and file buffer as same
    int numBytesReadLog = 0;
    /*System Log Initializtion End*/

    /* USAGE_ERR_START*/
    if(argc != 5){
        numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "Usage %s: [PortNumber] [CookthreadPoolSize] [DeliveryPoolSize] [k]\n", argv[1]);
        write(STDOUT_FILENO, log, numBytesReadLog);
        memset(log, 0, numBytesReadLog);
        exit(EXIT_FAILURE);
    }
    /*USAGE_ERR_END */



    /*INPUT PREPARATION START*/
    ipAddr = atoi(argv[1]);
    cookThreadPool = atoi(argv[2]);
    delivThreadPool = atoi(argv[3]);
    speed = atoi(argv[4]);
    pthread_t cook[cookThreadPool], motoThread[delivThreadPool];
    tInfo = (TimerInfo*) malloc(sizeof(TimerInfo) * cookThreadPool);
    moto = (courrier*) malloc(sizeof(courrier) * delivThreadPool);
    timerCount = cookThreadPool;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_NUM);
    if(inet_pton(AF_INET, argv[1], &addr.sin_addr) <= 0){
        numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "Invalid Address or not supported\n");
        write(STDOUT_FILENO, log, numBytesReadLog);
        memset(log, 0, numBytesReadLog);
        exit(EXIT_FAILURE);
    }
    /*INPUT PREPARATION END*/
    for(int i = 0; i<cookThreadPool; i++){
        tInfo[i].threadID = i + 1;
        pthread_create(&cook[i], NULL, pideShopAsci, &tInfo[i]);
    }
    for(int i = 0; i<delivThreadPool; i++){
        moto[i].motoID = i + 1;
        pthread_create(&motoThread[i], NULL, pideShopMoto, &moto[i]);
    }
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
    memset(log, 0, numBytesReadLog);

    if(listen(socketfd, 100) == -1){//burda sikinti var
        perror("listen failed\n");
        exit(EXIT_FAILURE);
    }

    while((newSocket = accept(socketfd, (struct sockaddr *)&addr, &addrLen)))
    {
        numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, ">Connection Established\n");
        write(STDOUT_FILENO, log, numBytesReadLog);
        memset(log, 0, numBytesReadLog);
        recv(newSocket, &environmentInfo, sizeof(sehirPopulasyon), 0);
        // for(int i = 0; i < 7; i++){
        //     setOrder(++orderID, clientInfos);
        // }
        printf("received %d, %d, %d \n",environmentInfo.clientCapacity, environmentInfo.cityX, environmentInfo.cityY);
        shopX = environmentInfo.cityX;
        shopY = environmentInfo.cityY;
        handleClient(newSocket, environmentInfo.clientCapacity);
        // setOrder(buffer, ++orderID);
    }

    /*SOCKET OPERATIONS END*/
    close(socketfd);
    close(newSocket);
    printf("Hello World\n");
    free(tInfo);
    free(ovenQueue);
    free(orderQueue);
    return 0;
}