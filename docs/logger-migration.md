# Modern Logger Migration Guide

## Overview

The OpenAuto logging system has been completely modernized to provide better performance, more detailed information, and comprehensive configurability. This guide explains how to migrate from the old boost-based logging to the new modern logging system.

## Key Improvements

### **Before (Boost Logging)**
- Basic severity levels only
- Limited context information
- No categorization
- Single output format
- File system dependencies
- Performance overhead

### **After (Modern Logger)**
- ✅ **Detailed categorization** (System, Android Auto, UI, Camera, Network, etc.)
- ✅ **Rich context** (component, function, file, line, thread ID)
- ✅ **Multiple output formats** (Console with colors, JSON, file format)
- ✅ **Asynchronous processing** for better performance
- ✅ **Configurable sinks** (console, file with rotation, remote)
- ✅ **Performance logging** with built-in timers
- ✅ **Thread-safe** operations
- ✅ **Memory efficient** with queue management

## Migration Steps

### 1. Update Include Statements

**Old:**
```cpp
#include <f1x/openauto/Common/Log.hpp>
```

**New:**
```cpp
#include "modern/Logger.hpp"
```

### 2. Replace Basic Logging Calls

**Old:**
```cpp
OPENAUTO_LOG(info) << "Android Auto connected";
OPENAUTO_LOG(error) << "Failed to initialize camera";
```

**New:**
```cpp
LOG_INFO(ANDROID_AUTO, "Android Auto connected");
LOG_ERROR(CAMERA, "Failed to initialize camera");
```

### 3. Add Categorized Logging

**Categories Available:**
- `SYSTEM` - System-level operations
- `ANDROID_AUTO` - Android Auto specific operations
- `UI` - User interface operations
- `CAMERA` - Camera operations
- `NETWORK` - Network operations
- `BLUETOOTH` - Bluetooth operations
- `AUDIO` - Audio operations
- `VIDEO` - Video operations
- `CONFIG` - Configuration operations
- `API` - REST API operations
- `EVENT` - Event bus operations
- `STATE` - State machine operations
- `GENERAL` - General purpose logging

**Examples:**
```cpp
LOG_DEBUG(SYSTEM, "Initializing hardware components");
LOG_INFO(ANDROID_AUTO, "Device connected via USB");
LOG_WARN(CAMERA, "Camera device not found, using default");
LOG_ERROR(NETWORK, "WiFi connection failed");
LOG_FATAL(SYSTEM, "Critical system failure");
```

### 4. Use Context Logging for Rich Information

**Old:**
```cpp
OPENAUTO_LOG(info) << "Button pressed: " << buttonId << " on screen: " << screenName;
```

**New:**
```cpp
std::map<std::string, std::string> context = {
    {"button_id", buttonId},
    {"screen", screenName},
    {"user_id", userId}
};
LOG_INFO_CTX(UI, "Button pressed", context);
```

### 5. Add Performance Logging

**Measure execution time:**
```cpp
void processVideoFrame() {
    LOG_PERF_START(video_processing);
    
    // Your video processing code here
    processFrame();
    
    LOG_PERF_END(VIDEO, video_processing);
}
```

**Output:**
```
[DEBUG] [VIDEO] Performance [video_processing]: 1250μs
```

### 6. Static Function Logging

**For static functions or C-style code:**
```cpp
static void initializeSystem() {
    SLOG_INFO(SYSTEM, "SystemInit", "Starting system initialization");
    
    if (!loadConfiguration()) {
        SLOG_ERROR(SYSTEM, "SystemInit", "Failed to load configuration");
        return;
    }
    
    SLOG_INFO(SYSTEM, "SystemInit", "System initialization complete");
}
```

## Configuration

### Basic Setup in Code

