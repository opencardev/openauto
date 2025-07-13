/*
*  This file is part of openauto project.
*  Copyright (C) 2025 OpenCarDev Team
*
*  openauto is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.

*  openauto is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with openauto. If not, see <http://www.gnu.org/licenses/>.
*/

#include "modern/RestApiServer.hpp"
#include "modern/Logger.hpp"
#include "modern/EventBus.hpp"
#include "modern/StateMachine.hpp"
#include "modern/ConfigurationManager.hpp"
#include <httplib.h>
#include <thread>
#include <chrono>
#include <ctime>

namespace openauto {
namespace modern {

// Helper function to format timestamp
std::int64_t getCurrentTimestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

// Helper function to create success response
nlohmann::json createSuccessResponse(const nlohmann::json& data = nlohmann::json::object(), const std::string& message = "Operation completed successfully") {
    nlohmann::json response;
    response["success"] = true;
    response["message"] = message;
    response["data"] = data;
    response["timestamp"] = getCurrentTimestamp();
    return response;
}

// Helper function to create error response
nlohmann::json createErrorResponse(int code, const std::string& message, const std::string& detail = "") {
    nlohmann::json response;
    response["success"] = false;
    response["error"]["code"] = code;
    response["error"]["message"] = message;
    if (!detail.empty()) {
        response["error"]["detail"] = detail;
    }
    response["timestamp"] = getCurrentTimestamp();
    return response;
}

// Helper function to create paginated response
nlohmann::json createPaginatedResponse(const nlohmann::json& data, int page, int limit, int total) {
    nlohmann::json response;
    response["success"] = true;
    response["data"] = data;
    response["pagination"]["page"] = page;
    response["pagination"]["limit"] = limit;
    response["pagination"]["total"] = total;
    response["pagination"]["pages"] = (total + limit - 1) / limit;
    response["timestamp"] = getCurrentTimestamp();
    return response;
}

class RestApiServer::ServerImpl {
public:
    std::unique_ptr<httplib::Server> server;
    std::thread serverThread;
    std::atomic<bool> shouldStop{false};
    
    ServerImpl() : server(std::make_unique<httplib::Server>()) {}
    
