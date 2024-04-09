#ifndef MY_STRUCTS_H
#define MY_STRUCTS_H


struct ThreadArgs {
    int server_socket;
    int client_socket;
    int *server_running;
    int *client_running;
};

void displayHelp();
void getCurrentTime(char *timeString);
void getHostname(char *hostname, int size);
char* getPrompt();

#endif /* MY_STRUCTS_H */