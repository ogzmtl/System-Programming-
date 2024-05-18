#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

#define sem_inChargeforPickup "/sem_InChangeforPickup"
#define sem_newAutomobile "/sem_newAutomobile"
#define sem_newPickup "/sem_newPickup"
#define sem_inChargeforAutomobile "/sem_inChargeforAutomobile"
#define size_InChargeAuto (sizeof(sem_inChargeforAutomobile)+1)
#define size_InChargePickup (sizeof(sem_inChargeforPickup)+1)
#define size_newAuto (sizeof(sem_newAutomobile)+1)
#define size_newPickup (sizeof(sem_newPickup)+1)
#define BUFF_SIZE 256

int carID;
int mFree_automobile=8, mFree_pickup=4;
sem_t *inChargeforAutomobile, *inChargeforPickup, *newPickup, *newAutomobile;
int runStatus = 1;

void handler(int sigNum)
{
    char log[BUFF_SIZE];
    int numBytes = 0;
    if(sigNum == SIGINT)
    {
        numBytes = snprintf(log, BUFF_SIZE, "SIGINT Handled\n");
        write(STDOUT_FILENO, log, numBytes);
        memset(log,0,numBytes);
        runStatus = 0;
    }
}

void* carOwner(void* arg)
{
    int numBytesWrittenLog = 0;
    char logBuffer[BUFF_SIZE];
    int x =0;
    srand(time(NULL));
    while(runStatus)
    {
        int r = rand()%2;
        carID++;

        if(r == 0)
        {
            if(mFree_automobile > 0)
            {
                sem_post(newAutomobile);
                numBytesWrittenLog = snprintf(logBuffer, BUFF_SIZE, "-------------------\nNew Automobile (carID : %d) comes to parking lot\n", carID);
                write(STDOUT_FILENO, logBuffer, numBytesWrittenLog);
                memset(logBuffer, 0, numBytesWrittenLog);
                sem_wait(inChargeforAutomobile);
            }
            else{
                numBytesWrittenLog = snprintf(logBuffer, BUFF_SIZE, "-------------------\nNew Automobile (carID : %d) comes to parking lot\nNo Available Space For Automobile: %d\nCould not parked Automobile, CarID: %d...\nLeaving...\n-------------------\n", carID, mFree_automobile, carID);
                write(STDOUT_FILENO, logBuffer, numBytesWrittenLog);
                memset(logBuffer, 0, numBytesWrittenLog);
            }
        }else{
            if(mFree_pickup > 0)
            {
                sem_post(newPickup);
                numBytesWrittenLog = snprintf(logBuffer, BUFF_SIZE, "-------------------\nNew Pickup (carID : %d) comes to parking lot\n", carID);
                write(STDOUT_FILENO, logBuffer, numBytesWrittenLog);
                memset(logBuffer, 0, numBytesWrittenLog);
                sem_wait(inChargeforPickup);
            }
            else{
                numBytesWrittenLog = snprintf(logBuffer, BUFF_SIZE, "-------------------\nNo Available Space For Pickup: %d\nCould not parked Pickup, CarID: %d...\nLeaving...\n-------------------\n", mFree_pickup, carID);
                write(STDOUT_FILENO, logBuffer, numBytesWrittenLog);
                memset(logBuffer, 0, numBytesWrittenLog);
            }

        }
        sleep(5);
        x = rand();
        if(x % 11 == 0)
        {
            if(x % 2 == 0){
                if(mFree_automobile < 8){
                    mFree_automobile++;
                    numBytesWrittenLog = snprintf(logBuffer, BUFF_SIZE, "********Automobile Leaving, New Available Space (mFreeAutomobile): %d\n", mFree_automobile);
                    write(STDOUT_FILENO, logBuffer, numBytesWrittenLog);
                    memset(logBuffer, 0, numBytesWrittenLog);               
                }
            }
            else
            {
                if(mFree_pickup < 4) 
                {
                    mFree_pickup++;
                    numBytesWrittenLog = snprintf(logBuffer, BUFF_SIZE, "********Pickup Leaving, New Available Space (mFreePickup): %d\n", mFree_pickup);
                    write(STDOUT_FILENO, logBuffer, numBytesWrittenLog);
                    memset(logBuffer, 0, numBytesWrittenLog);
                }
            }
        }
    }
    sem_post(newPickup);
    sem_post(newAutomobile);
}


