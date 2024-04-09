#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include "server_lib.h"
#include "utils.h"

#define BUFFER_SIZE 1024

int create_server_socket(int port) {
    int server_socket;
    struct sockaddr_in server_addr;

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating server socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind socket to address and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding server socket");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Error listening on server socket");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);

    return server_socket;
}


int accept_client_connection(int server_socket) {
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Accept incoming connection
    if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return -1;
        } else {
            perror("Error accepting connection");
            exit(EXIT_FAILURE);
        }
    }

    printf("\nConnection accepted from %s:%d, press enter to continue.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    return client_socket;
}


void handle_client(int client_socket, const int *server_running) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    while (*server_running == 1) {
        memset(buffer, 0, sizeof(buffer));
        // Receive message from client
        if ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No data available, retry after a short delay
                usleep(1000000); // 1 second
                continue;
            } else {
                perror("Error receiving message from client");
                exit(EXIT_FAILURE);
            }
        }

        if (bytes_received == 0) {
            // Connection closed by client
            printf("\nClient closed the connection, press enter to continue\n");
            break;
        }

        buffer[bytes_received] = '\0'; // Null-terminate the received data
        printf("\nReceived message from client: %s, press enter to continue.\n", buffer);

        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("Error creating pipe");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("Error forking process");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) { // Child process
            // Redirect standard output to pipe write end
            close(pipefd[0]); // Close unused read end
            dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
            close(pipefd[1]); // Close pipe write end
            // Parse command and arguments
            char *args[BUFFER_SIZE];
            char *token;
            token = strtok(buffer, " ");
            int i = 0;
            while (token != NULL) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;
            // Execute the command
            if (execvp(args[0], args) == -1) {
                perror("Error executing command");
                exit(EXIT_FAILURE);
            }
        }
        else { // Parent process
            // Close unused write end of pipe
            close(pipefd[1]);

            // Read command output from pipe
            char output_buffer[BUFFER_SIZE];
            ssize_t bytes_read;
            memset(output_buffer, 0, sizeof(output_buffer));
            while ((bytes_read = read(pipefd[0], output_buffer, BUFFER_SIZE)) > 0) {
                // Send output back to client
                send(client_socket, output_buffer, bytes_read, 0);
                memset(output_buffer, 0, sizeof(output_buffer));
            }

            // Close read end of pipe
            close(pipefd[0]);

            // Wait for child process to finish
            int status;
            waitpid(pid, &status, 0);

            // Check if child process terminated normally
            if (WIFEXITED(status)) {
                printf("Command execution completed\n");
            } else {
                printf("Command execution failed\n");
            }
        }
        printf("Press enter to continue");
    }
    const char *closing_message = "closing";
    if (send(client_socket, closing_message, strlen(closing_message), 0) == -1) {
        perror("Error sending message to client");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    // Close client socket
    close(client_socket);
}


void *accept_connections(void *arg) {
    struct ThreadArgs *args = (struct ThreadArgs *)arg;
    int flags = fcntl(args->server_socket, F_GETFL, 0);
    if (flags == -1) {
        perror("Error getting socket flags");
        exit(EXIT_FAILURE);
    }

    if (fcntl(args->server_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("Error setting socket to non-blocking mode");
        exit(EXIT_FAILURE);
    }
    while (*args->server_running == 1) {
        int client_socket = accept_client_connection(args->server_socket);
        if (client_socket != -1) {
            handle_client(client_socket, args->server_running);
        }
        usleep(1000000);
    }
    return NULL;
}

void close_server_socket(int client_socket) {
    close(client_socket);
}