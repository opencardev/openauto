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
#include <modern/Event.hpp>
#include <modern/EventBus.hpp>

namespace openauto {
namespace modern {

/**
 * @brief HTTP method enumeration
 */
enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    OPTIONS
};

/**
 * @brief HTTP request structure
 */
struct HttpRequest {
    HttpMethod method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> queryParams;
    std::string body;
    std::string clientAddress;
};

/**
 * @brief HTTP response structure
 */
struct HttpResponse {
    int statusCode = 200;
    std::map<std::string, std::string> headers;
    std::string body;
    std::string contentType = "application/json";
    
    HttpResponse() {
        headers["Access-Control-Allow-Origin"] = "*";
        headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, PATCH, OPTIONS";
        headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
    }
};

/**
 * @brief Route handler function type
 */
using RouteHandler = std::function<HttpResponse(const HttpRequest&)>;

/**
 * @brief Middleware function type
 */
using MiddlewareHandler = std::function<bool(HttpRequest&, HttpResponse&)>;

/**
 * @brief Route definition structure
 */
struct Route {
    HttpMethod method;
    std::string path;
    RouteHandler handler;
    std::vector<MiddlewareHandler> middleware;
};

/**
 * @brief Modern REST API server with OpenAPI support
 */
class RestApiServer {
public:
    using Pointer = std::shared_ptr<RestApiServer>;
    
    explicit RestApiServer(int port = 8080);
    ~RestApiServer();
    
    // Server control
    bool start();
    void stop();
    bool isRunning() const;
    
    // Route registration
    void addRoute(HttpMethod method, const std::string& path, RouteHandler handler);
    void addRoute(HttpMethod method, const std::string& path, RouteHandler handler, 
                  const std::vector<MiddlewareHandler>& middleware);
    
    // Convenience methods for HTTP methods
    void get(const std::string& path, RouteHandler handler);
    void post(const std::string& path, RouteHandler handler);
    void put(const std::string& path, RouteHandler handler);
    void del(const std::string& path, RouteHandler handler);
    void patch(const std::string& path, RouteHandler handler);
    void options(const std::string& path, RouteHandler handler);
    
    // Middleware
    void addGlobalMiddleware(MiddlewareHandler middleware);
    void enableCors(bool enable = true);
    void enableLogging(bool enable = true);
    
    // Event integration
    void setEventBus(std::shared_ptr<EventBus> eventBus);
    
    // OpenAPI/Swagger support
    void enableSwaggerUI(bool enable = true, const std::string& path = "/swagger");
    void setApiInfo(const std::string& title, const std::string& version, 
                    const std::string& description = "");
    void addApiRoute(HttpMethod method, const std::string& path, RouteHandler handler,
                     const std::string& summary = "", const std::string& description = "",
                     const std::map<std::string, std::string>& parameters = {});
    
    // Configuration
    void setPort(int port);
    int getPort() const;
    void setBindAddress(const std::string& address);
    std::string getBindAddress() const;
    
    // Statistics
    size_t getActiveConnections() const;
    size_t getTotalRequests() const;
    std::map<int, size_t> getStatusCodeStats() const;

private:
    // Internal server implementation
    class ServerImpl;
    std::unique_ptr<ServerImpl> impl_;
    
    // Configuration
    int port_;
    std::string bindAddress_;
    bool corsEnabled_;
    bool loggingEnabled_;
    bool swaggerEnabled_;
    std::string swaggerPath_;
    
    // API documentation
    std::string apiTitle_;
    std::string apiVersion_;
    std::string apiDescription_;
    
    // Routes and middleware
    std::vector<Route> routes_;
    std::vector<MiddlewareHandler> globalMiddleware_;
    
    // Event system integration
    std::shared_ptr<EventBus> eventBus_;
    
    // Server state
    std::atomic<bool> running_;
    std::thread serverThread_;
    mutable std::mutex statsMutex_;
    
    // Statistics
    std::atomic<size_t> activeConnections_;
    std::atomic<size_t> totalRequests_;
    std::map<int, size_t> statusCodeStats_;
    
    // Internal methods
    void serverLoop();
    HttpResponse handleRequest(const HttpRequest& request);
    HttpResponse executeRoute(const Route& route, const HttpRequest& request);
    bool matchRoute(const std::string& routePath, const std::string& requestPath,
                    std::map<std::string, std::string>& pathParams) const;
    std::string generateOpenApiSpec() const;
    std::string generateSwaggerUI() const;
    void logRequest(const HttpRequest& request, const HttpResponse& response);
    
    // Built-in route handlers
    HttpResponse handleSwaggerUI(const HttpRequest& request);
    HttpResponse handleOpenApiSpec(const HttpRequest& request);
    HttpResponse handleNotFound(const HttpRequest& request);
    HttpResponse handleMethodNotAllowed(const HttpRequest& request);
    HttpResponse handleInternalError(const HttpRequest& request, const std::exception& e);
    
    // Utility methods
    static std::string httpMethodToString(HttpMethod method);
    static HttpMethod stringToHttpMethod(const std::string& method);
    static std::string escapeJson(const std::string& input);
    static std::map<std::string, std::string> parseQueryString(const std::string& query);
    static std::string urlDecode(const std::string& str);
};

/**
 * @brief JSON utility class for API responses
 */
class JsonResponse {
public:
    static HttpResponse success(const std::string& data = "{}");
    static HttpResponse error(int statusCode, const std::string& message);
    static HttpResponse created(const std::string& data = "{}");
    static HttpResponse accepted(const std::string& message = "Request accepted");
    static HttpResponse noContent();
    static HttpResponse badRequest(const std::string& message = "Bad request");
    static HttpResponse unauthorized(const std::string& message = "Unauthorized");
    static HttpResponse forbidden(const std::string& message = "Forbidden");
    static HttpResponse notFound(const std::string& message = "Resource not found");
    static HttpResponse methodNotAllowed(const std::string& message = "Method not allowed");
    static HttpResponse conflict(const std::string& message = "Resource conflict");
    static HttpResponse internalServerError(const std::string& message = "Internal server error");
    
private:
    static HttpResponse createResponse(int statusCode, const std::string& body);
};

/**
 * @brief Authentication middleware
 */
class AuthMiddleware {
public:
    static MiddlewareHandler basicAuth(const std::string& username, const std::string& password);
    static MiddlewareHandler bearerToken(const std::string& token);
    static MiddlewareHandler apiKey(const std::string& headerName, const std::string& expectedKey);
    static MiddlewareHandler jwtAuth(const std::string& secret);
};

/**
 * @brief Rate limiting middleware
 */
class RateLimitMiddleware {
public:
    static MiddlewareHandler perIp(size_t requestsPerMinute);
    static MiddlewareHandler perApiKey(size_t requestsPerMinute);
    static MiddlewareHandler global(size_t requestsPerMinute);
};

} // namespace modern
} // namespace openauto