    ~ServerImpl() {
        shouldStop = true;
        if (server) {
            server->stop();
        }
        if (serverThread.joinable()) {
            serverThread.join();
        }
    }
};

RestApiServer::RestApiServer(
    int port,
    std::shared_ptr<EventBus> eventBus,
    std::shared_ptr<StateMachine> stateMachine,
    std::shared_ptr<ConfigurationManager> configManager
) : impl_(std::make_unique<ServerImpl>()),
    port_(port),
    bindAddress_("0.0.0.0"),
    running_(false),
    corsEnabled_(false),
    eventBus_(eventBus),
    stateMachine_(stateMachine),
    configManager_(configManager) {
    
    Logger::getInstance().info(LogCategory::GENERAL, "RestApiServer", __FUNCTION__, __FILE__, __LINE__, "RestApiServer created on port " + std::to_string(port));
}

RestApiServer::~RestApiServer() {
    stop();
}

bool RestApiServer::start() {
    if (running_) {
        Logger::getInstance().warn(LogCategory::API, "RestApiServer", __FUNCTION__, __FILE__, __LINE__, "Server already running");
        return true;
    }
    
    Logger::getInstance().info(LogCategory::API, "RestApiServer", __FUNCTION__, __FILE__, __LINE__, "Starting REST API server on port " + std::to_string(port_));
    
    // Setup all routes
    setupDefaultRoutes();
    setupApiRoutes();
    setupDocumentationRoutes();
    
    // Setup CORS if enabled
    if (corsEnabled_) {
        setupCorsHandling();
    }
    
    // Start server in separate thread
    impl_->serverThread = std::thread([this]() {
        try {
            Logger::getInstance().info(LogCategory::API, "RestApiServer", "serverThread", __FILE__, __LINE__, 
                                     "HTTP server listening on " + bindAddress_ + ":" + std::to_string(port_));
            impl_->server->listen(bindAddress_.c_str(), port_);
        } catch (const std::exception& e) {
            Logger::getInstance().error(LogCategory::API, "RestApiServer", "serverThread", __FILE__, __LINE__, 
                                      "Server error: " + std::string(e.what()));
            running_ = false;
        }
    });
    
    // Give the server a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    running_ = true;
    Logger::getInstance().info(LogCategory::API, "RestApiServer", __FUNCTION__, __FILE__, __LINE__, "REST API server started successfully");
    return true;
}

void RestApiServer::stop() {
    if (!running_) {
        return;
    }
    
    Logger::getInstance().info(LogCategory::API, "RestApiServer", __FUNCTION__, __FILE__, __LINE__, "Stopping REST API server");
    
    impl_->shouldStop = true;
    if (impl_->server) {
        impl_->server->stop();
    }
    
    if (impl_->serverThread.joinable()) {
        impl_->serverThread.join();
    }
    
    running_ = false;
    Logger::getInstance().info(LogCategory::API, "RestApiServer", __FUNCTION__, __FILE__, __LINE__, "REST API server stopped");
}

bool RestApiServer::isRunning() const {
    return running_;
}

void RestApiServer::setPort(int port) {
    port_ = port;
}

int RestApiServer::getPort() const {
    return port_;
}

void RestApiServer::setBindAddress(const std::string& address) {
    bindAddress_ = address;
}

std::string RestApiServer::getBindAddress() const {
    return bindAddress_;
}

void RestApiServer::setApiInfo(const ApiInfo& info) {
    apiInfo_ = info;
}

ApiInfo RestApiServer::getApiInfo() const {
    return apiInfo_;
}

void RestApiServer::addServer(const ServerInfo& server) {
    servers_.push_back(server);
}

void RestApiServer::addSecurityScheme(const std::string& name, const SecurityScheme& scheme) {
    securitySchemes_[name] = scheme;
}

void RestApiServer::addRoute(const Route& route) {
    routes_.push_back(route);
}

void RestApiServer::addRoute(HttpMethod method, const std::string& path, RouteHandler handler) {
    Route route;
    route.method = method;
    route.path = path;
    route.handler = handler;
    addRoute(route);
}

void RestApiServer::addRoute(HttpMethod method, const std::string& path, RouteHandler handler, const ApiOperation& operation) {
    Route route;
    route.method = method;
    route.path = path;
    route.handler = handler;
    route.operation = operation;
    addRoute(route);
}

void RestApiServer::addGlobalMiddleware(MiddlewareHandler middleware) {
    globalMiddlewares_.push_back(middleware);
}

void RestApiServer::addRouteMiddleware(const std::string& path, MiddlewareHandler middleware) {
    routeMiddlewares_[path].push_back(middleware);
}

std::string RestApiServer::getOpenApiSpec() const {
    nlohmann::json spec;
    spec["openapi"] = "3.0.0";
    spec["info"] = apiInfo_.toJson();
    return spec.dump(2);
}

void RestApiServer::enableSwaggerUI(const std::string& path) {
    Logger::getInstance().info(LogCategory::GENERAL, "RestApiServer", __FUNCTION__, __FILE__, __LINE__, "SwaggerUI enabled at: " + path);
}

void RestApiServer::enableReDoc(const std::string& path) {
    Logger::getInstance().info(LogCategory::GENERAL, "RestApiServer", __FUNCTION__, __FILE__, __LINE__, "ReDoc enabled at: " + path);
}

void RestApiServer::setAuthenticationHandler(std::function<bool(const HttpRequest&)> handler) {
    authHandler_ = handler;
}

void RestApiServer::requireAuthentication(const std::string& path) {
    protectedPaths_.push_back(path);
}

void RestApiServer::enableCors(const std::vector<std::string>& origins) {
    corsEnabled_ = true;
    corsOrigins_ = origins;
}

void RestApiServer::setCorsHeaders(const std::map<std::string, std::string>& headers) {
    corsHeaders_ = headers;
}

// Private method implementations
void RestApiServer::setupDefaultRoutes() {
    // Health endpoint
    impl_->server->Get("/health", [this](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json health;
        health["status"] = "healthy";
        health["uptime"] = getCurrentTimestamp() / 1000; // Simple uptime
        health["version"] = apiInfo_.version.empty() ? "1.0.0" : apiInfo_.version;
        
        auto response = createSuccessResponse(health, "Service is healthy");
        res.set_content(response.dump(), "application/json");
        res.status = 200;
    });
    
    // Info endpoint
    impl_->server->Get("/info", [this](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json info;
        info["title"] = apiInfo_.title.empty() ? "OpenAuto REST API" : apiInfo_.title;
        info["version"] = apiInfo_.version.empty() ? "1.0.0" : apiInfo_.version;
        info["description"] = apiInfo_.description.empty() ? "REST API for OpenAuto Android Auto implementation" : apiInfo_.description;
        
        auto response = createSuccessResponse(info);
        res.set_content(response.dump(), "application/json");
        res.status = 200;
    });
}

void RestApiServer::setupApiRoutes() {
    // Events endpoints
    impl_->server->Get("/api/v1/events", [this](const httplib::Request& req, httplib::Response& res) {
        // Parse pagination parameters
        int page = req.has_param("page") ? std::stoi(req.get_param_value("page")) : 1;
        int limit = req.has_param("limit") ? std::stoi(req.get_param_value("limit")) : 50;
        std::string eventType = req.has_param("type") ? req.get_param_value("type") : "";
        
        // Get events from event bus (simplified implementation)
        nlohmann::json events = nlohmann::json::array();
        
        // Mock events for demonstration
        for (int i = 0; i < limit; ++i) {
            nlohmann::json event;
            event["id"] = i + 1;
            event["type"] = "SYSTEM_EVENT";
            event["source"] = "rest_api";
            event["timestamp"] = getCurrentTimestamp();
            event["data"] = nlohmann::json::object();
            events.push_back(event);
        }
        
        auto response = createPaginatedResponse(events, page, limit, 100); // Mock total
        res.set_content(response.dump(), "application/json");
        res.status = 200;
    });
    
    impl_->server->Post("/api/v1/events", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = nlohmann::json::parse(req.body);
            
            // Validate required fields
            if (!body.contains("type")) {
                auto response = createErrorResponse(400, "Bad Request", "Missing required field: type");
                res.set_content(response.dump(), "application/json");
                res.status = 400;
                return;
            }
            
            // Create event
            nlohmann::json event;
            event["id"] = getCurrentTimestamp();
            event["type"] = body["type"];
            event["source"] = body.value("source", "rest_api");
            event["data"] = body.value("data", nlohmann::json::object());
            event["timestamp"] = getCurrentTimestamp();
            
            // Publish to event bus if available
            if (eventBus_) {
                // eventBus_->publish(event); // Would need to implement this
            }
            
            auto response = createSuccessResponse(event, "Event published successfully");
            res.set_content(response.dump(), "application/json");
            res.status = 201;
            
        } catch (const nlohmann::json::parse_error& e) {
            auto response = createErrorResponse(400, "Bad Request", "Invalid JSON: " + std::string(e.what()));
            res.set_content(response.dump(), "application/json");
            res.status = 400;
        }
    });
    
