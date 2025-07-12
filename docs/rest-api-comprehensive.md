# OpenAuto REST API Documentation

This document provides comprehensive documentation for the OpenAuto REST API server implementation.

## Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Quick Start](#quick-start)
4. [API Endpoints](#api-endpoints)
5. [Authentication](#authentication)
6. [Configuration](#configuration)
7. [Examples](#examples)
8. [OpenAPI Integration](#openapi-integration)
9. [Debugging](#debugging)
10. [Troubleshooting](#troubleshooting)

## Overview

The OpenAuto REST API server provides a modern, OpenAPI 3.0 compliant interface for interacting with the OpenAuto Android Auto implementation. It offers comprehensive functionality including:

- **OpenAPI 3.0 Specification**: Automatic documentation generation with Swagger UI and ReDoc
- **Modern Architecture Integration**: Seamless integration with EventBus, StateMachine, and ConfigurationManager
- **Security**: Pluggable authentication, authorization, and CORS support
- **Middleware Pipeline**: Extensible request/response processing
- **JSON-First**: Native JSON request/response handling
- **Thread-Safe**: Concurrent request handling with proper synchronization

### Key Features

- ✅ OpenAPI 3.0 compliant endpoint definitions
- ✅ Automatic Swagger UI documentation at `/docs`
- ✅ ReDoc documentation at `/redoc`
- ✅ Middleware support for cross-cutting concerns
- ✅ Path parameter extraction (`/users/{id}`)
- ✅ Query parameter handling
- ✅ JSON request/response serialization
- ✅ CORS support for web applications
- ✅ Bearer token authentication
- ✅ Route-based authorization
- ✅ Integration with OpenAuto's modern logging system

## Architecture

### Components

```
┌─────────────────────────────────────────────────┐
│                 REST API Server                 │
├─────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌──────────────┐  ┌─────────┐ │
│  │   Routes    │  │  Middleware  │  │  CORS   │ │
│  └─────────────┘  └──────────────┘  └─────────┘ │
│  ┌─────────────┐  ┌──────────────┐  ┌─────────┐ │
│  │    Auth     │  │   OpenAPI    │  │  JSON   │ │
│  └─────────────┘  └──────────────┘  └─────────┘ │
├─────────────────────────────────────────────────┤
│              cpp-httplib Server                 │
├─────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌──────────────┐  ┌─────────┐ │
│  │  EventBus   │  │ StateMachine │  │ Config  │ │
│  └─────────────┘  └──────────────┘  └─────────┘ │
└─────────────────────────────────────────────────┘
```

### Class Hierarchy

- `RestApiServer`: Main server class with lifecycle management
- `HttpRequest`: Request wrapper with convenient data access methods
- `HttpResponse`: Response builder with fluent interface
- `Route`: Route definition with handler and OpenAPI metadata
- `ApiOperation`: OpenAPI operation specification
- `ApiParameter`: Parameter definition with validation rules
- `ApiResponse`: Response specification with examples

## Quick Start

### Basic Server Setup

```cpp
#include "modern/RestApiServer.hpp"
#include "modern/Logger.hpp"

// Initialize dependencies (normally done by main application)
auto eventBus = std::make_shared<EventBus>();
auto stateMachine = std::make_shared<StateMachine>();
auto configManager = std::make_shared<ConfigurationManager>();

// Create server instance
auto server = std::make_shared<RestApiServer>(8080, eventBus, stateMachine, configManager);

// Configure API information
ApiInfo apiInfo;
apiInfo.title = "OpenAuto REST API";
apiInfo.version = "1.0.0";
apiInfo.description = "REST API for OpenAuto Android Auto implementation";
apiInfo.contact.name = "OpenCarDev Team";
apiInfo.contact.url = "https://github.com/opencardev/openauto";
apiInfo.license.name = "GPL-3.0";
server->setApiInfo(apiInfo);

// Add a simple health check endpoint
server->addRoute(HttpMethod::GET, "/api/health", [](const HttpRequest& req) {
    HttpResponse res;
    res.setJson({
        {"status", "healthy"},
        {"timestamp", std::time(nullptr)},
        {"version", "1.0.0"}
    });
    return res;
});

// Enable documentation
server->enableSwaggerUI("/docs");
server->enableReDoc("/redoc");

// Start the server
if (server->start()) {
    Logger::getInstance().info(LogCategory::API, "Main", "main", __FILE__, __LINE__,
                              "REST API server started on http://localhost:8080");
} else {
    Logger::getInstance().error(LogCategory::API, "Main", "main", __FILE__, __LINE__,
                               "Failed to start REST API server");
}
```

### Adding Routes with OpenAPI Documentation

```cpp
// Define parameter
ApiParameter userIdParam;
userIdParam.name = "userId";
userIdParam.type = ParameterType::INTEGER;
userIdParam.in = ParameterIn::PATH;
userIdParam.required = true;
userIdParam.description = "Unique identifier for the user";
userIdParam.example = "12345";

// Define responses
ApiResponse successResponse;
successResponse.statusCode = 200;
successResponse.description = "User retrieved successfully";
successResponse.example = R"({"id": 12345, "name": "John Doe", "email": "john@example.com"})";

ApiResponse notFoundResponse;
notFoundResponse.statusCode = 404;
notFoundResponse.description = "User not found";
notFoundResponse.example = R"({"error": "User with ID 12345 not found"})";

// Define operation
ApiOperation getUserOp;
getUserOp.operationId = "getUser";
getUserOp.summary = "Get user by ID";
getUserOp.description = "Retrieves detailed information about a specific user";
getUserOp.tags = {"Users"};
getUserOp.parameters = {userIdParam};
getUserOp.responses = {successResponse, notFoundResponse};

// Add route with operation
server->addRoute(HttpMethod::GET, "/api/users/{userId}", [](const HttpRequest& req) {
    std::string userId = req.getPathParam("userId");
    
    HttpResponse res;
    if (userId.empty()) {
        res.setStatus(400);
        res.setJson({{"error", "User ID is required"}});
    } else if (userId == "12345") {
        res.setJson({
            {"id", 12345},
            {"name", "John Doe"},
            {"email", "john@example.com"},
            {"created", "2023-01-15T10:30:00Z"}
        });
    } else {
        res.setStatus(404);
        res.setJson({{"error", "User with ID " + userId + " not found"}});
    }
    return res;
}, getUserOp);
```

## API Endpoints

### Core System Endpoints

#### GET /api/health
Returns system health status and basic information.

**Response:**
```json
{
  "status": "healthy",
  "timestamp": 1641988200,
  "version": "1.0.0",
  "uptime": 3600
}
```

#### GET /api/info
Returns detailed system information and capabilities.

#### GET /api/version
Returns version information for all components.

### Configuration Endpoints

#### GET /api/config
Returns current configuration settings.

#### PUT /api/config
Updates configuration settings.

#### POST /api/config/reload
Reloads configuration from files.

### State Management Endpoints

#### GET /api/state
Returns current system state.

#### POST /api/state/transition
Triggers a state transition.

### Event System Endpoints

#### GET /api/events/stream
Server-Sent Events stream for real-time updates.

#### POST /api/events/emit
Emits a custom event to the event bus.

### Documentation Endpoints

#### GET /docs
Swagger UI documentation interface.

#### GET /redoc
ReDoc documentation interface.

#### GET /api/openapi.json
OpenAPI 3.0 specification in JSON format.

## Authentication

### Bearer Token Authentication

```cpp
// Set up authentication handler
server->setAuthenticationHandler([](const HttpRequest& req) {
    std::string authHeader = req.getHeader("Authorization");
    if (!authHeader.starts_with("Bearer ")) {
        return false;
    }
    
    std::string token = authHeader.substr(7);
    return validateToken(token); // Your token validation logic
});

// Require authentication for specific paths
server->requireAuthentication("/api/config");
server->requireAuthentication("/api/state");
```

### API Key Authentication

```cpp
// API key in header
server->setAuthenticationHandler([](const HttpRequest& req) {
    std::string apiKey = req.getHeader("X-API-Key");
    return isValidApiKey(apiKey);
});

// API key in query parameter
server->setAuthenticationHandler([](const HttpRequest& req) {
    std::string apiKey = req.getQuery("api_key");
    return isValidApiKey(apiKey);
});
```

## Configuration

### Server Configuration

```cpp
// Basic configuration
server->setPort(8080);
server->setBindAddress("0.0.0.0"); // Bind to all interfaces

// Security configuration
server->enableCors({"http://localhost:3000", "https://myapp.com"});
server->setCorsHeaders({
    {"Access-Control-Max-Age", "86400"},
    {"Access-Control-Allow-Credentials", "true"}
});
```

### Middleware Configuration

```cpp
// Global logging middleware
server->addGlobalMiddleware([](HttpRequest& req, HttpResponse& res) {
    auto start = std::chrono::high_resolution_clock::now();
    
    Logger::getInstance().info(LogCategory::API, "Middleware", "request", __FILE__, __LINE__,
                              "Request: " + req.getPath());
    
    // Continue processing
    return true;
});

// Authentication middleware for specific routes
server->addRouteMiddleware("/api/admin", [](HttpRequest& req, HttpResponse& res) {
    if (!req.hasHeader("Authorization")) {
        res.setStatus(401);
        res.setJson({{"error", "Authorization required"}});
        return false; // Stop processing
    }
    return true; // Continue
});
```

## Examples

### Complete Example Application

```cpp
#include "modern/RestApiServer.hpp"
#include "modern/Logger.hpp"
#include <chrono>
#include <thread>

class ApiExample {
private:
    std::shared_ptr<RestApiServer> server_;
    std::shared_ptr<EventBus> eventBus_;
    std::shared_ptr<StateMachine> stateMachine_;
    std::shared_ptr<ConfigurationManager> configManager_;

public:
    ApiExample() {
        // Initialize dependencies
        eventBus_ = std::make_shared<EventBus>();
        stateMachine_ = std::make_shared<StateMachine>();
        configManager_ = std::make_shared<ConfigurationManager>();
        
        // Create server
        server_ = std::make_shared<RestApiServer>(8080, eventBus_, stateMachine_, configManager_);
        
        setupApiInfo();
        setupMiddleware();
        setupRoutes();
        setupDocumentation();
    }
    
    void setupApiInfo() {
        ApiInfo info;
        info.title = "OpenAuto Example API";
        info.version = "1.0.0";
        info.description = "Example REST API demonstrating OpenAuto capabilities";
        info.contact.name = "OpenCarDev Team";
        info.contact.email = "dev@opencardev.org";
        info.license.name = "GPL-3.0";
        info.license.url = "https://www.gnu.org/licenses/gpl-3.0.html";
        server_->setApiInfo(info);
        
        ServerInfo serverInfo;
        serverInfo.url = "http://localhost:8080";
        serverInfo.description = "Development server";
        server_->addServer(serverInfo);
    }
    
    void setupMiddleware() {
        // Request logging
        server_->addGlobalMiddleware([](HttpRequest& req, HttpResponse& res) {
            Logger::getInstance().info(LogCategory::API, "Middleware", "request", __FILE__, __LINE__,
                                      "Request: " + std::to_string(static_cast<int>(req.getMethod())) + 
                                      " " + req.getPath());
            return true;
        });
        
        // CORS
        server_->enableCors({"*"});
    }
    
    void setupRoutes() {
        // Health check
        setupHealthRoute();
        setupUsersRoutes();
        setupConfigRoutes();
        setupEventRoutes();
    }
    
    void setupHealthRoute() {
        ApiOperation healthOp;
        healthOp.operationId = "getHealth";
        healthOp.summary = "Health check";
        healthOp.description = "Returns system health status";
        healthOp.tags = {"System"};
        
        ApiResponse okResponse;
        okResponse.statusCode = 200;
        okResponse.description = "System is healthy";
        okResponse.example = R"({"status":"healthy","timestamp":1641988200})";
        healthOp.responses = {okResponse};
        
        server_->addRoute(HttpMethod::GET, "/api/health", [](const HttpRequest& req) {
            HttpResponse res;
            res.setJson({
                {"status", "healthy"},
                {"timestamp", std::time(nullptr)},
                {"uptime", getUptime()}
            });
            return res;
        }, healthOp);
    }
    
    void setupUsersRoutes() {
        // GET /api/users/{id}
        ApiParameter userIdParam;
        userIdParam.name = "id";
        userIdParam.type = ParameterType::INTEGER;
        userIdParam.in = ParameterIn::PATH;
        userIdParam.required = true;
        userIdParam.description = "User ID";
        
        ApiOperation getUserOp;
        getUserOp.operationId = "getUser";
        getUserOp.summary = "Get user by ID";
        getUserOp.tags = {"Users"};
        getUserOp.parameters = {userIdParam};
        
        server_->addRoute(HttpMethod::GET, "/api/users/{id}", [](const HttpRequest& req) {
            std::string userId = req.getPathParam("id");
            HttpResponse res;
            
            if (userId == "1") {
                res.setJson({
                    {"id", 1},
                    {"name", "Admin User"},
                    {"email", "admin@openauto.org"},
                    {"role", "administrator"}
                });
            } else {
                res.setStatus(404);
                res.setJson({{"error", "User not found"}});
            }
            return res;
        }, getUserOp);
    }
    
    void setupConfigRoutes() {
        // GET /api/config
        server_->addRoute(HttpMethod::GET, "/api/config", [this](const HttpRequest& req) {
            HttpResponse res;
            // Use configManager_ to get actual configuration
            res.setJson({
                {"video", {{"resolution", "1920x1080"}, {"fps", 30}}},
                {"audio", {{"volume", 75}, {"muted", false}}},
                {"bluetooth", {{"enabled", true}, {"discoverable", false}}}
            });
            return res;
        });
        
        // PUT /api/config
        server_->addRoute(HttpMethod::PUT, "/api/config", [this](const HttpRequest& req) {
            HttpResponse res;
            try {
                auto config = req.getJsonBody();
                // Use configManager_ to update configuration
                res.setJson({{"status", "Configuration updated successfully"}});
            } catch (const std::exception& e) {
                res.setStatus(400);
                res.setJson({{"error", "Invalid JSON in request body"}});
            }
            return res;
        });
    }
    
    void setupEventRoutes() {
        // POST /api/events
        server_->addRoute(HttpMethod::POST, "/api/events", [this](const HttpRequest& req) {
            HttpResponse res;
            try {
                auto eventData = req.getJsonBody();
                std::string eventType = eventData["type"];
                
                // Use eventBus_ to emit event
                res.setJson({{"status", "Event emitted successfully"}});
            } catch (const std::exception& e) {
                res.setStatus(400);
                res.setJson({{"error", "Invalid event data"}});
            }
            return res;
        });
    }
    
    void setupDocumentation() {
        server_->enableSwaggerUI("/docs");
        server_->enableReDoc("/redoc");
    }
    
    void start() {
        if (server_->start()) {
            Logger::getInstance().info(LogCategory::API, "ApiExample", "start", __FILE__, __LINE__,
                                      "API server started successfully");
            Logger::getInstance().info(LogCategory::API, "ApiExample", "start", __FILE__, __LINE__,
                                      "Documentation: http://localhost:8080/docs");
        } else {
            Logger::getInstance().error(LogCategory::API, "ApiExample", "start", __FILE__, __LINE__,
                                       "Failed to start API server");
        }
    }
    
    void stop() {
        server_->stop();
    }
    
private:
    static int64_t getUptime() {
        static auto startTime = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
    }
};

// Usage
int main() {
    ApiExample api;
    api.start();
    
    // Keep running
    std::this_thread::sleep_for(std::chrono::hours(24));
    
    api.stop();
    return 0;
}
```

## OpenAPI Integration

### Specification Generation

The server automatically generates OpenAPI 3.0 specifications from route definitions:

```cpp
// Get OpenAPI spec as JSON string
std::string spec = server->getOpenApiSpec();

// The spec includes:
// - API info (title, version, description, contact, license)
// - Server information
// - All registered routes with their operations
// - Parameter definitions with types and validation
// - Response definitions with examples
// - Security schemes
```

### Example Generated Specification

```json
{
  "openapi": "3.0.0",
  "info": {
    "title": "OpenAuto REST API",
    "version": "1.0.0",
    "description": "REST API for OpenAuto Android Auto implementation",
    "contact": {
      "name": "OpenCarDev Team",
      "url": "https://github.com/opencardev/openauto"
    },
    "license": {
      "name": "GPL-3.0"
    }
  },
  "servers": [
    {
      "url": "http://localhost:8080",
      "description": "Development server"
    }
  ],
  "paths": {
    "/api/health": {
      "get": {
        "operationId": "getHealth",
        "summary": "Health check",
        "description": "Returns system health status",
        "tags": ["System"],
        "responses": {
          "200": {
            "description": "System is healthy",
            "content": {
              "application/json": {
                "example": {
                  "status": "healthy",
                  "timestamp": 1641988200
                }
              }
            }
          }
        }
      }
    }
  }
}
```

## Debugging

### Enable Debug Logging

```cpp
// Enable verbose API logging
Logger::getInstance().setLevel(LogLevel::DEBUG);

// Add debug middleware
server->addGlobalMiddleware([](HttpRequest& req, HttpResponse& res) {
    Logger::getInstance().debug(LogCategory::API, "Debug", "request", __FILE__, __LINE__,
                               "Headers: " + req.getHeader("User-Agent"));
    Logger::getInstance().debug(LogCategory::API, "Debug", "request", __FILE__, __LINE__,
                               "Body: " + req.getBody());
    return true;
});
```

### Common Debug Scenarios

#### Route Not Found (404)
```cpp
// Debug route registration
server->addRoute(HttpMethod::GET, "/debug/routes", [this](const HttpRequest& req) {
    HttpResponse res;
    nlohmann::json routes = nlohmann::json::array();
    
    // List all registered routes (implement in server)
    for (const auto& route : getRegisteredRoutes()) {
        routes.push_back({
            {"method", methodToString(route.method)},
            {"path", route.path},
            {"operationId", route.operation.operationId}
        });
    }
    
    res.setJson({{"routes", routes}});
    return res;
});
```

#### Request Parsing Issues
```cpp
// Debug request parsing
server->addGlobalMiddleware([](HttpRequest& req, HttpResponse& res) {
    try {
        if (!req.getBody().empty()) {
            auto json = req.getJsonBody(); // Test JSON parsing
            Logger::getInstance().debug(LogCategory::API, "Debug", "json", __FILE__, __LINE__,
                                       "Parsed JSON successfully");
        }
    } catch (const std::exception& e) {
        Logger::getInstance().error(LogCategory::API, "Debug", "json", __FILE__, __LINE__,
                                   "JSON parsing failed: " + std::string(e.what()));
    }
    return true;
});
```

#### Authentication Issues
```cpp
// Debug authentication
server->setAuthenticationHandler([](const HttpRequest& req) {
    std::string authHeader = req.getHeader("Authorization");
    Logger::getInstance().debug(LogCategory::API, "Auth", "check", __FILE__, __LINE__,
                               "Auth header: " + authHeader);
    
    if (authHeader.empty()) {
        Logger::getInstance().debug(LogCategory::API, "Auth", "check", __FILE__, __LINE__,
                                   "No authorization header");
        return false;
    }
    
    // Your authentication logic here
    bool isValid = validateToken(authHeader);
    Logger::getInstance().debug(LogCategory::API, "Auth", "check", __FILE__, __LINE__,
                               "Token valid: " + std::to_string(isValid));
    return isValid;
});
```

## Troubleshooting

### Build Issues

#### cpp-httplib Not Found
```bash
# Install cpp-httplib
sudo apt-get install libhttplib-dev

# Or download manually
wget https://github.com/yhirose/cpp-httplib/releases/latest/download/httplib.h
sudo cp httplib.h /usr/include/
```

#### nlohmann/json Not Found
```bash
# Install nlohmann-json
sudo apt-get install nlohmann-json3-dev
```

#### Linking Errors
```bash
# Make sure HTTPLIB_IMPLEMENTATION is defined exactly once
# Check CMakeLists.txt for proper linking:
find_package(Httplib REQUIRED)
find_package(NlohmannJson REQUIRED)
target_link_libraries(your_target httplib::httplib nlohmann_json::nlohmann_json)
```

### Runtime Issues

#### Server Won't Start

**Problem**: `server->start()` returns false

**Solutions**:
1. Check if port is already in use:
   ```bash
   netstat -tulpn | grep :8080
   sudo lsof -i :8080
   ```

2. Check permissions (for ports < 1024):
   ```bash
   sudo ./your_application
   # Or use a port >= 1024
   ```

3. Check bind address:
   ```cpp
   server->setBindAddress("127.0.0.1"); // Local only
   server->setBindAddress("0.0.0.0");   // All interfaces
   ```

#### CORS Issues

**Problem**: Web browser blocks requests due to CORS

**Solutions**:
```cpp
// Enable CORS for all origins (development only)
server->enableCors({"*"});

// Enable CORS for specific origins (production)
server->enableCors({
    "http://localhost:3000",
    "https://yourdomain.com"
});

// Set additional CORS headers
server->setCorsHeaders({
    {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
    {"Access-Control-Allow-Headers", "Content-Type, Authorization"},
    {"Access-Control-Max-Age", "86400"}
});
```

#### Authentication Failures

**Problem**: 401 Unauthorized responses

**Debug Steps**:
1. Check if authentication handler is set:
   ```cpp
   // Temporarily disable auth for testing
   server->setAuthenticationHandler([](const HttpRequest& req) {
       return true; // Allow all requests
   });
   ```

2. Verify token format:
   ```bash
   curl -H "Authorization: Bearer your_token_here" http://localhost:8080/api/protected
   ```

3. Add debug logging to auth handler:
   ```cpp
   server->setAuthenticationHandler([](const HttpRequest& req) {
       std::string auth = req.getHeader("Authorization");
       Logger::getInstance().debug(LogCategory::API, "Auth", "debug", __FILE__, __LINE__,
                                  "Auth header: " + auth);
       // Your validation logic
   });
   ```

#### JSON Parsing Errors

**Problem**: 400 Bad Request for JSON endpoints

**Debug Steps**:
1. Validate JSON syntax:
   ```bash
   echo '{"test": "value"}' | jq .
   ```

2. Check Content-Type header:
   ```bash
   curl -H "Content-Type: application/json" \
        -d '{"key": "value"}' \
        http://localhost:8080/api/endpoint
   ```

3. Add request body logging:
   ```cpp
   server->addGlobalMiddleware([](HttpRequest& req, HttpResponse& res) {
       Logger::getInstance().debug(LogCategory::API, "Debug", "body", __FILE__, __LINE__,
                                  "Request body: " + req.getBody());
       return true;
   });
   ```

#### High Memory Usage

**Problem**: Server consumes too much memory

**Solutions**:
1. Limit request body size in cpp-httplib
2. Implement request rate limiting
3. Add memory usage monitoring:
   ```cpp
   server->addGlobalMiddleware([](HttpRequest& req, HttpResponse& res) {
       // Monitor memory usage
       std::ifstream status("/proc/self/status");
       std::string line;
       while (std::getline(status, line)) {
           if (line.find("VmRSS:") == 0) {
               Logger::getInstance().debug(LogCategory::API, "Memory", "usage", __FILE__, __LINE__, line);
               break;
           }
       }
       return true;
   });
   ```

### Performance Issues

#### Slow Response Times

**Solutions**:
1. Add timing middleware:
   ```cpp
   server->addGlobalMiddleware([](HttpRequest& req, HttpResponse& res) {
       auto start = std::chrono::high_resolution_clock::now();
       
       // Store start time in request context (if supported)
       // This would need to be implemented in HttpRequest class
       
       return true;
   });
   ```

2. Optimize JSON parsing:
   ```cpp
   // Use streaming JSON parsing for large payloads
   // Cache frequently accessed configuration
   ```

3. Implement connection pooling for external resources

#### High CPU Usage

**Solutions**:
1. Profile with tools like `perf` or `gprof`
2. Reduce logging verbosity in production
3. Optimize route matching algorithms
4. Consider request queuing for expensive operations

### Network Issues

#### Cannot Access from External Hosts

**Solutions**:
1. Check bind address:
   ```cpp
   server->setBindAddress("0.0.0.0"); // Not "127.0.0.1"
   ```

2. Check firewall:
   ```bash
   sudo ufw allow 8080
   sudo iptables -I INPUT -p tcp --dport 8080 -j ACCEPT
   ```

3. Check if service is listening:
   ```bash
   netstat -tulpn | grep :8080
   ss -tulpn | grep :8080
   ```

### Logging and Monitoring

#### Enable Comprehensive Logging

```cpp
// Create detailed logging middleware
server->addGlobalMiddleware([](HttpRequest& req, HttpResponse& res) {
    auto timestamp = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    
    Logger::getInstance().info(LogCategory::API, "Request", "incoming", __FILE__, __LINE__,
                              "Method: " + std::to_string(static_cast<int>(req.getMethod())) +
                              ", Path: " + req.getPath() +
                              ", Client: " + req.getClientAddress() +
                              ", User-Agent: " + req.getHeader("User-Agent"));
    return true;
});

// Response logging (would need post-processing hook)
```

#### Health Monitoring

```cpp
// Add comprehensive health endpoint
server->addRoute(HttpMethod::GET, "/api/health/detailed", [](const HttpRequest& req) {
    HttpResponse res;
    
    auto now = std::chrono::system_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
        now - getStartTime()).count();
    
    // Get system stats
    std::ifstream meminfo("/proc/meminfo");
    std::ifstream loadavg("/proc/loadavg");
    
    res.setJson({
        {"status", "healthy"},
        {"timestamp", std::chrono::system_clock::to_time_t(now)},
        {"uptime_seconds", uptime},
        {"memory", getMemoryStats()},
        {"load_average", getLoadAverage()},
        {"active_connections", getActiveConnections()},
        {"total_requests", getTotalRequests()},
        {"error_rate", getErrorRate()}
    });
    
    return res;
});
```

This comprehensive documentation provides everything needed to implement, debug, and troubleshoot the OpenAuto REST API server. The examples are practical and the troubleshooting section covers common real-world issues developers will encounter.
