# 🌡️ SensorNode 2.0

**Smart Sensor Node** - A robust C-based IoT sensor system that reads temperature data, transmits to cloud servers via HTTP, and provides local backup storage with intelligent offline data recovery.

## 📋 Overview

SensorNode 2.0 is a complete embedded systems project implementing a sensor-to-cloud data pipeline featuring:
- Real-time temperature monitoring with simulated sensor data
- HTTP POST communication to cloud servers (httpbin.org)
- Intelligent local data persistence during network outages
- State machine architecture with function pointers and callbacks
- Configurable measurement intervals (10-120 seconds)
- Automatic saved data transmission when connectivity resumes
- Comprehensive error handling and retry logic

## 🚀 Features

### Core Functionality
- ✅ **Temperature Sensing**: Realistic temperature simulation with drift patterns
- ✅ **Cloud Communication**: HTTP POST requests with JSON payloads to REST APIs
- ✅ **Offline Resilience**: Automatic data backup during network failures
- ✅ **Smart Recovery**: Automatic transmission of saved data when online
- ✅ **Error Recovery**: 3-retry logic with graceful fallback to offline mode
- ✅ **Configurable Timing**: Command-line interval control (10-120 seconds)

### Technical Implementation
- ✅ **State Machine**: 12-state pipeline with clean separation of concerns
- ✅ **Function Pointers**: Callback-driven task execution with smw_task_t
- ✅ **Pointer-to-Pointer**: Advanced memory management (char **argv parsing)
- ✅ **TCP/IP Networking**: Custom socket implementation for HTTP communication
- ✅ **JSON Serialization**: Standards-compliant sensor data formatting
- ✅ **Dual Buffer System**: Separate buffers for live data vs saved data processing
- ✅ **Memory Safety**: Leak-free allocation/deallocation patterns

## 🛠️ Requirements

### System Requirements
- **OS**: Linux (Ubuntu 20.04+, Debian, CentOS)
- **Compiler**: GCC with C99 support
- **Network**: Internet connection for cloud communication
- **Storage**: Minimal disk space for local data backup

### Dependencies
- Standard C libraries (stdio, stdlib, string, time)
- POSIX networking (sys/socket, netinet, netdb)
- No external dependencies required

## 📦 Installation & Build

### Clone and Build
```bash
git clone <repository-url>
cd sensornode2.0
make clean && make
```

### Build Targets
```bash
make all        # Standard build (default)
make debug      # Debug version with extra logging
make release    # Optimized production build
make clean      # Clean build artifacts
```

## 🔧 Usage

### Basic Usage
```bash
# Default 30-second intervals
./build/sensornode2.0

# Custom interval (10-120 seconds)
./build/sensornode2.0 --interval 60

# Show help
./build/sensornode2.0 --help
```

### Configuration
**Note: Configuration file parsing is not yet implemented (Story 4)**

The `bin/config.txt` file exists with settings format:
```ini
# Measurement settings
measurement_interval=60
device_id=node001

# Network settings
server_host=httpbin.org
server_port=80
connection_timeout=5

# Storage settings
backup_file=bin/saved_temp.txt

# Sensor settings  
temperature_min=-15.0
temperature_max=35.0
```

**Current behavior**: Server host and port are hardcoded in `include/http.h`

# Sensor settings  
temperature_min=-15.0
temperature_max=35.0
```

**Current behavior**: Server host and port are hardcoded in `include/http.h`

### Example Output

#### Normal Operation
```
✅ Using interval: 10 seconds
Created Task with function pointer
Initializing sensor system...
Sensor system initialized
Reading sensors...
Sensor ID: sensornode_001 Temperature: 23.36°C Timestamp: 2025-11-19T02:06:42Z
JSON built: 92 characters
Connected to httpbin.org:80
Send() 236 bytes:
POST /post HTTP/1.1
Host: httpbin.org
Content-Type: application/json
Content-Length: 92
User-Agent: SensorNode2.0/1.0
Connection: close

{
"sensor_id": "sensornode_001",
"timestamp": "2025-11-19T02:07:34Z",
"temperature": 22.68
}
Recv() 126 bytes
HTTP Status: HTTP/1.1 200 OK
Freeing Task resources
sleeping for 10 seconds before next measurement...
```

#### Network Failure Recovery
```
Reading sensors...
Sensor ID: sensornode_001 Temperature: 22.15°C Timestamp: 2025-11-19T02:07:15Z
JSON built: 92 characters
Connection to httpbin.org:80 failed (retry 1/3)
Connection to httpbin.org:80 failed (retry 2/3)  
Connection to httpbin.org:80 failed (retry 3/3)
All connection attempts failed. Saving data locally.
Data saved to bin/saved_temp.txt
Freeing Task resources

