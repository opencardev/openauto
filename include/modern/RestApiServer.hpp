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

/**
 * @file RestApiServer.hpp
 * @brief Modern REST API Server implementation with OpenAPI 3.0 support
 * 
 * This file provides a comprehensive REST API server framework for the OpenAuto project,
 * featuring OpenAPI 3.0 specification compliance, automatic documentation generation,
 * middleware support, authentication, CORS handling, and integration with the modern
 * architecture components (EventBus, StateMachine, ConfigurationManager).
 * 
 * Key Features:
 * - OpenAPI 3.0 compliant REST API endpoints
 * - Automatic Swagger UI and ReDoc documentation generation
 * - Route management with middleware support
 * - Authentication and authorization
 * - CORS (Cross-Origin Resource Sharing) support
 * - JSON request/response handling
 * - Thread-safe operation
 * - Integration with OpenAuto's modern architecture
 * 
 * @example Basic Usage:
 * @code
 * auto server = std::make_shared<RestApiServer>(8080, eventBus, stateMachine, configManager);
 * 
 * // Configure API information
 * ApiInfo info;
 * info.title = "OpenAuto REST API";
 * info.version = "1.0.0";
 * info.description = "REST API for OpenAuto Android Auto implementation";
 * server->setApiInfo(info);
 * 
 * // Add a simple route
 * server->addRoute(HttpMethod::GET, "/api/status", [](const HttpRequest& req) {
 *     HttpResponse res;
 *     res.setJson({{"status", "running"}, {"timestamp", std::time(nullptr)}});
 *     return res;
 * });
 * 
 * // Enable documentation
 * server->enableSwaggerUI("/docs");
 * server->enableReDoc("/redoc");
 * 
 * // Start the server
 * server->start();
 * @endcode
 * 
 * @author OpenCarDev Team
 * @version 1.0.0
 * @date 2025-07-12
 */

#pragma once

#include <memory>
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <nlohmann/json.hpp>

// Forward declaration for httplib
namespace httplib {
    class Request;
    class Response;
    class Server;
}

// Forward declarations
namespace openauto {
namespace modern {
    class EventBus;
    class StateMachine;
    class ConfigurationManager;
}
}

namespace openauto {
namespace modern {

/**
 * @brief HTTP method enumeration for REST API operations
 * 
 * Defines the standard HTTP methods supported by the REST API server.
 * These correspond to CRUD operations and follow RESTful conventions.
 */
enum class HttpMethod {
    GET,     ///< Retrieve resources (Read operation)
    POST,    ///< Create new resources
    PUT,     ///< Update/replace entire resources
    DEL,     ///< Remove resources (avoiding DELETE keyword conflict)
    PATCH,   ///< Partial resource updates
    OPTIONS  ///< CORS preflight and API discovery
};

/**
 * @brief OpenAPI parameter types for schema definition
 * 
 * Defines the JSON Schema types supported for API parameters.
 * These types are used for request validation and documentation generation.
 */
enum class ParameterType {
    STRING,   ///< Text values, URLs, IDs
    INTEGER,  ///< Whole numbers (32-bit)
    NUMBER,   ///< Floating-point numbers
    BOOLEAN,  ///< True/false values
    ARRAY,    ///< Lists/arrays of values
    OBJECT    ///< Complex nested objects
};

/**
 * @brief OpenAPI parameter location specification
 * 
 * Defines where parameters can be located in HTTP requests.
 * This affects how parameters are extracted and validated.
 */
enum class ParameterIn {
    QUERY,   ///< URL query string parameters (?key=value)
    HEADER,  ///< HTTP headers (Authorization, Content-Type, etc.)
    PATH,    ///< URL path segments (/users/{id})
    COOKIE,  ///< HTTP cookies
    BODY     ///< Request body (JSON, form data, etc.)
};

/**
 * @brief OpenAPI parameter definition structure
 * 
 * Represents a single parameter in an API operation, including its type,
 * location, validation rules, and documentation. Used for automatic
 * request validation and OpenAPI specification generation.
 * 
 * @example Parameter definition:
 * @code
 * ApiParameter userIdParam;
 * userIdParam.name = "userId";
 * userIdParam.type = ParameterType::INTEGER;
 * userIdParam.in = ParameterIn::PATH;
 * userIdParam.required = true;
 * userIdParam.description = "Unique identifier for the user";
 * userIdParam.example = "12345";
 * @endcode
 */
struct ApiParameter {
    std::string name;         ///< Parameter name (e.g., "userId", "limit")
    ParameterType type;       ///< Data type for validation
    ParameterIn in;          ///< Location in the HTTP request
    bool required = false;   ///< Whether parameter is mandatory
    std::string description; ///< Human-readable description
    std::string example;     ///< Example value for documentation
    std::string defaultValue; ///< Default value if not provided
    
