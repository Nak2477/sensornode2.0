#include "../include/sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

static int sensor_initialized = 0;

int Sensor_Init(void)
{
    if (sensor_initialized) {
        printf("Sensor already initialized\n");
        return 0;
    }
    printf("Initializing sensor system...\n");
    
    srand(time(NULL));
    
    sensor_initialized = 1;
    printf("Sensor system initialized\n");
    return 0;
}

Sensor_Data_t* Sensor_Read(void)
{
    if (!sensor_initialized) {
        printf("Sensor not initialized! Call Sensor_Init() first\n");
        return NULL;
    }
    printf("Reading sensors...\n");
    
    Sensor_Data_t* data = malloc(sizeof(Sensor_Data_t));
    if (!data) {
        printf("Failed to allocate memory for sensor data\n");
        return NULL;
    }

    data->temperature = Random_Temperature_Sensor();
    data->timestamp = Get_Current_Timestamp();
    data->sensor_id = DEFAULT_SENSOR_ID;
    
    printf("Sensor ID: %s Temperature: %.2f°C Timestamp: %s\n", data->sensor_id, data->temperature, data->timestamp);
    
    return data;
}

int Sensor_JSON(const Sensor_Data_t* data, char* json_buffer, int buffer_size)
{
    if (!data || !json_buffer) {
        printf("Invalid parameters for JSON builder\n");
        return -1;
    }
    int jsondata = snprintf(json_buffer, buffer_size,
                          "{\n"
                                "\"sensor_id\": \"%s\",\n"
                                "\"timestamp\": \"%s\",\n"
                                "\"temperature\": %.2f\n"
                          "}",
                          data->sensor_id ? data->sensor_id : "unknown",
                          data->timestamp ? data->timestamp : "unknown",
                          data->temperature);
    
    if (jsondata >= buffer_size) {
        printf("JSON buffer too small (%d needed, %d available)\n", jsondata, buffer_size);
        return -1;
    }
    
    printf("JSON built: %d characters\n", jsondata);
    return jsondata;
}

int Save_Sensor_Data_To_File(char* request)
{    
    FILE *file = fopen("bin/saved_temp.txt", "a");
    
    if (file == NULL) {
        printf("DEBUG: Failed to open file!\n");
        return -1;
    }
        
    fprintf(file, "%s\n", request);    
    fclose(file);
    
    return 0;
}
void Sensor_Free(Sensor_Data_t* data)
{
    if (data) {
        if (data->timestamp) {
            free(data->timestamp);
        }
        free(data);
        printf("🗑️  Sensor data memory freed\n");
    }
}

double Random_Temperature_Sensor(void)
{
    static double base_temperature = 23.0;
    static int initialized = 0;

    if(!initialized) {
        srand(time(NULL));
        initialized = 1;
    }

    double noise = ((double)rand() / RAND_MAX - 0.5) * 1.0;
    
    double drift = ((double)rand() / RAND_MAX - 0.5) * 0.1;
    base_temperature += drift;

    if (base_temperature < -15.0 || base_temperature > 35.0)
        base_temperature = 23.0;

    return base_temperature + noise;
}
char* Get_Current_Timestamp(void)
{
time_t now = time(NULL);
    char *timestamp = malloc(24);
    if (!timestamp) {
        return NULL;
    }
    
    struct tm *utc_tm = gmtime(&now);
        if (!utc_tm) {
        free(timestamp);
        return NULL;
    }
    strftime(timestamp, 24 - 1, "%Y-%m-%dT%H:%M:%SZ", utc_tm);
    
    return timestamp;
}

int parse_interval(int argc, char **argv) {
    int interval = 30; // Default interval in seconds
    
    // Parse command line arguments
    if (argc >= 3 && strcmp(argv[1], "--interval") == 0) {
        int user_interval = atoi(argv[2]);
        
        // Validate interval range (10-120 seconds)
        if (user_interval >= 10 && user_interval <= 120) {
            interval = user_interval;
            printf("✅ Using interval: %d seconds\n", interval);
        } else {
            printf("❌ Invalid interval %d. Must be between 10-120 seconds\n", user_interval);
            printf("Using default interval: %d seconds\n", interval);
        }
    } else if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        printf("Usage: %s [--interval <10-120>]\n", argv[0]);
        printf("Example: %s --interval 60\n", argv[0]);
        printf("Default interval: 30 seconds\n");
        exit(0);
    } else if (argc > 1) {
        printf("❌ Unknown arguments. Use --help for usage.\n");
        exit(1);
    }
    
    return interval;
    }
