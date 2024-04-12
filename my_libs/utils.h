#ifndef MY_STRUCTS_H
#define MY_STRUCTS_H
#include <stdbool.h>

struct ThreadArgs {
    int server_socket;
    int client_socket;
    int *server_running;
    int *client_running;
};

struct RedirectArgs {
    char *buffer;
    int pipe1;
    int pipe2;
};

void displayHelp();
void getCurrentTime(char *timeString);
void getHostname(char *hostname, int size);
char* getPrompt();
bool contains_hash(const char *str);
long get_file_size(FILE *file);

#endif /* MY_STRUCTS_H */