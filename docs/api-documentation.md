# OpenAuto API Documentation

## Overview

OpenAuto's modern architecture provides a comprehensive REST API for managing and monitoring the system. This API follows OpenAPI 3.0 standards and provides access to configuration, events, state management, and logging functionality.

## Base Information

- **Base URL**: `http://localhost:8080/api/v1`
- **Authentication**: Optional (configurable)
- **Content Type**: `application/json`
- **API Version**: 1.0
- **Protocol**: HTTP/HTTPS

## Authentication

### API Key Authentication (Optional)
```bash
# Configure API key in openauto.conf
[rest_api]
auth_required = true
api_key = "your-secure-api-key-here"

# Use API key in requests
curl -H "X-API-Key: your-secure-api-key-here" http://localhost:8080/api/v1/status
```

### Basic Authentication (Optional)
```bash
# Configure basic auth
[rest_api]
auth_type = basic
auth_username = admin
auth_password = secure_password

# Use basic auth
curl -u admin:secure_password http://localhost:8080/api/v1/status
```

## Events API

### Get Event History
Get a list of recent events from the event bus.

**Endpoint**: `GET /api/v1/events`

**Query Parameters**:
- `limit` (optional, default: 50): Maximum number of events to return
- `offset` (optional, default: 0): Offset for pagination
- `type` (optional): Filter events by type
- `category` (optional): Filter by event category
- `since` (optional): ISO timestamp to get events since a specific time

**Response**:
```json
{
  "status": "success",
  "events": [
    {
      "id": "event_12345",
      "type": "ANDROID_AUTO_CONNECTED",
      "category": "ANDROID_AUTO",
      "timestamp": "2025-07-12T10:30:15Z",
      "source": "AndroidAutoManager",
      "data": {
        "device_id": "ABC123",
        "vendor": "Google",
        "connection_type": "usb"
      },
      "priority": "HIGH"
    }
  ],
  "pagination": {
    "total": 150,
    "limit": 50,
    "offset": 0,
    "has_more": true
  }
}
```

### Publish Event
Publish a new event to the event bus.

**Endpoint**: `POST /api/v1/events`

**Request Body**:
```json
{
  "type": "CUSTOM_EVENT",
  "category": "USER",
  "data": {
    "button_id": "home_button",
    "user_action": "press",
    "screen": "main_menu"
  },
  "priority": "MEDIUM"
}
```

**Response**:
```json
{
  "status": "success",
  "event_id": "event_12347",
  "message": "Event published successfully"
}
```

### Get Events by Type
Get events of a specific type.

**Endpoint**: `GET /api/events/{type}`

**Response**: Same format as event history with filtered results.

## State Management API

### Get Current State
Get the current system state and available transitions.

**Endpoint**: `GET /api/state`

**Response**:
```json
{
  "current_state": "IDLE",
  "valid_transitions": [
    "ANDROID_AUTO_CONNECT",
    "CAMERA_BUTTON_PRESS",
    "SETTINGS_BUTTON_PRESS"
  ],
  "state_history": [
    {
      "state": "INITIALIZING",
      "timestamp": 1641234560000
    },
    {
      "state": "IDLE",
      "timestamp": 1641234567890
    }
  ]
}
```

### Trigger State Transition
Trigger a state transition using a specific trigger.

**Endpoint**: `POST /api/state/transition`

**Request Body**:
```json
{
  "trigger": "ANDROID_AUTO_CONNECT"
}
```

**Response**:
```json
{
  "success": true,
  "old_state": "IDLE",
  "new_state": "ANDROID_AUTO_ACTIVE",
  "trigger": "ANDROID_AUTO_CONNECT",
  "timestamp": 1641234567890
}
```

### Get Valid Transitions
Get all valid transitions from the current state.

**Endpoint**: `GET /api/state/transitions`

**Response**:
```json
{
  "current_state": "IDLE",
  "valid_transitions": [
    {
      "trigger": "ANDROID_AUTO_CONNECT",
      "target_state": "ANDROID_AUTO_ACTIVE",
      "description": "Connect to Android Auto device"
    },
    {
      "trigger": "CAMERA_BUTTON_PRESS",
      "target_state": "CAMERA_VIEW",
      "description": "Show camera view"
    }
  ]
}
```

## Configuration API

### Get Configuration
Get all configuration values or a specific value.

**Endpoint**: `GET /api/config`

**Query Parameters**:
- `key` (optional): Get specific configuration key

**Response**:
```json
{
  "config": {
    "audio.volume": 50,
    "audio.muted": false,
    "video.brightness": 75,
    "system.language": "en_US"
  },
  "timestamp": 1641234567890
}
```

### Update Configuration
Update configuration values.

**Endpoint**: `POST /api/config`

**Request Body**:
```json
{
  "audio.volume": 60,
  "video.brightness": 80
}
```

**Response**:
```json
{
  "success": true,
  "updated_keys": ["audio.volume", "video.brightness"],
  "timestamp": 1641234567890
}
```

### Set Configuration Value
Set a specific configuration value.

**Endpoint**: `PUT /api/config/{key}`

**Request Body**:
```json
{
  "value": 75
}
```

**Response**:
```json
{
  "success": true,
  "key": "audio.volume",
  "old_value": 60,
  "new_value": 75,
  "timestamp": 1641234567890
}
```

### Delete Configuration Value
Remove a configuration value.

**Endpoint**: `DELETE /api/config/{key}`

