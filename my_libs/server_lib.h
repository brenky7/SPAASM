#ifndef SERVER_LIB_H
#define SERVER_LIB_H

#include <pthread.h>

#define MAX_CLIENTS 10

int create_server_socket(int port);
int accept_client_connection(int server_socket);
void *handle_client(void *arg);
void *accept_connections(void *arg);
void close_server_socket(int client_socket);
#endif
