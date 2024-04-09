#ifndef SERVER_LIB_H
#define SERVER_LIB_H

#include <pthread.h>

#define MAX_CLIENTS 10

int create_server_socket(int port);
int accept_client_connection(int server_socket);
void handle_client(int client_socket, const int *server_running);
void *accept_connections(void *arg);
void close_server_socket(int client_socket);
#endif
