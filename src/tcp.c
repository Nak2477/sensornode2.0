#define _POSIX_C_SOURCE 200112L
#include "../include/tcp.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

int Tcp_Init(const char* hostname, int port)
{
    struct addrinfo hints = {0};
    struct addrinfo* res = NULL;
    char port_str[10];

    memset(&hints, 0, sizeof(hints));
    snprintf(port_str, sizeof(port_str), "%d", port);

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(hostname, port_str, &hints, &res) != 0) {
        printf("getaddrinfo() failed\n");
        return -1;
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        printf("socket() failed\n");
        freeaddrinfo(res);
        return -1;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        printf("connect() failed\n");
        close(sockfd);
        freeaddrinfo(res);
        return -1;
    }

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        printf("setsockopt() failed\n");
        close(sockfd);
        freeaddrinfo(res);
        return -1;
    }

    printf("Connected to %s:%d\n", hostname, port);
    freeaddrinfo(res);
    return sockfd;
}

int Tcp_Send(int sockfd, const char* data, int length)
{
    if (sockfd < 0 || !data)
    {
        return -1;
    }

    int bytes_sent = send(sockfd, data, length, 0);
    if (bytes_sent < 0)
    {
        printf("Send() failed\n");
        return -1;

    } else
    {
    printf("Send() %d bytes:\n%s\n", bytes_sent, data);
    return bytes_sent;
    }
}

int Tcp_Recv(int sockfd, char* buffer, int buffer_size)
{
    if (sockfd < 0 || !buffer) {
    return -1;
    }

    int bytes_received = recv(sockfd, buffer, buffer_size -1, 0);
    if (bytes_received < 0) {
        printf("recv() failed\n");
        return -1;
    }

    buffer[bytes_received] = '\0';
    printf("Recv() %d bytes\n", bytes_received);
     
    return bytes_received;
}

void Tcp_Close(int sockfd)
{
    if (sockfd > 0) {
        close(sockfd);
        printf("connection close() fd: %d\n", sockfd);
    }
}