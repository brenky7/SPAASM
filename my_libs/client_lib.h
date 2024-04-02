

#ifndef CLIENT_LIB_H
#define CLIENT_LIB_H

//int openSocket(const char *serverIP, int port);
int create_client_socket();
void connect_to(int client_socket, const char *serverIP, int port);
void sendMessage(int server_socket, const char *message);
void receiveMessage(int server_socket, char *buffer, int bufferSize);
void close_client_socket(int client_socket);

#endif