[Network reconnects...]

"bin/saved_temp" file contains saved data
Send_Saved_Sensor_Data() returned: 1
Connected to httpbin.org:80
[HTTP transmission...]
HTTP Status: HTTP/1.1 200 OK
Removed sent object from file (0 objects remaining)
"bin/saved_temp" file is empty
```

## 🏗️ Architecture

### Project Structure
```
sensornode2.0/
├── src/
│   ├── main.c          # Program entry point & main loop
│   ├── sensor.c        # Temperature reading & JSON formatting  
│   ├── tcp.c           # TCP socket communication
│   ├── http.c          # HTTP request building & parsing
│   └── smw.c           # State machine & task management
├── include/
│   ├── sensor.h        # Sensor data structures & functions
│   ├── tcp.h           # Network communication interface
│   ├── http.h          # HTTP protocol definitions
│   └── smw.h           # State machine & task definitions
├── bin/
│   ├── config.txt      # Configuration parameters
│   └── saved_temp.txt  # Local backup storage
├── build/              # Compiled executable
├── obj/                # Object files
└── Makefile           # Build system
```

### State Machine Flow
```
INITIALIZE → READ_SENSOR → CONNECT → POST_REQUEST → RECEIVE_RESPONSE → CLEANUP → DONE
     ↓              ↓           ↓            ↓              ↓             ↓
   FAILED        FAILED      RETRY(3)      SAVE_DATA      SAVE_DATA      SEND_SAVED_DATA
                                ↓            ↓              ↓             ↓
                             SAVE_DATA     CLEANUP        CLEANUP    REMOVE_SENT_DATA
                                                                          ↓
                                                                        CLEANUP
```

### Complete State Definitions
```c
typedef enum {
    STATE_INITIALIZE,      // Initialize sensor system
    STATE_READ_SENSOR,     // Read temperature and format JSON
    STATE_CONNECT,         // Establish TCP connection
    STATE_POST_REQUEST,    // Send HTTP POST with sensor data
    STATE_SAVE_DATA,       // Save data locally on network failure
    STATE_RECEIVE_RESPONSE,// Parse HTTP response
    STATE_RETRY,          // Retry failed network operations
    STATE_SEND_SAVED_DATA,// Retrieve and send previously saved data
    STATE_REMOVE_SENT_DATA,// Remove successfully transmitted saved data
    STATE_CLEANUP,        // Clean resources and prepare for next cycle
    STATE_DONE,           // Successful completion
    STATE_FAILED          // Terminal failure state
} smw_state_t;
```

### Data Flow Architecture
```
Fresh Sensor Reading:
Temperature → JSON → json_buffer → HTTP POST → Cloud Server

Network Failure Path:
Temperature → JSON → json_buffer → Save to bin/saved_temp.txt

Recovery Path:
bin/saved_temp.txt → fromfile_buffer → json_buffer → HTTP POST → Remove from file
```

## 🔍 Technical Details

### Function Pointers Implementation
```c
typedef struct {
    void* context;
    void (*callback)(void* context, uint64_t timestamp);
    int active;
} smw_task_t;

// Usage
smw_task_t* task = Create_Smw_Task(&ctx, Sensor_State_Machine);
Execute_Smw_Task(task, time(NULL));
```

### Pointer-to-Pointer Examples
```c
// Command line arguments
int main(int argc, char **argv)  // char** = array of string pointers

// Function implementation
int parse_interval(int argc, char **argv) {
    // Uses argv[1], argv[2] - accessing string array via pointer-to-pointer
    if (argc >= 3 && strcmp(argv[1], "--interval") == 0) {
        int user_interval = atoi(argv[2]);
        // ...
    }
}
```

### JSON Output Format
```json
{
  "sensor_id": "sensornode_001",
  "timestamp": "2025-11-18T14:30:15Z", 
  "temperature": 23.45
}
```

## 🧪 Testing

### Build and Run Tests
```bash
# Clean build and test
make clean && make all

# Basic functionality test
./build/sensornode2.0 --interval 10

