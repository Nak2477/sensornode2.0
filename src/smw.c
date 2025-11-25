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

void Sensor_State_Machine(void* context, uint64_t monTime)
{
    (void)monTime; // Mark as unused to suppress warning

    task_context_t* ctx = (task_context_t*)context;

    switch(ctx->state)
    {
        case STATE_INITIALIZE:
            if (Sensor_Init() == 0) {
                ctx->state = STATE_READ_SENSOR;
            } else {
                ctx->state = STATE_FAILED;
            }
        break;
        
        case STATE_READ_SENSOR:
            ctx->sensor_data = Sensor_Read();
            if (ctx->sensor_data) {
                int json_len = Sensor_JSON(ctx->sensor_data, ctx->json_buffer, sizeof(ctx->json_buffer));
                if (json_len > 0) {
                    ctx->state = STATE_HTTP_TRANSACTION;
                } else {
                    ctx->state = STATE_FAILED;
                    ctx->result_code = -2;
                }
            } else {
                ctx->state = STATE_FAILED;
                ctx->result_code = -2;
            }
        break;

        case STATE_HTTP_TRANSACTION:

            ctx->sockfd = Tcp_Init(SERVER_HOST, SERVER_PORT);
            if (ctx->sockfd < 0) {
                ctx->state = STATE_RETRY;
                ctx->result_code = -3;
                break;
            }
            
            ctx->http_request = build_http_request(METHOD_POST, SERVER_HOST, ctx->json_buffer);
            if (!ctx->http_request) {
                ctx->state = STATE_FAILED;
                ctx->result_code = -6;
                break;
            }
            
            if (Tcp_Send(ctx->sockfd, ctx->http_request, strlen(ctx->http_request)) < 0) {
                ctx->state = STATE_SAVE_DATA;
                ctx->result_code = -5;
                break;
            }
            
            int bytes_received = Tcp_Recv(ctx->sockfd, ctx->http_response, sizeof(ctx->http_response) - 1);
            if (bytes_received > 0) {
                ctx->http_response[bytes_received] = '\0';
                Print_HTTP_Status(ctx->http_response);
                // Check if response indicates success (status code 200-299)
                if (strstr(ctx->http_response, "HTTP/1.1 2") != NULL) {
                    ctx->result_code = 0;
                    // If we just sent saved data, go back to process more
                    if (strlen(ctx->fromfile_buffer) > 0) {
                        ctx->state = STATE_PROCESS_SAVED_DATA;
                    } else {
                        // Fresh data was sent successfully - check for saved data
                        ctx->state = STATE_PROCESS_SAVED_DATA;
                    }
                } else {
                    ctx->state = STATE_SAVE_DATA; // Save data if server rejected it
                    ctx->result_code = -7;
                }
            } else {
                ctx->state = STATE_SAVE_DATA; // Save data if no response
                ctx->result_code = -8;
            }
        break;

        case STATE_RETRY:
            ctx->attempt_count++;
            if (ctx->attempt_count < 3) {
                ctx->state = STATE_HTTP_TRANSACTION;
            } else {
                ctx->state = STATE_SAVE_DATA; // Save data locally after max attempts
                ctx->result_code = -4;
                ctx->attempt_count = 0;
            }
        break;

        case STATE_SAVE_DATA:
            Save_Sensor_Data_To_File(ctx->json_buffer);
            ctx->state = STATE_DONE;
        break;

        case STATE_PROCESS_SAVED_DATA:
            // If buffer has data, it was just sent successfully - remove it
            if (strlen(ctx->fromfile_buffer) > 0) {
                if (Remove_Sent_Object_From_File(ctx->fromfile_buffer) == 0) {
                    printf("✅ Removed sent data from file\n");
                } else {
                    printf("⚠️  Failed to remove sent data\n");
                }
                memset(ctx->fromfile_buffer, 0, sizeof(ctx->fromfile_buffer));
            }
            
            // Try to get next saved data to send
            if (Send_Saved_Sensor_Data(ctx->fromfile_buffer) == 0) {
                // Found more saved data - send it
                strcpy(ctx->json_buffer, ctx->fromfile_buffer);
                ctx->state = STATE_HTTP_TRANSACTION;
            } else {
                // No more saved data to process
                printf("📭d No saved data to send\n");
                ctx->state = STATE_DONE;
            }
        break;

        case STATE_DONE:
            // Cleanup resources and complete
            if (ctx->sockfd >= 0) {
                close(ctx->sockfd);
                ctx->sockfd = -1;
            }
            if (ctx->http_request) {
                free(ctx->http_request);
                ctx->http_request = NULL;
            }
            if (ctx->sensor_data) {
                Sensor_Free(ctx->sensor_data);
                ctx->sensor_data = NULL;
            }
            
            // Trigger success callback
            if (ctx->on_success) {
                ctx->on_success(ctx->user_data);
            } else {
                printf("Sensor task completed successfully\n");
            }
        break;
        
        case STATE_FAILED:
            if (ctx->on_failure)
            {
                ctx->on_failure(ctx->result_code, ctx->user_data);  // ← Call failure callback
            } else {
                printf("Sensor task failed with code: %d\n", ctx->result_code);
            }
        break;
    }
}