    impl_->server->Get("/api/v1/events/types", [this](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json eventTypes = nlohmann::json::array({
            "SYSTEM_STARTUP", "SYSTEM_SHUTDOWN", "SYSTEM_REBOOT", "SYSTEM_ERROR",
            "ANDROID_AUTO_CONNECTED", "ANDROID_AUTO_DISCONNECTED", "ANDROID_AUTO_START", "ANDROID_AUTO_STOP",
            "UI_BUTTON_PRESSED", "UI_BRIGHTNESS_CHANGED", "UI_VOLUME_CHANGED", "UI_MODE_CHANGED",
            "CAMERA_SHOW", "CAMERA_HIDE", "CAMERA_RECORD_START", "CAMERA_RECORD_STOP",
            "WIFI_CONNECTED", "WIFI_DISCONNECTED", "BLUETOOTH_CONNECTED", "BLUETOOTH_DISCONNECTED",
            "MEDIA_PLAY", "MEDIA_PAUSE", "MEDIA_STOP", "MEDIA_NEXT", "MEDIA_PREVIOUS"
        });
        
        auto response = createSuccessResponse(eventTypes);
        res.set_content(response.dump(), "application/json");
        res.status = 200;
    });
    
    // State endpoints
    impl_->server->Get("/api/v1/state", [this](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json state;
        
        if (stateMachine_) {
            // state["current"] = stateMachine_->getCurrentState(); // Would need to implement this
            state["current"] = "idle"; // Mock state
        } else {
            state["current"] = "unknown";
        }
        
        state["last_transition"] = getCurrentTimestamp();
        state["available_states"] = nlohmann::json::array({"idle", "connected", "projection", "error"});
        
        auto response = createSuccessResponse(state);
        res.set_content(response.dump(), "application/json");
        res.status = 200;
    });
    
