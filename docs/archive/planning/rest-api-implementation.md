# OpenAuto REST API Implementation Guide

This guide provides detailed instructions for implementing REST API endpoints in the OpenAuto system using the modern REST API server framework.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Basic Endpoint Implementation](#basic-endpoint-implementation)
3. [Advanced Features](#advanced-features)
4. [OpenAPI Documentation](#openapi-documentation)
5. [Best Practices](#best-practices)
6. [Testing Strategies](#testing-strategies)
7. [Deployment Considerations](#deployment-considerations)

## Getting Started

### Project Structure

```
openauto/
├── include/modern/
│   ├── RestApiServer.hpp       # Main API server interface
│   ├── EventBus.hpp           # Event system integration
│   ├── StateMachine.hpp       # State management
│   └── ConfigurationManager.hpp
├── src/modern/
│   ├── RestApiServer.cpp      # Server implementation
│   └── [your_api_modules].cpp # Custom API modules
└── docs/
    ├── rest-api-comprehensive.md
    ├── rest-api-debugging.md
    └── rest-api-implementation.md  # This file
```

### Basic Setup

```cpp
#include "modern/RestApiServer.hpp"
#include "modern/Logger.hpp"
#include "modern/EventBus.hpp"
#include "modern/StateMachine.hpp"
#include "modern/ConfigurationManager.hpp"

class OpenAutoApi {
private:
    std::shared_ptr<RestApiServer> server_;
    std::shared_ptr<EventBus> eventBus_;
    std::shared_ptr<StateMachine> stateMachine_;
    std::shared_ptr<ConfigurationManager> configManager_;

public:
    OpenAutoApi(int port = 8080) {
        // Initialize dependencies
        eventBus_ = std::make_shared<EventBus>();
        stateMachine_ = std::make_shared<StateMachine>();
        configManager_ = std::make_shared<ConfigurationManager>();
        
        // Create server
        server_ = std::make_shared<RestApiServer>(port, eventBus_, stateMachine_, configManager_);
        
        // Setup API
        configureServer();
        setupRoutes();
    }
    
    void configureServer() {
        // API information
        ApiInfo info;
        info.title = "OpenAuto REST API";
        info.version = "1.0.0";
        info.description = "REST API for OpenAuto Android Auto system";
        info.contact.name = "OpenCarDev Team";
        info.license.name = "GPL-3.0";
        server_->setApiInfo(info);
        
        // Enable CORS for web applications
        server_->enableCors({"*"}); // For development; restrict in production
        
        // Enable documentation
        server_->enableSwaggerUI("/docs");
        server_->enableReDoc("/redoc");
        
        // Add global middleware
        addMiddleware();
    }
    
    void addMiddleware() {
        // Request logging
        server_->addGlobalMiddleware([](HttpRequest& req, HttpResponse& res) {
            Logger::getInstance().info(LogCategory::API, "Request", "incoming", __FILE__, __LINE__,
                "HTTP " + std::to_string(static_cast<int>(req.getMethod())) + " " + req.getPath());
            return true;
        });
        
        // Error handling
        server_->addGlobalMiddleware([](HttpRequest& req, HttpResponse& res) {
            try {
                return true; // Continue processing
            } catch (const std::exception& e) {
                Logger::getInstance().error(LogCategory::API, "Request", "error", __FILE__, __LINE__,
                    "Unhandled exception: " + std::string(e.what()));
                res.setStatus(500);
                res.setJson({{"error", "Internal server error"}});
                return false; // Stop processing
            }
        });
    }
    
    void setupRoutes() {
        setupSystemRoutes();
        setupConfigurationRoutes();
        setupStateRoutes();
        setupEventRoutes();
        setupMediaRoutes();
        setupDeviceRoutes();
    }
    
    bool start() {
        return server_->start();
    }
    
    void stop() {
        server_->stop();
    }
};
```

## Basic Endpoint Implementation

### Simple GET Endpoint

```cpp
void setupSystemRoutes() {
    // Health check endpoint
    ApiOperation healthOp;
    healthOp.operationId = "getHealth";
    healthOp.summary = "System health check";
    healthOp.description = "Returns the current health status of the OpenAuto system";
    healthOp.tags = {"System"};
    
    ApiResponse healthResponse;
    healthResponse.statusCode = 200;
    healthResponse.description = "System health information";
    healthResponse.example = R"({
        "status": "healthy",
        "timestamp": 1641988200,
        "uptime": 3600,
        "version": "1.0.0"
    })";
    healthOp.responses = {healthResponse};
    
    server_->addRoute(HttpMethod::GET, "/api/health", [this](const HttpRequest& req) {
        HttpResponse res;
        
        auto now = std::chrono::system_clock::now();
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
            now - getStartTime()).count();
        
        res.setJson({
            {"status", "healthy"},
            {"timestamp", std::chrono::system_clock::to_time_t(now)},
            {"uptime", uptime},
            {"version", "1.0.0"},
            {"components", {
                {"event_bus", eventBus_->isRunning()},
                {"state_machine", stateMachine_->isReady()},
                {"configuration", configManager_->isLoaded()}
            }}
        });
        
        return res;
    }, healthOp);
}
```

### POST Endpoint with JSON Body

```cpp
void setupEventRoutes() {
    // Emit event endpoint
    ApiParameter eventParam;
    eventParam.name = "event";
    eventParam.type = ParameterType::OBJECT;
    eventParam.in = ParameterIn::BODY;
    eventParam.required = true;
    eventParam.description = "Event data to emit";
    
    ApiOperation emitOp;
    emitOp.operationId = "emitEvent";
    emitOp.summary = "Emit system event";
    emitOp.description = "Emits a custom event to the OpenAuto event bus";
    emitOp.tags = {"Events"};
    emitOp.parameters = {eventParam};
    
    ApiResponse successResponse;
    successResponse.statusCode = 200;
    successResponse.description = "Event emitted successfully";
    
    ApiResponse errorResponse;
    errorResponse.statusCode = 400;
    errorResponse.description = "Invalid event data";
    
    emitOp.responses = {successResponse, errorResponse};
    
    server_->addRoute(HttpMethod::POST, "/api/events", [this](const HttpRequest& req) {
        HttpResponse res;
        
        try {
            auto eventData = req.getJsonBody();
            
            // Validate required fields
            if (!eventData.contains("type")) {
                res.setStatus(400);
                res.setJson({{"error", "Event type is required"}});
                return res;
            }
            
            std::string eventType = eventData["type"];
            nlohmann::json payload = eventData.value("payload", nlohmann::json{});
            
            // Create and emit event
            Event event(eventType, payload);
            eventBus_->emit(event);
            
            Logger::getInstance().info(LogCategory::API, "Events", "emit", __FILE__, __LINE__,
                "Emitted event: " + eventType);
            
            res.setJson({
                {"status", "success"},
                {"message", "Event emitted successfully"},
                {"event_type", eventType}
            });
            
        } catch (const nlohmann::json::parse_error& e) {
            res.setStatus(400);
            res.setJson({
                {"error", "Invalid JSON"},
                {"details", e.what()}
            });
        } catch (const std::exception& e) {
            res.setStatus(500);
            res.setJson({
                {"error", "Failed to emit event"},
                {"details", e.what()}
            });
        }
        
        return res;
    }, emitOp);
}
```

### Path Parameter Endpoints

```cpp
void setupConfigurationRoutes() {
    // Get specific configuration section
    ApiParameter sectionParam;
    sectionParam.name = "section";
    sectionParam.type = ParameterType::STRING;
    sectionParam.in = ParameterIn::PATH;
    sectionParam.required = true;
    sectionParam.description = "Configuration section name";
    sectionParam.example = "audio";
    
    ApiOperation getConfigOp;
    getConfigOp.operationId = "getConfigSection";
    getConfigOp.summary = "Get configuration section";
    getConfigOp.description = "Retrieves configuration for a specific section";
    getConfigOp.tags = {"Configuration"};
    getConfigOp.parameters = {sectionParam};
    
    server_->addRoute(HttpMethod::GET, "/api/config/{section}", [this](const HttpRequest& req) {
        HttpResponse res;
        
        std::string section = req.getPathParam("section");
        if (section.empty()) {
            res.setStatus(400);
            res.setJson({{"error", "Section parameter is required"}});
            return res;
        }
        
        try {
            auto config = configManager_->getSection(section);
            if (config.empty()) {
                res.setStatus(404);
                res.setJson({{"error", "Configuration section not found: " + section}});
                return res;
            }
            
            res.setJson({
                {"section", section},
                {"configuration", config}
            });
            
        } catch (const std::exception& e) {
            res.setStatus(500);
            res.setJson({
                {"error", "Failed to retrieve configuration"},
                {"details", e.what()}
            });
        }
        
        return res;
    }, getConfigOp);
    
    // Update configuration section
    server_->addRoute(HttpMethod::PUT, "/api/config/{section}", [this](const HttpRequest& req) {
        HttpResponse res;
        
        std::string section = req.getPathParam("section");
        if (section.empty()) {
            res.setStatus(400);
            res.setJson({{"error", "Section parameter is required"}});
            return res;
        }
        
        try {
            auto newConfig = req.getJsonBody();
            configManager_->updateSection(section, newConfig);
            
            Logger::getInstance().info(LogCategory::API, "Config", "update", __FILE__, __LINE__,
                "Updated configuration section: " + section);
            
            res.setJson({
                {"status", "success"},
                {"message", "Configuration updated successfully"},
                {"section", section}
            });
            
        } catch (const nlohmann::json::parse_error& e) {
            res.setStatus(400);
            res.setJson({
                {"error", "Invalid JSON"},
                {"details", e.what()}
            });
        } catch (const std::exception& e) {
            res.setStatus(500);
            res.setJson({
                {"error", "Failed to update configuration"},
                {"details", e.what()}
            });
        }
        
        return res;
    });
}
```

### Query Parameter Handling

```cpp
void setupStateRoutes() {
    // Get system state with optional filtering
    ApiParameter filterParam;
    filterParam.name = "filter";
    filterParam.type = ParameterType::STRING;
    filterParam.in = ParameterIn::QUERY;
    filterParam.required = false;
    filterParam.description = "Filter state information";
    filterParam.example = "connection,audio";
    
    ApiParameter formatParam;
    formatParam.name = "format";
    formatParam.type = ParameterType::STRING;
    formatParam.in = ParameterIn::QUERY;
    formatParam.required = false;
    formatParam.description = "Response format";
    formatParam.example = "detailed";
    formatParam.defaultValue = "summary";
    
    ApiOperation getStateOp;
    getStateOp.operationId = "getSystemState";
    getStateOp.summary = "Get system state";
    getStateOp.description = "Retrieves current system state information";
    getStateOp.tags = {"State"};
    getStateOp.parameters = {filterParam, formatParam};
    
    server_->addRoute(HttpMethod::GET, "/api/state", [this](const HttpRequest& req) {
        HttpResponse res;
        
        std::string filter = req.getQuery("filter");
        std::string format = req.getQuery("format");
        if (format.empty()) format = "summary";
        
        try {
            auto state = stateMachine_->getCurrentState();
            nlohmann::json response;
            
            response["current_state"] = state.name;
            response["timestamp"] = std::time(nullptr);
            
            if (format == "detailed") {
                response["state_history"] = stateMachine_->getStateHistory();
                response["available_transitions"] = stateMachine_->getAvailableTransitions();
            }
            
            // Apply filter if specified
            if (!filter.empty()) {
                std::vector<std::string> filters;
                std::stringstream ss(filter);
                std::string item;
                while (std::getline(ss, item, ',')) {
                    filters.push_back(item);
                }
                
                nlohmann::json filteredData;
                for (const auto& f : filters) {
                    if (f == "connection") {
                        filteredData["connection"] = state.connectionStatus;
                    } else if (f == "audio") {
                        filteredData["audio"] = state.audioStatus;
                    }
                    // Add more filter options as needed
                }
                response["filtered_data"] = filteredData;
            }
            
            res.setJson(response);
            
        } catch (const std::exception& e) {
            res.setStatus(500);
            res.setJson({
                {"error", "Failed to retrieve system state"},
                {"details", e.what()}
            });
        }
        
        return res;
    }, getStateOp);
}
```

## Advanced Features

### Authentication Implementation

```cpp
void setupAuthentication() {
    // Set up Bearer token authentication
    server_->setAuthenticationHandler([this](const HttpRequest& req) {
        std::string authHeader = req.getHeader("Authorization");
        
        if (!authHeader.starts_with("Bearer ")) {
            return false;
        }
        
        std::string token = authHeader.substr(7);
        return validateApiToken(token);
    });
    
    // Protect sensitive endpoints
    server_->requireAuthentication("/api/config");
    server_->requireAuthentication("/api/state/transition");
    server_->requireAuthentication("/api/system/restart");
}

bool validateApiToken(const std::string& token) {
    // Implement your token validation logic
    // This could check against a database, validate JWT, etc.
    
    // Simple example: check against configured tokens
    auto validTokens = configManager_->getArray("api.valid_tokens");
    return std::find(validTokens.begin(), validTokens.end(), token) != validTokens.end();
}
```

### File Upload Endpoints

```cpp
void setupMediaRoutes() {
    // File upload endpoint
    server_->addRoute(HttpMethod::POST, "/api/media/upload", [this](const HttpRequest& req) {
        HttpResponse res;
        
        try {
            std::string contentType = req.getHeader("Content-Type");
            if (contentType.find("multipart/form-data") == std::string::npos) {
                res.setStatus(400);
                res.setJson({{"error", "Content-Type must be multipart/form-data"}});
                return res;
            }
            
            // Parse multipart data (simplified - would need proper multipart parser)
            std::string body = req.getBody();
            
            // Save file (implement your file handling logic)
            std::string filename = saveUploadedFile(body);
            
            Logger::getInstance().info(LogCategory::API, "Media", "upload", __FILE__, __LINE__,
                "File uploaded: " + filename);
            
            res.setJson({
                {"status", "success"},
                {"filename", filename},
                {"size", body.length()}
            });
            
        } catch (const std::exception& e) {
            res.setStatus(500);
            res.setJson({
                {"error", "File upload failed"},
                {"details", e.what()}
            });
        }
        
        return res;
    });
}
```

### Server-Sent Events (SSE)

```cpp
void setupEventStreaming() {
    // Server-Sent Events endpoint for real-time updates
    server_->addRoute(HttpMethod::GET, "/api/events/stream", [this](const HttpRequest& req) {
        HttpResponse res;
        
        // Set SSE headers
        res.setHeader("Content-Type", "text/event-stream");
        res.setHeader("Cache-Control", "no-cache");
        res.setHeader("Connection", "keep-alive");
        
        // Subscribe to event bus
        auto subscription = eventBus_->subscribe("*", [&res](const Event& event) {
            std::stringstream sse;
            sse << "event: " << event.getType() << "\n";
            sse << "data: " << event.getData().dump() << "\n\n";
            
            // Send SSE data (this would need proper streaming implementation)
            res.setBody(res.getBody() + sse.str());
        });
        
        // Keep connection alive (simplified implementation)
        res.setBody("data: {\"status\": \"connected\"}\n\n");
        
        return res;
    });
}
```

### Batch Operations

```cpp
void setupBatchRoutes() {
    // Batch configuration update
    server_->addRoute(HttpMethod::POST, "/api/config/batch", [this](const HttpRequest& req) {
        HttpResponse res;
        
        try {
            auto batchData = req.getJsonBody();
            
            if (!batchData.is_array()) {
                res.setStatus(400);
                res.setJson({{"error", "Request body must be an array of operations"}});
                return res;
            }
            
            nlohmann::json results = nlohmann::json::array();
            bool hasErrors = false;
            
            for (size_t i = 0; i < batchData.size(); ++i) {
                auto operation = batchData[i];
                nlohmann::json result;
                result["index"] = i;
                
                try {
                    std::string section = operation["section"];
                    auto config = operation["config"];
                    
                    configManager_->updateSection(section, config);
                    
                    result["status"] = "success";
                    result["section"] = section;
                    
                } catch (const std::exception& e) {
                    result["status"] = "error";
                    result["error"] = e.what();
                    hasErrors = true;
                }
                
                results.push_back(result);
            }
            
            res.setStatus(hasErrors ? 207 : 200); // 207 Multi-Status
            res.setJson({
                {"status", hasErrors ? "partial_success" : "success"},
                {"results", results}
            });
            
        } catch (const std::exception& e) {
            res.setStatus(400);
            res.setJson({
                {"error", "Invalid batch request"},
                {"details", e.what()}
            });
        }
        
        return res;
    });
}
```

## OpenAPI Documentation

### Complete OpenAPI Specification

```cpp
void setupCompleteOpenApiSpec() {
    // Detailed API information
    ApiInfo info;
    info.title = "OpenAuto REST API";
    info.version = "1.0.0";
    info.description = R"(
# OpenAuto REST API

This API provides programmatic access to the OpenAuto Android Auto system.

## Features
- System health monitoring
- Configuration management
- State machine control
- Event system integration
- Real-time updates via Server-Sent Events

## Authentication
Bearer token authentication is required for certain endpoints.
Include the token in the Authorization header:
```
Authorization: Bearer <your-token>
```
    )";
    info.termsOfService = "https://github.com/opencardev/openauto/blob/master/LICENSE";
    info.contact.name = "OpenCarDev Team";
    info.contact.url = "https://github.com/opencardev/openauto";
    info.contact.email = "contact@opencardev.org";
    info.license.name = "GPL-3.0";
    info.license.url = "https://www.gnu.org/licenses/gpl-3.0.html";
    
    server_->setApiInfo(info);
    
    // Server information
    ServerInfo devServer;
    devServer.url = "http://localhost:8080";
    devServer.description = "Development server";
    server_->addServer(devServer);
    
    ServerInfo prodServer;
    prodServer.url = "https://api.openauto.example.com";
    prodServer.description = "Production server";
    server_->addServer(prodServer);
    
    // Security schemes
    SecurityScheme bearerAuth;
    bearerAuth.type = SecurityType::HTTP;
    bearerAuth.scheme = "bearer";
    bearerAuth.bearerFormat = "JWT";
    bearerAuth.description = "Bearer token authentication";
    server_->addSecurityScheme("bearerAuth", bearerAuth);
}
```

### Rich Parameter Documentation

```cpp
ApiParameter createPaginationParameter(const std::string& name, const std::string& description, int defaultValue) {
    ApiParameter param;
    param.name = name;
    param.type = ParameterType::INTEGER;
    param.in = ParameterIn::QUERY;
    param.required = false;
    param.description = description;
    param.example = std::to_string(defaultValue);
    param.defaultValue = std::to_string(defaultValue);
    return param;
}

void setupPaginatedEndpoint() {
    auto limitParam = createPaginationParameter("limit", "Maximum number of items to return", 50);
    auto offsetParam = createPaginationParameter("offset", "Number of items to skip", 0);
    
    ApiParameter sortParam;
    sortParam.name = "sort";
    sortParam.type = ParameterType::STRING;
    sortParam.in = ParameterIn::QUERY;
    sortParam.required = false;
    sortParam.description = "Sort field and direction (e.g., 'name:asc', 'created:desc')";
    sortParam.example = "created:desc";
    sortParam.defaultValue = "created:desc";
    
    ApiOperation listOp;
    listOp.operationId = "listEvents";
    listOp.summary = "List system events";
    listOp.description = "Retrieves a paginated list of system events with optional sorting";
    listOp.tags = {"Events"};
    listOp.parameters = {limitParam, offsetParam, sortParam};
    
    // Define comprehensive responses
    ApiResponse successResponse;
    successResponse.statusCode = 200;
    successResponse.description = "Events retrieved successfully";
    successResponse.example = R"({
        "events": [
            {
                "id": "evt_123",
                "type": "connection.established",
                "timestamp": 1641988200,
                "data": {"device": "phone_001"}
            }
        ],
        "pagination": {
            "limit": 50,
            "offset": 0,
            "total": 150,
            "has_more": true
        }
    })";
    
    ApiResponse badRequestResponse;
    badRequestResponse.statusCode = 400;
    badRequestResponse.description = "Invalid pagination parameters";
    badRequestResponse.example = R"({
        "error": "Invalid limit parameter",
        "details": "Limit must be between 1 and 100"
    })";
    
    listOp.responses = {successResponse, badRequestResponse};
    
    server_->addRoute(HttpMethod::GET, "/api/events", [this](const HttpRequest& req) {
        // Implementation here
        return handleListEvents(req);
    }, listOp);
}
```

## Best Practices

### Error Handling

```cpp
class ApiException : public std::exception {
private:
    int statusCode_;
    std::string message_;
    std::string details_;

public:
    ApiException(int statusCode, const std::string& message, const std::string& details = "")
        : statusCode_(statusCode), message_(message), details_(details) {}
    
    int getStatusCode() const { return statusCode_; }
    const std::string& getMessage() const { return message_; }
    const std::string& getDetails() const { return details_; }
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
};

// Global error handling middleware
void addErrorHandlingMiddleware() {
    server_->addGlobalMiddleware([](HttpRequest& req, HttpResponse& res) {
        try {
            return true; // Continue processing
        } catch (const ApiException& e) {
            res.setStatus(e.getStatusCode());
            nlohmann::json error = {{"error", e.getMessage()}};
            if (!e.getDetails().empty()) {
                error["details"] = e.getDetails();
            }
            res.setJson(error);
            return false; // Stop processing
        } catch (const std::exception& e) {
            Logger::getInstance().error(LogCategory::API, "Error", "unhandled", __FILE__, __LINE__,
                "Unhandled exception: " + std::string(e.what()));
            res.setStatus(500);
            res.setJson({{"error", "Internal server error"}});
            return false;
        }
    });
}
```

### Input Validation

```cpp
class RequestValidator {
public:
    static void validateConfigSection(const std::string& section) {
        static const std::vector<std::string> validSections = {
            "audio", "video", "bluetooth", "ui", "system"
        };
        
        if (std::find(validSections.begin(), validSections.end(), section) == validSections.end()) {
            throw ApiException(400, "Invalid configuration section", 
                "Valid sections: " + joinStrings(validSections, ", "));
        }
    }
    
    static void validatePagination(int limit, int offset) {
        if (limit < 1 || limit > 100) {
            throw ApiException(400, "Invalid limit parameter", "Limit must be between 1 and 100");
        }
        if (offset < 0) {
            throw ApiException(400, "Invalid offset parameter", "Offset must be non-negative");
        }
    }
    
    static void validateJsonStructure(const nlohmann::json& data, const std::vector<std::string>& requiredFields) {
        for (const auto& field : requiredFields) {
            if (!data.contains(field)) {
                throw ApiException(400, "Missing required field: " + field);
            }
        }
    }

private:
    static std::string joinStrings(const std::vector<std::string>& strings, const std::string& delimiter) {
        if (strings.empty()) return "";
        
        std::string result = strings[0];
        for (size_t i = 1; i < strings.size(); ++i) {
            result += delimiter + strings[i];
        }
        return result;
    }
};
```

### Response Standardization

```cpp
class ApiResponse {
public:
    static HttpResponse success(const nlohmann::json& data = nlohmann::json{}) {
        HttpResponse res;
        nlohmann::json response = {
            {"status", "success"},
            {"timestamp", std::time(nullptr)}
        };
        
        if (!data.is_null()) {
            response["data"] = data;
        }
        
        res.setJson(response);
        return res;
    }
    
    static HttpResponse error(int statusCode, const std::string& message, const std::string& details = "") {
        HttpResponse res;
        res.setStatus(statusCode);
        
        nlohmann::json response = {
            {"status", "error"},
            {"error", {
                {"message", message},
                {"code", statusCode}
            }},
            {"timestamp", std::time(nullptr)}
        };
        
        if (!details.empty()) {
            response["error"]["details"] = details;
        }
        
        res.setJson(response);
        return res;
    }
    
    static HttpResponse paginated(const nlohmann::json& items, int limit, int offset, int total) {
        HttpResponse res;
        res.setJson({
            {"status", "success"},
            {"data", items},
            {"pagination", {
                {"limit", limit},
                {"offset", offset},
                {"total", total},
                {"has_more", offset + limit < total}
            }},
            {"timestamp", std::time(nullptr)}
        });
        return res;
    }
};
```

## Testing Strategies

### Unit Testing

```cpp
#include <gtest/gtest.h>

class RestApiServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        eventBus_ = std::make_shared<EventBus>();
        stateMachine_ = std::make_shared<StateMachine>();
        configManager_ = std::make_shared<ConfigurationManager>();
        
        server_ = std::make_shared<RestApiServer>(0, eventBus_, stateMachine_, configManager_);
        api_ = std::make_unique<OpenAutoApi>(server_);
    }
    
    void TearDown() override {
        if (server_->isRunning()) {
            server_->stop();
        }
    }
    
    std::shared_ptr<EventBus> eventBus_;
    std::shared_ptr<StateMachine> stateMachine_;
    std::shared_ptr<ConfigurationManager> configManager_;
    std::shared_ptr<RestApiServer> server_;
    std::unique_ptr<OpenAutoApi> api_;
};

TEST_F(RestApiServerTest, HealthCheckReturnsValidResponse) {
    // Start server
    ASSERT_TRUE(server_->start());
    
    // Make request
    auto response = makeRequest("GET", "/api/health");
    
    EXPECT_EQ(response.getStatus(), 200);
    
    auto json = nlohmann::json::parse(response.getBody());
    EXPECT_EQ(json["status"], "healthy");
    EXPECT_TRUE(json.contains("timestamp"));
    EXPECT_TRUE(json.contains("uptime"));
}

TEST_F(RestApiServerTest, InvalidEndpointReturns404) {
    ASSERT_TRUE(server_->start());
    
    auto response = makeRequest("GET", "/api/nonexistent");
    EXPECT_EQ(response.getStatus(), 404);
}
```

### Integration Testing

```bash
#!/bin/bash
# integration_test.sh

API_BASE="http://localhost:8080"

# Test health endpoint
echo "Testing health endpoint..."
response=$(curl -s -w "%{http_code}" "$API_BASE/api/health")
if [[ "${response: -3}" == "200" ]]; then
    echo "✓ Health check passed"
else
    echo "✗ Health check failed"
    exit 1
fi

# Test authentication
echo "Testing authentication..."
response=$(curl -s -w "%{http_code}" -H "Authorization: Bearer invalid_token" "$API_BASE/api/config")
if [[ "${response: -3}" == "401" ]]; then
    echo "✓ Authentication test passed"
else
    echo "✗ Authentication test failed"
    exit 1
fi

# Test JSON parsing
echo "Testing JSON parsing..."
response=$(curl -s -w "%{http_code}" \
    -H "Content-Type: application/json" \
    -d '{"type": "test_event", "payload": {"key": "value"}}' \
    "$API_BASE/api/events")
if [[ "${response: -3}" == "200" ]]; then
    echo "✓ JSON parsing test passed"
else
    echo "✗ JSON parsing test failed"
    exit 1
fi

echo "All integration tests passed!"
```

### Performance Testing

```bash
#!/bin/bash
# performance_test.sh

# Install Apache Bench if not available
if ! command -v ab &> /dev/null; then
    sudo apt-get install apache2-utils
fi

API_BASE="http://localhost:8080"

echo "Running performance tests..."

# Test health endpoint
echo "Testing /api/health endpoint (1000 requests, 10 concurrent)..."
ab -n 1000 -c 10 -H "Accept: application/json" "$API_BASE/api/health"

# Test with POST requests
echo "Testing /api/events endpoint with POST..."
temp_file=$(mktemp)
echo '{"type": "test", "payload": {}}' > "$temp_file"

ab -n 100 -c 5 \
   -H "Content-Type: application/json" \
   -p "$temp_file" \
   "$API_BASE/api/events"

rm "$temp_file"

echo "Performance tests completed!"
```

## Deployment Considerations

### Production Configuration

```cpp
void configureForProduction() {
    // Restrict CORS to specific domains
    server_->enableCors({
        "https://yourdomain.com",
        "https://app.yourdomain.com"
    });
    
    // Set secure headers
    server_->setCorsHeaders({
        {"Access-Control-Allow-Credentials", "false"},
        {"X-Content-Type-Options", "nosniff"},
        {"X-Frame-Options", "DENY"},
        {"X-XSS-Protection", "1; mode=block"}
    });
    
    // Configure authentication
    setupProductionAuthentication();
    
    // Set appropriate log level
    Logger::getInstance().setLevel(LogLevel::WARN);
    
    // Enable rate limiting (would need implementation)
    enableRateLimiting();
}
```

### Systemd Service Configuration

```ini
# /etc/systemd/system/openauto-api.service
[Unit]
Description=OpenAuto REST API Server
After=network.target

[Service]
Type=simple
User=openauto
WorkingDirectory=/opt/openauto
ExecStart=/opt/openauto/bin/autoapp --enable-rest-api --port 8080
Restart=always
RestartSec=10

# Security settings
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ProtectHome=true
ReadWritePaths=/var/log/openauto

[Install]
WantedBy=multi-user.target
```

### Nginx Reverse Proxy Configuration

```nginx
# /etc/nginx/sites-available/openauto-api
server {
    listen 80;
    server_name api.yourdomain.com;
    
    # Redirect to HTTPS
    return 301 https://$server_name$request_uri;
}

server {
    listen 443 ssl http2;
    server_name api.yourdomain.com;
    
    ssl_certificate /path/to/certificate.crt;
    ssl_certificate_key /path/to/private.key;
    
    # Security headers
    add_header X-Content-Type-Options nosniff;
    add_header X-Frame-Options DENY;
    add_header X-XSS-Protection "1; mode=block";
    
    # Rate limiting
    limit_req_zone $binary_remote_addr zone=api:10m rate=10r/s;
    limit_req zone=api burst=20 nodelay;
    
    location / {
        proxy_pass http://localhost:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        
        # Timeouts
        proxy_connect_timeout 30s;
        proxy_send_timeout 30s;
        proxy_read_timeout 30s;
    }
    
    # Static documentation
    location /docs {
        proxy_pass http://localhost:8080/docs;
        # Cache static assets
        expires 1h;
        add_header Cache-Control "public, immutable";
    }
}
```

This comprehensive implementation guide provides the foundation for building robust, documented, and maintainable REST API endpoints in the OpenAuto system.