```cpp
#include "modern/Logger.hpp"

void setupLogging() {
    auto& logger = openauto::modern::Logger::getInstance();
    
    // Set global log level
    logger.setLevel(openauto::modern::LogLevel::INFO);
    
    // Set category-specific levels
    logger.setCategoryLevel(openauto::modern::LogCategory::SYSTEM, 
                           openauto::modern::LogLevel::DEBUG);
    logger.setCategoryLevel(openauto::modern::LogCategory::ANDROID_AUTO, 
                           openauto::modern::LogLevel::INFO);
    
    // Enable async processing for better performance
    logger.setAsync(true);
    logger.setMaxQueueSize(10000);
    
    // Add file logging with rotation
    auto fileSink = std::make_shared<openauto::modern::FileSink>(
        "openauto.log", 10 * 1024 * 1024, 5);  // 10MB max, 5 files
    logger.addSink(fileSink);
    
    // Use JSON format for structured logging
    auto jsonFormatter = std::make_shared<openauto::modern::JsonFormatter>(true);
    logger.setFormatter(jsonFormatter);
}
```

### Console Output Example

```
2025-07-11 14:30:15.123 [INFO] [ANDROID_AUTO] [12345] [AndroidAutoManager::onDeviceConnected] (AndroidAutoManager.cpp:45) - Device connected via USB {device_id=ABC123, vendor=Google}
```

### JSON Output Example

```json
{
  "timestamp": "2025-07-11T14:30:15.123Z",
  "level": "INFO",
  "category": "ANDROID_AUTO",
  "component": "AndroidAutoManager",
  "function": "onDeviceConnected",
  "file": "AndroidAutoManager.cpp",
  "line": 45,
  "thread_id": "12345",
  "message": "Device connected via USB",
  "context": {
    "device_id": "ABC123",
    "vendor": "Google"
  }
}
```

## Advanced Features

### 1. Custom Sinks

```cpp
// Remote logging to external system
auto remoteSink = std::make_shared<openauto::modern::RemoteSink>(
    "http://log-server:8080/logs");
logger.addSink(remoteSink);

// Console with custom formatting
auto consoleSink = std::make_shared<openauto::modern::ConsoleSink>(false);
logger.addSink(consoleSink);
```

### 2. Custom Formatters

```cpp
class CustomFormatter : public openauto::modern::LogFormatter {
public:
    std::string format(const openauto::modern::LogEntry& entry) override {
        return "[" + openauto::modern::Logger::levelToString(entry.level) + "] " + 
               entry.message + "\n";
    }
};

logger.setFormatter(std::make_shared<CustomFormatter>());
```

### 3. Runtime Configuration

```cpp
// Change log levels at runtime
logger.setLevel(openauto::modern::LogLevel::DEBUG);

// Enable/disable specific categories
logger.setCategoryLevel(openauto::modern::LogCategory::CAMERA, 
                       openauto::modern::LogLevel::TRACE);
```

## Backward Compatibility

The modern logger provides backward compatibility for existing code:

### Automatic Migration

When `ENABLE_MODERN_API` is enabled, old `OPENAUTO_LOG` macros automatically use the modern logger:

```cpp
// This still works and uses modern logger
OPENAUTO_LOG(info) << "Legacy log message";
```

### Gradual Migration

You can migrate code gradually:

```cpp
class MyComponent {
public:
    void oldMethod() {
        // Old style - still works
        OPENAUTO_LOG(info) << "Processing data";
    }
    
    void newMethod() {
        // New style - recommended
        LOG_INFO(SYSTEM, "Processing data with better context");
    }
};
```

## Performance Benefits

### Asynchronous Logging

```cpp
// Non-blocking logging - messages queued and processed in background
LOG_INFO(SYSTEM, "This doesn't block the main thread");
```

### Memory Management

```cpp
// Automatic queue management prevents memory leaks
logger.setMaxQueueSize(5000);  // Drops oldest messages when full

// Check statistics
size_t queueSize = logger.getQueueSize();
size_t dropped = logger.getDroppedMessages();
```

### Thread Safety

```cpp
// Safe to use from multiple threads
std::thread worker([]() {
    LOG_INFO(SYSTEM, "Worker thread message");
});
```

