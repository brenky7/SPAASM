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

#define BUFFER_SIZE 2048

int create_server_socket(const char *ip_address, int port) {
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
    server_addr.sin_addr.s_addr = inet_addr(ip_address);
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


struct RedirectArgs define_redirection(char *buffer, int pipefd[2]){
    struct RedirectArgs args1, args2, args;
    args1 = input_redirection_check(buffer, pipefd[0]);
    args2 = output_redirection_check(args1.buffer, pipefd[1]);
    args.pipe1 = args1.pipe1;
    args.pipe2 = args2.pipe2;
    args.buffer = args2.buffer;
    return args;
}


struct RedirectArgs output_redirection_check(char *buffer, int pipe2) {
    struct RedirectArgs execution_args;
    execution_args.pipe2 = pipe2;
    char* token = strtok(buffer, " ");
    char* output_buffer = malloc(BUFFER_SIZE * sizeof(char));
    while (token != NULL) {
        if (strcmp(token, ">") == 0) {
            // Open the file specified for input redirection
            execution_args.pipe2 = open(strtok(NULL, " "), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (execution_args.pipe2 == -1) {
                perror("Error opening file for input redirection");
                exit(EXIT_FAILURE);
            }
            break; // Exit loop after finding input redirection
        }
        else{
            strcat(output_buffer, token);
            strcat(output_buffer, " ");
        }
        token = strtok(NULL, " ");
    }
    execution_args.buffer = output_buffer;
    return execution_args;
}


struct RedirectArgs input_redirection_check(char *buffer, int pipe1) {
    struct RedirectArgs execution_args;
    execution_args.pipe1 = pipe1;
    char* token = strtok(buffer, " ");
    char* output_buffer = malloc(BUFFER_SIZE * sizeof(char));
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            // Open the file specified for input redirection
            execution_args.pipe1 = open(strtok(NULL, " "), O_RDONLY);
            if (execution_args.pipe1 == -1) {
                perror("Error opening file for input redirection");
                exit(EXIT_FAILURE);
            }
            break; // Exit loop after finding input redirection
        }
        else{
            strcat(output_buffer, token);
            strcat(output_buffer, " ");
        }
        token = strtok(NULL, " ");
    }
    execution_args.buffer = output_buffer;
    return execution_args;
}



