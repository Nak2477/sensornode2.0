#include "../include/smw.h"

smw_task_t* Create_Smw_Task(void* ctx, void (*cb)(void*, uint64_t))
{
    smw_task_t* task = malloc(sizeof(smw_task_t));
    if (!task) return NULL;

    task->context = ctx;
    task->callback = cb;
    task->active = 1;

    printf("Created Task with function pointer\n");
    return task;
}

void Execute_Smw_Task(smw_task_t* task, uint64_t timestamp)
{
    if (!task || !task->active || !task->callback) 
    return;

    task->callback(task->context, timestamp);
}

void Free_Smw_Task(smw_task_t* task)
{
    if (task)
    {
        printf("Freeing Task resources\n");
        free(task);
    }
}

void Sensor_State_Machine(void* context, uint64_t monTime) {
    (void)monTime; // Mark as unused to suppress warning

    task_context_t* ctx = (task_context_t*)context;

    switch(ctx->state)
    {
        case TASK_INITIALIZE:

            if (Sensor_Init() == 0) {
                ctx->state = TASK_READ_SENSOR;
            } else {
                ctx->state = TASK_FAILED;
                ctx->result_code = -1;
            }
        break;
        
        case TASK_READ_SENSOR:
            ctx->sensor_data = Sensor_Read();
            if (ctx->sensor_data)
            {
                int json_len = Sensor_JSON(ctx->sensor_data, ctx->json_buffer, sizeof(ctx->json_buffer));
                if (json_len > 0)
                {
                    ctx->state = TASK_CONNECT;
                }
            } else {
                ctx->state = TASK_FAILED;
                ctx->result_code = -2;
            }
        break;

        case TASK_CONNECT:

            ctx->sockfd = Tcp_Init(SERVER_HOST, SERVER_PORT);
            if (ctx->sockfd >= 0)
            {
                ctx->state = TASK_POST_REQUEST;
            } else {
                ctx->state = TASK_RETRY;
                ctx->result_code = -3;
            }
        break;

        case TASK_RETRY:
            ctx->attempt_count++;
            if (ctx->attempt_count < 3) {
                ctx->state = TASK_CONNECT;
            } else {
                ctx->state = TASK_SAVE_DATA; // Save data locally after max attempts
                ctx->result_code = -4;
                ctx->attempt_count = 0;
            }
        break;

        case TASK_POST_REQUEST:

            ctx->http_request = build_http_request(METHOD_POST, SERVER_HOST, ctx->json_buffer);
            if (ctx->http_request)
            {   
                if(Tcp_Send(ctx->sockfd, ctx->http_request, strlen(ctx->http_request)) >= 0)
                {
                    ctx->state = TASK_RECEIVE_RESPONSE;
                } else {
                    ctx->state = TASK_SAVE_DATA;
                    ctx->result_code = -5;
                }
            } else {
                ctx->state = TASK_FAILED;
                ctx->result_code = -6;
            }
        break;

        case TASK_SAVE_DATA:
            Save_Sensor_Data_To_File(ctx->json_buffer);
            ctx->state = TASK_CLEANUP;
        break;

    case TASK_RECEIVE_RESPONSE:
        {
            int bytes_received = Tcp_Recv(ctx->sockfd, ctx->http_response, sizeof(ctx->http_response) - 1);
            if (bytes_received > 0) {
                ctx->http_response[bytes_received] = '\0';
                Print_HTTP_Status(ctx->http_response);
                // Check if response indicates success (status code 200-299)
                if (strstr(ctx->http_response, "HTTP/1.1 2") != NULL) {
                    ctx->state = TASK_CLEANUP;
                    ctx->result_code = 0; // Success
                } else {
                    ctx->state = TASK_SAVE_DATA; // Save data if server rejected it
                    ctx->result_code = -7;
                }
            } else {
                ctx->state = TASK_SAVE_DATA; // Save data if no response
                ctx->result_code = -8;
            }
        }
    break;

    case TASK_CLEANUP:
        // Close socket if open
        if (ctx->sockfd >= 0) {
            close(ctx->sockfd);
            ctx->sockfd = -1;
        }
        // Free HTTP request memory if allocated
        if (ctx->http_request) {
            free(ctx->http_request);
            ctx->http_request = NULL;
        }
        // Free sensor data if allocated
        if (ctx->sensor_data) {
            free(ctx->sensor_data);
            ctx->sensor_data = NULL;
        }
        ctx->state = TASK_DONE;
    break;

    case TASK_DONE:
        // Task completed successfully, could trigger callback or set flag
        printf("Sensor task completed successfully\n");
    break;
    
    case TASK_FAILED:
        printf("Sensor task failed with code: %d\n", ctx->result_code);
        ctx->state = TASK_CLEANUP; // Clean up resources even on failure
    break;
    }


}
