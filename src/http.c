#include "../include/http.h"


char* build_http_request(const char *path, const char *hostname, const char *body)
{
    char *request = malloc(BUFFER_SIZE +1);
    if (!request) {
        return NULL;
    }
    if (!body) {
        body = "";
    }
    int content_length = strlen(body);
    int total_size = BUFFER_SIZE;

        snprintf(request, total_size,
                 "POST %s HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "Content-Type: application/json\r\n"        
                 "Content-Length: %d\r\n"                   
                 "User-Agent: SensorNode2.0/1.0\r\n"         
                 "Connection: close\r\n"                      
                 "\r\n"
                 "%s",
                 path, hostname, content_length, body);

    return request;
}

void Print_HTTP_Status(const char* response)
{
    if (!response) return;

    // Make a copy since strtok modifies the string
    char temp_buffer[256];
    strncpy(temp_buffer, response, sizeof(temp_buffer) - 1);
    temp_buffer[sizeof(temp_buffer) - 1] = '\0';

    char* first_line = strtok(temp_buffer, "\r\n");
    if (first_line) {
        printf("HTTP Status: %s\n", first_line);
    }
}