#ifndef TCP_H
#define TCP_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


int Tcp_Init(const char* hostname, int port);
int Tcp_Send(int sockfd, const char* data, int length);
int Tcp_Recv(int sockfd, char* buffer, int buffer_size);
void Tcp_Close(int sockfd);

#endif // TCP_H