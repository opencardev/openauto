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
#include <iostream>
#include <sstream>
#include <regex>

namespace openauto {
namespace modern {

RestApiServer::RestApiServer(
    int port,
    std::shared_ptr<EventBus> eventBus,
    std::shared_ptr<StateMachine> stateMachine,
    std::shared_ptr<ConfigurationManager> configManager)
    : port_(port)
    , running_(false)
    , eventBus_(eventBus)
    , stateMachine_(stateMachine)
    , configManager_(configManager)
    , apiVersion_("1.0.0")
    , apiTitle_("OpenAuto REST API")
    , apiDescription_("Modern REST API for OpenAuto event management, state control, and configuration")
{
    server_ = std::make_unique<httplib::Server>();
    setupRoutes();
}

RestApiServer::~RestApiServer() {
    stop();
}

bool RestApiServer::start() {
    if (running_) {
        return true;
    }
    
    setupCorsHeaders();
    
    serverThread_ = std::thread([this]() {
        running_ = true;
        std::cout << "Starting OpenAuto REST API server on port " << port_ << std::endl;
        server_->listen("0.0.0.0", port_);
        running_ = false;
    });
    
    // Give the server a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return running_;
}

void RestApiServer::stop() {
    if (!running_) {
        return;
    }
    
    std::cout << "Stopping OpenAuto REST API server" << std::endl;
    server_->stop();
    
    if (serverThread_.joinable()) {
        serverThread_.join();
    }
    
    running_ = false;
}

void RestApiServer::setupRoutes() {
    setupOpenApiRoutes();
    setupInfoRoutes();
    setupEventRoutes();
    setupStateRoutes();
    setupConfigRoutes();
}

void RestApiServer::setupCorsHeaders() {
    server_->set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
        res.set_header("Access-Control-Max-Age", "3600");
        return httplib::Server::HandlerResponse::Unhandled;
    });

    // Handle preflight OPTIONS requests
    server_->Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.status = 200;
        return;
    });
}

void RestApiServer::setupOpenApiRoutes() {
    // OpenAPI specification endpoint
    server_->Get("/openapi.json", [this](const httplib::Request&, httplib::Response& res) {
        auto spec = generateOpenApiSpec();
        res.set_content(spec.dump(2), "application/json");
    });
    
    // Swagger UI serving endpoint
    server_->Get("/docs", [](const httplib::Request&, httplib::Response& res) {
        std::string html = R"(
<!DOCTYPE html>
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
    <script src="https://unpkg.com/swagger-ui-dist@3.52.5/swagger-ui-standalone-preset.js"></script>
    <script>
        window.onload = function() {
            SwaggerUIBundle({
                url: '/openapi.json',
                dom_id: '#swagger-ui',
                deepLinking: true,
                presets: [
                    SwaggerUIBundle.presets.apis,
                    SwaggerUIStandalonePreset
                ],
                plugins: [
                    SwaggerUIBundle.plugins.DownloadUrl
                ],
                layout: "StandaloneLayout"
            });
        };
    </script>
</body>
</html>
        )";
        res.set_content(html, "text/html");
    });
}

void RestApiServer::setupInfoRoutes() {
    // Health check endpoint
    server_->Get("/health", [this](const httplib::Request&, httplib::Response& res) {
        nlohmann::json health = {
            {"status", "healthy"},
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()},
            {"version", apiVersion_},
            {"services", {
                {"eventBus", eventBus_ ? "available" : "unavailable"},
                {"stateMachine", stateMachine_ ? "available" : "unavailable"},
                {"configManager", configManager_ ? "available" : "unavailable"}
            }}
        };
        res.set_content(createSuccessResponse(health).dump(2), "application/json");
    });
    
    // API information endpoint
    server_->Get("/info", [this](const httplib::Request&, httplib::Response& res) {
        nlohmann::json info = {
            {"title", apiTitle_},
            {"description", apiDescription_},
            {"version", apiVersion_},
            {"contact", {
                {"name", "OpenCarDev Team"},
                {"url", "https://github.com/opencardev/openauto"}
            }},
            {"license", {
                {"name", "GPL-3.0"},
                {"url", "https://www.gnu.org/licenses/gpl-3.0.html"}
            }}
        };
        res.set_content(createSuccessResponse(info).dump(2), "application/json");
    });
}