    /**
     * @brief Convert parameter to OpenAPI JSON representation
     * @return JSON object conforming to OpenAPI 3.0 parameter schema
     */
    nlohmann::json toJson() const;
};

/**
 * @brief OpenAPI response definition structure
 * 
 * Defines a possible response from an API operation, including status code,
 * description, content type, and example payload. Multiple responses can
 * be defined for different status codes (200, 400, 404, 500, etc.).
 * 
 * @example Response definition:
 * @code
 * ApiResponse successResponse;
 * successResponse.statusCode = 200;
 * successResponse.description = "User successfully retrieved";
 * successResponse.contentType = "application/json";
 * successResponse.example = R"({"id": 123, "name": "John Doe"})";
 * @endcode
 */
struct ApiResponse {
    int statusCode;                              ///< HTTP status code (200, 404, 500, etc.)
    std::string description;                     ///< Description of when this response occurs
    std::string contentType = "application/json"; ///< MIME type of response body
    std::string example;                         ///< Example response payload
    
    /**
     * @brief Convert response to OpenAPI JSON representation
     * @return JSON object conforming to OpenAPI 3.0 response schema
     */
    nlohmann::json toJson() const;
};

/**
 * @brief OpenAPI operation definition structure
 * 
 * Represents a complete API operation (endpoint), including all metadata
 * needed for documentation and validation. Contains parameters, responses,
 * tags for grouping, and other OpenAPI specification details.
 * 
 * @example Operation definition:
 * @code
 * ApiOperation getUserOp;
 * getUserOp.operationId = "getUser";
 * getUserOp.summary = "Get user by ID";
 * getUserOp.description = "Retrieves a user's details by their unique identifier";
 * getUserOp.tags = {"Users", "Authentication"};
 * getUserOp.parameters = {userIdParam};
 * getUserOp.responses = {successResponse, notFoundResponse};
 * @endcode
 */
struct ApiOperation {
    std::string operationId;                  ///< Unique identifier for this operation
    std::string summary;                      ///< Brief description (shown in UI)
    std::string description;                  ///< Detailed description with usage notes
    std::vector<std::string> tags;           ///< Tags for grouping operations
    std::vector<ApiParameter> parameters;    ///< Input parameters for this operation
    std::vector<ApiResponse> responses;      ///< Possible responses from this operation
    bool deprecated = false;                 ///< Whether this operation is deprecated
    
    /**
     * @brief Convert operation to OpenAPI JSON representation
     * @return JSON object conforming to OpenAPI 3.0 operation schema
     */
    nlohmann::json toJson() const;
};

/**
 * @brief HTTP request wrapper providing convenient access to request data
 * 
 * Abstracts the underlying HTTP library and provides a clean interface
 * for accessing request information such as headers, query parameters,
 * path parameters, and body content. Supports JSON parsing and validation.
 * 
 * @example Using HttpRequest:
 * @code
 * auto handler = [](const HttpRequest& req) {
 *     std::string userId = req.getPathParam("id");
 *     std::string authToken = req.getHeader("Authorization");
 *     auto jsonBody = req.getJsonBody();
 *     
 *     HttpResponse res;
 *     if (!authToken.empty()) {
 *         res.setJson({{"userId", userId}, {"data", jsonBody}});
 *     } else {
 *         res.setStatus(401);
 *         res.setJson({{"error", "Unauthorized"}});
 *     }
 *     return res;
 * };
 * @endcode
 */
class HttpRequest {
public:
    /**
     * @brief Constructor from underlying HTTP library request
     * @param req The underlying httplib::Request object
     */
    explicit HttpRequest(const httplib::Request& req);
    