# Memory leak testing
make valgrind-short

# Help and argument validation
./build/sensornode2.0 --help
```

### Offline Recovery Testing
```bash
# 1. Start program with network
./build/sensornode2.0 --interval 10

# 2. Disconnect network (unplug ethernet/wifi)
# Observe: Data gets saved to bin/saved_temp.txt

# 3. Reconnect network
# Observe: Saved data automatically transmitted and file cleaned
```

### Expected Behavior
- **Success**: HTTP 200 responses with JSON echo from httpbin.org
- **Network Failure**: Data automatically saved to `bin/saved_temp.txt`  
- **Network Recovery**: Saved data transmitted, then removed from file
- **Invalid Intervals**: Falls back to 30-second default (10-120s range)
- **Memory Safety**: Zero memory leaks (validated with valgrind)
- **Graceful Shutdown**: Clean resource cleanup on Ctrl+C

## 🎯 Educational Objectives

This project demonstrates C programming concepts:
- **Function Pointers**: Callback-driven architecture
- **Pointer-to-Pointer**: Advanced memory management  
- **State Machines**: Clean separation of concerns
- **Network Programming**: TCP sockets & HTTP protocol
- **Error Handling**: Robust failure recovery
- **Memory Management**: Safe allocation patterns
- **Modular Design**: Clean header/implementation separation

## 📈 Performance

### Typical Metrics
- **Startup Time**: < 1 second
- **Memory Usage**: ~50KB runtime footprint
- **Network Latency**: 1-3 seconds per HTTP transaction
- **CPU Usage**: Minimal (< 1% on modern systems)
- **Disk Usage**: < 1MB for executable + data

## 🛡️ Error Handling

### Network Resilience
- **Connection Failures**: Automatic retry (3 attempts)
- **Timeout Handling**: 5-second connection timeout
- **Graceful Degradation**: Falls back to local storage
- **Memory Safety**: No buffer overflows or memory leaks

### Failure Modes
- DNS resolution failures → Retry logic
- Network timeouts → Local backup storage  
- Server errors (4xx/5xx) → Data preservation
- Memory allocation failures → Graceful shutdown

## 📊 Current Implementation Status

### ✅ Completed Features (95% Complete)
- [x] **Core sensor reading and JSON formatting**
- [x] **HTTP POST communication to cloud servers**  
- [x] **Local data persistence during network failures**
- [x] **Automatic saved data transmission on recovery**
- [x] **State machine with 12 states and proper transitions**
- [x] **Function pointer architecture with task callbacks**
- [x] **Dual buffer system (json_buffer + fromfile_buffer)**
- [x] **Memory leak prevention and resource cleanup**
- [x] **Command-line argument parsing (--interval, --help)**
- [x] **Comprehensive error handling and retry logic**

### 🚧 Remaining Work (Story 4 - 5% Remaining)
- [ ] **Configuration file parsing** (`bin/config.txt` reading)
  - File exists with proper format
  - Parsing functions need implementation
  - Currently uses hardcoded values in headers

## 🔮 Future Enhancements

### Educational Extensions
- [ ] Real hardware sensor integration (I2C/SPI interfaces)
- [ ] Multiple sensor types (humidity, pressure, light)
- [ ] HTTPS/TLS encrypted communication
- [ ] MQTT protocol support for IoT platforms
- [ ] SQLite local database storage

### Production Improvements  
- [ ] Monotonic clock timing system (replace sleep())
- [ ] Data compression for bandwidth efficiency
- [ ] Persistent retry queue with timestamp ordering
- [ ] Watchdog timer integration
- [ ] Over-the-air configuration updates

## 📄 License

This project is developed for educational purposes as part of embedded systems curriculum.

## 👥 Contributing

This is an educational project. For improvements or bug reports, please follow standard C coding practices:
- Use consistent indentation (4 spaces)
- Add comprehensive comments for complex logic
- Follow POSIX standards for portability
- Test all changes thoroughly

## 📚 References

- [C99 Standard](https://www.iso.org/standard/29237.html)
- [POSIX Socket Programming](https://pubs.opengroup.org/onlinepubs/9699919799/)
- [HTTP/1.1 Specification](https://tools.ietf.org/html/rfc7230)
- [JSON Data Format](https://tools.ietf.org/html/rfc7159)

---

*Built with ❤️ and C99 for embedded systems education*