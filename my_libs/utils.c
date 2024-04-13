#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include "utils.h"


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
    printf("  [port <port_number>]\t\t\tChange port number\n");
    printf("  [connect <server_ip> <server_port>]\tConnect to server\n");
    printf("  [send <command>]\t\t\tSend command to server\n");
    printf("  [execfile <file_name>]\t\t\tExecute commands from file\n");
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

bool contains_hash(const char *str) {
    while (*str != '\0') {
        if (*str == '#' && *(str -1) != '\\') {
            return true; // Found the '#' character
        }
        str++; // Move to the next character
    }
    return false; // '#' character not found
}

long get_file_size(FILE *file) {
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    return size;
}
