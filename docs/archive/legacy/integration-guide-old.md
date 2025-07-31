# Integration Guide - Modern OpenAuto Architecture

## Overview

This guide explains how to integrate with the modernized OpenAuto architecture using the event bus, state machine, and REST API components.

## Architecture Components

### 1. Event Bus

The event bus provides a decoupled communication mechanism between components.

#### Basic Usage

```cpp
#include "modern/EventBus.hpp"
#include "modern/Event.hpp"

// Create event bus
auto eventBus = std::make_shared<openauto::modern::EventBus>();

// Subscribe to events
eventBus->subscribe(openauto::modern::EventType::UI_BUTTON_PRESSED, 
    [](const openauto::modern::Event::Pointer& event) {
        std::cout << "Button pressed: " << event->toString() << std::endl;
    });

// Publish events
auto event = std::make_shared<openauto::modern::Event>(
    openauto::modern::EventType::UI_BUTTON_PRESSED, "main_ui");
event->setData("button_id", std::string("home"));
eventBus->publish(event);
```

#### External Process Integration

```cpp
// External process can connect via REST API
#include <httplib.h>
#include <nlohmann/json.hpp>

httplib::Client client("localhost", 8080);

// Publish event via REST API
nlohmann::json eventData = {
    {"type", "CUSTOM_EVENT"},
    {"source", "external_process"},
    {"data", {{"message", "Hello from external process"}}}
};

auto response = client.Post("/api/v1/events", 
    eventData.dump(), "application/json");
```

### 2. State Machine

The state machine manages application states and transitions.

#### Basic Usage

```cpp
#include "modern/StateMachine.hpp"

// Create state machine
auto stateMachine = std::make_shared<openauto::modern::StateMachine>();

// Define states
stateMachine->addState("idle", "System is idle");
stateMachine->addState("connected", "Android Auto connected");
stateMachine->addState("projection", "Projecting Android Auto");

// Define transitions
stateMachine->addTransition("idle", "connected", "device_connected");
stateMachine->addTransition("connected", "projection", "start_projection");
stateMachine->addTransition("projection", "connected", "stop_projection");
stateMachine->addTransition("connected", "idle", "device_disconnected");

// Set initial state
stateMachine->transitionTo("idle");

// Listen for state changes
stateMachine->onStateChanged([](const std::string& from, const std::string& to) {
    std::cout << "State changed: " << from << " -> " << to << std::endl;
});
```

#### Remote State Control

```bash
# Get current state
curl http://localhost:8080/api/v1/state

# Trigger state transition
curl -X POST http://localhost:8080/api/v1/state/transition \
  -H "Content-Type: application/json" \
  -d '{"target": "connected"}'
```

### 3. Configuration Management

Centralized configuration with REST API access.

#### Basic Usage

```cpp
#include "modern/ConfigurationManager.hpp"

// Create configuration manager
auto configManager = std::make_shared<openauto::modern::ConfigurationManager>();

// Set configuration values
configManager->set("ui.brightness", 75);
configManager->set("audio.volume", 50);
configManager->set("network.wifi.enabled", true);

// Get configuration values
int brightness = configManager->get("ui.brightness").get<int>();
bool wifiEnabled = configManager->get("network.wifi.enabled").get<bool>();

// Save to file
configManager->save();
```

#### Remote Configuration

```bash
# Get all configuration
curl http://localhost:8080/api/v1/config

# Get specific value
curl http://localhost:8080/api/v1/config/ui.brightness

# Set configuration value
curl -X PUT http://localhost:8080/api/v1/config/ui.brightness \
  -H "Content-Type: application/json" \
  -d '{"value": 80}'

# Save configuration
curl -X POST http://localhost:8080/api/v1/config/save
```

## Complete Integration Example

### Main Application Setup