void* carAttendantAutomobile(void* arg)
{
    char log[BUFF_SIZE];
    int numBytesWrittenCarOwner = 0;

    while(runStatus)
    {
        sem_wait(newAutomobile);    
            if(mFree_automobile > 0)    
                mFree_automobile--;
            numBytesWrittenCarOwner = snprintf(log, BUFF_SIZE, "Available Space For Automobile: %d\nParking Automobile, CarID: %d...\nAvailable Space After Park: %d\n-------------------\n", mFree_automobile+1, carID, mFree_automobile);
            write(STDOUT_FILENO, log, numBytesWrittenCarOwner);
            memset(log, 0, numBytesWrittenCarOwner);
        sem_post(inChargeforAutomobile);
    }
}

void* carAttendantPickup(void* arg)
{
    char log[BUFF_SIZE];
    int numBytesWrittenCarOwner = 0;

    while(runStatus)
    {
        sem_wait(newPickup);
            if(mFree_pickup > 0) 
                mFree_pickup--;
            numBytesWrittenCarOwner = snprintf(log, BUFF_SIZE, "-------------------\nAvailable Space For Pickup: %d\nParking Pickup, CarID: %d...\nAvailable Space After Park: %d\n-------------------\n", mFree_pickup+1, carID, mFree_pickup);
            write(STDOUT_FILENO, log, numBytesWrittenCarOwner);
            memset(log, 0, numBytesWrittenCarOwner);
        sem_post(inChargeforPickup);
    }
}

int main()
{
    pthread_t parkinLotSystem[3];
    char inChargePickup[size_InChargePickup];
    char inChargeAutomobile[size_InChargeAuto];
    char newAuto[size_newAuto];
    char newPick[size_newPickup];

    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    snprintf(inChargePickup, size_InChargePickup, sem_inChargeforPickup);
    inChargeforPickup = sem_open(inChargePickup, O_CREAT, 0666, 0);
    if(inChargeforPickup == SEM_FAILED){
        perror("sem_open error\n");
    }
    snprintf(inChargeAutomobile, size_InChargeAuto, sem_inChargeforAutomobile);
    inChargeforAutomobile = sem_open(inChargeAutomobile, O_CREAT, 0666, 0);
    if(inChargeforAutomobile == SEM_FAILED){
        sem_unlink(inChargePickup);
        perror("sem_open error\n");
        exit(EXIT_FAILURE);
    }
    snprintf(newPick, size_newPickup, sem_newPickup);
    newPickup = sem_open(newPick, O_CREAT, 0666, 0);
    if(newPickup == SEM_FAILED){
        sem_unlink(inChargePickup);
        sem_unlink(inChargeAutomobile);
        perror("sem_open error\n");
        exit(EXIT_FAILURE);
    }
    snprintf(newAuto, size_newAuto, sem_newAutomobile);
    newAutomobile = sem_open(newAuto, O_CREAT, 0666, 0);
    if(newAutomobile == SEM_FAILED){
        perror("sem_open error\n");
        sem_unlink(inChargePickup);
        sem_unlink(inChargeAutomobile);
        sem_unlink(newPick);
        exit(EXIT_FAILURE);
    }

    pthread_create(&parkinLotSystem[0], NULL, &carOwner, NULL);
    pthread_create(&parkinLotSystem[1], NULL, &carAttendantAutomobile, NULL);
    pthread_create(&parkinLotSystem[2], NULL, &carAttendantPickup, NULL);

    for(int i = 0; i < 3; i++){
        pthread_join(parkinLotSystem[i], NULL);
    }
    sem_close(inChargeforAutomobile);
    sem_close(newPickup);
    sem_close(newAutomobile);
    sem_close(inChargeforPickup);
    sem_unlink(inChargePickup);
    sem_unlink(inChargeAutomobile);
    sem_unlink(newAuto);
    sem_unlink(newPick);
    return 0;
}