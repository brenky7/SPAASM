#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "my_libs/client_lib.h"
#include "my_libs/server_lib.h"

//ssh -oKexAlgorithms=+diffie-hellman-group1-sha1 -oHostKeyAlgorithms=+ssh-dss -c aes128-cbc xbrenkus@student.fiit.stuba.sk
typedef enum {
    false,
    true
} bool;

int sock = -1;
int server_connections[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
pthread_t thread;
pthread_mutex_t connection_mutex = PTHREAD_MUTEX_INITIALIZER;

void *incoming_connections_handler(void *arg) {
    while (1) {
        pthread_mutex_lock(&connection_mutex);
        int new_client_socket = incoming_connections(sock, server_connections);
        pthread_mutex_unlock(&connection_mutex);

        if (new_client_socket != -1) {
            printf("Incoming connection detected. [accept/close]: ");
            char response[10];
            fgets(response, sizeof(response), stdin);
            response[strcspn(response, "\n")] = '\0';
            char *token = strtok(response, " ");
            if (strcmp(token, "accept") == 0) {
                pthread_mutex_lock(&connection_mutex);
                for (int i = 0; i < 10; i++) {
                    if (server_connections[i] == -1) {
                        server_connections[i] = new_client_socket;
                        break;
                    }
                }
                pthread_mutex_unlock(&connection_mutex);
                printf("Accepted incoming connection from client\n");
            } else {
                // Close the connection
                close(new_client_socket);
                printf("Rejected incoming connection from client\n");
            }
        }
    }
}


void displayHelp() {
    printf("---------- Author: Peter Brenkus, xbrenkus@stuba.sk ---------\n");
    printf("----------- Usage: ./shell [-s | -c] [-p <port>] ------------\n\n");
    printf("Options:\n");
    printf("  -s\tRun as server\n");
    printf("  -c\tRun as client\n");
    printf("  -p\tPort number\n");
    printf("  -h\tDisplay help\n");
    printf("Or alternatively run the program without any arguments for basic I/O mode.\n");
    printf("\n-------------------- Supported commands ---------------------\n\n");
    printf("  [help]\t\t\t\tDisplay help\n");
    printf("  [server]\t\t\t\tRun in server mode\n");
    printf("  [client]\t\t\t\tRun in client mode\n");
    printf("  [port <port_number>]\t\t\tChange port number\n");
    printf("  [sock]\t\t\t\tCreate sock\n");
    printf("  [listen]\t\t\t\tStart listening for incoming connections\n");
    printf("  [connect <server_ip> <server_port>]\tConnect to server\n");
    printf("  [quit]\t\t\t\tDisconnect from server\n");
    printf("  [halt]\t\t\t\tClose all connections and shutdown [in server mode]\n");
    printf("  [exit]\t\t\t\tExit the program\n");
}

void getCurrentTime(char *timeString) {
    time_t currentTime;
    struct tm *localTime;
    time(&currentTime);
    localTime = localtime(&currentTime);
    strftime(timeString, 9, "%H:%M", localTime);
}

void getHostname(char *hostname, int size) {
    if (gethostname(hostname, size) != 0) {
        perror("Error getting hostname");
        exit(EXIT_FAILURE);
    }
}

char* getPrompt() {
    char *username = getenv("USER");

    char hostname[1024];
    getHostname(hostname, sizeof(hostname));

    char timeString[9];
    getCurrentTime(timeString);

    char terminationCharacter = '%';
    char *prompt = (char *) malloc(256 * sizeof(char));

    snprintf(prompt, 1024, "%s %s@%s %c ", timeString, username, hostname,  terminationCharacter);
    return prompt;
}


int main(int argc, char *argv[]) {

    int port = -1;
    bool server = false;
    bool client = false;

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
        printf("Running in client mode.\n");
    }
    else if (server == true && client == false) {
        printf("Running in server mode.\n");
    }
    else {
        printf("No mode specified... running basic mode.\n");
    }

    char command[100];
    while (1) {

        char *prompt = getPrompt();
        printf("%s", prompt);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';
        char *token = strtok(command, " ");
        if (token != NULL) {
            if (strcmp(token, "port") == 0) {
                token = strtok(NULL, " ");
                if (token != NULL) {
                    if (atoi(token) < 0) {
                        printf("Invalid port number.\n");
                    } else {
                        port = atoi(token);
                        printf("Port number set to %d\n", port);
                    }
                } else {
                    printf("Port number not specified, current: %d\n", port);
                }
            }
            else if (strcmp(token, "server") == 0) {
                server = true;
                client = false;
                printf("Running in server mode.\n");
                printf("Port number: %d, use 'port' command to change.\n", port);
            }
            else if (strcmp(token, "client") == 0) {
                client = true;
                server = false;
                printf("Running in client mode.\n");
                printf("Port number: %d, use 'port' command to change.\n", port);
            }
            else if (strcmp(token, "help") == 0) {
                displayHelp();
            }
            else if (strcmp(token, "exit") == 0) {
                printf("Shutting down.\n");
                break;
            }
            else if (strcmp(token, "socket") == 0) {
                if (server == true) {
                    pthread_mutex_lock(&connection_mutex);
                    sock = create_server_socket(port);
                    pthread_mutex_unlock(&connection_mutex);
                    printf("Server socket created.\n");
                }
                else if (client == true) {
                    sock = create_client_socket();
                    printf("Client socket created.\n");
                } else {
                    printf("Error: Mode not specified. Use 'server' or 'client' command to change.\n");
                }
            }
            else if (strcmp(token, "listen") == 0) {
                if (server == true) {
                    start_listening(sock);
                    printf("Server socket listening for incoming connections.\n");
                    if (pthread_create(&thread, NULL, incoming_connections_handler, NULL) != 0) {
                        perror("Error creating thread");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    printf("Error: Not in server mode. Use 'server' command to change.\n");
                }
            }
            else if (strcmp(token, "connect") == 0) {
                if (client == true) {
                    char *server_ip = strtok(NULL, " ");
                    int server_port = atoi(strtok(NULL, " "));
                    if (server_ip == NULL || server_port < 0) {
                        printf("Error: Invalid server IP or port number.\n");
                        printf("Usage: connect <server_ip> <server_port>\n");
                    } else {
                        connect_to(sock, server_ip, server_port);
                        printf("Connected to server\n");
                    }
                } else {
                    printf("Error: Not in client mode. Use 'client' command to change.\n");
                }
            }
            else if (strcmp(token, "quit") == 0) {
                if (client == true) {
                    close_client_socket(sock);
                    printf("Disconnected from server\n");
                } else {
                    printf("Error: Not in client mode. Use '-c' to run in client mode.\n");
                }
            }
            else if (strcmp(token, "halt") == 0) {
                if (server == true) {
                    if (pthread_join(thread, NULL) != 0) {
                        perror("Error joining thread");
                        exit(EXIT_FAILURE);
                    }
                    pthread_mutex_lock(&connection_mutex);
                    close_server_socket(sock, server_connections);
                    pthread_mutex_unlock(&connection_mutex);
                    printf("Server socket closed and all connections terminated\n");
                } else {
                    printf("Error: Not in server mode. Use '-s' to run in server mode.\n");
                }
            }

            else {
                printf("Unknown command: %s\n", token);
            }
        }
    }

    return 0;
}