```cpp
#include "modern/EventBus.hpp"
#include "modern/StateMachine.hpp"
#include "modern/ConfigurationManager.hpp"
#include "modern/RestApiServer.hpp"

int main() {
    // Create core components
    auto eventBus = std::make_shared<openauto::modern::EventBus>();
    auto stateMachine = std::make_shared<openauto::modern::StateMachine>();
    auto configManager = std::make_shared<openauto::modern::ConfigurationManager>();
    
    // Setup state machine
    setupStateMachine(stateMachine);
    
    // Load configuration
    configManager->load();
    
    // Create REST API server
    auto apiServer = std::make_shared<openauto::modern::RestApiServer>(
        8080, eventBus, stateMachine, configManager);
    
    // Start API server
    apiServer->start();
    
    // Connect event bus to state machine
    eventBus->subscribe(openauto::modern::EventType::ANDROID_AUTO_CONNECTED,
        [stateMachine](const openauto::modern::Event::Pointer& event) {
            stateMachine->transitionTo("connected");
        });
    
    eventBus->subscribe(openauto::modern::EventType::ANDROID_AUTO_DISCONNECTED,
        [stateMachine](const openauto::modern::Event::Pointer& event) {
            stateMachine->transitionTo("idle");
        });
    
    // Application main loop
    while (running) {
        // Process events
        eventBus->processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Cleanup
    apiServer->stop();
    configManager->save();
    
    return 0;
}

void setupStateMachine(std::shared_ptr<openauto::modern::StateMachine> stateMachine) {
    // Define application states
    stateMachine->addState("initializing", "System initializing");
    stateMachine->addState("idle", "Waiting for device");
    stateMachine->addState("connected", "Device connected");
    stateMachine->addState("projection", "Android Auto active");
    stateMachine->addState("camera", "Camera view active");
    stateMachine->addState("settings", "Settings menu active");
    stateMachine->addState("error", "Error state");
    
    // Define valid transitions
    stateMachine->addTransition("initializing", "idle", "init_complete");
    stateMachine->addTransition("idle", "connected", "device_connected");
    stateMachine->addTransition("connected", "projection", "start_projection");
    stateMachine->addTransition("projection", "connected", "stop_projection");
    stateMachine->addTransition("connected", "idle", "device_disconnected");
    stateMachine->addTransition("*", "camera", "camera_triggered");
    stateMachine->addTransition("camera", "*", "camera_exit");
    stateMachine->addTransition("*", "settings", "settings_opened");
    stateMachine->addTransition("settings", "*", "settings_closed");
    stateMachine->addTransition("*", "error", "error_occurred");
    
    // Set initial state
    stateMachine->transitionTo("initializing");
}
```

### External Process Integration

#### Python Integration

```python
#!/usr/bin/env python3
"""
External process integration example for OpenAuto
"""

import requests
import json
import time
import websocket
import threading

class OpenAutoClient:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url
        self.ws = None
        
    def publish_event(self, event_type, source, data=None):
        """Publish an event to OpenAuto"""
        event = {
            "type": event_type,
            "source": source,
            "data": data or {}
        }
        
        response = requests.post(
            f"{self.base_url}/api/v1/events",
            headers={"Content-Type": "application/json"},
            json=event
        )
        return response.json()
    
    def get_state(self):
        """Get current state"""
        response = requests.get(f"{self.base_url}/api/v1/state")
        return response.json()
    
    def set_config(self, key, value):
        """Set configuration value"""
        response = requests.put(
            f"{self.base_url}/api/v1/config/{key}",
            headers={"Content-Type": "application/json"},
            json={"value": value}
        )
        return response.json()
    
    def get_config(self, key=None):
        """Get configuration value(s)"""
        url = f"{self.base_url}/api/v1/config"
        if key:
            url += f"/{key}"
        
        response = requests.get(url)
        return response.json()
    
    def start_event_listener(self, callback):
        """Start WebSocket event listener"""
        def on_message(ws, message):
            try:
                event = json.loads(message)
                callback(event)
            except json.JSONDecodeError:
                print(f"Invalid JSON received: {message}")
        
        def on_error(ws, error):
            print(f"WebSocket error: {error}")
        
        def on_close(ws, close_status_code, close_msg):
            print("WebSocket connection closed")
        
        self.ws = websocket.WebSocketApp(
            f"ws://localhost:8080/ws/events",
            on_message=on_message,
            on_error=on_error,
            on_close=on_close
        )
        
        # Start WebSocket in a separate thread
        ws_thread = threading.Thread(target=self.ws.run_forever)
        ws_thread.daemon = True
        ws_thread.start()

# Example usage
if __name__ == "__main__":
    client = OpenAutoClient()
    
    # Event handler
    def handle_event(event):
        print(f"Received event: {event}")
        
        if event.get("type") == "UI_BUTTON_PRESSED":
            button_id = event.get("data", {}).get("button_id")
            if button_id == "volume_up":
                # Increase volume
                current_config = client.get_config("audio.volume")
                current_volume = current_config["data"]["value"]
                new_volume = min(100, current_volume + 10)
                client.set_config("audio.volume", new_volume)
    
    # Start listening for events
    client.start_event_listener(handle_event)
    
    # Publish some events
    client.publish_event("SYSTEM_STARTUP", "external_monitor", {
        "process_id": "monitor_script",
        "version": "1.0.0"
    })
    
    # Monitor state changes
    while True:
        state = client.get_state()
        print(f"Current state: {state['data']['current']}")
        time.sleep(5)
```

#### Node.js Integration

