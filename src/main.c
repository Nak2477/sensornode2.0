#define _DEFAULT_SOURCE
#include "../include/smw.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char **argv) {
    int measurment_interval = parse_interval(argc, argv);

    while (1)
    { 
        task_context_t ctx = {0};
        ctx.state = TASK_INITIALIZE;
        ctx.sockfd = -1;
        
        smw_task_t* sensor_task = Create_Smw_Task(&ctx, Sensor_State_Machine);
                

        while (ctx.state != TASK_DONE && ctx.state != TASK_FAILED)
        {
                Execute_Smw_Task(sensor_task, time(NULL));
        }            
            Free_Smw_Task(sensor_task);
            sleep(measurment_interval);
    }
    
    return 0;
}