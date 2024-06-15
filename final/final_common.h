#ifndef _F_COMMON_H__
#define _F_COMMON_H__

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
#include <math.h>
#include <complex.h>
#include <sys/time.h>

#define openFlags  O_CREAT | O_RDWR | O_NONBLOCK
#define PORT_NUM 8080
#define LOG_BUFFER_LEN 256
#define BUFF_SIZE 1024
#define M 40
#define N 30


typedef enum {
    BURNED,
    CANCELLED,
    REQUESTED,
    PREPARED, 
    ON_DELIVERY,
    DELIVERED
}orderStatus;

typedef struct{ 
    int X;
    int Y;
    int clientCapacity;
    int sockfd;
    int orderID;
    int cookID;
    int motoID;
    int pid;
    orderStatus status;
}sehirPopulasyon;

typedef struct{ 
    int home_locationX;
    int home_locationY;
    int cityX;
    int cityY;
    int clientCapacity;
}variables;

int writeToLog(const char *log);
complex double random_complex();
void generate_matrix(complex double matrix[M][N]);
void pseudo_inversex(complex double matrix[M][N], complex double pseudo_inv[N][M]);
long int pseudo_inverse();
float customSqrt(float num);
float calculateDistance(float x1, float y1, float x2, float y2);
#endif