    /**
     * @brief Get the HTTP method of this request
     * @return HttpMethod enum value (GET, POST, etc.)
     */
    HttpMethod getMethod() const;
    
    /**
     * @brief Get the request path (without query string)
     * @return URL path (e.g., "/api/users/123")
     */
    std::string getPath() const;
    
    /**
     * @brief Get a header value by name
     * @param name Header name (case-insensitive)
     * @return Header value or empty string if not found
     */
    std::string getHeader(const std::string& name) const;
    
    /**
     * @brief Get a query parameter value by name
     * @param name Query parameter name
     * @return Parameter value or empty string if not found
     */
    std::string getQuery(const std::string& name) const;
    
    /**
     * @brief Get a path parameter value by name
     * @param name Path parameter name (from route pattern like /users/{id})
     * @return Parameter value or empty string if not found
     */
    std::string getPathParam(const std::string& name) const;
    
    /**
     * @brief Get the raw request body as string
     * @return Request body content
     */
    std::string getBody() const;
    
    /**
     * @brief Get the client IP address
     * @return Client IP address string
     */
    std::string getClientAddress() const;
    
    /**
     * @brief Parse request body as JSON
     * @return Parsed JSON object or null if parsing fails
     * @throws nlohmann::json::parse_error if body is not valid JSON
     */
    nlohmann::json getJsonBody() const;
    
    /**
     * @brief Check if a header exists
     * @param name Header name to check
     * @return true if header exists, false otherwise
     */
    bool hasHeader(const std::string& name) const;
    
    /**
     * @brief Check if a query parameter exists
     * @param name Query parameter name to check
     * @return true if parameter exists, false otherwise
     */
    bool hasQuery(const std::string& name) const;
    
    /**
     * @brief Check if a path parameter exists
     * @param name Path parameter name to check
     * @return true if parameter exists, false otherwise
     */
    bool hasPathParam(const std::string& name) const;
    
    /**
     * @brief Set path parameters (used internally by router)
     * @param params Map of parameter names to values
     */
    void setPathParams(const std::map<std::string, std::string>& params);

private:
    const httplib::Request* request_ = nullptr;  ///< Reference to underlying request
    std::map<std::string, std::string> pathParams_; ///< Extracted path parameters
};

/**
 * @brief HTTP response builder for constructing API responses
 * 
 * Provides a fluent interface for building HTTP responses with proper
 * status codes, headers, and content. Supports JSON serialization,
 * custom content types, and header management.
 * 
 * @example Building responses:
 * @code
 * HttpResponse res;
 * res.setStatus(200);
 * res.setHeader("Cache-Control", "no-cache");
 * res.setJson({
 *     {"status", "success"},
 *     {"data", {{"id", 123}, {"name", "John"}}},
 *     {"timestamp", std::time(nullptr)}
 * });
 * return res;
 * @endcode
 */
class HttpResponse {
public:
    /**
     * @brief Default constructor with 200 OK status
     */
    HttpResponse();
    
    /**
     * @brief Set HTTP status code
     * @param code HTTP status code (200, 404, 500, etc.)
     */
    void setStatus(int code);
    
    /**
     * @brief Set a response header
     * @param name Header name (e.g., "Content-Type", "Cache-Control")
     * @param value Header value
     */
    void setHeader(const std::string& name, const std::string& value);
    
    /**
     * @brief Set response body as plain text
     * @param body Response body content
     */
    void setBody(const std::string& body);
    
    /**
     * @brief Set response body as JSON and set appropriate content type
     * @param json JSON object to serialize as response body
     */
    void setJson(const nlohmann::json& json);
    
    /**
     * @brief Set response content type
     * @param contentType MIME type (e.g., "application/json", "text/html")
     */
    void setContentType(const std::string& contentType);
    
    /**
     * @brief Get current status code
     * @return HTTP status code
     */
    int getStatus() const;
    
    /**
     * @brief Get a response header value
     * @param name Header name
     * @return Header value or empty string if not set
     */
    std::string getHeader(const std::string& name) const;
    
