# Integration Guide - Modern OpenAuto Components

## Overview

This guide explains how to integrate and use the modern OpenAuto architecture components including the event bus, state machine, REST API, and logging system.

## Modern Components Overview

### Available Components
- **Logger System** - High-performance categorized logging
- **Event Bus** - Type-safe event publishing and subscription
- **State Machine** - Centralized state management
- **Configuration Manager** - Dynamic configuration handling
- **REST API Server** - External integration interface
- **Integration Layer** - Unified component management

## 1. Modern Logger Integration

### Basic Usage
```cpp
#include "modern/Logger.hpp"

// Basic logging with categories
SLOG_INFO(GENERAL, "component", "Application started");
SLOG_ERROR(NETWORK, "wifi", "Connection failed: {}", error_msg);
SLOG_DEBUG(UI, "settings", "User changed value: {} = {}", key, value);

// Performance logging
SLOG_PERF_START(DATABASE, "query", "SELECT * FROM users");
// ... database operation ...
SLOG_PERF_END(DATABASE, "query", "Query completed in {}ms", elapsed);
```

### Available Log Categories
```cpp
// System categories
SLOG_INFO(SYSTEM, "boot", "System initialization");
SLOG_ERROR(ANDROID_AUTO, "projection", "Projection failed");

// Feature categories  
SLOG_DEBUG(UI, "main_window", "Button clicked");
SLOG_INFO(CAMERA, "recorder", "Recording started");
SLOG_WARN(NETWORK, "connection", "Weak signal detected");
SLOG_ERROR(MEDIA, "player", "Codec not supported");
```

### Logger Configuration
```cpp
// Configure logger programmatically
openauto::modern::Logger::configure({
    .asyncLogging = true,
    .fileOutput = true,
    .consoleOutput = true,
    .logLevel = openauto::modern::LogLevel::INFO
});
```

## 2. Event Bus Integration

### Subscribe to Events
```cpp
#include "modern/EventBus.hpp"
#include "modern/Event.hpp"

// Get event bus instance
auto eventBus = std::make_shared<openauto::modern::EventBus>();

// Subscribe to specific events
eventBus->subscribe(openauto::modern::EventType::ANDROID_AUTO_CONNECTED, 
    [](const openauto::modern::Event::Pointer& event) {
        SLOG_INFO(ANDROID_AUTO, "main", "Device connected: {}", 
                  event->getData("device_name"));
    });

eventBus->subscribe(openauto::modern::EventType::UI_BUTTON_PRESSED,
    [](const openauto::modern::Event::Pointer& event) {
        std::string buttonId = event->getData("button_id");
        SLOG_DEBUG(UI, "handler", "Button {} pressed", buttonId);
    });
```

### Publish Events
```cpp
// Create and publish events
auto event = std::make_shared<openauto::modern::Event>(
    openauto::modern::EventType::UI_BUTTON_PRESSED, "settings_ui");
event->setData("button_id", std::string("wifi_settings"));
event->setData("timestamp", std::chrono::system_clock::now());
eventBus->publish(event);

// System events
auto systemEvent = std::make_shared<openauto::modern::Event>(
    openauto::modern::EventType::SYSTEM_STATUS_CHANGED, "system_monitor");
systemEvent->setData("status", std::string("operational"));
systemEvent->setData("cpu_usage", 25.5);
eventBus->publish(systemEvent);
```

### Available Event Types
```cpp
// System events
EventType::SYSTEM_STARTUP
EventType::SYSTEM_SHUTDOWN
EventType::SYSTEM_ERROR

// Android Auto events
EventType::ANDROID_AUTO_CONNECTED
EventType::ANDROID_AUTO_DISCONNECTED
EventType::ANDROID_AUTO_PROJECTION_STARTED

// UI events
EventType::UI_BUTTON_PRESSED
EventType::UI_SCREEN_CHANGED
EventType::UI_DIALOG_OPENED

// Media events
EventType::MEDIA_PLAY
EventType::MEDIA_PAUSE
EventType::MEDIA_TRACK_CHANGED

// Camera events
EventType::CAMERA_RECORDING_STARTED
EventType::CAMERA_RECORDING_STOPPED
EventType::CAMERA_PHOTO_TAKEN
```

## 3. State Machine Integration

