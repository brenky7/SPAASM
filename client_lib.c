
// client_lib.c

#include "client_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int openSocket(const char *serverIP, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

void sendMessage(int sockfd, const char *message) {
    if (send(sockfd, message, strlen(message), 0) < 0) {
        perror("Error sending message");
        exit(EXIT_FAILURE);
    }
}

void receiveMessage(int sockfd, char *buffer, int bufferSize) {
    ssize_t bytesReceived = recv(sockfd, buffer, bufferSize - 1, 0);
    if (bytesReceived < 0) {
        perror("Error receiving message");
        exit(EXIT_FAILURE);
    }
    buffer[bytesReceived] = '\0'; // Null-terminate the received message
}

void closeSocket(int sockfd) {
    close(sockfd);
}
