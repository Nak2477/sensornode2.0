#ifndef SENSOR_H
#define SENSOR_H

#include <time.h>

// Sensor configuration
#define DEFAULT_SENSOR_ID "sensornode_001"
#define SENSOR_READ_INTERVAL 60  // seconds

// Sensor data structure
typedef struct {
    double temperature;
    char* timestamp;
    const char* sensor_id;
} Sensor_Data_t;

double Random_Temperature_Sensor(void);
char* Get_Current_Timestamp(void);

int Sensor_Init(void);
Sensor_Data_t* Sensor_Read(void);

int Sensor_JSON(const Sensor_Data_t* data, char* json_buffer, int buffer_size);
int Save_Sensor_Data_To_File(char* request);

void Sensor_Free(Sensor_Data_t* data);

int parse_interval(int argc, char **argv);
#endif