#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

// Structure to hold timer information
typedef struct {
    timer_t timerID;
    int threadID;
} TimerInfo;

// Function to be executed when the timer expires
void timerHandler(union sigval sv) {
    TimerInfo *tInfo = (TimerInfo *)sv.sival_ptr;
    printf("Timer expired in thread ID: %d (thread: %ld)\n", tInfo->threadID, pthread_self());
}

// Function to create and start a timer
void startTimer(TimerInfo *tInfo, int seconds) {
    struct sigevent sev;
    struct itimerspec its;

    // Specify the timer expiration callback
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_value.sival_ptr = tInfo;
    sev.sigev_notify_function = timerHandler;
    sev.sigev_notify_attributes = NULL;

    // Create the timer
    if (timer_create(CLOCK_REALTIME, &sev, &tInfo->timerID) == -1) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }

    // Set the timer to expire after 'seconds' seconds
    its.it_value.tv_sec = seconds;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;  // Non-repeating timer
    its.it_interval.tv_nsec = 0;

    if (timer_settime(tInfo->timerID, 0, &its, NULL) == -1) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }
}

void *timerThread(void *arg) {
    TimerInfo *tInfo = (TimerInfo *)arg;

    // Start a timer that expires after 5 seconds
    startTimer(tInfo, 5);

    // The thread can perform other tasks while waiting for the timer to expire
    for (int i = 0; i < 10; ++i) {
        printf("Thread ID %d is doing work %d (thread: %ld)\n", tInfo->threadID, i, pthread_self());
        sleep(1);
    }

    // Wait for the timer to expire

    return NULL;
}

int main() {
    pthread_t thread1, thread2, thread3;
    TimerInfo tInfo1 = {.threadID = 1};
    TimerInfo tInfo2 = {.threadID = 2};
    TimerInfo tInfo3 = {.threadID = 3};

    // Create and start three timer threads
    pthread_create(&thread1, NULL, timerThread, &tInfo1);
    sleep(2);
    pthread_create(&thread2, NULL, timerThread, &tInfo2);
    pthread_create(&thread3, NULL, timerThread, &tInfo3);

    // Join the threads (optional, depending on your application needs)
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    return 0;
}