void RestApiServer::setupEventRoutes() {
    // GET /api/v1/events - List events with pagination and filtering
    server_->Get("/api/v1/events", [this](const httplib::Request& req, httplib::Response& res) {
        if (!eventBus_) {
            res.status = 503;
            res.set_content(createErrorResponse("Event bus not available", 503).dump(), "application/json");
            return;
        }
        
        // Parse query parameters
        int page = std::stoi(req.get_param_value("page", "1"));
        int limit = std::stoi(req.get_param_value("limit", "50"));
        std::string eventType = req.get_param_value("type", "");
        std::string source = req.get_param_value("source", "");
        
        // Get events from event bus (implement getEvents method in EventBus)
        auto events = nlohmann::json::array();
        
        res.set_content(createPaginatedResponse(events, page, limit, 0).dump(2), "application/json");
    });
    
    // POST /api/v1/events - Publish a new event
    server_->Post("/api/v1/events", [this](const httplib::Request& req, httplib::Response& res) {
        if (!eventBus_) {
            res.status = 503;
            res.set_content(createErrorResponse("Event bus not available", 503).dump(), "application/json");
            return;
        }
        
        try {
            auto json = nlohmann::json::parse(req.body);
            
            if (!validateEventData(json)) {
                res.status = 400;
                res.set_content(createErrorResponse("Invalid event data").dump(), "application/json");
                return;
            }
            
            auto event = Event::fromJson(json);
            eventBus_->publish(event);
            
            nlohmann::json response = {
                {"id", event->toString()},
                {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                    event->getTimestamp().time_since_epoch()).count()}
            };
            
            res.status = 201;
            res.set_content(createSuccessResponse(response, "Event published successfully").dump(2), "application/json");
            
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(createErrorResponse("Invalid JSON: " + std::string(e.what())).dump(), "application/json");
        }
    });
    
    // GET /api/v1/events/types - Get available event types
    server_->Get("/api/v1/events/types", [this](const httplib::Request&, httplib::Response& res) {
        nlohmann::json types = nlohmann::json::array();
        
        // Add all event types
        for (int i = 0; i <= static_cast<int>(EventType::CUSTOM_EVENT); ++i) {
            auto eventType = static_cast<EventType>(i);
            types.push_back({
                {"name", Event::eventTypeToString(eventType)},
                {"value", i},
                {"description", "Event type: " + Event::eventTypeToString(eventType)}
            });
        }
        
        res.set_content(createSuccessResponse(types).dump(2), "application/json");
    });
}

void RestApiServer::setupStateRoutes() {
    // GET /api/v1/state - Get current state
    server_->Get("/api/v1/state", [this](const httplib::Request&, httplib::Response& res) {
        if (!stateMachine_) {
            res.status = 503;
            res.set_content(createErrorResponse("State machine not available", 503).dump(), "application/json");
            return;
        }
        
        nlohmann::json state = {
            {"current", stateMachine_->getCurrentStateName()},
            {"previous", stateMachine_->getPreviousStateName()},
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()}
        };
        
        res.set_content(createSuccessResponse(state).dump(2), "application/json");
    });
    
    // POST /api/v1/state/transition - Trigger state transition
    server_->Post("/api/v1/state/transition", [this](const httplib::Request& req, httplib::Response& res) {
        if (!stateMachine_) {
            res.status = 503;
            res.set_content(createErrorResponse("State machine not available", 503).dump(), "application/json");
            return;
        }
        
        try {
            auto json = nlohmann::json::parse(req.body);
            
            if (!json.contains("target") || !json["target"].is_string()) {
                res.status = 400;
                res.set_content(createErrorResponse("Missing or invalid 'target' field").dump(), "application/json");
                return;
            }
            
            std::string targetState = json["target"];
            bool success = stateMachine_->transitionTo(targetState);
            
            if (success) {
                nlohmann::json response = {
                    {"previous", stateMachine_->getPreviousStateName()},
                    {"current", stateMachine_->getCurrentStateName()},
                    {"success", true}
                };
                res.set_content(createSuccessResponse(response, "State transition successful").dump(2), "application/json");
            } else {
                res.status = 400;
                res.set_content(createErrorResponse("State transition failed", 400, "Invalid state or transition not allowed").dump(), "application/json");
            }
            
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(createErrorResponse("Invalid JSON: " + std::string(e.what())).dump(), "application/json");
        }
    });
    
    // GET /api/v1/state/history - Get state transition history
    server_->Get("/api/v1/state/history", [this](const httplib::Request& req, httplib::Response& res) {
        if (!stateMachine_) {
            res.status = 503;
            res.set_content(createErrorResponse("State machine not available", 503).dump(), "application/json");
            return;
        }
        
        int limit = std::stoi(req.get_param_value("limit", "100"));
        auto history = stateMachine_->getHistory(limit);
        
        res.set_content(createSuccessResponse(history).dump(2), "application/json");
    });
}

