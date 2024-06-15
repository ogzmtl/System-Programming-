#include "final_common.h"
#include "queue.h"

typedef struct {
    timer_t timerID;
    int threadID;
} TimerInfo;

typedef struct {
    int motoID;
    Pide bag[3];
    int received;
}courrier;

sehirPopulasyon environmentInfo;
int timerCount = 0;
int speed = 0;
int shopX=0;
int shopY=0;
int cookSize = 0;
int delivSize= 0;
int siparisIptal = 0;
int totalReceived = 0;
int totalOrderSize = 0;
int onlysendFlag = 1;;
int cancelled = 0;

pthread_mutex_t totalReceivedMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t onlySendFlag= PTHREAD_MUTEX_INITIALIZER;
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



void handler(int signum)
{
    char logg[LOG_BUFFER_LEN];
    int written = 0;

    if(signum == SIGINT){
        sehirPopulasyon temp = {0,0,0,0,0,0,0,0,BURNED};
        temp.sockfd = environmentInfo.sockfd;

        written = snprintf(logg, LOG_BUFFER_LEN, "> SIGINT catched, Burned Down\n");
        write(STDOUT_FILENO, logg, written);
        memset(logg, 0, LOG_BUFFER_LEN);
        if(temp.sockfd == -1){
            exit(0);
        }
        send(temp.sockfd, &temp, sizeof(sehirPopulasyon), 0);

        // siparisIptal = 1;
        cancelled = 1;
        pthread_cond_broadcast(&orderFull);
        pthread_cond_signal(&apparatusCond);
        pthread_cond_broadcast(&ovenCondEmpty);
        pthread_cond_broadcast(&preparedFull);
        pthread_cond_broadcast(&ovenCondFull);
        pthread_cond_broadcast(&preparedEmpty);
        
        // exit(0);
    }
    else if(signum == SIGUSR1){
        written = snprintf(logg, LOG_BUFFER_LEN, "> ^C.. Upps quiting.. writing log file\n");
        writeToLog(logg);
        write(STDOUT_FILENO, logg, written);
        memset(logg, 0, LOG_BUFFER_LEN);

        siparisIptal = 1;
        pthread_cond_broadcast(&apparatusCond);
        pthread_cond_broadcast(&ovenCondFull);
        pthread_cond_broadcast(&ovenCondEmpty);
        // pthread_cond_broadcast(&preparedFull);
    }
}

// long int pseudo_inverse(int m, int n){
//     struct timeval initTime;

//     if (gettimeofday(&initTime, NULL) != 0)
//     {
//         fprintf(stderr, "gettimeofday error\n");
//         return 1;
//     }
//     long int initmsec = initTime.tv_sec * 1000 + initTime.tv_usec / 1000;
//     complex double matrix[M][N];
//     complex double pseudo[M][N];

//     generate_matrix(matrix);
//     pseudo_inverse(matrix, pseudo);

//     struct timeval lastTime;
//     long int lastmsec = lastTime.tv_sec * 1000 + lastTime.tv_usec / 1000;
//     if (gettimeofday(&lastTime, NULL) != 0)
//     {
//         fprintf(stderr, "gettimeofday error\n");
//         return 1;
//     }
//     return (lastmsec - initmsec);
// }
// float customSqrt(float num) {
//     float guess = num / 2.0f;
//     float epsilon = 0.001f;

//     if (num < 0) return -1;

//     while ((guess * guess - num > epsilon) || (num - guess * guess > epsilon)) {
//         guess = (guess + num / guess) / 2.0f;
//     }

//     return guess;
// }

// float calculateDistance(float x1, float y1, float x2, float y2) {
//     float dx = x2 - x1;
//     float dy = y2 - y1;
//     return customSqrt(dx * dx + dy * dy);
// }

