#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "utils.h"

// Function to display help
void displayHelp(void) {
    printf("------------------------------ FIIT STU -------------------------------\n\n");
    printf("--------------- Author: Peter Brenkus, xbrenkus@stuba.sk --------------\n\n");
    printf("----------- Run: ./configure [-s | -c] [-p <port>] [-i <ip>]-----------\n\n");
    printf("---Run: make -f Makefile.mk; ./shell [-s | -c] [-p <port>] [-i <ip>]---\n\n");
    printf("[Note: -i only for server mode]\n");
    printf("------------------------- Supported commands --------------------------\n\n");
    printf("  [help]\t\t\tDisplay help\n");
    printf("  [connect <server_port>]\tConnect to server\n");
    printf("  [send <command>]\t\t\tSend command to server\n");
    printf("  [execfile <file_name>]\t\t\tExecute commands from file\n");
    printf("  [stat]\t\t\tDisplay active connections [server mode]\n");
    printf("  [quit]\t\t\tDisconnect from server\n");
    printf("  [halt]\t\t\tClose all connections and shutdown [in server mode]\n");
    printf("  [exit]\t\t\tExit the program\n");
    printf("  [bonus]\t\t\tDisplay optional bonuses from assignment\n");
}

// Function to get current time
void getCurrentTime(char *timeString) {
    time_t currentTime;
    struct tm *localTime;
    time(&currentTime);
    localTime = localtime(&currentTime);
    strftime(timeString, 9, "%H:%M", localTime);
}

// Function to get hostname
void getHostname(char *hostname, int size) {
    if (gethostname(hostname, size) != 0) {
        perror("Error getting hostname");
        exit(EXIT_FAILURE);
    }
}

// Function to create prompt
char* getPrompt(void) {
    // Get user
    char *username = getenv("USER");
    // Get host
    char hostname[1024];
    getHostname(hostname, sizeof(hostname));
    // Get time
    char timeString[9];
    getCurrentTime(timeString);
    // Create prompt
    char terminationCharacter = '%';
    char *prompt = (char *) malloc(256 * sizeof(char));

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(prompt, (unsigned int)1024, "%s %s@%s %c ", timeString, username, hostname, terminationCharacter);
    #pragma GCC diagnostic pop
    return prompt;
}

// Function to process the command for # character
char* process_hash(char* command) {
    // Allocate memory for the result
    char* result = (char*)malloc(strlen(command) + 1);
    if (result == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    strcpy(result, ""); // Initialize result as empty string

    // Tokenize the command based on whitespace
    char* token = strtok(command, " ");
    while (token != NULL) {
        // Check if the token contains '#'
        if (strchr(token, '#') != NULL) {
            break; // Stop processing if '#' is found
        } else {
            // Append the token to the result
            strcat(result, token);
            strcat(result, " ");
        }
        token = strtok(NULL, " ");
    }

    // Remove trailing whitespace
    if (strlen(result) > 0) {
        result[strlen(result) - 1] = '\0';
    }

    return result;
}

// Function to get file size
long get_file_size(FILE *file) {
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    return size;
}