    /**
     * @brief Get response body content
     * @return Response body as string
     */
    std::string getBody() const;
    
    /**
     * @brief Get response content type
     * @return Current content type
     */
    std::string getContentType() const;
    
    /**
     * @brief Apply this response to underlying HTTP library response
     * @param res The underlying httplib::Response object to modify
     */
    void applyTo(httplib::Response& res) const;

private:
    int statusCode_ = 200;                              ///< HTTP status code
    std::map<std::string, std::string> headers_;       ///< Response headers
    std::string body_;                                  ///< Response body content
    std::string contentType_ = "application/json";     ///< Content type header
};

/**
 * @brief Route handler function type
 * 
 * Function signature for handling HTTP requests. Route handlers receive
 * an HttpRequest object and must return an HttpResponse object.
 * 
 * @param request The incoming HTTP request
 * @return HttpResponse object with status, headers, and body
 */
using RouteHandler = std::function<HttpResponse(const HttpRequest&)>;

/**
 * @brief Middleware function type
 * 
 * Function signature for middleware that can intercept and modify requests
 * and responses. Middleware can perform authentication, logging, validation,
 * etc. Return true to continue processing, false to stop and return current response.
 * 
 * @param request The HTTP request (can be modified)
 * @param response The HTTP response (can be modified)
 * @return true to continue to next middleware/handler, false to stop processing
 */
using MiddlewareHandler = std::function<bool(HttpRequest&, HttpResponse&)>;

/**
 * @brief Route definition structure
 * 
 * Combines an HTTP method, URL pattern, handler function, OpenAPI operation
 * metadata, and middleware stack into a complete route definition.
 * 
 * @example Route definition:
 * @code
 * Route userRoute;
 * userRoute.method = HttpMethod::GET;
 * userRoute.path = "/api/users/{id}";
 * userRoute.handler = getUserHandler;
 * userRoute.operation = getUserOperation;
 * userRoute.middlewares = {authMiddleware, loggingMiddleware};
 * @endcode
 */
struct Route {
    HttpMethod method;                           ///< HTTP method for this route
    std::string path;                           ///< URL pattern (supports {param} placeholders)
    RouteHandler handler;                       ///< Function to handle requests to this route
    ApiOperation operation;                     ///< OpenAPI metadata for documentation
    std::vector<MiddlewareHandler> middlewares; ///< Route-specific middleware stack
};

/**
 * @brief Security scheme types for OpenAPI authentication
 * 
 * Defines the types of authentication mechanisms supported by the API.
 * Used for OpenAPI specification generation and documentation.
 */
enum class SecurityType {
    HTTP,              ///< HTTP authentication (Basic, Bearer, etc.)
    API_KEY,          ///< API key in header, query, or cookie
    OAUTH2,           ///< OAuth 2.0 flows
    OPEN_ID_CONNECT   ///< OpenID Connect Discovery
};

/**
 * @brief Security scheme definition
 */
struct SecurityScheme {
    SecurityType type;
    std::string scheme; // For HTTP type (basic, bearer, etc.)
    std::string bearerFormat; // For bearer tokens
    std::string name; // For API key
    ParameterIn in; // For API key location
    std::string description;
    
    nlohmann::json toJson() const;
};

/**
 * @brief OpenAPI server information
 */
struct ServerInfo {
    std::string url;
    std::string description;
    
    nlohmann::json toJson() const;
};

/**
 * @brief OpenAPI contact information
 */
struct ContactInfo {
    std::string name;
    std::string url;
    std::string email;
    
    nlohmann::json toJson() const;
};

/**
 * @brief OpenAPI license information
 */
struct LicenseInfo {
    std::string name;
    std::string url;
    
    nlohmann::json toJson() const;
};

/**
 * @brief OpenAPI info object
 */
struct ApiInfo {
    std::string title;
    std::string description;
    std::string version;
    std::string termsOfService;
    ContactInfo contact;
    LicenseInfo license;
    