void *handle_client(void *arg) {
    struct ThreadArgs *client_args = (struct ThreadArgs *)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    while (*client_args->server_running == 1) {
        memset(buffer, 0, sizeof(buffer));
        // Receive message from client
        if ((bytes_received = recv(client_args->client_socket, buffer, BUFFER_SIZE, 0)) == -1) {
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
        //printf("pipefd[0]: %d, pipefd[1]: %d\n", pipefd[0], pipefd[1]);
        struct RedirectArgs exec_args = define_redirection(buffer, pipefd);
        //printf("exec_args.pipe1: %d, exec_args.pipe2: %d\n", exec_args.pipe1, exec_args.pipe2);
        pid_t pid = fork();
        if (pid == -1) {
            perror("Error forking process");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) { // Child process
            // Redirect standard input if needed
            if (exec_args.pipe1 != STDIN_FILENO) {
                if (dup2(exec_args.pipe1, STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
                close(exec_args.pipe1); // Close pipe read end
            }
            // Redirect output if needed
            if (exec_args.pipe2 != STDOUT_FILENO) {
                if (dup2(exec_args.pipe2, STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
                close(exec_args.pipe2); // Close pipe write end
            }
            // Close remaining pipe ends
            close(pipefd[0]);
            close(pipefd[1]);
            // Parse command and arguments
            char *args[BUFFER_SIZE];
            char *token;
            token = strtok(exec_args.buffer, " ");
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
            // Wait for child process to finish
            int status;
            waitpid(pid, &status, 0);
            // Check if child process terminated normally
            if (WIFEXITED(status)) {
                printf("Command execution completed\n");
            } else {
                printf("Command execution failed\n");
            }

            // Read command output from pipe
            char output_buffer[BUFFER_SIZE];
            memset(output_buffer, 0, sizeof(output_buffer));
            ssize_t bytes_read;
            if (pipefd[1] == exec_args.pipe2) {
                bytes_read = read(pipefd[0], output_buffer, BUFFER_SIZE);
                send(client_args->client_socket, output_buffer, bytes_read, 0);
            }

            // Close read end of pipe
            close(exec_args.pipe1);
            // Close write end of pipe
            close(exec_args.pipe2);

            if (pipefd[1] != exec_args.pipe2) {
                bytes_read = read(pipefd[0], output_buffer, BUFFER_SIZE);
                write(exec_args.pipe2, output_buffer, bytes_read);
                send(client_args->client_socket, "Command executed and output redirected successfully.", 52, 0);
            }

        }
        printf("Press enter to continue");
    }
    const char *closing_message = "closing";
    if (send(client_args->client_socket, closing_message, strlen(closing_message), 0) == -1) {
        perror("Error sending message to client");
        close(client_args->client_socket);
        exit(EXIT_FAILURE);
    }
    printf("Client disconnected: %d\n", client_args->client_socket);
    remove_client_socket(client_args->client_socket, client_args->client_sockets, client_args->num_clients);
    // Close client socket
    close(client_args->client_socket);
    return NULL;
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
            if (*args->num_clients < MAX_CLIENTS) {
                *args->client_sockets[*args->num_clients] = client_socket; // Update client socket
                (*args->num_clients)++;
            } else {
                printf("Maximum number of clients reached. Ignoring new connection.\n");
                close(client_socket);
            }
            struct ThreadArgs *client_args = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
            client_args->client_socket = client_socket;
            client_args->server_running = args->server_running;
            client_args->num_clients = args->num_clients;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                client_args->client_sockets[i] = args->client_sockets[i];
            }
            pthread_t client_thread;
            if (pthread_create(&client_thread, NULL, handle_client, (void *)client_args) != 0) {
                perror("Error creating client thread");
                exit(EXIT_FAILURE);
            }
            pthread_detach(client_thread);
        }
        usleep(1000000);
    }
    return NULL;
}

void execute_command(char *command){
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("Error creating pipe");
        exit(EXIT_FAILURE);
    }
    struct RedirectArgs exec_args = define_redirection(command, pipefd);
    pid_t pid = fork();
    if (pid == -1) {
        perror("Error forking process");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // Child process
        // Redirect standard input if needed
        if (exec_args.pipe1 != STDIN_FILENO) {
            if (dup2(exec_args.pipe1, STDIN_FILENO) == -1) {
                perror("dup2");
                exit(EXIT_FAILURE);
            }
            close(exec_args.pipe1); // Close pipe read end
        }
        // Redirect output if needed
        if (exec_args.pipe2 != STDOUT_FILENO) {
            if (dup2(exec_args.pipe2, STDOUT_FILENO) == -1) {
                perror("dup2");
                exit(EXIT_FAILURE);
            }
            close(exec_args.pipe2); // Close pipe write end
        }
        // Close remaining pipe ends
        close(pipefd[0]);
        close(pipefd[1]);

        // Parse command and arguments
        char *args[BUFFER_SIZE];
        char *token;
        token = strtok(exec_args.buffer, " ");
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
    } else { // Parent process
        // Wait for child process to finish
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Command execution completed\n");
        } else {
            printf("Command execution failed\n");
        }
        // Read command output from pipe
        char output_buffer[BUFFER_SIZE];
        memset(output_buffer, 0, sizeof(output_buffer));
        ssize_t bytes_read;
        if (pipefd[1] == exec_args.pipe2) {
            bytes_read = read(pipefd[0], output_buffer, BUFFER_SIZE);
            printf("%s\n", output_buffer);
        }

        // Close read end of pipe
        close(exec_args.pipe1);
        // Close write end of pipe
        close(exec_args.pipe2);

        if (pipefd[1] != exec_args.pipe2) {
            bytes_read = read(pipefd[0], output_buffer, BUFFER_SIZE);
            write(exec_args.pipe2, output_buffer, bytes_read);
            printf("Command executed and output redirected successfully.\n");
        }

        close(pipefd[0]); // Close read end of pipe
        close(pipefd[1]); // Close write end of pipe
    }
}

void close_server_socket(int client_socket) {
    close(client_socket);
}

void remove_client_socket(int client_socket, int *client_sockets[], int *num_clients) {
    // Find the index of the client socket in the array
    int index = -1;
    for (int i = 0; i < *num_clients; i++) {
        if (*client_sockets[i] == client_socket) {
            index = i;
            break;
        }
    }
    if (index != -1) {
        // Shift elements to the left to fill the gap
        for (int i = index; i < *num_clients - 1; i++) {
            *client_sockets[i] = *client_sockets[i + 1];
        }
        // Decrement the number of clients
        (*num_clients)--;
    }
}