    impl_->server->Post("/api/v1/state/transition", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = nlohmann::json::parse(req.body);
            
            if (!body.contains("state")) {
                auto response = createErrorResponse(400, "Bad Request", "Missing required field: state");
                res.set_content(response.dump(), "application/json");
                res.status = 400;
                return;
            }
            
            std::string newState = body["state"];
            
            // Trigger state transition
            if (stateMachine_) {
                // stateMachine_->transitionTo(newState); // Would need to implement this
            }
            
            nlohmann::json result;
            result["previous_state"] = "idle"; // Mock
            result["new_state"] = newState;
            result["transition_time"] = getCurrentTimestamp();
            
            auto response = createSuccessResponse(result, "State transition initiated");
            res.set_content(response.dump(), "application/json");
            res.status = 200;
            
        } catch (const nlohmann::json::parse_error& e) {
            auto response = createErrorResponse(400, "Bad Request", "Invalid JSON: " + std::string(e.what()));
            res.set_content(response.dump(), "application/json");
            res.status = 400;
        }
    });
    
    impl_->server->Get("/api/v1/state/history", [this](const httplib::Request& req, httplib::Response& res) {
        int limit = req.has_param("limit") ? std::stoi(req.get_param_value("limit")) : 10;
        
        nlohmann::json history = nlohmann::json::array();
        
        // Mock history
        for (int i = 0; i < limit; ++i) {
            nlohmann::json transition;
            transition["from"] = "idle";
            transition["to"] = "connected";
            transition["timestamp"] = getCurrentTimestamp() - (i * 60000); // 1 minute apart
            transition["duration"] = 500; // ms
            history.push_back(transition);
        }
        
        auto response = createSuccessResponse(history);
        res.set_content(response.dump(), "application/json");
        res.status = 200;
    });
    
    // Configuration endpoints
    impl_->server->Get("/api/v1/config", [this](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json config;
        
        if (configManager_) {
            // config = configManager_->getAllConfig(); // Would need to implement this
            // Mock configuration
            config["ui"]["brightness"] = 75;
            config["ui"]["theme"] = "dark";
            config["audio"]["volume"] = 80;
            config["audio"]["mute"] = false;
            config["network"]["wifi"]["enabled"] = true;
            config["android_auto"]["enabled"] = true;
        } else {
            config = nlohmann::json::object();
        }
        
        auto response = createSuccessResponse(config);
        res.set_content(response.dump(), "application/json");
        res.status = 200;
    });
    
    impl_->server->Get(R"(/api/v1/config/(.+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string key = req.matches[1];
        
        nlohmann::json value;
        
        if (configManager_) {
            // value = configManager_->getValue(key); // Would need to implement this
            // Mock values based on key
            if (key == "ui.brightness") value = 75;
            else if (key == "audio.volume") value = 80;
            else if (key == "ui.theme") value = "dark";
            else {
                auto response = createErrorResponse(404, "Not Found", "Configuration key not found: " + key);
                res.set_content(response.dump(), "application/json");
                res.status = 404;
                return;
            }
        } else {
            auto response = createErrorResponse(503, "Service Unavailable", "Configuration manager not available");
            res.set_content(response.dump(), "application/json");
            res.status = 503;
            return;
        }
        
        nlohmann::json result;
        result["key"] = key;
        result["value"] = value;
        
        auto response = createSuccessResponse(result);
        res.set_content(response.dump(), "application/json");
        res.status = 200;
    });
    
    impl_->server->Put(R"(/api/v1/config/(.+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string key = req.matches[1];
        
        try {
            auto body = nlohmann::json::parse(req.body);
            
            if (!body.contains("value")) {
                auto response = createErrorResponse(400, "Bad Request", "Missing required field: value");
                res.set_content(response.dump(), "application/json");
                res.status = 400;
                return;
            }
            
            auto value = body["value"];
            
            if (configManager_) {
                // configManager_->setValue(key, value); // Would need to implement this
            }
            
            nlohmann::json result;
            result["key"] = key;
            result["old_value"] = nullptr; // Would get from config manager
            result["new_value"] = value;
            result["updated_at"] = getCurrentTimestamp();
            
            auto response = createSuccessResponse(result, "Configuration updated successfully");
            res.set_content(response.dump(), "application/json");
            res.status = 200;
            
        } catch (const nlohmann::json::parse_error& e) {
            auto response = createErrorResponse(400, "Bad Request", "Invalid JSON: " + std::string(e.what()));
            res.set_content(response.dump(), "application/json");
            res.status = 400;
        }
    });
    
    impl_->server->Post("/api/v1/config/save", [this](const httplib::Request& req, httplib::Response& res) {
        if (configManager_) {
            // configManager_->save(); // Would need to implement this
        }
        
        nlohmann::json result;
        result["saved_at"] = getCurrentTimestamp();
        result["status"] = "saved";
        
        auto response = createSuccessResponse(result, "Configuration saved successfully");
        res.set_content(response.dump(), "application/json");
        res.status = 200;
    });
}

