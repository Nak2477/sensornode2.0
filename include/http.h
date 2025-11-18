#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>

#define SERVER_HOST "httpbin.org"
#define SERVER_PORT 80
#define METHOD_POST "/post"


#define BUFFER_SIZE 1024
#define MAX_RESPONSE_SIZE 128

char* build_http_request(const char *path, const char *hostname, const char *body);
void Print_HTTP_Status(const char* response);

#endif //HTTP_H

