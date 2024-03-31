

#ifndef CLIENT_LIB_H
#define CLIENT_LIB_H

int openSocket(const char *serverIP, int port);
void sendMessage(int sockfd, const char *message);
void receiveMessage(int sockfd, char *buffer, int bufferSize);
void closeSocket(int sockfd);

#endif