### Monitor State Changes
```cpp
#include "modern/StateMachine.hpp"

// Get state machine instance
auto stateMachine = std::make_shared<openauto::modern::StateMachine>();

// Subscribe to state changes
stateMachine->onStateChange([](openauto::modern::State oldState, 
                              openauto::modern::State newState) {
    SLOG_INFO(STATE, "monitor", "State changed: {} -> {}", 
              toString(oldState), toString(newState));
});

// Get current state
auto currentState = stateMachine->getCurrentState();
SLOG_DEBUG(STATE, "query", "Current state: {}", toString(currentState));
```

### Trigger State Transitions
```cpp
// Trigger state transitions
bool success = stateMachine->transition(openauto::modern::Trigger::ANDROID_AUTO_CONNECT);
if (success) {
    SLOG_INFO(STATE, "control", "Successfully transitioned to connected state");
} else {
    SLOG_ERROR(STATE, "control", "State transition failed");
}

// Available states and triggers
State::INITIALIZING -> Trigger::INITIALIZATION_COMPLETE -> State::IDLE
State::IDLE -> Trigger::ANDROID_AUTO_CONNECT -> State::CONNECTED
State::CONNECTED -> Trigger::START_PROJECTION -> State::PROJECTION
```

## 4. Configuration Manager Integration

### Access Configuration
```cpp
#include "modern/ConfigurationManager.hpp"

// Get configuration manager
auto configManager = std::make_shared<openauto::modern::ConfigurationManager>();

// Get configuration values
bool wifiEnabled = configManager->getBool("network.wifi.enabled", true);
int apiPort = configManager->getInt("api.port", 8080);
std::string deviceName = configManager->getString("device.name", "OpenAuto");

SLOG_INFO(CONFIG, "main", "WiFi enabled: {}, API port: {}, Device: {}", 
          wifiEnabled, apiPort, deviceName);
```

### Update Configuration
```cpp
// Update configuration values
configManager->setBool("network.wifi.enabled", false);
configManager->setInt("api.port", 8081);
configManager->setString("device.name", "My Car");

// Save configuration
configManager->save();
SLOG_INFO(CONFIG, "main", "Configuration saved");
```

## 5. REST API Integration

### API Server Setup
```cpp
#include "modern/RestApiServer.hpp"

// Create and configure API server
auto apiServer = std::make_shared<openauto::modern::RestApiServer>();
apiServer->setPort(8080);
apiServer->enableCORS(true);

// Start the server
if (apiServer->start()) {
    SLOG_INFO(API, "server", "REST API server started on port 8080");
} else {
    SLOG_ERROR(API, "server", "Failed to start REST API server");
}
```

### External API Access
```bash
# Health check
curl http://localhost:8080/api/v1/health

# Get current state
curl http://localhost:8080/api/v1/state

# Publish event via API
curl -X POST http://localhost:8080/api/v1/events \
  -H "Content-Type: application/json" \
  -d '{
    "type": "UI_BUTTON_PRESSED",
    "source": "external_app",
    "data": {
      "button_id": "home",
      "user": "admin"
    }
  }'

# Get configuration
curl http://localhost:8080/api/v1/config

# Update configuration
curl -X PUT http://localhost:8080/api/v1/config \
  -H "Content-Type: application/json" \
  -d '{
    "network.wifi.enabled": true,
    "api.port": 8080
  }'

# Interactive documentation
open http://localhost:8080/docs
```

## 6. Modern Integration Layer

### Initialize All Components
```cpp
#include "modern/ModernIntegration.hpp"

int main() {
    // Get singleton instance
    auto& integration = openauto::modern::ModernIntegration::getInstance();
    
    // Initialize all modern components
    if (!integration.initialize()) {
        std::cerr << "Failed to initialize modern components" << std::endl;
        return 1;
    }
    
    SLOG_INFO(SYSTEM, "main", "Modern components initialized successfully");
    
    // Your application code here
    // Components are now available for use
    
    // Shutdown when done
    integration.shutdown();
    return 0;
}
```

### Component Access
```cpp
// Access components through integration layer
auto& integration = openauto::modern::ModernIntegration::getInstance();

// Publish events to legacy system bridge
integration.publishLegacyEvent("CUSTOM_EVENT", "my_component", "Event data");

// Update system state
integration.updateLegacyState("projection_active");

// Get/set configuration
std::string value = integration.getLegacyConfig("my_setting", "default_value");
integration.setLegacyConfig("my_setting", "new_value");
```

## 7. Migration from Legacy Code

### Replace Legacy Logging
```cpp
// OLD: Legacy logging
OPENAUTO_LOG(info) << "Connection established";

// NEW: Modern logging
SLOG_INFO(NETWORK, "connection", "Connection established");
```

