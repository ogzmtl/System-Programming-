#include "final_common.h"

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

complex double random_complex() {
    double real = (double)rand() / RAND_MAX * 10;
    double imag = (double)rand() / RAND_MAX * 10;
    return real + imag * I;
}

void generate_matrix(complex double matrix[M][N]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = random_complex();
        }
    }
}

void pseudo_inversex(complex double matrix[M][N], complex double pseudo_inv[N][M]) {
    // Allocate memory for U, S, and V matrices
    complex double U[M][N], S[M][N], V[N][N];


    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            U[i][j] = matrix[i][j];
            V[j][i] = conj(matrix[i][j]); // Conjugate transpose
        }
    }

    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            S[i][j] = (i == j) ? cabs(matrix[i][j]) : 0;
        }
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            pseudo_inv[i][j] = 0;
            for (int k = 0; k < N; k++) {
                if (S[k][k] != 0) {
                    pseudo_inv[i][j] += V[i][k] * (1 / S[k][k]) * conj(U[j][k]);
                }
            }
        }
    }
}

long int pseudo_inverse(){
    struct timeval initTime, lastTime;

    if (gettimeofday(&initTime, NULL) != 0) {
        fprintf(stderr, "gettimeofday error\n");
        return 1;
    }
    
    long int initmsec = initTime.tv_sec * 1000 + initTime.tv_usec / 1000;
    // printf("init time: %ld ms\n", initmsec);
    
    complex double matrix[M][N];
    complex double pseudo[N][M];

    generate_matrix(matrix);
    pseudo_inversex(matrix, pseudo);

    if (gettimeofday(&lastTime, NULL) != 0) {
        fprintf(stderr, "gettimeofday error\n");
        return 1;
    }
    
    long int lastmsec = lastTime.tv_sec * 1000 + lastTime.tv_usec / 1000;
    // printf("last time: %ld ms\n", lastmsec);

    return (lastmsec - initmsec);
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
