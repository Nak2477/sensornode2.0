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
    
    Sensor_Data_t* SensorData_t = malloc(sizeof(Sensor_Data_t));
    if (!SensorData_t) {
        printf("Failed to allocate memory for sensor data\n");
        return NULL;
    }

    SensorData_t->temperature = Random_Temperature_Sensor();
    SensorData_t->timestamp = Get_Current_Timestamp();
    SensorData_t->sensor_id = DEFAULT_SENSOR_ID;
    
    printf("Sensor ID: %s Temperature: %.2f°C Timestamp: %s\n", SensorData_t->sensor_id, SensorData_t->temperature, SensorData_t->timestamp);
    
    return SensorData_t;
}

int Sensor_JSON(const Sensor_Data_t* SensorData_t, char* json_buffer, int buffer_size)
{
    if (!SensorData_t || !json_buffer) {
        printf("Invalid parameters for JSON builder\n");
        return -1;
    }
    int jsondata = snprintf(json_buffer, buffer_size,
                          "{\n"
                                "\"sensor_id\": \"%s\",\n"
                                "\"timestamp\": \"%s\",\n"
                                "\"temperature\": %.2f\n"
                          "}",
                          SensorData_t->sensor_id ? SensorData_t->sensor_id : "unknown",
                          SensorData_t->timestamp ? SensorData_t->timestamp : "unknown",
                          SensorData_t->temperature);
    
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

int Send_Saved_Sensor_Data(char* buffer)
{
    FILE *file = fopen("bin/saved_temp.txt", "r");
    if (file == NULL) {
        printf("No saved sensor data to send.\n");
        return 1;  // No file = no data
    }
    
    // Check if file is empty
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    if (size == 0) {
        printf("\"bin/saved_temp\" file is empty\n");
        fclose(file);
        return 1;  // Empty file
    }
    
    // Reset to beginning for reading
    fseek(file, 0, SEEK_SET);
    
    char line[256];
    
    // Find and read first JSON object
    if (fgets(line, sizeof(line), file) && line[0] == '{') {
        strcpy(buffer, line);
        
        do {
            if (!fgets(line, sizeof(line), file)) {
                break;
            }
            strcat(buffer, line);
        } while (line[0] != '}');

        char* brace_pos = strchr(buffer, '}');
        if (brace_pos) {
            *(brace_pos + 1) = '\0';  // Null terminate after }
        }
        
        fclose(file);
        printf("📋 Retrieved JSON object from file\n");
        return 0;  // Success - object found and copied to buffer
    }
    
    fclose(file);
    return 1;  // No JSON objects found
}
        //NOT USED YET 
int Read_Complete_Saved_File(char** buffer, size_t* file_size)
{
    if (!buffer) {
        printf("Invalid buffer pointer provided.\n");
        return -1;
    }
    
    *buffer = NULL;  // Initialize to NULL
    if (file_size) *file_size = 0;
    
    FILE *file = fopen("bin/saved_temp.txt", "r");
    if (!file) {
        printf("No saved sensor data file found.\n");
        return 1;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (size <= 0) {
        printf("Saved data file is empty or invalid.\n");
        fclose(file);
        return 1; 
    }
    
    // Allocate buffer for entire file content + null terminator
    *buffer = malloc(size + 1);
    if (!*buffer) {
        printf("Failed to allocate memory for file content (%ld bytes).\n", size);
        fclose(file);
        return -1; 
    }
    
    // Read entire file in one operation
    size_t bytes_read = fread(*buffer, 1, size, file);
    (*buffer)[bytes_read] = '\0';  // Null terminate
    
    fclose(file);
    
    if (file_size) *file_size = bytes_read;
    printf("📋 Read complete file: %zu bytes\n", bytes_read);
    
    return 0;  // Success
}

int Remove_Sent_Object_From_File(const char* sent_object)
{
    FILE *file = fopen("bin/saved_temp.txt", "r");
    FILE *temp = fopen("bin/temp_backup.txt", "w");
    
    if (!file || !temp) {
        if (file) fclose(file);
        if (temp) fclose(temp);
        return -1;
    }
    
    char buffer[512];
    char line[256];
    int objects_kept = 0;
    
    // Copy all objects except the sent one to temp file
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '{') {
            strcpy(buffer, line);
            
            do {
                if (!fgets(line, sizeof(line), file)) break;
                strcat(buffer, line);
            } while (line[0] != '}');
            
            char* brace_pos = strchr(buffer, '}');
            if (brace_pos) {
                *(brace_pos + 1) = '\0';
            }
            
            // If this object doesn't match the sent one, keep it
            if (strcmp(buffer, sent_object) != 0) {
                fprintf(temp, "%s\n", buffer);
                objects_kept++;
            }
        }
    }
    
    fclose(file);
    fclose(temp);
    
    // Replace original with temp file
    if (rename("bin/temp_backup.txt", "bin/saved_temp.txt") == 0) {
        printf("Removed sent object from file (%d objects remaining)\n", objects_kept);
        return 0;
    } else {
        printf("❌ Failed to update saved data file\n");
        return -1;
    }
}

void Sensor_Free(Sensor_Data_t* SensorData_t)
{
    if (SensorData_t) {
        // No need to free timestamp - it's now a static buffer
        free(SensorData_t);
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
    static char timestamp[24]; // Static buffer - no malloc needed
    time_t now = time(NULL);
    
    struct tm *utc_tm = gmtime(&now);
    if (!utc_tm) {
        return NULL;
    }
    
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", utc_tm);
    
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
