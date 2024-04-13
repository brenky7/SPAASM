#ifndef MY_STRUCTS_H
#define MY_STRUCTS_H
#include <stdbool.h>
#include <stdio.h>

#define MAX_CLIENTS 10

void displayHelp(void);
void getCurrentTime(char *timeString);
void getHostname(char *hostname, int size);
char* getPrompt(void);
bool contains_hash(const char *str);
long get_file_size(FILE *file);


struct ThreadArgs {
    int server_socket;
    int client_socket;
    int *server_running;
    int *client_running;
    int *num_clients;
    int *client_sockets[MAX_CLIENTS];
};

struct RedirectArgs {
    char *buffer;
    int pipe1;
    int pipe2;
};

#endif
