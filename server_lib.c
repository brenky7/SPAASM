#include "server_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_PENDING_CONNECTIONS 5

// Function to create a server socket
int create_server_socket(int port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating server socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding server socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_PENDING_CONNECTIONS) < 0) {
        perror("Error listening on server socket");
        exit(EXIT_FAILURE);
    }

    return server_socket;
}

// Function to accept a client connection
int accept_client_connection(int server_socket) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_socket < 0) {
        perror("Error accepting client connection");
        exit(EXIT_FAILURE);
    }

    return client_socket;
}

// Function to receive a message from the client
int receive_message(int client_socket, char *buffer, int buffer_size) {
    int bytes_received = recv(client_socket, buffer, buffer_size, 0);
    if (bytes_received < 0) {
        perror("Error receiving message from client");
        exit(EXIT_FAILURE);
    }

    return bytes_received;
}

// Function to send a message to the client
int send_message(int client_socket, const char *message) {
    int bytes_sent = send(client_socket, message, strlen(message), 0);
    if (bytes_sent < 0) {
        perror("Error sending message to client");
        exit(EXIT_FAILURE);
    }

    return bytes_sent;
}

// Function to close a socket
void close_socket(int socket_fd) {
    close(socket_fd);
}