void RestApiServer::setupDocumentationRoutes() {
    // OpenAPI specification
    impl_->server->Get("/openapi.json", [this](const httplib::Request& req, httplib::Response& res) {
        auto spec = generateOpenApiSpec();
        res.set_content(spec.dump(2), "application/json");
        res.status = 200;
    });
    
    // Swagger UI
    impl_->server->Get("/docs", [this](const httplib::Request& req, httplib::Response& res) {
        std::string swaggerHtml = generateSwaggerUI();
        res.set_content(swaggerHtml, "text/html");
        res.status = 200;
    });
    
    // ReDoc
    impl_->server->Get("/redoc", [this](const httplib::Request& req, httplib::Response& res) {
        std::string redocHtml = generateReDocUI();
        res.set_content(redocHtml, "text/html");
        res.status = 200;
    });
}

void RestApiServer::setupCorsHandling() {
    impl_->server->set_pre_routing_handler([this](const httplib::Request& req, httplib::Response& res) {
        // Handle CORS preflight
        if (req.method == "OPTIONS") {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            res.set_header("Access-Control-Max-Age", "86400");
            res.status = 200;
            return httplib::Server::HandlerResponse::Handled;
        }
        
        // Add CORS headers to all responses
        res.set_header("Access-Control-Allow-Origin", "*");
        return httplib::Server::HandlerResponse::Unhandled;
    });
}

