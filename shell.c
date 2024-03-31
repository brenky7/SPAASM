#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "my_libs/client_lib.h"
#include "my_libs/server_lib.h"

//ssh -oKexAlgorithms=+diffie-hellman-group1-sha1 -oHostKeyAlgorithms=+ssh-dss -c aes128-cbc xbrenkus@student.fiit.stuba.sk
typedef enum {
    false,
    true
} bool;

void displayHelp() {
    printf("------ Usage: ./shell [-s | -c] [-p port] ------\n");
    printf("Author: Peter Brenkus, xbrenkus@stuba.sk\n");
    printf("Options:\n");
    printf("  -s\tRun as server\n");
    printf("  -c\tRun as client\n");
    printf("  -p\tPort number\n");
    printf("  -h\tDisplay help\n");
    printf("Or alternatively run the program without any arguments for basic I/O mode.\n");
    printf("------ Supported commands ------\n\thelp\n\texit\n\tport\n");
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
    int port = 0;
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
                   if (port == 0) {
                       printf("Invalid port number... use 'port command to set'.\n");
                   } else {
                       printf("Port number: %d\n", port);
                   }
               }
               else{
                     printf("No port number specified... use 'port' command to set.\n");
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
                    port = atoi(token);
                    printf("Port number set to %d\n", port);
                } else {
                    printf("Port number not specified, current: %d\n", port);
                }
            }
            else if (strcmp(token, "server") == 0) {
                server = true;
                printf("Running in server mode.\n");
                printf("Port number: %d, use 'port' command to set.\n", port);
            }
            else if (strcmp(token, "help") == 0) {
                displayHelp();
            }
            else if (strcmp(token, "exit") == 0) {
                printf("Shutting down.\n");
                break;
            }
            else {
                printf("Unknown command: %s\n", token);
            }
        }
    }

    return 0;
}