void delivery(int id, Pide bag[3], int size)
{
    char logBuf[LOG_BUFFER_LEN];
    int numBytesWritten = 0;
    int temp = 0;
    memset(logBuf, 0, LOG_BUFFER_LEN);

    for (int i = 0; i < size; i++) {
        float distance;
        if (i == 0) {
            distance = calculateDistance(shopX, shopY, bag[i].loc_x, bag[i].loc_y);
            moto[id-1].received++;
            pthread_mutex_lock(&totalReceivedMutex);
            totalReceived++;
            pthread_mutex_unlock(&totalReceivedMutex);
        } else{
            distance = calculateDistance(bag[i - 1].loc_x, bag[i - 1].loc_y, bag[i].loc_x, bag[i].loc_y);
            moto[id-1].received++;
            pthread_mutex_lock(&totalReceivedMutex);
            totalReceived++;
            pthread_mutex_unlock(&totalReceivedMutex);
        }
        if(!siparisIptal){
            numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN,"Moto %d, Delivering Pide %d. Location = (%d, %d), Distance to travel = %f units\n",id, bag[i].id,bag[i].loc_x, bag[i].loc_y, distance);
            writeToLog(logBuf);
            memset(logBuf,0, numBytesWritten);

            usleep(distance / speed * 100000);

            numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN,"Moto %d, Delivered Pide id: %d\n",id, bag[i].id);
            writeToLog(logBuf);
            memset(logBuf,0, numBytesWritten);
        }
        else{
            temp = i;
            while(i < size){
                numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN,"Moto %d, Throw Pide id: %d to thrash\n",id, bag[i].id);
                // send(environmentInfo.sockfd, logBuf, LOG_BUFFER_LEN,0);
                writeToLog(logBuf);
                memset(logBuf,0, numBytesWritten);   
                i++;
            }
        }

    }
    if(!siparisIptal){
        if(cancelled){
            pthread_exit(NULL);
        }
        float moveBack = calculateDistance(bag[size - 1].loc_x, bag[size - 1].loc_y, shopX, shopY);
        numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN, "> Moto %d, moving back to Shop, Distance to travel = %f units\n", id, moveBack);
        // write(STDOUT_FILENO, logBuf, numBytesWritten);
        memset(logBuf, 0,numBytesWritten); 

        usleep(moveBack / speed * 100000);
        // printf("only send flag %d\n", onlysendFlag);
        pthread_mutex_lock(&onlySendFlag);
        if(onlysendFlag)
        {
            // printf("total Received: %d\n", totalReceived);
            // printf("total Order %d\n", totalOrderSize);
            if(totalReceived == totalOrderSize)
            {
                sehirPopulasyon temp;
                temp.clientCapacity = environmentInfo.clientCapacity;
                temp.sockfd = environmentInfo.sockfd;
                temp.status = DELIVERED;
                send(temp.sockfd, &temp, sizeof(sehirPopulasyon),0);
                onlysendFlag = 0;
            }
        }
        pthread_mutex_unlock(&onlySendFlag);
    }else{
        float moveBack = calculateDistance(bag[temp].loc_x, bag[temp].loc_y, shopX, shopY);
        numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN, "> Moto %d, moving back to Shop, Distance to travel = %f units\n", id, moveBack);
        // write(STDOUT_FILENO, logBuf, numBytesWritten);
        memset(logBuf, 0,numBytesWritten); 
        usleep(moveBack / speed * 100000);
    }


}

