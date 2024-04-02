#ifndef SERVER_LIB_H
#define SERVER_LIB_H
#define MAX_CLIENTS 10

int create_server_socket(int port);
void start_listening(int server_socket);
int accept_client_connection(int server_socket);
long receive_message(int client_socket, char *buffer, int buffer_size);
int send_message(int client_socket, const char *message);
void close_server_socket(int socket, int server_connections[MAX_CLIENTS]);
int incoming_connections(int server_socket, const int server_connections[MAX_CLIENTS]);
#endif
