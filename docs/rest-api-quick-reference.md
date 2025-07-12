# OpenAuto REST API Quick Reference

## Server Status
```bash
# Health check
curl http://localhost:8080/api/health

# Server info
curl http://localhost:8080/api/info
```

## Documentation Access
| Resource | URL | Description |
|----------|-----|-------------|
| **Swagger UI** | `http://localhost:8080/docs` | Interactive API documentation |
| **ReDoc** | `http://localhost:8080/redoc` | Alternative API documentation |
| **OpenAPI Spec** | `http://localhost:8080/openapi.json` | Raw OpenAPI 3.0 specification |

## Core Endpoints

### Configuration
```bash
# Get all configuration
curl http://localhost:8080/api/config

# Get specific config value
curl http://localhost:8080/api/config/ui.brightness

# Update configuration
curl -X PUT http://localhost:8080/api/config/ui.brightness \
  -H "Content-Type: application/json" \
  -d '{"value": 75}'

# Reset configuration
curl -X DELETE http://localhost:8080/api/config/ui.brightness
```

### State Management
```bash
# Get current state
curl http://localhost:8080/api/state

# Trigger state transition
curl -X POST http://localhost:8080/api/state/transition \
  -H "Content-Type: application/json" \
  -d '{"state": "connected"}'

# Get state history
curl "http://localhost:8080/api/state/history?limit=10"
```

### Event System
```bash
# Get event status
curl http://localhost:8080/api/events/status

# Publish event
curl -X POST http://localhost:8080/api/events \
  -H "Content-Type: application/json" \
  -d '{"type": "ui.brightness.changed", "data": {"brightness": 75}}'

# Get event history
curl "http://localhost:8080/api/events/history?limit=10"

# Clear event queue
curl -X DELETE http://localhost:8080/api/events/queue
```

### Logging
```bash
# Get recent logs
curl "http://localhost:8080/api/logs?limit=50"

# Get logs by level
curl "http://localhost:8080/api/logs?level=ERROR&limit=20"

# Get logs by category
curl "http://localhost:8080/api/logs?category=API&limit=20"

# Change log level
curl -X PUT http://localhost:8080/api/logs/level \
  -H "Content-Type: application/json" \
  -d '{"level": "DEBUG"}'

# Get log statistics
curl http://localhost:8080/api/logs/stats
```

## Authentication (if enabled)
```bash
# Access protected endpoint
curl -H "Authorization: Bearer your_token_here" \
  http://localhost:8080/api/admin/status

# Login (if implemented)
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username": "admin", "password": "password"}'
```

## Testing Commands

### Basic Health Check
```bash
#!/bin/bash
response=$(curl -s -w "%{http_code}" http://localhost:8080/api/health)
http_code="${response: -3}"
body="${response%???}"

if [ "$http_code" = "200" ]; then
  echo "✅ API Server is healthy"
  echo "$body" | jq .
else
  echo "❌ API Server is not responding (HTTP $http_code)"
fi
```

### Full Endpoint Test
```bash
#!/bin/bash
endpoints=(
  "/api/health"
  "/api/config"
  "/api/state"
  "/api/events/status"
  "/api/logs?limit=5"
)

for endpoint in "${endpoints[@]}"; do
  response=$(curl -s -w "%{http_code}" "http://localhost:8080$endpoint")
  http_code="${response: -3}"
  
  if [ "$http_code" = "200" ]; then
    echo "✅ $endpoint"
  else
    echo "❌ $endpoint (HTTP $http_code)"
  fi
done
```

## Common HTTP Status Codes
| Code | Meaning | Common Causes |
|------|---------|---------------|
| **200** | OK | Request successful |
| **201** | Created | Resource created successfully |
| **400** | Bad Request | Invalid JSON or parameters |
| **401** | Unauthorized | Missing or invalid authentication |
| **403** | Forbidden | Insufficient permissions |
| **404** | Not Found | Endpoint or resource doesn't exist |
| **405** | Method Not Allowed | Wrong HTTP method |
| **429** | Too Many Requests | Rate limit exceeded |
| **500** | Internal Server Error | Server-side error |
| **503** | Service Unavailable | Server overloaded or maintenance |

## Error Response Format
```json
{
  "error": {
    "code": "VALIDATION_ERROR",
    "message": "Invalid request data",
    "details": {
      "field": "brightness",
      "issue": "Value must be between 0 and 100"
    },
    "timestamp": "2024-01-01T12:00:00Z",
    "request_id": "req_123456"
  }
}
```

## Configuration Examples

### Common Configuration Keys
| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `ui.brightness` | integer | 50 | Screen brightness (0-100) |
| `audio.volume` | integer | 50 | Audio volume (0-100) |
| `display.orientation` | string | "landscape" | Display orientation |
| `network.wifi.enabled` | boolean | true | WiFi enable status |
| `android.auto.enabled` | boolean | true | Android Auto enable status |

### Update Examples
```bash
# Set brightness
curl -X PUT http://localhost:8080/api/config/ui.brightness \
  -H "Content-Type: application/json" \
  -d '{"value": 75}'

# Set volume
curl -X PUT http://localhost:8080/api/config/audio.volume \
  -H "Content-Type: application/json" \
  -d '{"value": 80}'

# Enable/disable feature
curl -X PUT http://localhost:8080/api/config/network.wifi.enabled \
  -H "Content-Type: application/json" \
  -d '{"value": false}'
```

## Debugging Tips

### Server Not Responding
1. Check if process is running: `ps aux | grep autoapp`
2. Check port binding: `netstat -tlnp | grep 8080`
3. Check firewall: `iptables -L | grep 8080`
4. Check logs: `journalctl -u autoapp -f`

### API Errors
1. Enable debug logging: `curl -X PUT http://localhost:8080/api/logs/level -d '{"level": "DEBUG"}'`
2. Check request format: Ensure Content-Type is `application/json`
3. Validate JSON: Use `jq` to check JSON syntax
4. Check authentication: Verify Bearer token format

### Performance Issues
1. Monitor response times: `curl -w "%{time_total}\n" -o /dev/null -s http://localhost:8080/api/health`
2. Check server load: `htop` or `top`
3. Monitor network: `iotop` or `nethogs`
4. Check logs for errors: `curl http://localhost:8080/api/logs?level=ERROR`

## Development Tools

### HTTPie (Alternative to curl)
```bash
# Install HTTPie
pip install httpie

# GET request
http GET localhost:8080/api/health

# POST request
http POST localhost:8080/api/events type=test data:='{"message": "hello"}'

# PUT request with authentication
http PUT localhost:8080/api/config/ui.brightness value:=75 Authorization:"Bearer token"
```

### jq (JSON processing)
```bash
# Pretty print JSON
curl -s http://localhost:8080/api/config | jq .

# Extract specific field
curl -s http://localhost:8080/api/config | jq '.ui.brightness'

# Filter logs by level
curl -s "http://localhost:8080/api/logs?limit=100" | jq '.logs[] | select(.level == "ERROR")'
```

### Watch (Live monitoring)
```bash
# Monitor health endpoint
watch -n 1 'curl -s http://localhost:8080/api/health | jq .'

# Monitor log count
watch -n 1 'curl -s http://localhost:8080/api/logs/stats | jq .total_logs'

# Monitor event queue
watch -n 1 'curl -s http://localhost:8080/api/events/status | jq .queue_size'
```
