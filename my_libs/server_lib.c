#include "server_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#define MAX_CLIENTS 10
#define TIMEOUT 1 // seconds

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

    int flags = fcntl(server_socket, F_GETFL, 0);
    if (flags == -1) {
        perror("Error getting socket flags");
        exit(EXIT_FAILURE);
    }

    if (fcntl(server_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("Error setting socket to non-blocking mode");
        exit(EXIT_FAILURE);
    }

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding server socket");
        exit(EXIT_FAILURE);
    }

    return server_socket;
}

void start_listening(int server_socket) {
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Error listening on server socket");
        exit(EXIT_FAILURE);
    }
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
long receive_message(int client_socket, char *buffer, int buffer_size) {
    long bytes_received = recv(client_socket, buffer, buffer_size, 0);
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

// Function to close server socket
void close_server_socket(int server_socket, int server_connections[10]) {
    for (int i = 0; i < 10; i++) {
        if (server_connections[i] != -1) {
            close(server_connections[i]);
        }
    }
    close(server_socket);
}


int incoming_connections(int server_socket, const int server_connections[MAX_CLIENTS]) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(server_socket, &read_fds);

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    int activity = select(server_socket + 1, &read_fds, NULL, NULL, &timeout);
    if (activity < 0) {
        perror("Error in select");
        exit(EXIT_FAILURE);
    } else if (activity > 0 && FD_ISSET(server_socket, &read_fds)) {
        // Incoming connection detected
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (server_connections[i] == -1) {
                return accept_client_connection(server_socket);
            }
        }
    }
    return -1; // No incoming connection
}