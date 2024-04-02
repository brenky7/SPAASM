
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

int create_client_socket() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error opening sock");
        exit(EXIT_FAILURE);
    }
    return client_socket;
}

void connect_to(int client_socket, const char *serverIP, int port) {
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);

    if (connect(client_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }
}

void sendMessage(int server_socket, const char *message) {
    if (send(server_socket, message, strlen(message), 0) < 0) {
        perror("Error sending message");
        exit(EXIT_FAILURE);
    }
}

void receiveMessage(int server_socket, char *buffer, int bufferSize) {
    ssize_t bytesReceived = recv(server_socket, buffer, bufferSize - 1, 0);
    if (bytesReceived < 0) {
        perror("Error receiving message");
        exit(EXIT_FAILURE);
    }
    buffer[bytesReceived] = '\0';
}

void close_client_socket(int client_socket) {
    close(client_socket);
}