    nlohmann::json toJson() const;
};

/**
 * @brief Modern REST API Server with comprehensive OpenAPI 3.0 support
 * 
 * This class provides a full-featured REST API server implementation that integrates
 * seamlessly with OpenAuto's modern architecture. It offers:
 * 
 * Core Features:
 * - OpenAPI 3.0 compliant endpoint definition and documentation
 * - Automatic Swagger UI and ReDoc documentation generation
 * - Route management with pattern matching and parameter extraction
 * - Middleware pipeline for cross-cutting concerns (auth, logging, validation)
 * - JSON request/response handling with automatic serialization
 * - CORS support for web application integration
 * - Thread-safe concurrent request handling
 * - Integration with EventBus, StateMachine, and ConfigurationManager
 * 
 * Security Features:
 * - Pluggable authentication mechanisms (Bearer tokens, API keys, etc.)
 * - Path-based authorization controls
 * - HTTPS support (when properly configured)
 * - Request validation against OpenAPI schemas
 * 
 * Development Features:
 * - Hot-reloadable route configuration
 * - Comprehensive error handling and reporting
 * - Request/response logging and metrics
 * - Health check endpoints
 * - API versioning support
 * 
 * @example Complete server setup:
 * @code
 * // Initialize server with dependencies
 * auto server = std::make_shared<RestApiServer>(8080, eventBus, stateMachine, configManager);
 * 
 * // Configure API metadata
 * ApiInfo apiInfo;
 * apiInfo.title = "OpenAuto REST API";
 * apiInfo.version = "1.0.0";
 * apiInfo.description = "REST API for OpenAuto Android Auto implementation";
 * apiInfo.contact.name = "OpenCarDev Team";
 * apiInfo.contact.url = "https://github.com/opencardev/openauto";
 * apiInfo.license.name = "GPL-3.0";
 * server->setApiInfo(apiInfo);
 * 
 * // Add server information
 * ServerInfo serverInfo;
 * serverInfo.url = "http://localhost:8080";
 * serverInfo.description = "Development server";
 * server->addServer(serverInfo);
 * 
 * // Configure authentication
 * server->setAuthenticationHandler([](const HttpRequest& req) {
 *     std::string authHeader = req.getHeader("Authorization");
 *     return authHeader.starts_with("Bearer ") && validateToken(authHeader.substr(7));
 * });
 * 
 * // Add global middleware
 * server->addGlobalMiddleware([](HttpRequest& req, HttpResponse& res) {
 *     Logger::getInstance().info(LogCategory::API, "RestApi", "middleware", __FILE__, __LINE__,
 *                               "Request: " + req.getMethod() + " " + req.getPath());
 *     return true; // Continue processing
 * });
 * 
 * // Define API operations
 * ApiOperation statusOp;
 * statusOp.operationId = "getStatus";
 * statusOp.summary = "Get system status";
 * statusOp.description = "Returns current system status and health information";
 * statusOp.tags = {"System"};
 * 
 * ApiResponse okResponse;
 * okResponse.statusCode = 200;
 * okResponse.description = "Status retrieved successfully";
 * okResponse.example = R"({"status":"running","uptime":3600,"version":"1.0.0"})";
 * statusOp.responses = {okResponse};
 * 
 * // Add routes
 * server->addRoute(HttpMethod::GET, "/api/status", [](const HttpRequest& req) {
 *     HttpResponse res;
 *     res.setJson({
 *         {"status", "running"},
 *         {"uptime", getUptime()},
 *         {"version", "1.0.0"},
 *         {"timestamp", std::time(nullptr)}
 *     });
 *     return res;
 * }, statusOp);
 * 
 * // Enable CORS for web applications
 * server->enableCors({"http://localhost:3000", "https://myapp.com"});
 * 
 * // Enable documentation endpoints
 * server->enableSwaggerUI("/docs");    // Swagger UI at http://localhost:8080/docs
 * server->enableReDoc("/redoc");       // ReDoc at http://localhost:8080/redoc
 * 
 * // Start the server
 * if (server->start()) {
 *     Logger::getInstance().info(LogCategory::API, "Main", "main", __FILE__, __LINE__,
 *                               "REST API server started on port 8080");
 * }
 * @endcode
 * 
 * @note This implementation uses cpp-httplib as the underlying HTTP server library
 *       and nlohmann/json for JSON processing. It integrates with OpenAuto's modern
 *       logging, event bus, and configuration systems.
 * 
 * @warning Ensure proper authentication and authorization before exposing this API
 *          to external networks. Consider using HTTPS in production environments.
 * 
 * @see EventBus For event-driven communication
 * @see StateMachine For state management integration
 * @see ConfigurationManager For configuration management
 * @see Logger For comprehensive logging support
 */
class RestApiServer {
public:
    /**
     * @brief Constructor
     */
    RestApiServer(
        int port,
        std::shared_ptr<EventBus> eventBus,
        std::shared_ptr<StateMachine> stateMachine,
        std::shared_ptr<ConfigurationManager> configManager
    );
    
