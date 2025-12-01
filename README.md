# 🌡️ SensorNode 2.0

**Smart Sensor Node** - A robust C-based IoT sensor system that reads temperature data, transmits to cloud servers via HTTP, and provides local backup storage with intelligent offline data recovery.

## 📋 Overview

SensorNode 2.0 is a complete embedded systems project implementing a sensor-to-cloud data pipeline featuring:
- Real-time temperature monitoring with simulated sensor data
- HTTP POST communication to cloud servers (httpbin.org)
- Intelligent local data persistence during network outages
- State machine architecture with function pointers and callbacks
- **Configurable measurement intervals** (default: 30 seconds via `--interval` '%d' (10-120) flags)
- **Non-blocking timing** using CLOCK_MONOTONIC for reliable interval control
- Automatic saved data transmission when not ready for new measurement
- Comprehensive error handling and retry logic

## 🚀 Features

### Core Functionality
- ✅ **Temperature Sensing**: Realistic temperature simulation with drift patterns
- ✅ **Cloud Communication**: HTTP POST requests with JSON payloads to REST APIs
- ✅ **Offline Resilience**: Automatic data backup during network failures
- ✅ **Smart Recovery**: Automatic transmission of saved data while waiting for next measurement
- ✅ **Error Recovery**: 3-retry logic with graceful fallback to offline mode
- ✅ **Configurable Timing**: Command-line interval control (10-120 seconds, default 10s)
- ✅ **Non-blocking Loop**: CLOCK_MONOTONIC timing prevents CPU blocking

### Technical Implementation
- ✅ **State Machine**: 8-state pipeline with clean separation of concerns
- ✅ **Function Pointers**: Callback-driven task execution with smw_task_t
- ✅ **Pointer-to-Pointer**: Advanced memory management (char **argv parsing)
- ✅ **TCP/IP Networking**: Custom socket implementation for HTTP communication
- ✅ **JSON Serialization**: Standards-compliant sensor data formatting
- ✅ **Dual Buffer System**: Separate buffers for live data vs saved data processing
- ✅ **Memory Safety**: Leak-free allocation/deallocation patterns
- ✅ **Monotonic Timing**: CLOCK_MONOTONIC for reliable, non-blocking measurement intervals

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

#### Normal Operation - Continuous Cycling
```
✅ Using interval: 10 seconds
Created Task with function pointer
Sensor initialized
Cycle 1:
  Reading sensors...
  Sensor ID: sensornode_001 Temperature: 23.36°C
  Connected to httpbin.org:80
  HTTP Status: HTTP/1.1 200 OK
  
[Wait ~10 seconds with CLOCK_MONOTONIC]

Cycle 2:
  Time elapsed, reading sensors again...
  Sensor ID: sensornode_001 Temperature: 23.42°C
  Connected to httpbin.org:80
  HTTP Status: HTTP/1.1 200 OK
  
[Continuous 10-second intervals...]
```

#### Operation with Saved Data Recovery
```
Cycle 1: Fresh sensor read → HTTP success → Check for saved data (none) → Done

Cycle 2: Fresh sensor read → HTTP connection fails
         Retry 3 times → all fail → Save to bin/saved_temp.txt

[Network unavailable for 15-20 seconds]

Cycle 3: Measurement interval not passed yet (only 5 seconds)
         Try to send saved data from bin/saved_temp.txt
         Connection still down → Keep data in file
         Wait for next interval...

Cycle 4: Measurement interval passed (10+ seconds), new sensor read
         Fresh data ready, but try saved data first
         [Network restored] → Old data sent successfully
         Remove from saved file → Read fresh sensor → Send it → Done

Cycle 5: Continue normal 10-second cycling...
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

**Main Measurement Loop:**
```
INITIALIZE → READ_SENSOR (check interval)
                    ↓
            (interval passed?)
            ↙              ↖
        YES               NO
        ↓                  ↓
    Read Sensor    PROCESS_SAVED_DATA (try old data)
        ↓                  ↓
    HTTP_TRANSACTION ← ←─ ┘
        ↓
    Response OK?
    ↙        ↖
  YES        NO
   ↓         ↓
PROCESS_SAVED_DATA → SAVE_DATA (backup)
   ↓              ↓
DONE/FAILED ← ← ← ┘

Retry Logic:
HTTP_TRANSACTION (connection fails)
         ↓
    STATE_RETRY
    (attempt_count < 3?)
      ↙         ↖
    YES          NO
     ↓            ↓
RETRY    SAVE_DATA
 ↓           ↓
back    STATE_DONE
```

### Complete State Definitions
```c
typedef enum {
    STATE_INITIALIZE,        // Initialize sensor system
    STATE_READ_SENSOR,       // Read temperature, check measurement interval
    STATE_HTTP_TRANSACTION,  // TCP connect + HTTP POST + receive
    STATE_SAVE_DATA,         // Save data locally on network failure
    STATE_RETRY,            // Retry failed network operations (3 attempts)
    STATE_PROCESS_SAVED_DATA,// Load and send previously saved data
    STATE_DONE,             // Cleanup and prepare for next cycle
    STATE_FAILED            // Terminal failure state
} task_state_t;
```

### Data Flow Architecture

**Fresh Sensor Reading (Interval Elapsed):**
```
Check last_read_time vs current_time (monotonic)
    ↓