void RestApiServer::setupConfigRoutes() {
    // GET /api/v1/config - Get all configuration
    server_->Get("/api/v1/config", [this](const httplib::Request&, httplib::Response& res) {
        if (!configManager_) {
            res.status = 503;
            res.set_content(createErrorResponse("Configuration manager not available", 503).dump(), "application/json");
            return;
        }
        
        auto config = configManager_->getAllConfig();
        res.set_content(createSuccessResponse(config).dump(2), "application/json");
    });
    
    // GET /api/v1/config/{key} - Get specific configuration value
    server_->Get(R"(/api/v1/config/([^/]+))", [this](const httplib::Request& req, httplib::Response& res) {
        if (!configManager_) {
            res.status = 503;
            res.set_content(createErrorResponse("Configuration manager not available", 503).dump(), "application/json");
            return;
        }
        
        std::string key = req.matches[1];
        auto value = configManager_->get(key);
        
        if (value.is_null()) {
            res.status = 404;
            res.set_content(createErrorResponse("Configuration key not found", 404).dump(), "application/json");
            return;
        }
        
        nlohmann::json response = {
            {"key", key},
            {"value", value}
        };
        
        res.set_content(createSuccessResponse(response).dump(2), "application/json");
    });
    
    // PUT /api/v1/config/{key} - Set specific configuration value
    server_->Put(R"(/api/v1/config/([^/]+))", [this](const httplib::Request& req, httplib::Response& res) {
        if (!configManager_) {
            res.status = 503;
            res.set_content(createErrorResponse("Configuration manager not available", 503).dump(), "application/json");
            return;
        }
        
        try {
            std::string key = req.matches[1];
            auto json = nlohmann::json::parse(req.body);
            
            if (!json.contains("value")) {
                res.status = 400;
                res.set_content(createErrorResponse("Missing 'value' field").dump(), "application/json");
                return;
            }
            
            configManager_->set(key, json["value"]);
            
            nlohmann::json response = {
                {"key", key},
                {"value", json["value"]},
                {"updated", true}
            };
            
            res.set_content(createSuccessResponse(response, "Configuration updated successfully").dump(2), "application/json");
            
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(createErrorResponse("Invalid JSON: " + std::string(e.what())).dump(), "application/json");
        }
    });
    
    // POST /api/v1/config/save - Save configuration to file
    server_->Post("/api/v1/config/save", [this](const httplib::Request&, httplib::Response& res) {
        if (!configManager_) {
            res.status = 503;
            res.set_content(createErrorResponse("Configuration manager not available", 503).dump(), "application/json");
            return;
        }
        
        bool success = configManager_->save();
        
        if (success) {
            res.set_content(createSuccessResponse(nlohmann::json::object(), "Configuration saved successfully").dump(2), "application/json");
        } else {
            res.status = 500;
            res.set_content(createErrorResponse("Failed to save configuration", 500).dump(), "application/json");
        }
    });
}

