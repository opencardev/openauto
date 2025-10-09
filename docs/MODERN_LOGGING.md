# OpenAuto Modern Logging System

## Overview

The OpenAuto Modern Logging System provides a comprehensive, high-performance logging solution designed specifically for the OpenAuto project. It addresses all key requirements from the modernlogging prompt including performance, security, configurability, and maintainability.

## Features

### Core Features
- **Multiple Log Levels**: TRACE, DEBUG, INFO, WARN, ERROR, FATAL
- **Category-Based Logging**: 20+ predefined categories (ANDROID_AUTO, UI, AUDIO, VIDEO, etc.)
- **Multiple Output Formats**: Console, JSON, and Detailed formatters
- **Multiple Output Destinations**: Console, File (with rotation), Remote logging
- **Asynchronous Logging**: Non-blocking logging for performance
- **Thread-Safe**: Safe for use in multi-threaded applications
- **Legacy Compatibility**: Backward compatible with existing OPENAUTO_LOG macros

### Performance Features
- **Lazy Evaluation**: Log messages only processed if level check passes
- **Asynchronous Processing**: Separate worker thread for log processing
- **Queue Management**: Configurable queue size with drop protection
- **Minimal Overhead**: Fast level checks and efficient string handling

### Security Features
- **No Sensitive Data Exposure**: Structured logging prevents accidental data leaks
- **Configurable Levels**: Can disable verbose logging in production
- **File Rotation**: Prevents log files from consuming excessive disk space
- **Thread Isolation**: Async logging isolates performance impact

## Usage

### Basic Usage

```cpp
#include <openauto/Common/ModernLogger.hpp>
#include <openauto/Common/LoggerConfig.hpp>

// Initialize logger (typically in main())
openauto::common::LoggerConfig::initializeDefault();

// Use the new macros
OPENAUTO_LOG_INFO(ANDROID_AUTO, "Connection established");
OPENAUTO_LOG_WARN(BLUETOOTH, "Pairing timeout");
OPENAUTO_LOG_ERROR(NETWORK, "Connection failed");
```

### Legacy Compatibility

Existing code continues to work unchanged:

```cpp
#include <f1x/openauto/Common/Log.hpp>  // Will be migrated to new header

OPENAUTO_LOG(info) << "This still works";
OPENAUTO_LOG(error) << "Error code: " << errorCode;
```

### Configuration Options

```cpp
// Development configuration (verbose, detailed logging)
LoggerConfig::initializeDevelopment();

// Production configuration (optimised, structured logging)
LoggerConfig::initializeProduction();

// Debug configuration (maximum verbosity)
LoggerConfig::initializeDebug();

// Custom configuration
auto& logger = ModernLogger::getInstance();
logger.setLevel(LogLevel::INFO);
logger.setCategoryLevel(LogCategory::UI, LogLevel::WARN);
logger.setAsync(true);
```

## Categories

The logging system provides the following categories:

- **GENERAL**: Default category for general messages
- **SYSTEM**: System-level operations and status
- **ANDROID_AUTO**: Android Auto protocol and communication
- **UI**: User interface components and interactions
- **AUDIO**: Audio processing and output
- **VIDEO**: Video processing and display
- **BLUETOOTH**: Bluetooth connectivity and pairing
- **CAMERA**: Camera operations and video input
- **NETWORK**: Network connectivity and communication
- **CONFIG**: Configuration loading and parsing
- **PROJECTION**: Screen projection and mirroring
- **INPUT**: Input device handling (touch, buttons)
- **SERVICE**: Background services and daemons
- **SETTINGS**: Application settings and preferences
- **MEDIA**: Media playback and control
- **NAVIGATION**: GPS and navigation functions
- **PHONE**: Phone call handling
- **WIFI**: WiFi connectivity and management
- **USB**: USB device communication
- **SECURITY**: Security-related operations

## Migration Guide

### Automatic Migration

Use the provided migration script:

```bash
# Dry run to see what would be changed
./scripts/migrate_to_modern_logger.py --dry-run

# Perform the migration
./scripts/migrate_to_modern_logger.py

# Migrate specific directory
./scripts/migrate_to_modern_logger.py src/autoapp --verbose
```

