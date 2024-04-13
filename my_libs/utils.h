#ifndef MY_STRUCTS_H
#define MY_STRUCTS_H
#include <stdio.h>

// Constants
#define MAX_CLIENTS 10

// Function prototypes
void displayHelp(void);
void getCurrentTime(char *timeString);
void getHostname(char *hostname, int size);
char* getPrompt(void);
char* process_hash(char* command);
long get_file_size(FILE *file);

// Struct for thread arguments
struct ThreadArgs {
    int server_socket;
    int client_socket;
    int *server_running;
    int *client_running;
    int *num_clients;
    int *client_sockets[MAX_CLIENTS];
};

// Struct for redirect arguments
struct RedirectArgs {
    char *buffer;
    int pipe1;
    int pipe2;
};

#endif
