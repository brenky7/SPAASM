#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "my_libs/client_lib.h"
#include "my_libs/server_lib.h"
#include "my_libs/utils.h"

// Global variables
int sock = -1; // Program instance socket descriptor
pthread_t thread; // Thread for server/client
volatile sig_atomic_t ctrl_c_received = 0; // Signal flag for handling SIGINT

// Signal handler function for Ctrl+C
void ctrl_c_handler(int signum) {
    ctrl_c_received = 1;
}

// Main function, entry point of the program
int main(int argc, char *argv[]) {
    // Initialize variables
    int port = -1;
    int *client_sockets = (int *)malloc(MAX_CLIENTS * sizeof(int));
    int *num_clients = (int *)malloc(sizeof(int));
    *num_clients = 0;
    char *ip_adress = "";
    bool server = false;
    bool client = false;
    int *server_running = NULL;
    server_running = (int *)malloc(sizeof(int));
    *server_running = 0;
    int *client_running = NULL;
    client_running = (int *)malloc(sizeof(int));
    *client_running = 0;
    signal(SIGINT, ctrl_c_handler);
    // Parse command line arguments
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            // Check for command line options
            if (strcmp(argv[i], "-s") == 0) {   // Server mode
                server = true;
            } else if (strcmp(argv[i], "-c") == 0) {    // Client mode
                client = true;
            } else if (strcmp(argv[i], "-p") == 0) {    // Port number
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
            } else if (strcmp(argv[i], "-h") == 0) {    // Display help
                displayHelp();
            } else if (strcmp(argv[i], "-i") == 0){ // IP address of server
                if (i + 1 < argc) {
                    ip_adress = argv[i + 1];
                }
            }
        }
    }
    if (server == true && client == true) { // Check if both -s and -c were specified
        printf("Cannot run in both server and client mode.\n");
        exit(EXIT_FAILURE);
    }
    else if (server == false && client == true) {   // Check if client mode was specified
        sock = create_client_socket(port);
        printf("Running in client mode.\n");
    }
    // If server mode was specified
    else if ((client == false && server == true) || (client == false && server == false)) {
        if (port == -1) {   // Check if port number was specified
            port = 60000;
            printf("Port number not specified. Using default port: %d\n", port);
        }
        sock = create_server_socket(ip_adress, port);
        printf("Server socket: %d\n", sock);
        printf("Running in server mode.\n");
        *server_running = 1;
        struct ThreadArgs args;
        args.server_socket = sock;
        args.server_running = server_running;
        args.num_clients = num_clients;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            args.client_sockets[i] = &client_sockets[i];
        }
        // Create a thread for accepting connections
        if (pthread_create(&thread, NULL, accept_connections, (void *)&args) != 0) {
            perror("Error creating accept thread");
            exit(EXIT_FAILURE);
        }
    }
    //Main loop, read commands from stdin
    char command[100];
    while (1) {
        if (ctrl_c_received) {
            printf("Ctrl+C received. Shutting down ...\n");
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
            }
            else {
                *client_running = 0; // Signal client thread to stop
                if (pthread_join(thread, NULL) != 0) {
                    perror("Error joining client thread");
                    exit(EXIT_FAILURE);
                }
                printf("Disconnected from server\n");
                close_client_socket(sock);
                printf("Client socket closed\n");
                printf("Shutting down.\n");
            }
            break;
        }
        char *prompt = getPrompt();
        printf("%s", prompt);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';
        char *token = strtok(command, " "); // Tokenize the command based on whitespace
        if (token != NULL) {
            if (strcmp(token, "help") == 0) {   // Display help
                displayHelp();
            }
            else if (strcmp(token, "exit") == 0) {  // Exit the program
                printf("Shutting down.\n");
                break;
            }
            else if (strcmp(token, "connect") == 0) {   // Connect to server
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
            else if (strcmp(token, "send") == 0) {  // Send commands to server
                if (client == true) {
                    char *message = strtok(NULL, "");
                    if (message != NULL) {
                        char *command2 = strtok(message, ";"); // Tokenize individual commands by semicolon
                        while (command2 != NULL) {
                            // Process and send each command to the server
                            //char *processed_command = process_hash(command2);
                            printf("Sending command: %s\n", command2);
                            send_commands(sock, command2);
                            usleep(3000000); // Sleep for 3 seconds (adjust as needed)
                            command2 = strtok(NULL, ";"); // Move to the next command
                        }
                    } else {
                        printf("Error: No message specified.\n");
                    }
                } else {
                    printf("Error: Not in client mode. Use '-c' command to change.\n");
                }
            }
            else if ((strcmp(token, "execfile") == 0)){ // Execute commands from file
                if (server == true) {
                    char *filename = strtok(NULL, " ");
                    if (filename != NULL) {
                        FILE *file = fopen(filename, "r");
                        if (file == NULL) {
                            printf("Error: Could not open file.\n");
                        } else {
                            // Allocate memory for the buffer
                            long file_size = get_file_size(file);
                            char *file_buffer = (char *)malloc(file_size + 1);
                            if (file_buffer == NULL) {
                                perror("Error allocating memory for file buffer");
                                exit(EXIT_FAILURE);
                            }
                            // Read the entire file into the buffer
                            fread(file_buffer, 1, file_size, file);
                            fclose(file);
                            printf("File contents: %s\n", file_buffer);
                            // Null-terminate the buffer
                            file_buffer[file_size] = '\0';
                            // Tokenize the buffer based on ";"
                            char *delimiter = ";";
                            char *file_buffer_copy = strdup(file_buffer); // Make a copy to preserve original
                            char *command2 = strtok(file_buffer_copy, delimiter);
                            char *commands[100];
                            for (int i = 0; i < 100; i++) {
                                commands[i] = NULL;
                            }
                            int i = 0;
                            while (command2 != NULL){
                                commands[i] = command2;
                                printf("command: %s\n", commands[i]);
                                command2 = strtok(NULL, delimiter);
                                i++;
                            }
                            i = 0;
                            while (commands[i] != NULL) {
                                // Ignore empty tokens
                                printf("Command: %s\n", commands[i]);
                                commands[i] = process_hash(commands[i]);
                                printf("Processed command: %s\n", commands[i]);
                                execute_command(commands[i]);
                                usleep(2000000); // Sleep for 3 seconds
                                i++;
                            }
                            // Free allocated memory for the copy
                            free(file_buffer_copy);
                            free (file_buffer);
                        }
                    } else {
                        printf("Error: No filename specified.\n");
                        printf("Usage: execfile <filename>\n");
                    }
                } else {
                    printf("Error: Not in server mode. Use '-s' to run in server mode.\n");

                }
            }
            else if (strcmp(token, "quit") == 0) {  // Disconnect from server
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
            else if (strcmp(token, "halt") == 0) {  // Close all connections and shutdown
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
            else if (strcmp(token, "bonus") == 0) {
                printf("Bonusove ulohy:\n");
                printf("Uloha 1: Neinteraktivny rezim (2b)\n");
                printf("Uloha 3: Interny prikaz stat (3b)\n");
                printf("Uloha 11: IP servera cez -i (2b)\n");
                printf("Uloha 17: Prilinkovane externe kniznice (2b)\n");
                printf("Uloha 27: Zachytenie signalu ctrl+c (??b)\n");
                printf("Uloha 28: Funkcny makefile (2b)\n");
                printf("Uloha 29: Konfiguracny skript (2b)\n");
                printf("Uloha 30: Komentare v anglictine (1b)\n");
            }
            else if (strcmp(token, "") == 0) {  // Empty command
                continue;
            }
            else if (strcmp(token, "stat") == 0){   // Display active connections
                if (server == true) {
                    printf("Number of active connections: %d\n", *num_clients);
                    for (int i = 0; i < *num_clients; ++i) {
                        printf("Client %d: Socket %d\n", i + 1, client_sockets[i]);
                    }
                } else {
                    printf("Error: Not in server mode. Use '-s' to run in server mode.\n");
                }
            }
            else {  // Unknown command
                printf("Unknown command: %s\n", token);
            }
        }
    }

    return 0;
}
