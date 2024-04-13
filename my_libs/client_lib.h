#ifndef CLIENT_LIB_H
#define CLIENT_LIB_H

#include "utils.h"

// Function prototypes
int create_client_socket(int port);
void connect_to_server(int client_socket, int server_port);
void send_commands(int client_socket, const char *message);
void *client_listener(void *arg);
void close_client_socket(int client_socket);
#endif
