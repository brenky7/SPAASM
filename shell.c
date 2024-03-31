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
    printf("Help - Display help information\n");
    printf("Exit - Exit the shell\n");
}

void getCurrentTime(char *timeString) {
    time_t currentTime;
    struct tm *localTime;
    time(&currentTime);
    localTime = localtime(&currentTime);
    strftime(timeString, 9, "%H:%M", localTime); // Format: HH:MM
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
                port = atoi(argv[i + 1]);
                if (port == 0) {
                    printf("Invalid port number... shutting down.\n");
                    exit(EXIT_FAILURE);
                }
                else{
                    printf("Port number: %d\n", port);
                }
            } else if (strcmp(argv[i], "-h") == 0) {
                printf("Usage: %s [-s | -c] [-p port]\n", argv[0]);
                printf("Author: Peter Brenkus, xbrenkus@stuba.sk\n");
                printf("Options:\n");
                printf("  -s\tRun as server\n");
                printf("  -c\tRun as client\n");
                printf("  -p\tPort number\n");
                printf("  -h\tDisplay help\n");
                printf("Or alternatively run the program without any arguments for basic I/O mode.\n");
            }
        }
    }
    if (!server && !client){
        printf("No command-line arguments... basic mode.\n");
    }


    char command[100];
    while (1) {
        char *prompt = getPrompt();
        printf("%s", prompt);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "help") == 0) {
            displayHelp();
        } else if (strcmp(command, "exit") == 0) {
            printf("Exiting the shell.\n");
            break;
        } else {
            printf("Unknown command: %s\n", command);
        }
    }

    return 0;
}