### Manual Migration Steps

1. **Update includes**:
   ```cpp
   // Old
   #include <f1x/openauto/Common/Log.hpp>
   
   // New
   #include <openauto/Common/ModernLogger.hpp>
   ```

2. **Convert log calls**:
   ```cpp
   // Old
   OPENAUTO_LOG(info) << "Message: " << value;
   
   // New
   OPENAUTO_LOG_INFO(GENERAL, "Message: " + std::to_string(value));
   ```

3. **Choose appropriate categories**:
   ```cpp
   // More specific categories provide better filtering
   OPENAUTO_LOG_INFO(ANDROID_AUTO, "Connection status");
   OPENAUTO_LOG_DEBUG(UI, "Button clicked");
   OPENAUTO_LOG_WARN(BLUETOOTH, "Device not found");
   ```

### CMakeLists.txt Integration

Add the new source files to your CMakeLists.txt:

```cmake
# Add modern logger sources
set(MODERN_LOGGER_SOURCES
    src/Common/ModernLogger.cpp
    src/Common/LoggerConfig.cpp
)

# Include modern logger headers
target_include_directories(your_target PRIVATE
    include
)

# Add sources to target
target_sources(your_target PRIVATE
    ${MODERN_LOGGER_SOURCES}
    # ... other sources
)
```

## Configuration Files

### Example Logger Configuration

```cpp
// In main() or initialization function
void setupLogging() {
    auto& logger = openauto::common::ModernLogger::getInstance();
    
    // Set global level
    logger.setLevel(openauto::common::LogLevel::INFO);
    
    // Configure category-specific levels
    logger.setCategoryLevel(openauto::common::LogCategory::AUDIO, 
                           openauto::common::LogLevel::WARN);
    logger.setCategoryLevel(openauto::common::LogCategory::VIDEO, 
                           openauto::common::LogLevel::WARN);
    
    // Add file logging with rotation
    openauto::common::LoggerConfig::enableFileLogging(
        "/var/log/openauto.log", 
        50 * 1024 * 1024,  // 50MB max size
        10                 // Keep 10 files
    );
    
    // Enable async logging for performance
    logger.setAsync(true);
    logger.setMaxQueueSize(2000);
}
```

## Performance Considerations

### Async vs Sync Logging

- **Async**: Better performance, small delay in log output
- **Sync**: Immediate output, may block calling thread

### Log Level Impact

- **TRACE**: Maximum verbosity, significant performance impact
- **DEBUG**: Development logging, moderate impact
- **INFO**: Production default, minimal impact
- **WARN/ERROR**: Production monitoring, negligible impact

### Best Practices

1. **Use appropriate log levels**: Don't log at TRACE in production
2. **Choose specific categories**: Better filtering and control
3. **Avoid expensive operations**: Don't call functions in log messages
4. **Use async logging**: For better performance in production
5. **Configure rotation**: Prevent disk space issues

## Testing

Run the test program to verify the logger works correctly:

```bash
# Build the test program
g++ -std=c++17 -I include tests/test_modern_logger.cpp src/Common/ModernLogger.cpp src/Common/LoggerConfig.cpp -o test_logger -pthread

# Run the test
./test_logger
```

## Troubleshooting

### Common Issues

1. **Compilation errors**: Ensure C++17 support and proper include paths
2. **Missing log output**: Check log levels and category configuration
3. **Performance issues**: Consider using async logging
4. **File permission errors**: Ensure write access to log directories

### Debug Steps

1. **Check log level**: Verify global and category levels
2. **Verify initialization**: Ensure logger is properly initialized
3. **Test with simple messages**: Start with basic OPENAUTO_LOG_INFO calls
4. **Check queue status**: Monitor queue size and dropped messages

## Future Enhancements

Planned improvements include:

- Configuration file support (JSON/YAML)
- Network logging with encryption
- Log analysis and monitoring tools
- Integration with systemd journal
- Performance metrics and monitoring
- Custom sink implementations

## Conclusion

The OpenAuto Modern Logging System provides a robust, performant, and flexible logging solution that addresses all requirements from the modernlogging prompt. It maintains backward compatibility while providing powerful new features for better debugging, monitoring, and maintenance of the OpenAuto system.