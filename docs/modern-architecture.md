# OpenAuto Modern Architecture

This directory contains the modernized components of OpenAuto, designed to replace temporary file usage with a robust event-driven architecture.

## Components

### Event System
- **Event.hpp/cpp**: Core event class with JSON serialization support
- **EventBus.hpp/cpp**: Thread-safe event distribution system
- **StateMachine.hpp/cpp**: Finite state machine for system state management

### API & Configuration
- **RestApiServer.hpp/cpp**: HTTP REST API for remote control and monitoring
- **ConfigurationManager.hpp/cpp**: Modern configuration management without /tmp files

## Features

### Event Bus
- Thread-safe event publishing and subscription
- Event history and filtering
- JSON serialization for network transport
- Automatic cleanup and memory management

### State Machine
- Predefined system states (Idle, Android Auto Active, Camera View, etc.)
- Event-driven state transitions
- Callback support for state changes
- Validation of state transitions

### REST API
- RESTful endpoints for event publishing and state management
- JSON-based communication
- Configuration management endpoints
- Real-time event streaming (planned)

### Configuration Management
- JSON-based configuration storage
- Type-safe value access
- Event notifications on configuration changes
- Validation and default values

## API Endpoints

### Events
- `GET /api/events` - Get event history
- `POST /api/events` - Publish new event
- `GET /api/events/{type}` - Get events of specific type

### State Management
- `GET /api/state` - Get current system state
- `POST /api/state/transition` - Trigger state transition
- `GET /api/state/transitions` - Get valid transitions

### Configuration
- `GET /api/config` - Get configuration values
- `POST /api/config` - Update configuration
- `PUT /api/config/{key}` - Set specific configuration value
- `DELETE /api/config/{key}` - Remove configuration value

### System
- `GET /api` - API documentation
- `GET /api/health` - System health check

## Usage Example

```cpp
#include "modern/EventBus.hpp"
#include "modern/StateMachine.hpp"
#include "modern/RestApiServer.hpp"
#include "modern/ConfigurationManager.hpp"

// Initialize components
auto eventBus = std::make_shared<openauto::modern::EventBus>();
auto stateMachine = std::make_shared<openauto::modern::StateMachine>();
auto configManager = std::make_shared<openauto::modern::ConfigurationManager>();
auto restApi = std::make_shared<openauto::modern::RestApiServer>();

// Configure
eventBus->start();
restApi->setEventBus(eventBus);
restApi->setStateMachine(stateMachine);
restApi->start();

// Subscribe to events
eventBus->subscribe(openauto::modern::EventType::ANDROID_AUTO_CONNECTED, 
    [](const auto& event) {
        std::cout << "Android Auto connected!" << std::endl;
    });

// Publish events
auto event = std::make_shared<openauto::modern::Event>(
    openauto::modern::EventType::ANDROID_AUTO_CONNECTED, "main");
eventBus->publish(event);
```

## Migration from /tmp Files

The modern architecture eliminates the need for temporary files by:

1. **Event Communication**: Instead of writing status files to /tmp, components communicate via events
2. **Configuration Storage**: Configuration is managed in-memory with persistent JSON storage
3. **State Persistence**: System state is maintained in the state machine with optional persistence
4. **Inter-Process Communication**: REST API enables external processes to interact with the system

## Thread Safety

All components are designed to be thread-safe:
- EventBus uses locks and condition variables for safe concurrent access
- StateMachine protects state transitions with mutexes
- ConfigurationManager ensures atomic configuration updates
- RestApiServer handles concurrent HTTP requests safely

## Dependencies

- **nlohmann/json**: JSON serialization
- **Boost.Beast**: HTTP server implementation
- **Boost.Asio**: Asynchronous I/O
- **C++17**: Modern C++ features (std::variant, std::filesystem)