nlohmann::json RestApiServer::generateOpenApiSpec() const {
    nlohmann::json spec;
    
    spec["openapi"] = "3.0.0";
    
    // Info section
    nlohmann::json info;
    info["title"] = apiInfo_.title.empty() ? "OpenAuto REST API" : apiInfo_.title;
    info["version"] = apiInfo_.version.empty() ? "1.0.0" : apiInfo_.version;
    info["description"] = apiInfo_.description.empty() ? "REST API for OpenAuto Android Auto implementation" : apiInfo_.description;
    spec["info"] = info;
    
    // Servers
    spec["servers"] = nlohmann::json::array();
    for (const auto& server : servers_) {
        spec["servers"].push_back(server.toJson());
    }
    if (spec["servers"].empty()) {
        nlohmann::json defaultServer;
        defaultServer["url"] = "http://localhost:" + std::to_string(port_);
        defaultServer["description"] = "Development server";
        spec["servers"].push_back(defaultServer);
    }
    
    // Paths - simplified version
    spec["paths"] = nlohmann::json::object();
    
    // Health endpoint
    spec["paths"]["/health"]["get"]["summary"] = "Health check";
    spec["paths"]["/health"]["get"]["responses"]["200"]["description"] = "Service is healthy";
    
    // Events endpoints
    spec["paths"]["/api/v1/events"]["get"]["summary"] = "List events";
    spec["paths"]["/api/v1/events"]["post"]["summary"] = "Publish event";
    spec["paths"]["/api/v1/events/types"]["get"]["summary"] = "Get event types";
    
    // State endpoints
    spec["paths"]["/api/v1/state"]["get"]["summary"] = "Get current state";
    spec["paths"]["/api/v1/state/transition"]["post"]["summary"] = "Trigger state transition";
    
    // Config endpoints
    spec["paths"]["/api/v1/config"]["get"]["summary"] = "Get configuration";
    spec["paths"]["/api/v1/config/{key}"]["get"]["summary"] = "Get configuration value";
    spec["paths"]["/api/v1/config/{key}"]["put"]["summary"] = "Set configuration value";
    
    return spec;
}

std::string RestApiServer::generateSwaggerUI() const {
    return R"(<!DOCTYPE html>
<html>
<head>
    <title>OpenAuto API Documentation</title>
    <link rel="stylesheet" type="text/css" href="https://unpkg.com/swagger-ui-dist@3.52.5/swagger-ui.css" />
    <style>
        html { box-sizing: border-box; overflow: -moz-scrollbars-vertical; overflow-y: scroll; }
        *, *:before, *:after { box-sizing: inherit; }
        body { margin:0; background: #fafafa; }
    </style>
</head>
<body>
    <div id="swagger-ui"></div>
    <script src="https://unpkg.com/swagger-ui-dist@3.52.5/swagger-ui-bundle.js"></script>
    <script>
        SwaggerUIBundle({
            url: '/openapi.json',
            dom_id: '#swagger-ui',
            deepLinking: true,
            presets: [
                SwaggerUIBundle.presets.apis,
                SwaggerUIBundle.presets.standalone
            ],
            plugins: [
                SwaggerUIBundle.plugins.DownloadUrl
            ],
            layout: "StandaloneLayout"
        });
    </script>
</body>
</html>)";
}

std::string RestApiServer::generateReDocUI() const {
    return R"(<!DOCTYPE html>
<html>
<head>
    <title>OpenAuto API Documentation</title>
    <meta charset="utf-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="https://fonts.googleapis.com/css?family=Montserrat:300,400,700|Roboto:300,400,700" rel="stylesheet">
    <style>
        body { margin: 0; padding: 0; }
    </style>
</head>
<body>
    <redoc spec-url='/openapi.json'></redoc>
    <script src="https://cdn.jsdelivr.net/npm/redoc@2.0.0/bundles/redoc.standalone.js"></script>
</body>
</html>)";
}

// Stub implementations for helper classes
nlohmann::json ApiParameter::toJson() const {
    nlohmann::json j;
    j["name"] = name;
    j["type"] = "string";  // simplified
    j["required"] = required;
    return j;
}

nlohmann::json ApiResponse::toJson() const {
    nlohmann::json j;
    j["description"] = description;
    return j;
}

nlohmann::json ApiOperation::toJson() const {
    nlohmann::json j;
    j["summary"] = summary;
    j["description"] = description;
    return j;
}

nlohmann::json SecurityScheme::toJson() const {
    nlohmann::json j;
    j["type"] = "http";  // simplified
    return j;
}

nlohmann::json ServerInfo::toJson() const {
    nlohmann::json j;
    j["url"] = url;
    j["description"] = description;
    return j;
}

nlohmann::json ContactInfo::toJson() const {
    nlohmann::json j;
    if (!name.empty()) j["name"] = name;
    if (!url.empty()) j["url"] = url;
    if (!email.empty()) j["email"] = email;
    return j;
}

