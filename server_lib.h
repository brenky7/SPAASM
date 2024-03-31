#ifndef SERVER_LIB_H
#define SERVER_LIB_H

int create_server_socket(int port);
int accept_client_connection(int server_socket);
int receive_message(int client_socket, char *buffer, int buffer_size);
int send_message(int client_socket, const char *message);
void close_socket(int socket_fd);

#endif