void timerHandler(union sigval sv) {
    TimerInfo *tInfo = (TimerInfo *)sv.sival_ptr;
    Pide sicakPide;
    sehirPopulasyon temp = {0,0,0,0,0,0,0,0,REQUESTED};
    char logBuf[LOG_BUFFER_LEN];
    int numBytesWritten = 0;
    memset(logBuf, 0, LOG_BUFFER_LEN);

    
    numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN,"Timer expired... cook ID: %d returns to oven\n", tInfo->threadID);
    writeToLog(logBuf);
    memset(logBuf,0, numBytesWritten);
    
    pthread_mutex_lock(&apparatusMutex);
    while(aparatus == 0){
        pthread_cond_wait(&apparatusCond, &apparatusMutex);
        if(siparisIptal ) break;
        if(cancelled){
            pthread_cond_signal(&apparatusCond);
            pthread_mutex_unlock(&apparatusMutex);
            pthread_exit(NULL);
        }
    }
    aparatus--;
    pthread_cond_signal(&apparatusCond);
    pthread_mutex_unlock(&apparatusMutex);
    
    numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN,"The cook: %d took the aparatus...\n", tInfo->threadID);
    writeToLog(logBuf);
    memset(logBuf,0, numBytesWritten);
    
        pthread_mutex_lock(&ovenMutex);
        while (isEmpty(ovenQueue)) {
            pthread_cond_wait(&ovenCondFull, &ovenMutex);
            if(siparisIptal){
                break;
            }
            if(cancelled){
                pthread_mutex_lock(&apparatusMutex);
                aparatus++;
                pthread_cond_signal(&apparatusCond);
                pthread_mutex_unlock(&apparatusMutex);
                pthread_cond_broadcast(&ovenCondEmpty);
                pthread_mutex_unlock(&ovenMutex);
                pthread_exit(NULL);
            }
        }

        sicakPide = dequeue(ovenQueue);

        pthread_cond_broadcast(&ovenCondEmpty);
        pthread_mutex_unlock(&ovenMutex);

        //log icin if check continue ile
        if(!siparisIptal){
            numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN,"The cook: %d, took order id: %d from oven \n", tInfo->threadID,sicakPide.id);
            writeToLog(logBuf);
            // send(environmentInfo.sockfd, logBuf, LOG_BUFFER_LEN,0);
            temp.clientCapacity = environmentInfo.clientCapacity;
            temp.sockfd = environmentInfo.sockfd;
            temp.orderID = sicakPide.id;
            temp.cookID = tInfo->threadID;
            temp.status = PREPARED;
            send(temp.sockfd, &temp, sizeof(sehirPopulasyon),0);
            memset(logBuf,0, numBytesWritten);
        }else{

            numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN,"The cook: %d, took order id: %d andthrow it to garbage \n", tInfo->threadID,sicakPide.id);
            writeToLog(logBuf);
            // send(environmentInfo.sockfd, logBuf, LOG_BUFFER_LEN,0);
            memset(logBuf,0, numBytesWritten);
        }

        pthread_mutex_lock(&apparatusMutex);
        aparatus++;
        pthread_cond_signal(&apparatusCond);
        pthread_mutex_unlock(&apparatusMutex);

    numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN,"The cook: %d left the aparatus...to take out of the oven \n", tInfo->threadID);
    writeToLog(logBuf);
    memset(logBuf,0, numBytesWritten);

    //if check continue
    pthread_mutex_lock(&preparedMutex);
    while (isFull(preparedQueue)) {
        pthread_cond_wait(&preparedEmpty, &preparedMutex);
        if(siparisIptal) break;
        if(cancelled){
            pthread_cond_broadcast(&preparedFull);
            pthread_mutex_unlock(&preparedMutex);
            pthread_exit(NULL);
        }
    }
    //if check continue
    if(!siparisIptal)
        enqueue(preparedQueue, sicakPide);

    pthread_cond_broadcast(&preparedFull);
    pthread_mutex_unlock(&preparedMutex);
    if(cancelled){
        pthread_cond_broadcast(&preparedFull);
        pthread_mutex_unlock(&preparedMutex);
        pthread_exit(NULL);
    }
    //log bastirma icin check
    if(!siparisIptal){
        numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN,"The cook: %d, sends order id: %d to manager \n", tInfo->threadID,sicakPide.id);
        writeToLog(logBuf);
        memset(logBuf,0, numBytesWritten);
    }else{
        numBytesWritten = snprintf(logBuf, LOG_BUFFER_LEN,"The cook: %d, sends order id: %d to trash \n", tInfo->threadID,sicakPide.id);
        writeToLog(logBuf);
        memset(logBuf,0, numBytesWritten);
    }


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
    int aparatAzaltildi = 0;

    while(1)
    {
        Pide preparePide;
        // sehirPopulasyon temp = {0,0,0,0,0,0,0,0,REQUESTED};
        
        pthread_mutex_lock(&orderMutex);
        
        while(isEmpty(orderQueue))
        {
            numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN, "Cook: %d waits for new order\n",tInfo->threadID);
            writeToLog(logBuff);
            memset(logBuff, 0, numBytesWritten);
            pthread_cond_wait(&orderFull, &orderMutex);// hepsi kapaninca deadlock olabilir dikkat et
            if(cancelled){
                pthread_cond_broadcast(&orderFull);
                pthread_mutex_unlock(&orderMutex);
                pthread_exit(NULL);
            }
        }

        preparePide = dequeue(orderQueue);
        pthread_cond_broadcast(&orderFull);
        pthread_mutex_unlock(&orderMutex);

        // temp.clientCapacity = environmentInfo.clientCapacity;
        // temp.sockfd = environmentInfo.sockfd;
        // temp.orderID = preparePide.id;
        // temp.cookID = tInfo->threadID;

        numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN, "> Pide id %d, preparing... thanks to cook:%d\n", preparePide.id,tInfo->threadID);
        writeToLog(logBuff);
        // write(STDOUT_FILENO, logBuff, numBytesWritten);
        memset(logBuff, 0, numBytesWritten);

        long int sleep_time = pseudo_inverse();//it is just sleep function, make cpu busy for a time pide prepration
        // printf("sleep time %ld\n", sleep_time);
        // int minutes = sleep_time / (1000 * 60);
        // sleep_time %= (1000 * 60);
        // int seconds = sleep_time / 1000;

        
        pthread_mutex_lock(&apparatusMutex);
        while(aparatus == 0){
            numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN, "Cook: %d waits for aparatus for (Order id: %d)\n",tInfo->threadID, preparePide.id);
            writeToLog(logBuff);
            memset(logBuff, 0, numBytesWritten);
            pthread_cond_wait(&apparatusCond, &apparatusMutex);
            if(siparisIptal) break;
            if(cancelled){
                pthread_cond_signal(&apparatusCond);
                pthread_mutex_unlock(&apparatusMutex);
                pthread_exit(NULL);
            }
        }

        if(cancelled){
                pthread_cond_signal(&apparatusCond);
                pthread_mutex_unlock(&apparatusMutex);
                pthread_exit(NULL);
        }
        if(!siparisIptal){
            aparatus--;
            aparatAzaltildi = 1;
            pthread_cond_broadcast(&apparatusCond);
            pthread_mutex_unlock(&apparatusMutex);
            numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN,"Cook took the aparatus (cook id: %d)\n", tInfo->threadID);
            writeToLog(logBuff);
            memset(logBuff, 0, numBytesWritten);
        }else{
            pthread_cond_broadcast(&apparatusCond);
            pthread_mutex_unlock(&apparatusMutex);
        }
        
        pthread_mutex_lock(&ovenMutex);
        while (isFull(ovenQueue)) {
                pthread_mutex_lock(&apparatusMutex);
                aparatus++;
                pthread_cond_signal(&apparatusCond);
                pthread_mutex_unlock(&apparatusMutex);
            numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN,"Order id: %d is waiting oven queue... left the aparatus (cook: %d)\n", preparePide.id, tInfo->threadID);
            writeToLog(logBuff);
            memset(logBuff, 0, numBytesWritten);
                pthread_mutex_lock(&apparatusMutex);
                aparatus--;
                pthread_cond_signal(&apparatusCond);
                pthread_mutex_unlock(&apparatusMutex);
            pthread_cond_wait(&ovenCondEmpty, &ovenMutex);
            if(siparisIptal)break;
            if(cancelled){
                pthread_cond_broadcast(&ovenCondFull);
                pthread_mutex_unlock(&ovenMutex);
                pthread_mutex_lock(&apparatusMutex);
                aparatus++;
                pthread_cond_signal(&apparatusCond);
                pthread_mutex_unlock(&apparatusMutex);
                pthread_exit(NULL);
            }
        }

        if(!siparisIptal)
            enqueue(ovenQueue, preparePide);

        pthread_cond_broadcast(&ovenCondFull);
        pthread_mutex_unlock(&ovenMutex);
        if(cancelled){
            pthread_exit(NULL);
        }

        if(siparisIptal && aparatAzaltildi){
            pthread_mutex_lock(&apparatusMutex);
            aparatus++;
            pthread_cond_signal(&apparatusCond);
            pthread_mutex_unlock(&apparatusMutex);
        }
        else if(!siparisIptal){
            pthread_mutex_lock(&apparatusMutex);
            aparatus++;
            pthread_cond_signal(&apparatusCond);
            pthread_mutex_unlock(&apparatusMutex);
        }

        if(!siparisIptal)
        {
            numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN,"> Cook left the aparatus (cook id: %d)\n", tInfo->threadID);
            writeToLog(logBuff);
            memset(logBuff, 0, numBytesWritten);

            numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN, "> Order %d placed to oven (Cook: %d)\n",preparePide.id, tInfo->threadID);
            writeToLog(logBuff);
            // write(STDOUT_FILENO, logBuff, numBytesWritten);
            memset(logBuff, 0,numBytesWritten);
            if(cancelled){
                pthread_cond_broadcast(&orderFull);
                pthread_exit(NULL);
            }
            startTimer(tInfo, (sleep_time/sleep_time));
        }else{
 
            numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN, "Order %d cancelled (Cook: %d)\n",preparePide.id, tInfo->threadID);
            writeToLog(logBuff);
        }
        aparatAzaltildi = 0;
    }
}