### Replace Legacy Event Handling
```cpp
// OLD: Direct function calls or signals
onButtonPressed("home");

// NEW: Event-driven architecture
auto event = std::make_shared<openauto::modern::Event>(
    openauto::modern::EventType::UI_BUTTON_PRESSED, "main_ui");
event->setData("button_id", std::string("home"));
eventBus->publish(event);
```

## 8. Complete Integration Example

### Example Application
```cpp
#include "modern/ModernIntegration.hpp"
#include "modern/Logger.hpp"
#include "modern/EventBus.hpp"
#include "modern/StateMachine.hpp"

class MyComponent {
private:
    std::shared_ptr<openauto::modern::EventBus> eventBus_;
    std::shared_ptr<openauto::modern::StateMachine> stateMachine_;

public:
    MyComponent() {
        // Get modern components
        auto& integration = openauto::modern::ModernIntegration::getInstance();
        eventBus_ = integration.getEventBus();
        stateMachine_ = integration.getStateMachine();
        
        // Subscribe to events
        setupEventSubscriptions();
        
        SLOG_INFO(GENERAL, "MyComponent", "Component initialized");
    }
    
    void setupEventSubscriptions() {
        // Subscribe to Android Auto events
        eventBus_->subscribe(openauto::modern::EventType::ANDROID_AUTO_CONNECTED,
            [this](const openauto::modern::Event::Pointer& event) {
                handleAndroidAutoConnected(event);
            });
            
        // Subscribe to UI events
        eventBus_->subscribe(openauto::modern::EventType::UI_BUTTON_PRESSED,
            [this](const openauto::modern::Event::Pointer& event) {
                handleButtonPressed(event);
            });
    }
    
    void handleAndroidAutoConnected(const openauto::modern::Event::Pointer& event) {
        SLOG_INFO(ANDROID_AUTO, "MyComponent", "Android Auto device connected");
        
        // Trigger state transition
        stateMachine_->transition(openauto::modern::Trigger::ANDROID_AUTO_CONNECT);
        
        // Publish response event
        auto response = std::make_shared<openauto::modern::Event>(
            openauto::modern::EventType::SYSTEM_STATUS_CHANGED, "MyComponent");
        response->setData("status", std::string("android_auto_ready"));
        eventBus_->publish(response);
    }
    
    void handleButtonPressed(const openauto::modern::Event::Pointer& event) {
        std::string buttonId = event->getData("button_id");
        SLOG_DEBUG(UI, "MyComponent", "Handling button press: {}", buttonId);
        
        if (buttonId == "settings") {
            stateMachine_->transition(openauto::modern::Trigger::ENTER_SETTINGS);
        }
    }
};

int main() {
    // Initialize modern architecture
    auto& integration = openauto::modern::ModernIntegration::getInstance();
    if (!integration.initialize()) {
        return 1;
    }
    
    // Create your components
    MyComponent component;
    
    // Run your application
    // ...
    
    // Cleanup
    integration.shutdown();
    return 0;
}
```

## 9. Best Practices

### Logging Best Practices
- Use appropriate log categories for organized output
- Include relevant context in log messages
- Use structured logging with parameters
- Avoid logging sensitive information

### Event Bus Best Practices
- Use specific event types rather than generic ones
- Include relevant data in event payloads
- Clean up subscriptions when components are destroyed
- Handle events asynchronously when possible

### State Machine Best Practices
- Validate state transitions before triggering
- Use state change callbacks for side effects
- Keep state logic simple and predictable
- Log state transitions for debugging

### API Integration Best Practices
- Handle API errors gracefully
- Use appropriate HTTP status codes
- Validate input data
- Implement rate limiting for production use

## 10. Troubleshooting

### Common Issues
- **Modern components not available**: Ensure `ENABLE_MODERN_API=ON` during build
- **API server not starting**: Check port availability and permissions
- **Events not received**: Verify event type and subscription setup
- **State transitions failing**: Check current state and valid transitions

### Debugging Tips
- Use Swagger UI at `/docs` for API testing
- Enable debug logging: `SLOG_DEBUG`
- Monitor event flow with event bus statistics
- Check state machine history for transition issues

### Performance Considerations
- Event processing is asynchronous - don't rely on immediate processing
- Logger uses async queues - configure queue sizes appropriately
- REST API is lightweight but consider rate limiting
- State machine transitions are synchronous and fast