(elapsed >= measurement_interval * 1000)?
    ↙        ↖
  YES        NO
   ↓         ↓
Read Sensor  Try Send Saved Data
   ↓             ↓
Format JSON  (No saved data? → Done)
   ↓
HTTP POST → Cloud
```

**Offline/Timing Logic:**
- **During interval wait**: STATE_READ_SENSOR repeatedly checks elapsed time
- **When not time yet**: Transitions to STATE_PROCESS_SAVED_DATA to attempt sending backed-up data
- **After saving**: Waits for next measurement interval before reading fresh data again

**Network Failure Path:**
```
HTTP POST fails
    ↓
RETRY (3 attempts)
    ↓
All failed? → Save to bin/saved_temp.txt
    ↓
Next cycle: Try sending saved data while waiting for interval
```

**Recovery/Saved Data Path:**
```
bin/saved_temp.txt loaded → fromfile_buffer
    ↓
HTTP POST (send old data)
    ↓
Server accepts (200-299)?
    ↓
YES → Remove from saved file
   ↓
Continue with next saved object or wait for interval
```

## 🔍 Technical Details

### Measurement Interval Control (CLOCK_MONOTONIC)

The program uses non-blocking timing to enforce measurement intervals:

```c
// In task_context_t
uint64_t last_read_time;      // When the last measurement was taken (ms)
int measurement_interval;     // How often to take measurements (seconds)

// In STATE_READ_SENSOR
struct timespec current_time;
clock_gettime(CLOCK_MONOTONIC, &current_time);
uint64_t current_ms = (current_time.tv_sec * 1000) + (current_time.tv_nsec / 1000000);
uint64_t elapsed = current_ms - ctx->last_read_time;

if (elapsed >= (uint64_t)ctx->measurement_interval * 1000) {
    // Time to take new measurement
    ctx->last_read_time = current_ms;
    // Read sensor...
} else {
    // Not time yet - try to send old data
    ctx->state = STATE_PROCESS_SAVED_DATA;
}
```

**Key Benefits:**
- ✅ CLOCK_MONOTONIC is immune to system clock adjustments
- ✅ Millisecond precision (1000ms = 1 second interval)
- ✅ No CPU-blocking sleep() calls
- ✅ Continuous loop checking enables responsive saved data transmission
- ✅ First read forced immediately by setting `last_read_time = 0` on startup

### Pointer-to-Pointer Examples
```c
// Command line arguments
int main(int argc, char **argv)  // char** = array of string pointers

// Function implementation
int parse_interval(int argc, char **argv) {
    int interval = 30; 
    
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
# 1. Start program with network (10-second intervals)
./build/sensornode2.0 --interval 10

# 2. Disconnect network (unplug ethernet/wifi)
# Observe: First read succeeds, data gets saved on next failure
# Note: With 10-second intervals, saved data is attempted while waiting

# 3. Reconnect network within measurement interval
# Observe: Saved data automatically transmitted
# Then waits for next measurement cycle

# 4. Let run through multiple cycles
# Observe: Every 10 seconds, fresh measurement taken
# Between measurements: Saved data transmitted if available
```

### Expected Behavior
- **Success**: HTTP 200 responses with JSON transmission from fresh or saved data
- **Interval Enforcement**: Measurements taken exactly at configured intervals (10s default)
- **Non-blocking Timing**: Program continuously loops, checking CLOCK_MONOTONIC
- **While Waiting**: Attempts to send saved data during interval gaps
- **Network Failure**: Data automatically saved to `bin/saved_temp.txt`  
- **Network Recovery**: Saved data transmitted on reconnection, then removed from file
- **Invalid Intervals**: Falls back to 30-second default (10-120s range validated)
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
- DNS resolution failures → Retry logic (3 attempts)
- Network timeouts → Local backup storage with automatic retry
- Server errors (4xx/5xx) → Data preservation and retry
- Memory allocation failures → Graceful shutdown

## 📊 Current Implementation Status

### ✅ Completed Features (100% Complete)
- [x] **Core sensor reading and JSON formatting**
- [x] **HTTP POST communication to cloud servers**  
- [x] **Local data persistence during network failures**
- [x] **Automatic saved data transmission during interval waits**
- [x] **State machine with 8 states and proper transitions**
- [x] **Function pointer architecture with task callbacks**
- [x] **Dual buffer system (json_buffer + fromfile_buffer)**
- [x] **Memory leak prevention and resource cleanup**
- [x] **Command-line argument parsing (--interval, --help)**
- [x] **Comprehensive error handling and retry logic**
- [x] **CLOCK_MONOTONIC timing for non-blocking intervals**
- [x] **Modern TCP with getaddrinfo() instead of deprecated gethostbyname()**

### 🚧 Future Enhancements
- [ ] **Configuration file parsing** (`bin/config.txt` reading)

## 🔮 Future Enhancements

### Educational Extensions
- [ ] Real hardware sensor integration (I2C/SPI interfaces)
- [ ] Multiple sensor types (humidity, pressure, light)
- [ ] HTTPS/TLS encrypted communication
- [ ] MQTT protocol support for IoT platforms
- [ ] SQLite local database storage
- [ ] Configuration file parsing from bin/config.txt

### Production Improvements  
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