nlohmann::json LicenseInfo::toJson() const {
    nlohmann::json j;
    j["name"] = name;
    if (!url.empty()) j["url"] = url;
    return j;
}

nlohmann::json ApiInfo::toJson() const {
    nlohmann::json j;
    j["title"] = title;
    j["version"] = version;
    if (!description.empty()) j["description"] = description;
    return j;
}

// HttpRequest implementation  
HttpRequest::HttpRequest(const httplib::Request& req) : request_(&req) {
    // Store reference to the actual request
}

HttpMethod HttpRequest::getMethod() const {
    if (!request_) return HttpMethod::GET;
    
    if (request_->method == "GET") return HttpMethod::GET;
    if (request_->method == "POST") return HttpMethod::POST;
    if (request_->method == "PUT") return HttpMethod::PUT;
    if (request_->method == "DELETE") return HttpMethod::DEL;
    if (request_->method == "PATCH") return HttpMethod::PATCH;
    if (request_->method == "OPTIONS") return HttpMethod::OPTIONS;
    
    return HttpMethod::GET;
}

std::string HttpRequest::getPath() const {
    return request_ ? request_->path : "/";
}

std::string HttpRequest::getHeader(const std::string& name) const {
    if (!request_) return "";
    
    auto it = request_->headers.find(name);
    return it != request_->headers.end() ? it->second : "";
}

std::string HttpRequest::getQuery(const std::string& name) const {
    if (!request_) return "";
    
    return request_->has_param(name.c_str()) ? request_->get_param_value(name.c_str()) : "";
}

std::string HttpRequest::getPathParam(const std::string& name) const {
    auto it = pathParams_.find(name);
    return it != pathParams_.end() ? it->second : "";
}

std::string HttpRequest::getBody() const {
    return request_ ? request_->body : "";
}

std::string HttpRequest::getClientAddress() const {
    return request_ ? request_->remote_addr : "127.0.0.1";
}

nlohmann::json HttpRequest::getJsonBody() const {
    try {
        std::string body = getBody();
        return body.empty() ? nlohmann::json{} : nlohmann::json::parse(body);
    } catch (const nlohmann::json::parse_error&) {
        return nlohmann::json{};
    }
}

bool HttpRequest::hasHeader(const std::string& name) const {
    return request_ && request_->headers.find(name) != request_->headers.end();
}

bool HttpRequest::hasQuery(const std::string& name) const {
    return request_ && request_->has_param(name.c_str());
}

bool HttpRequest::hasPathParam(const std::string& name) const {
    return pathParams_.find(name) != pathParams_.end();
}

void HttpRequest::setPathParams(const std::map<std::string, std::string>& params) {
    pathParams_ = params;
}

// HttpResponse implementation
HttpResponse::HttpResponse() = default;

void HttpResponse::setStatus(int code) {
    statusCode_ = code;
}

void HttpResponse::setHeader(const std::string& name, const std::string& value) {
    headers_[name] = value;
}

void HttpResponse::setBody(const std::string& body) {
    body_ = body;
}

void HttpResponse::setJson(const nlohmann::json& json) {
    body_ = json.dump();
    contentType_ = "application/json";
}

void HttpResponse::setContentType(const std::string& contentType) {
    contentType_ = contentType;
}

int HttpResponse::getStatus() const {
    return statusCode_;
}

std::string HttpResponse::getHeader(const std::string& name) const {
    auto it = headers_.find(name);
    return it != headers_.end() ? it->second : "";
}

std::string HttpResponse::getBody() const {
    return body_;
}

std::string HttpResponse::getContentType() const {
    return contentType_;
}

void HttpResponse::applyTo(httplib::Response& res) const {
    res.status = statusCode_;
    res.body = body_;
    
    // Set content type
    res.set_header("Content-Type", contentType_);
    
    // Set all custom headers
    for (const auto& header : headers_) {
        res.set_header(header.first.c_str(), header.second.c_str());
    }
}

} // namespace modern  
} // namespace openauto