nlohmann::json RestApiServer::generateOpenApiSpec() {
    return nlohmann::json{
        {"openapi", "3.0.3"},
        {"info", {
            {"title", apiTitle_},
            {"description", apiDescription_},
            {"version", apiVersion_},
            {"contact", {
                {"name", "OpenCarDev Team"},
                {"url", "https://github.com/opencardev/openauto"}
            }},
            {"license", {
                {"name", "GPL-3.0"},
                {"url", "https://www.gnu.org/licenses/gpl-3.0.html"}
            }}
        }},
        {"servers", nlohmann::json::array({
            {{"url", "http://localhost:" + std::to_string(port_)}, {"description", "Local development server"}}
        })},
        {"paths", {
            {"/health", {
                {"get", {
                    {"summary", "Health check"},
                    {"description", "Check the health status of the API and its dependencies"},
                    {"responses", {
                        {"200", {
                            {"description", "Service is healthy"},
                            {"content", {
                                {"application/json", {
                                    {"schema", {{"$ref", "#/components/schemas/SuccessResponse"}}}
                                }}
                            }}
                        }}
                    }}
                }}
            }},
            {"/api/v1/events", {
                {"get", {
                    {"summary", "List events"},
                    {"description", "Retrieve a paginated list of events with optional filtering"},
                    {"parameters", nlohmann::json::array({
                        {
                            {"name", "page"},
                            {"in", "query"},
                            {"description", "Page number"},
                            {"schema", {{"type", "integer"}, {"default", 1}, {"minimum", 1}}}
                        },
                        {
                            {"name", "limit"},
                            {"in", "query"},
                            {"description", "Number of events per page"},
                            {"schema", {{"type", "integer"}, {"default", 50}, {"minimum", 1}, {"maximum", 1000}}}
                        },
                        {
                            {"name", "type"},
                            {"in", "query"},
                            {"description", "Filter by event type"},
                            {"schema", {{"type", "string"}}}
                        }
                    })},
                    {"responses", {
                        {"200", {
                            {"description", "List of events"},
                            {"content", {
                                {"application/json", {
                                    {"schema", {{"$ref", "#/components/schemas/PaginatedResponse"}}}
                                }}
                            }}
                        }}
                    }}
                }},
                {"post", {
                    {"summary", "Publish event"},
                    {"description", "Publish a new event to the event bus"},
                    {"requestBody", {
                        {"required", true},
                        {"content", {
                            {"application/json", {
                                {"schema", {{"$ref", "#/components/schemas/Event"}}}
                            }}
                        }}
                    }},
                    {"responses", {
                        {"201", {
                            {"description", "Event published successfully"},
                            {"content", {
                                {"application/json", {
                                    {"schema", {{"$ref", "#/components/schemas/SuccessResponse"}}}
                                }}
                            }}
                        }},
                        {"400", {
                            {"description", "Invalid event data"},
                            {"content", {
                                {"application/json", {
                                    {"schema", {{"$ref", "#/components/schemas/ErrorResponse"}}}
                                }}
                            }}
                        }}
                    }}
                }}
            }},
            {"/api/v1/state", {
                {"get", {
                    {"summary", "Get current state"},
                    {"description", "Get the current state of the state machine"},
                    {"responses", {
                        {"200", {
                            {"description", "Current state information"},
                            {"content", {
                                {"application/json", {
                                    {"schema", {{"$ref", "#/components/schemas/StateResponse"}}}
                                }}
                            }}
                        }}
                    }}
                }}
            }}
        }},
        {"components", {
            {"schemas", {
                {"Event", getEventSchema()},
                {"State", getStateSchema()},
                {"Config", getConfigSchema()},
                {"SuccessResponse", {
                    {"type", "object"},
                    {"properties", {
                        {"success", {{"type", "boolean"}, {"example", true}}},
                        {"message", {{"type", "string"}, {"example", "Operation completed successfully"}}},
                        {"data", {{"type", "object"}}},
                        {"timestamp", {{"type", "integer"}, {"format", "int64"}}}
                    }}
                }},
                {"ErrorResponse", getErrorSchema()},
                {"PaginatedResponse", {
                    {"type", "object"},
                    {"properties", {
                        {"success", {{"type", "boolean"}, {"example", true}}},
                        {"data", {{"type", "array"}, {"items", {{"type", "object"}}}}},
                        {"pagination", {
                            {"type", "object"},
                            {"properties", {
                                {"page", {{"type", "integer"}}},
                                {"limit", {{"type", "integer"}}},
                                {"total", {{"type", "integer"}}},
                                {"pages", {{"type", "integer"}}}
                            }}
                        }}
                    }}
                }},
                {"StateResponse", {
                    {"type", "object"},
                    {"properties", {
                        {"success", {{"type", "boolean"}}},
                        {"data", {
                            {"type", "object"},
                            {"properties", {
                                {"current", {{"type", "string"}}},
                                {"previous", {{"type", "string"}}},
                                {"timestamp", {{"type", "integer"}, {"format", "int64"}}}
                            }}
                        }}
                    }}
                }}
            }}
        }}
    };
}

