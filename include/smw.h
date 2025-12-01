#ifndef SMW_H
#define SMW_H

#define _POSIX_C_SOURCE 200112L

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
    STATE_INITIALIZE,
    STATE_READ_SENSOR,
    STATE_HTTP_TRANSACTION,    // Combined: connect + POST + receive
    STATE_SAVE_DATA,
    STATE_RETRY,
    STATE_PROCESS_SAVED_DATA,  // Combined: complete saved data lifecycle
    STATE_DONE,                // Combined: cleanup + completion
    STATE_FAILED,
} task_state_t;


typedef struct {
    task_state_t state;
    int sockfd;
    Sensor_Data_t* sensor_data;
    char json_buffer[BUFFER_JSON_SIZE];
    char fromfile_buffer[BUFFER_JSON_SIZE];   
    char* http_request;
    char http_response[MAX_RESPONSE_SIZE];
    int attempt_count;
    int result_code;

    // Timing fields
    uint64_t last_read_time;      // When the last measurement was taken (ms)
    int measurement_interval;     // How often to take measurements (seconds)

} task_context_t;


typedef struct {
    void* context;
    void (*callback)(void* context, uint64_t monTime);
    int active;
} smw_task_t;

smw_task_t* Create_Smw_Task(void* ctx, void (*cb)(void*, uint64_t));
void Execute_Smw_Task(smw_task_t* task, uint64_t timestamp);
void Free_Smw_Task(smw_task_t* task);


void Sensor_State_Machine(task_context_t* context, uint64_t monTime);

#endif