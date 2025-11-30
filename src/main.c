#define _DEFAULT_SOURCE
#include "../include/smw.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

typedef struct {
    int success_count;
    int failure_count;
} sensor_stats_t;

void on_sensor_success(void* user_data) {
    sensor_stats_t* stats = (sensor_stats_t*)user_data;
    stats->success_count++;
    printf("✅ Sensor cycle #%d completed successfully!\n", stats->success_count);
}

void on_sensor_failure(int error_code, void* user_data) {
    sensor_stats_t* stats = (sensor_stats_t*)user_data;
    stats->failure_count++;
    printf("❌ Sensor failure #%d - Error code: %d\n", stats->failure_count, error_code);
}

int main(int argc, char **argv) {
    int measurment_interval = parse_interval(argc, argv);
    sensor_stats_t stats = {0, 0};

    while (1)
    { 
        task_context_t ctx = {0};
        ctx.state = STATE_INITIALIZE;
        ctx.sockfd = -1;

        ctx.on_success = on_sensor_success;
        ctx.on_failure = on_sensor_failure;
        ctx.user_data = &stats;
        
        smw_task_t* sensor_task = Create_Smw_Task(&ctx, (void (*)(void*, uint64_t))Sensor_State_Machine);
                

        do {
            Execute_Smw_Task(sensor_task, time(NULL));
            //usleep(500000);  // 500ms delay between state transitions
        } while (ctx.state != STATE_DONE && ctx.state != STATE_FAILED);
        
        // Execute final state (STATE_DONE or STATE_FAILED) to trigger callbacks
        Execute_Smw_Task(sensor_task, time(NULL));            
            Free_Smw_Task(sensor_task);
            printf("sleeping for %d seconds before next measurement...\n", measurment_interval);
            sleep(measurment_interval);
    }
    
    return 0;
}