```javascript
// openauto-client.js
const axios = require('axios');
const WebSocket = require('ws');
const EventEmitter = require('events');

class OpenAutoClient extends EventEmitter {
    constructor(baseUrl = 'http://localhost:8080') {
        super();
        this.baseUrl = baseUrl;
        this.ws = null;
    }
    
    async publishEvent(type, source, data = {}) {
        try {
            const response = await axios.post(`${this.baseUrl}/api/v1/events`, {
                type,
                source,
                data
            });
            return response.data;
        } catch (error) {
            throw new Error(`Failed to publish event: ${error.message}`);
        }
    }
    
    async getState() {
        try {
            const response = await axios.get(`${this.baseUrl}/api/v1/state`);
            return response.data;
        } catch (error) {
            throw new Error(`Failed to get state: ${error.message}`);
        }
    }
    
    async setConfig(key, value) {
        try {
            const response = await axios.put(`${this.baseUrl}/api/v1/config/${key}`, {
                value
            });
            return response.data;
        } catch (error) {
            throw new Error(`Failed to set config: ${error.message}`);
        }
    }
    
    startEventListener() {
        this.ws = new WebSocket('ws://localhost:8080/ws/events');
        
        this.ws.on('message', (data) => {
            try {
                const event = JSON.parse(data);
                this.emit('event', event);
            } catch (error) {
                console.error('Invalid JSON received:', data.toString());
            }
        });
        
        this.ws.on('error', (error) => {
            this.emit('error', error);
        });
        
        this.ws.on('close', () => {
            this.emit('disconnect');
        });
    }
}

// Example usage
const client = new OpenAutoClient();

client.on('event', (event) => {
    console.log('Received event:', event);
    
    if (event.type === 'MEDIA_PLAY') {
        console.log('Media started playing');
        // Update UI or trigger other actions
    }
});

client.on('error', (error) => {
    console.error('Client error:', error);
});

// Start listening for events
client.startEventListener();

// Publish events
client.publishEvent('CUSTOM_EVENT', 'nodejs_app', {
    message: 'Hello from Node.js!'
});

module.exports = OpenAutoClient;
```

## Migration from Legacy Code

### Removing /tmp File Usage

Replace file-based communication with event bus:

```cpp
// OLD: Writing to /tmp file
void notifyVolumeChange(int volume) {
    std::ofstream file("/tmp/openauto_volume");
    file << volume;
    file.close();
}

// NEW: Publishing event
void notifyVolumeChange(int volume) {
    auto event = std::make_shared<openauto::modern::Event>(
        openauto::modern::EventType::UI_VOLUME_CHANGED, "audio_manager");
    event->setData("volume", volume);
    eventBus_->publish(event);
}
```

### Replacing Direct Function Calls

Replace tight coupling with event-driven architecture:

```cpp
// OLD: Direct function call
class UIManager {
    CameraManager* cameraManager_;
    
public:
    void onCameraButtonPressed() {
        cameraManager_->showCamera();  // Tight coupling
    }
};

// NEW: Event-driven
class UIManager {
    std::shared_ptr<EventBus> eventBus_;
    
public:
    void onCameraButtonPressed() {
        auto event = std::make_shared<Event>(
            EventType::CAMERA_SHOW, "ui_manager");
        eventBus_->publish(event);  // Loose coupling
    }
};
```

## Best Practices

1. **Event Naming**: Use descriptive, hierarchical event names
2. **Error Handling**: Always handle exceptions in event handlers
3. **State Validation**: Validate state transitions before executing
4. **Configuration**: Use configuration for all user-customizable settings
5. **Logging**: Log all significant events and state changes
6. **Testing**: Write unit tests for event handlers and state transitions

## Troubleshooting

### Common Issues

1. **Event Bus Not Receiving Events**
   - Check if event types match exactly
   - Verify event bus is running
   - Check for exception in event handlers

2. **REST API Not Responding**
   - Verify server is started
   - Check port conflicts
   - Review firewall settings

3. **State Transitions Failing**
   - Verify transition is defined
   - Check current state
   - Review transition conditions

### Debug Tools

```bash
# Check API health
curl http://localhost:8080/health

# List recent events
curl http://localhost:8080/api/v1/events?limit=10

# Get state history
curl http://localhost:8080/api/v1/state/history
```

## Performance Considerations

1. **Event Frequency**: Limit high-frequency events
2. **Handler Complexity**: Keep event handlers lightweight
3. **Memory Usage**: Monitor event queue size
4. **Network Traffic**: Use WebSocket for real-time updates
5. **Threading**: Use thread-safe operations

This integration guide provides the foundation for modernizing existing OpenAuto code and integrating external processes through the event-driven architecture.
