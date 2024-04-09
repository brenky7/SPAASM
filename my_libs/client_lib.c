#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include "client_lib.h"
#include "utils.h"

#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

int create_client_socket(int port) {
    int client_socket;

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating client socket");
        exit(EXIT_FAILURE);
    }

    // Bind socket to specific port (optional for client sockets)
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY; // Bind to any available interface
    client_addr.sin_port = htons(port);

    if (bind(client_socket, (struct sockaddr *)&client_addr, sizeof(client_addr)) == -1) {
        perror("Error binding client socket");
        exit(EXIT_FAILURE);
    }

    return client_socket;
}

void connect_to_server(int client_socket, int server_port) {
    struct sockaddr_in server_addr;

    // Initialize server address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(server_port);

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

}

void send_commands(int client_socket, const char *message) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    // Send message to server
    if (send(client_socket, message, strlen(message), 0) == -1) {
        perror("Error sending message to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
}

void *client_listener(void *arg) {
    struct ThreadArgs *args = (struct ThreadArgs *)arg;
    int flags = fcntl(args->client_socket, F_GETFL, 0);
    if (fcntl(args->client_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("Error setting socket to non-blocking mode");
        exit(EXIT_FAILURE);
    }
    while (*(args->client_running) == 1) {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_received;

        // Receive message from server
        if ((bytes_received = recv(args->client_socket, buffer, BUFFER_SIZE, 0)) == -1) {
            if (args->client_running == 0){
                break;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No data available, sleep for a short while
                usleep(1000000); // 1 second
                continue;
            } else {
                perror("Error receiving message from server");
                close(args->client_socket);
                exit(EXIT_FAILURE);
            }
        }

        if (strcmp(buffer, "closing") == 0) {
            printf("\nServer sent a halt signal. Shutting down.\n");
            close(args->client_socket);
            exit(EXIT_SUCCESS);
        }

        if (bytes_received > 0) {
            buffer[bytes_received] = '\0'; // Null-terminate the received data
            printf("\n%s\n", buffer);
        }
    }
    return NULL;
}

void close_client_socket(int client_socket) {
    close(client_socket);
}
