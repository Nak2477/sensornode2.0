#define _DEFAULT_SOURCE
#include "../include/smw.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>



int main(int argc, char **argv) {
    int measurment_interval = parse_interval(argc, argv);
    
    task_context_t ctx = {0};
    ctx.state = STATE_INITIALIZE;
    ctx.sockfd = -1;
    ctx.measurement_interval = measurment_interval;
    ctx.last_read_time = 0;  // Force first read immediately

    while (1)
    { 
        smw_task_t* sensor_task = Create_Smw_Task(&ctx, (void (*)(void*, uint64_t))Sensor_State_Machine);
        
        do
        {
            Execute_Smw_Task(sensor_task, time(NULL));
        } while (ctx.state != STATE_DONE && ctx.state != STATE_FAILED);
        
        Free_Smw_Task(sensor_task);
        
        // Reset state for next cycle
        ctx.state = STATE_INITIALIZE;
        ctx.sockfd = -1;
    }
    
    return 0;
}