## Best Practices

### 1. Choose Appropriate Categories

```cpp
// Good - specific category
LOG_ERROR(CAMERA, "Camera initialization failed");

// Avoid - generic category
LOG_ERROR(GENERAL, "Something failed");
```

### 2. Use Context for Rich Information

```cpp
// Good - provides context
std::map<std::string, std::string> ctx = {
    {"device_id", deviceId},
    {"retry_count", std::to_string(retryCount)}
};
LOG_ERROR_CTX(NETWORK, "Connection failed", ctx);

// Avoid - limited information
LOG_ERROR(NETWORK, "Connection failed");
```

### 3. Use Appropriate Log Levels

- **TRACE**: Very detailed debugging information
- **DEBUG**: Debugging information
- **INFO**: General information about program execution
- **WARN**: Warning messages about potential issues
- **ERROR**: Error messages for recoverable errors
- **FATAL**: Critical errors that may cause program termination

### 4. Performance Logging

```cpp
// Measure critical operations
LOG_PERF_START(database_query);
auto result = database.query(sql);
LOG_PERF_END(SYSTEM, database_query);
```

### 5. Structured Logging

```cpp
// Use context for machine-readable logs
std::map<std::string, std::string> metrics = {
    {"cpu_usage", std::to_string(cpuUsage)},
    {"memory_usage", std::to_string(memoryUsage)},
    {"active_connections", std::to_string(connections)}
};
LOG_INFO_CTX(SYSTEM, "System metrics", metrics);
```

## Integration with Modern Architecture

The logger is automatically integrated with other modern components:

### Event Bus Integration

```cpp
// Events automatically logged with context
eventBus->subscribe(EventType::ANDROID_AUTO_CONNECTED, [](const Event::Pointer& event) {
    LOG_INFO(EVENT, "Event received: " + event->toString());
});
```

### Configuration Integration

```cpp
// Log levels can be configured via ConfigurationManager
auto logLevel = configManager->get("logging.level", "INFO");
logger.setLevel(Logger::stringToLevel(logLevel));
```

### REST API Integration

```cpp
// API calls automatically logged
// GET /api/v1/logs - retrieve recent log entries
// PUT /api/v1/logs/level - change log level at runtime
```

## Troubleshooting

### Common Issues

1. **Messages not appearing**: Check log level configuration
2. **Performance issues**: Enable async logging
3. **File not created**: Check file permissions and path
4. **Memory usage**: Adjust queue size and enable rotation

### Debug Commands

```cpp
// Check logger status
size_t queueSize = logger.getQueueSize();
size_t dropped = logger.getDroppedMessages();
LOG_INFO(SYSTEM, "Logger stats: queue=" + std::to_string(queueSize) + 
                 ", dropped=" + std::to_string(dropped));

// Force flush
logger.flush();
```

## Build System Migration

### Important: Clean Build Required

After migrating to the modern logger, you **must** clean your build directory to avoid conflicts with legacy cached files:

```bash
# Clean build directory
rm -rf build/
# or on Windows
rmdir /s build

# Create fresh build
mkdir build
cd build
cmake ..
make
```

### Legacy File Conflicts

If you encounter build errors like:
```
fatal error: f1x/openauto/autoapp/EventBus/Event.hpp: No such file or directory
```

This indicates legacy cached files or old EventBus files. To fix:

1. **Clean build directory completely**
2. **Remove any legacy EventBus directories**:
   ```bash
   find . -name "*EventBus*" -type d -exec rm -rf {} +
   ```
3. **Verify no legacy includes remain**:
   ```bash
   grep -r "f1x.*EventBus" . --exclude-dir=build
   ```

The modern architecture uses `src/modern/Event.cpp` and `include/modern/Event.hpp` instead of the legacy EventBus structure.

The modern logger provides a significant improvement in debugging capabilities, performance, and maintainability while maintaining backward compatibility with existing code.
