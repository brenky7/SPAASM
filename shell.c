#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "my_libs/client_lib.h"
#include "my_libs/server_lib.h"
#include "my_libs/utils.h"

//ssh -oKexAlgorithms=+diffie-hellman-group1-sha1 -oHostKeyAlgorithms=+ssh-dss -c aes128-cbc xbrenkus@student.fiit.stuba.sk

int sock = -1;
pthread_t thread;

int main(int argc, char *argv[]) {
    int port = -1;
    bool server = false;
    bool client = false;
    int *server_running = NULL;
    server_running = (int *)malloc(sizeof(int));
    *server_running = 0;
    int *client_running = NULL;
    client_running = (int *)malloc(sizeof(int));
    *client_running = 0;
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-s") == 0) {
                server = true;
            } else if (strcmp(argv[i], "-c") == 0) {
                client = true;
            } else if (strcmp(argv[i], "-p") == 0) {
               if (i + 1 < argc) {
                   port = atoi(argv[i + 1]);
                   if (port < 0) {
                       printf("Invalid port number... use 'port command to change'.\n");
                   } else {
                       printf("Port number: %d\n", port);
                   }
               }
               else{
                     printf("No port number specified... use 'port' command to change.\n");
               }
            } else if (strcmp(argv[i], "-h") == 0) {
                displayHelp();
            }
        }
    }
    if (server == true && client == true) {
        printf("Cannot run in both server and client mode.\n");
        exit(EXIT_FAILURE);
    }
    else if (server == false && client == true) {
        sock = create_client_socket(port);
        printf("Running in client mode.\n");
    }
    else if ((client == false && server == true) || (client == false && server == false)) {
        if (port == -1) {
            port = 60000;
            printf("Port number not specified. Using default port: %d\n", port);
        }
        sock = create_server_socket(port);
        printf("Server socket: %d\n", sock);
        printf("Running in server mode.\n");
        *server_running = 1;
        struct ThreadArgs args;
        args.server_socket = sock;
        args.server_running = server_running;
        if (pthread_create(&thread, NULL, accept_connections, (void *)&args) != 0) {
            perror("Error creating accept thread");
            exit(EXIT_FAILURE);
        }
    }

    char command[100];
    while (1) {

        char *prompt = getPrompt();
        printf("%s", prompt);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';
        char *token = strtok(command, " ");
        if (token != NULL) {
            if (strcmp(token, "help") == 0) {
                displayHelp();
            }
            else if (strcmp(token, "exit") == 0) {
                printf("Shutting down.\n");
                break;
            }
            else if (strcmp(token, "connect") == 0) {
                if (client == true) {
                    char *port_str = strtok(NULL, " ");
                    if (port_str != NULL) {
                        int server_port = atoi(port_str);
                        printf("Server port: %d\n", server_port);
                        if (server_port <= 0) {
                            printf("Error: Invalid port number.\n");
                            printf("Usage: connect <server_port>\n");
                        } else {
                            connect_to_server(sock, server_port);
                            printf("Connected to server\n");
                            *client_running = 1;
                            struct ThreadArgs args;
                            args.client_socket = sock;
                            args.client_running = client_running;
                            if (pthread_create(&thread, NULL, client_listener, (void *) &args) != 0) {
                                perror("Error creating client thread");
                                exit(EXIT_FAILURE);
                            }
                        }
                    } else {
                        printf("Error: No server port specified.\n");
                        printf("Usage: connect <server_port>\n");
                    }
                }
            }
            else if (strcmp(token, "send") == 0) {
                if (client == true) {
                    char *message = strtok(NULL, "");
                    char *delimiter = ";";
                    char* command2 = strtok(message, delimiter);
                    while (command2 != NULL) {
                        send_commands(sock, command2);
                        usleep(1500000); // Sleep for 1 second
                        command2 = strtok(NULL, delimiter);
                    }
                } else {
                    printf("Error: Not in client mode. Use '-c' command to change.\n");
                }
            }
            else if (strcmp(token, "quit") == 0) {
                if (client == true) {
                    *client_running = 0; // Signal client thread to stop
                    if (pthread_join(thread, NULL) != 0) {
                        perror("Error joining client thread");
                        exit(EXIT_FAILURE);
                    }
                    printf("Disconnected from server\n");
                    close_client_socket(sock);
                    printf("Client socket closed\n");
                    printf("Shutting down.\n");
                    break;
                } else {
                    printf("Error: Not in client mode. Use '-c' to run in client mode.\n");
                }
            }
            else if (strcmp(token, "halt") == 0) {
                if (server == true) {
                    *server_running = 0;
                    usleep(2000000); // Wait for server to stop
                    if (pthread_join(thread, NULL) != 0) {
                        perror("Error joining thread");
                        exit(EXIT_FAILURE);
                    }
                    close_server_socket(sock);
                    printf("Server socket closed and all connections terminated\n");
                    exit(EXIT_SUCCESS);
                } else {
                    printf("Error: Not in server mode. Use '-s' to run in server mode.\n");
                }
            }
            else if (strcmp(token, "") == 0) {
                continue;
            }
            else {
                printf("Unknown command: %s\n", token);
            }
        }
    }

    return 0;
}