**Response**:
```json
{
  "success": true,
  "key": "custom.setting",
  "deleted": true,
  "timestamp": 1641234567890
}
```

## System API

### API Documentation
Get this API documentation.

**Endpoint**: `GET /api`

**Response**: Returns this documentation in JSON format.

### Health Check
Check system health and status.

**Endpoint**: `GET /api/health`

**Response**:
```json
{
  "status": "healthy",
  "uptime": 3600,
  "components": {
    "event_bus": "running",
    "state_machine": "active",
    "config_manager": "loaded"
  },
  "memory_usage": {
    "event_history_size": 150,
    "subscribers_count": 12
  },
  "timestamp": 1641234567890
}
```

## Event Types

### System Events
- `SYSTEM_STARTUP`: System initialization complete
- `SYSTEM_SHUTDOWN`: System shutdown initiated
- `SYSTEM_REBOOT`: System reboot initiated
- `SYSTEM_ERROR`: System error occurred

### Android Auto Events
- `ANDROID_AUTO_CONNECTED`: Android Auto device connected
- `ANDROID_AUTO_DISCONNECTED`: Android Auto device disconnected
- `ANDROID_AUTO_START`: Android Auto projection started
- `ANDROID_AUTO_STOP`: Android Auto projection stopped
- `ANDROID_AUTO_PAUSE`: Android Auto projection paused
- `ANDROID_AUTO_RESUME`: Android Auto projection resumed

### UI Events
- `UI_BUTTON_PRESSED`: UI button was pressed
- `UI_BRIGHTNESS_CHANGED`: Screen brightness changed
- `UI_VOLUME_CHANGED`: Audio volume changed
- `UI_MODE_CHANGED`: UI mode changed (day/night)
- `UI_SCREEN_TOUCH`: Touchscreen interaction

### Camera Events
- `CAMERA_SHOW`: Camera view displayed
- `CAMERA_HIDE`: Camera view hidden
- `CAMERA_RECORD_START`: Camera recording started
- `CAMERA_RECORD_STOP`: Camera recording stopped
- `CAMERA_SAVE`: Camera image/video saved

### Network Events
- `WIFI_CONNECTED`: WiFi connection established
- `WIFI_DISCONNECTED`: WiFi connection lost
- `HOTSPOT_ENABLED`: WiFi hotspot enabled
- `HOTSPOT_DISABLED`: WiFi hotspot disabled
- `BLUETOOTH_CONNECTED`: Bluetooth device connected
- `BLUETOOTH_DISCONNECTED`: Bluetooth device disconnected
- `BLUETOOTH_PAIRING_REQUEST`: Bluetooth pairing requested

### Media Events
- `MEDIA_PLAY`: Media playback started
- `MEDIA_PAUSE`: Media playback paused
- `MEDIA_STOP`: Media playback stopped
- `MEDIA_NEXT`: Next track requested
- `MEDIA_PREVIOUS`: Previous track requested
- `MEDIA_TRACK_CHANGED`: Current track changed

### Configuration Events
- `CONFIG_CHANGED`: Configuration value changed
- `CONFIG_SAVED`: Configuration saved to disk

## System States

- `INITIALIZING`: System is starting up
- `IDLE`: System ready, no active projection
- `ANDROID_AUTO_ACTIVE`: Android Auto projection active
- `CAMERA_VIEW`: Camera view displayed
- `SETTINGS`: Settings interface displayed
- `BLUETOOTH_PAIRING`: Bluetooth pairing in progress
- `WIFI_SETUP`: WiFi setup in progress
- `UPDATING`: System update in progress
- `SHUTTING_DOWN`: System shutdown in progress
- `ERROR_STATE`: System error state

## Example Usage

### JavaScript/Node.js Example
```javascript
// Publish an event
const response = await fetch('http://localhost:8080/api/events', {
  method: 'POST',
  headers: {
    'Content-Type': 'application/json',
  },
  body: JSON.stringify({
    type: 'UI_BUTTON_PRESSED',
    source: 'web_interface',
    data: {
      button: 'camera',
      user: 'admin'
    }
  })
});

// Get current state
const stateResponse = await fetch('http://localhost:8080/api/state');
const state = await stateResponse.json();
console.log('Current state:', state.current_state);

// Update configuration
await fetch('http://localhost:8080/api/config', {
  method: 'POST',
  headers: {
    'Content-Type': 'application/json',
  },
  body: JSON.stringify({
    'audio.volume': 75,
    'video.brightness': 85
  })
});
```

### Python Example
```python
import requests
import json

# Publish an event
event_data = {
    'type': 'CAMERA_SHOW',
    'source': 'python_script',
    'data': {
        'trigger': 'external_button'
    }
}

response = requests.post('http://localhost:8080/api/events', 
                        json=event_data)
print(f"Event published: {response.json()}")

# Get configuration
config_response = requests.get('http://localhost:8080/api/config')
config = config_response.json()
print(f"Current volume: {config['config']['audio.volume']}")
```

### curl Examples
```bash
# Get current state
curl http://localhost:8080/api/state

# Publish event
curl -X POST http://localhost:8080/api/events \
  -H "Content-Type: application/json" \
  -d '{"type":"UI_BUTTON_PRESSED","source":"curl","data":{"button":"settings"}}'

# Update configuration
curl -X POST http://localhost:8080/api/config \
  -H "Content-Type: application/json" \
  -d '{"audio.volume":80,"video.brightness":90}'

# Get event history
curl "http://localhost:8080/api/events?limit=50&type=ANDROID_AUTO_CONNECTED"
```
