# OpenAuto REST API Documentation

## Overview

The OpenAuto REST API provides a modern, OpenAPI 3.0 compliant interface for interacting with the OpenAuto system. This API enables external processes to send and receive events, manage state transitions, and configure the system remotely.

## Features

- **OpenAPI 3.0 Compliance**: Full OpenAPI specification with Swagger UI documentation
- **Event Management**: Publish and subscribe to system events
- **State Machine Control**: Monitor and control application state
- **Configuration Management**: Get and set configuration values
- **Real-time Updates**: WebSocket support for real-time event streaming
- **CORS Support**: Cross-origin resource sharing for web applications
- **Authentication Ready**: Bearer token authentication support

## Base URL

```
http://localhost:8080
```

## Authentication

The API supports Bearer token authentication. Include the token in the Authorization header:

```
Authorization: Bearer <your-token>
```

## API Endpoints

### Documentation

- `GET /docs` - Interactive Swagger UI documentation
- `GET /openapi.json` - OpenAPI 3.0 specification in JSON format

### Health & Info

- `GET /health` - Health check endpoint
- `GET /info` - API information and version

### Events

- `GET /api/v1/events` - List events with pagination and filtering
- `POST /api/v1/events` - Publish a new event
- `GET /api/v1/events/types` - Get available event types

### State Management

- `GET /api/v1/state` - Get current state
- `POST /api/v1/state/transition` - Trigger state transition
- `GET /api/v1/state/history` - Get state transition history

### Configuration

- `GET /api/v1/config` - Get all configuration
- `GET /api/v1/config/{key}` - Get specific configuration value
- `PUT /api/v1/config/{key}` - Set specific configuration value
- `POST /api/v1/config/save` - Save configuration to file

## Response Format

All API responses follow a consistent format:

### Success Response

```json
{
  "success": true,
  "message": "Operation completed successfully",
  "data": {},
  "timestamp": 1640995200000
}
```

### Error Response

```json
{
  "success": false,
  "error": {
    "code": 400,
    "message": "Bad Request",
    "detail": "Invalid input data"
  },
  "timestamp": 1640995200000
}
```

### Paginated Response

```json
{
  "success": true,
  "data": [],
  "pagination": {
    "page": 1,
    "limit": 50,
    "total": 100,
    "pages": 2
  },
  "timestamp": 1640995200000
}
```

## Event Types

The following event types are supported:

### System Events
- `SYSTEM_STARTUP` - System is starting up
- `SYSTEM_SHUTDOWN` - System is shutting down
- `SYSTEM_REBOOT` - System is rebooting
- `SYSTEM_ERROR` - System error occurred

### Android Auto Events
- `ANDROID_AUTO_CONNECTED` - Android Auto device connected
- `ANDROID_AUTO_DISCONNECTED` - Android Auto device disconnected
- `ANDROID_AUTO_START` - Android Auto session started
- `ANDROID_AUTO_STOP` - Android Auto session stopped

### UI Events
- `UI_BUTTON_PRESSED` - UI button was pressed
- `UI_BRIGHTNESS_CHANGED` - Screen brightness changed
- `UI_VOLUME_CHANGED` - Audio volume changed
- `UI_MODE_CHANGED` - UI mode changed

### Camera Events
- `CAMERA_SHOW` - Camera view shown
- `CAMERA_HIDE` - Camera view hidden
- `CAMERA_RECORD_START` - Recording started
- `CAMERA_RECORD_STOP` - Recording stopped

### Network Events
- `WIFI_CONNECTED` - WiFi connected
- `WIFI_DISCONNECTED` - WiFi disconnected
- `BLUETOOTH_CONNECTED` - Bluetooth device connected
- `BLUETOOTH_DISCONNECTED` - Bluetooth device disconnected

### Media Events
- `MEDIA_PLAY` - Media playback started
- `MEDIA_PAUSE` - Media playback paused
- `MEDIA_STOP` - Media playback stopped
- `MEDIA_NEXT` - Next track
- `MEDIA_PREVIOUS` - Previous track

## Examples

### Publishing an Event

```bash
curl -X POST http://localhost:8080/api/v1/events \
  -H "Content-Type: application/json" \
  -d '{
    "type": "UI_BUTTON_PRESSED",
    "source": "main_ui",
    "data": {
      "button_id": "home",
      "screen": "main_menu"
    }
  }'
```

### Getting Current State

```bash
curl http://localhost:8080/api/v1/state
```

### Setting Configuration

```bash
curl -X PUT http://localhost:8080/api/v1/config/ui.brightness \
  -H "Content-Type: application/json" \
  -d '{"value": 75}'
```

## Error Codes

- `200` - OK
- `201` - Created
- `400` - Bad Request
- `401` - Unauthorized
- `404` - Not Found
- `500` - Internal Server Error
- `503` - Service Unavailable

## Rate Limiting

The API implements rate limiting to prevent abuse:
- **Events**: 100 requests per minute
- **Configuration**: 50 requests per minute
- **State**: 200 requests per minute

## WebSocket Support

For real-time event streaming, connect to the WebSocket endpoint:

```
ws://localhost:8080/ws/events
```

## Integration Examples

### JavaScript/Web

```javascript
// Publish an event
const response = await fetch('/api/v1/events', {
  method: 'POST',
  headers: {
    'Content-Type': 'application/json',
  },
  body: JSON.stringify({
    type: 'CUSTOM_EVENT',
    source: 'web_app',
    data: { message: 'Hello from web!' }
  })
});

// Get current state
const state = await fetch('/api/v1/state').then(r => r.json());
console.log('Current state:', state.data.current);
```

### Python

```python
import requests
import json

# Publish an event
event_data = {
    "type": "MEDIA_PLAY",
    "source": "python_script",
    "data": {"track": "song.mp3"}
}

response = requests.post(
    'http://localhost:8080/api/v1/events',
    headers={'Content-Type': 'application/json'},
    json=event_data
)

# Get configuration
config = requests.get('http://localhost:8080/api/v1/config').json()
print("Current config:", config['data'])
```

### Shell/Bash

```bash
#!/bin/bash

# Function to publish event
publish_event() {
    local event_type="$1"
    local source="$2"
    local data="$3"
    
    curl -s -X POST http://localhost:8080/api/v1/events \
        -H "Content-Type: application/json" \
        -d "{
            \"type\": \"$event_type\",
            \"source\": \"$source\",
            \"data\": $data
        }"
}

# Example usage
publish_event "SYSTEM_STARTUP" "boot_script" '{"boot_time": 30}'
```

## Security Considerations

1. **Authentication**: Implement proper authentication in production
2. **HTTPS**: Use HTTPS in production environments
3. **Input Validation**: All inputs are validated and sanitized
4. **Rate Limiting**: Prevents abuse and DoS attacks
5. **CORS**: Configure CORS policies appropriately

## Monitoring and Logging

The API provides comprehensive logging and monitoring:

- All requests are logged with timestamps
- Error responses include correlation IDs
- Health check endpoint for monitoring
- Metrics endpoint for performance monitoring

## Development

To start the development server:

```bash
# Build the project
mkdir build && cd build
cmake ..
make

# Run the server
./openauto --enable-rest-api --api-port 8080
```

## Support

For questions and support:
- GitHub Issues: [OpenCarDev/OpenAuto](https://github.com/opencardev/openauto)
- Documentation: [API Docs](http://localhost:8080/docs)
