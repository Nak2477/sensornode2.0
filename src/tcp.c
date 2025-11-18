#include "../include/tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>


    int Tcp_Init(const char* hostname, int port)
    {
        struct hostent *host = gethostbyname(hostname);
        if (host == NULL)
        {
        printf("gethostbyname() Failed\n");
        return -1;
        }

        struct sockaddr_in server_addr = {0};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr = *((struct in_addr*)host->h_addr_list[0]);

        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            printf("Socket() failed\n");
            return -1;
        }

        int status = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
        
        if (status == -1) {
            printf("Connect() failed\n");
            close(sockfd);
            return -1;
        } else {
            printf("Connected to %s:%d\n", hostname, port);
            return sockfd;
        }
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
        } else {
        printf("Sent %d bytes\n", bytes_sent);
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



//recv()