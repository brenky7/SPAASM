#ifndef SERVER_LIB_H
#define SERVER_LIB_H

#include <pthread.h>
#include "utils.h"


int create_server_socket(const char *ip_address, int port) ;
int accept_client_connection(int server_socket);
struct RedirectArgs define_redirection(char *buffer, int pipefd[2]);
struct RedirectArgs output_redirection_check(char *buffer, int pipe2);
struct RedirectArgs input_redirection_check(char *buffer, int pipe1);
void *handle_client(void *arg);
void *accept_connections(void *arg);
void execute_command(char *command);
void close_server_socket(int client_socket);
void remove_client_socket(int client_socket, int *client_sockets[], int *num_clients);
#endif