void* pideShopMoto(void* arg)
{
    courrier *motokurye = (courrier *)arg;
    int bag_count = 0;
    // int i = 0;
    int flag = 0;    
    int numBytesWritten =  0;
    char logBuff[LOG_BUFFER_LEN];
    memset(logBuff, 0, LOG_BUFFER_LEN);

    while (1) {
        Pide bag[3];
        pthread_mutex_lock(&preparedMutex);
        while (totalOrder > 0 && bag_count < 3) {
            sehirPopulasyon temp = {0,0,0,0,0,0,0,0,REQUESTED};

            while (isEmpty(preparedQueue)) {
                numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN, "> Moto id: %d is waiting for bag to be filled\n", motokurye->motoID);
                writeToLog(logBuff);
                // write(STDOUT_FILENO, logBuff, numBytesWritten);
                memset(logBuff, 0, numBytesWritten);

                pthread_cond_wait(&preparedFull, &preparedMutex);
                if (totalOrder == 0)break;
                if (siparisIptal){
                    bag_count = 0;
                }
                if(cancelled){

                    pthread_cond_broadcast(&preparedEmpty);
                    pthread_mutex_unlock(&preparedMutex);
                    pthread_exit(NULL);
                }
            }

            if (!siparisIptal && totalOrder != 0) {
                bag[bag_count] = dequeue(preparedQueue);
                temp.orderID = bag[bag_count].id;
                temp.motoID = motokurye->motoID;
                temp.sockfd = environmentInfo.sockfd;
                temp.status = ON_DELIVERY;

                send(temp.sockfd, &temp, sizeof(sehirPopulasyon), 0);

                numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN, "> Moto id: %d is taking order id %d\n", motokurye->motoID, bag[bag_count].id);
                writeToLog(logBuff);
                // write(STDOUT_FILENO, logBuff, numBytesWritten);
                memset(logBuff, 0, numBytesWritten);

                bag_count++;
                pthread_mutex_lock(&totalOrderMutex);
                totalOrder--;
                pthread_mutex_unlock(&totalOrderMutex);
                flag = 1;
            } else if (!siparisIptal && bag_count > 0) {
                flag = 1;
            } else {
                break;
            }
        }
        pthread_cond_broadcast(&preparedEmpty);
        pthread_mutex_unlock(&preparedMutex);
        pthread_cond_broadcast(&preparedFull);
        if(cancelled){
            pthread_cond_broadcast(&preparedEmpty);
            pthread_mutex_unlock(&preparedMutex);
            pthread_exit(NULL);
        }
        if (flag == 1 && !siparisIptal && bag_count > 0) {
            delivery(motokurye->motoID, bag, bag_count);
            numBytesWritten = snprintf(logBuff, LOG_BUFFER_LEN, "> Moto id: %d returned\n", motokurye->motoID);
            writeToLog(logBuff);
            // write(STDOUT_FILENO, logBuff, numBytesWritten);
            memset(logBuff, 0, numBytesWritten);
            bag_count = 0;
        } else if (siparisIptal) {
            bag_count = 0;
        }
        flag = 0;
        if(cancelled){
            pthread_cond_broadcast(&preparedEmpty);
            pthread_mutex_unlock(&preparedMutex);
            pthread_exit(NULL);
        }
    }
}
void handleClient(int sockfd, int argumentSize) {
    variables myVariables[argumentSize];
    Pide *newOrder = (Pide*)malloc(sizeof(Pide)*argumentSize);
    memset(newOrder, 0, sizeof(Pide)*argumentSize);
    totalOrderSize = argumentSize;
    pthread_mutex_lock(&totalReceivedMutex);
    totalReceived = 0;
    pthread_mutex_unlock(&totalReceivedMutex);
    // int id;
    char buff[256];
    memset(buff, 0, 256);
    for (int i = 0; i < argumentSize; i++) {

        if (recv(sockfd, &myVariables[i], sizeof(variables), 0) == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }

        newOrder[i].id = i+1; 
        newOrder[i].loc_x = (int) myVariables[i].home_locationX;
        newOrder[i].loc_y = (int) myVariables[i].home_locationY;

    }

    for (int i = 0; i < argumentSize; i++) {

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

    int numBytesReaded = snprintf(buff, 256, "> %d new customers.. Serving...\n", argumentSize);
    writeToLog(buff);
    write(STDOUT_FILENO, buff, numBytesReaded);
    memset(buff, 0, numBytesReaded);
    free(newOrder);
}

int main(int argc, char **argv){
    int socketfd; 
    struct sockaddr_in addr; //Structures for handling internet addresses (netinet/in.h)
    socklen_t addrLen = sizeof(addr);
    int ipAddr;
    // int cookThreadPool, delivThreadPool;
    int newSocket; 
    // char buffer[BUFF_SIZE];
    // int orderID=0;

    // variables clientInfos;
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
        numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "> Usage %s: [PortNumber] [CookthreadPoolSize] [DeliveryPoolSize] [k]\n", argv[0]);
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

    orderQueue = createQueue(200);
    ovenQueue = createQueue(6);
    preparedQueue = createQueue(200);

    addr.sin_family = AF_INET;
    // addr.sin_port = htons(PORT_NUM);
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(ipAddr);
    // if(inet_pton(AF_INET, argv[1], &addr.sin_addr) <= 0){
    //     numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "> Invalid Address or not supported\n");
    //     write(STDOUT_FILENO, log, numBytesReadLog);
    //     writeToLog(log);
    //     memset(log, 0, numBytesReadLog);
    //     exit(EXIT_FAILURE);
    // }
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

    numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "> PideShop active waiting for connection...\n");
    write(STDOUT_FILENO, log, numBytesReadLog);
    writeToLog(log);
    memset(log, 0, numBytesReadLog);

    numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "> listening on port num %s\n\n", argv[1]);
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

    environmentInfo.sockfd = -1;
    while((newSocket = accept(socketfd, (struct sockaddr *)&addr, &addrLen)))
    {
        int cl_pid=0;
        int burned_down = 0;
        for(int i = 0; i < delivSize; i++)
        {
            moto[i].received = 0;
        }
        siparisIptal = 0;
        pthread_mutex_lock(&onlySendFlag);
        onlysendFlag = 1;
        pthread_mutex_unlock(&onlySendFlag);
        // newSocket = accept(socketfd, (struct sockaddr *)&addr, &addrLen);
        numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "> Connection Established\n");
        // write(STDOUT_FILENO, log, numBytesReadLog);
        writeToLog(log);
        memset(log, 0, numBytesReadLog);

        recv(newSocket, &environmentInfo, sizeof(sehirPopulasyon), 0);

        cl_pid = environmentInfo.pid;
        shopX = environmentInfo.X/2;
        shopY = environmentInfo.Y/2;
        // printf("newSocket: %d\n", newSocket);
        environmentInfo.sockfd = newSocket;
        handleClient(environmentInfo.sockfd, environmentInfo.clientCapacity);

        while(1){
            recv(newSocket, &environmentInfo, sizeof(sehirPopulasyon), 0);
            // printf("newSocket: %d\n", environmentInfo.status);
            if(environmentInfo.status == CANCELLED)
            {
                // printf("Signal Handled in server\n");
                numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "> order cancelled @ %d\n",cl_pid);
                write(STDOUT_FILENO, log, numBytesReadLog);
                writeToLog(log);
                memset(log, 0, numBytesReadLog);  
                raise(SIGUSR1);
                break;
            }
            if(environmentInfo.status == DELIVERED)
            {
                int largest = 0;
                int index; 

                for(int i = 0; i < delivSize; i++)
                {
                    if(largest < moto[i].received){
                        index = moto[i].motoID; 
                        largest = moto[i].received;
                    }
                    moto[i].received = 0;
                }
                numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "> Done serving client @ %d\n",cl_pid);
                write(STDOUT_FILENO, log, numBytesReadLog);
                writeToLog(log);
                memset(log, 0, numBytesReadLog);  

                numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "> Most Efficient moto is %d, delivered %d orders\n",index,largest);
                write(STDOUT_FILENO, log, numBytesReadLog);
                writeToLog(log);
                memset(log, 0, numBytesReadLog);


                numBytesReadLog = snprintf(log, LOG_BUFFER_LEN, "> PideShop active waiting for connection...\n\n");
                write(STDOUT_FILENO, log, numBytesReadLog);
                writeToLog(log);
                memset(log, 0, numBytesReadLog);

                break;
            }
            if(environmentInfo.status == BURNED){
                burned_down = 1;
                break;
            }
        }
        if(burned_down == 1)break;
    }
    
    for(int i = 0; i < cookSize; i++){
        pthread_join(cook[i], NULL);
    }
    for(int i = 0; i < delivSize; i++){
        pthread_join(motoThread[i], NULL);
    }

    close(socketfd);
    close(newSocket);
    free(tInfo);
    destroy(ovenQueue);
    destroy(orderQueue);
    free(cook);
    free(motoThread);
    free(moto);
    return 0;
}