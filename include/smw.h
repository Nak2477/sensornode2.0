#ifndef SMW_H
#define SMW_H

#include "../include/tcp.h"
#include "../include/http.h"
#include "../include/sensor.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#define SERVER_HOST "httpbin.org"
#define SERVER_PORT 80

#define BUFFER_SIZE 1024
#define BUFFER_JSON_SIZE 256
#define MAX_RESPONSE_SIZE 128


typedef enum {
    TASK_INITIALIZE,
    TASK_READ_SENSOR,
    TASK_CONNECT,
    TASK_POST_REQUEST,
    TASK_SAVE_DATA,
    TASK_RECEIVE_RESPONSE,
    TASK_RETRY,
    TASK_CLEANUP,
    TASK_DONE,
    TASK_FAILED,
} task_state_t;

typedef struct {
    task_state_t state;
    int sockfd;
    Sensor_Data_t* sensor_data;
    char json_buffer[BUFFER_JSON_SIZE];
    char* http_request;
    char http_response[MAX_RESPONSE_SIZE];
    int attempt_count;
    int result_code;
} task_context_t;

typedef struct {
    void* context;
    void (*callback)(void* context, uint64_t monTime);
    int active;
} smw_task_t;

smw_task_t* Create_Smw_Task(void* ctx, void (*cb)(void*, uint64_t));
void Execute_Smw_Task(smw_task_t* task, uint64_t timestamp);
void Free_Smw_Task(smw_task_t* task);


void Sensor_State_Machine(void* context, uint64_t monTime);

#endif