    /**
     * @brief Destructor
     */
    ~RestApiServer();
    
    // Server lifecycle
    bool start();
    void stop();
    bool isRunning() const;
    
    // Server configuration
    void setPort(int port);
    int getPort() const;
    void setBindAddress(const std::string& address);
    std::string getBindAddress() const;
    
    // OpenAPI configuration
    void setApiInfo(const ApiInfo& info);
    ApiInfo getApiInfo() const;
    void addServer(const ServerInfo& server);
    void addSecurityScheme(const std::string& name, const SecurityScheme& scheme);
    
    // Route management
    void addRoute(const Route& route);
    void addRoute(HttpMethod method, const std::string& path, RouteHandler handler);
    void addRoute(HttpMethod method, const std::string& path, RouteHandler handler, const ApiOperation& operation);
    
    // Middleware
    void addGlobalMiddleware(MiddlewareHandler middleware);
    void addRouteMiddleware(const std::string& path, MiddlewareHandler middleware);
    
    // OpenAPI endpoints
    std::string getOpenApiSpec() const;
    void enableSwaggerUI(const std::string& path = "/docs");
    void enableReDoc(const std::string& path = "/redoc");
    
    // Authentication
    void setAuthenticationHandler(std::function<bool(const HttpRequest&)> handler);
    void requireAuthentication(const std::string& path);
    
    // CORS
    void enableCors(const std::vector<std::string>& origins = {"*"});
    void setCorsHeaders(const std::map<std::string, std::string>& headers);

private:
    class ServerImpl;
    std::unique_ptr<ServerImpl> impl_;
    
    // Server configuration
    int port_;
    std::string bindAddress_;
    std::atomic<bool> running_;
    
    // OpenAPI configuration
    ApiInfo apiInfo_;
    std::vector<ServerInfo> servers_;
    std::map<std::string, SecurityScheme> securitySchemes_;
    
    // Routes and middleware
    std::vector<Route> routes_;
    std::vector<MiddlewareHandler> globalMiddlewares_;
    std::map<std::string, std::vector<MiddlewareHandler>> routeMiddlewares_;
    
    // Authentication
    std::function<bool(const HttpRequest&)> authHandler_;
    std::vector<std::string> protectedPaths_;
    
    // CORS configuration
    bool corsEnabled_;
    std::vector<std::string> corsOrigins_;
    std::map<std::string, std::string> corsHeaders_;
    
    // Components
    std::shared_ptr<EventBus> eventBus_;
    std::shared_ptr<StateMachine> stateMachine_;
    std::shared_ptr<ConfigurationManager> configManager_;
    
    // Server thread
    std::thread serverThread_;
    // std::unique_ptr<httplib::Server> server_;  // Commented out for stub
    
    // Internal methods
    void setupDefaultRoutes();
    void setupApiRoutes();
    void setupDocumentationRoutes();
    void setupCorsHandling();
    void setupDefaultMiddleware();
    void registerRoute(const Route& route);
    bool handleAuthentication(const HttpRequest& req, HttpResponse& res);
    void handleCors(const HttpRequest& req, HttpResponse& res);
    HttpMethod stringToMethod(const std::string& method);
    std::string methodToString(HttpMethod method) const;
    nlohmann::json generateOpenApiSpec() const;
    std::string generateSwaggerUI() const;
    std::string generateReDocUI() const;
    std::string extractBearerToken(const HttpRequest& req);
    bool matchesPath(const std::string& pattern, const std::string& path);
    std::map<std::string, std::string> extractPathParams(const std::string& pattern, const std::string& path);
};

} // namespace modern
} // namespace openauto