nlohmann::json RestApiServer::getEventSchema() {
    return nlohmann::json{
        {"type", "object"},
        {"required", nlohmann::json::array({"type", "source"})},
        {"properties", {
            {"type", {
                {"type", "string"},
                {"enum", nlohmann::json::array({
                    "SYSTEM_STARTUP", "SYSTEM_SHUTDOWN", "ANDROID_AUTO_CONNECTED",
                    "UI_BUTTON_PRESSED", "CAMERA_SHOW", "WIFI_CONNECTED", "MEDIA_PLAY"
                })},
                {"description", "Type of the event"}
            }},
            {"source", {
                {"type", "string"},
                {"description", "Source component that generated the event"}
            }},
            {"data", {
                {"type", "object"},
                {"additionalProperties", true},
                {"description", "Additional event data"}
            }},
            {"timestamp", {
                {"type", "integer"},
                {"format", "int64"},
                {"description", "Event timestamp in milliseconds since epoch"}
            }}
        }}
    };
}

nlohmann::json RestApiServer::getStateSchema() {
    return nlohmann::json{
        {"type", "object"},
        {"properties", {
            {"current", {{"type", "string"}, {"description", "Current state name"}}},
            {"previous", {{"type", "string"}, {"description", "Previous state name"}}},
            {"timestamp", {{"type", "integer"}, {"format", "int64"}}}
        }}
    };
}

nlohmann::json RestApiServer::getConfigSchema() {
    return nlohmann::json{
        {"type", "object"},
        {"additionalProperties", true},
        {"description", "Configuration key-value pairs"}
    };
}

nlohmann::json RestApiServer::getErrorSchema() {
    return nlohmann::json{
        {"type", "object"},
        {"properties", {
            {"success", {{"type", "boolean"}, {"example", false}}},
            {"error", {
                {"type", "object"},
                {"properties", {
                    {"code", {{"type", "integer"}, {"example", 400}}},
                    {"message", {{"type", "string"}, {"example", "Bad Request"}}},
                    {"detail", {{"type", "string"}, {"example", "Invalid input data"}}}
                }}
            }},
            {"timestamp", {{"type", "integer"}, {"format", "int64"}}}
        }}
    };
}

nlohmann::json RestApiServer::createErrorResponse(const std::string& message, int code, const std::string& detail) {
    nlohmann::json error = {
        {"success", false},
        {"error", {
            {"code", code},
            {"message", message}
        }},
        {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count()}
    };
    
    if (!detail.empty()) {
        error["error"]["detail"] = detail;
    }
    
    return error;
}

nlohmann::json RestApiServer::createSuccessResponse(const nlohmann::json& data, const std::string& message) {
    return nlohmann::json{
        {"success", true},
        {"message", message},
        {"data", data},
        {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count()}
    };
}

nlohmann::json RestApiServer::createPaginatedResponse(const nlohmann::json& data, int page, int limit, int total) {
    int pages = (total + limit - 1) / limit;
    
    return nlohmann::json{
        {"success", true},
        {"data", data},
        {"pagination", {
            {"page", page},
            {"limit", limit},
            {"total", total},
            {"pages", pages}
        }},
        {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count()}
    };
}

bool RestApiServer::validateEventData(const nlohmann::json& json) {
    return json.contains("type") && json["type"].is_string() &&
           json.contains("source") && json["source"].is_string();
}

bool RestApiServer::validateConfigData(const nlohmann::json& json) {
    return json.contains("value");
}

std::string RestApiServer::extractBearerToken(const httplib::Request& req) {
    auto auth_header = req.get_header_value("Authorization");
    if (auth_header.empty()) {
        return "";
    }
    
    std::regex bearer_regex("Bearer\\s+(.+)");
    std::smatch matches;
    if (std::regex_match(auth_header, matches, bearer_regex)) {
        return matches[1];
    }
    
    return "";
}

bool RestApiServer::isAuthenticated(const httplib::Request& req) {
    // For now, allow all requests. In a production environment,
    // implement proper authentication here
    return true;
}

} // namespace modern
} // namespace openauto
