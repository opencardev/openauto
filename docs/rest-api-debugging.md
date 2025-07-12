# OpenAuto REST API Debugging & Troubleshooting Guide

This guide provides comprehensive debugging techniques and troubleshooting solutions for the OpenAuto REST API server.

## Table of Contents

1. [Debug Environment Setup](#debug-environment-setup)
2. [Build-Time Issues](#build-time-issues)
3. [Runtime Debugging](#runtime-debugging)
4. [Network & Connectivity Issues](#network--connectivity-issues)
5. [Performance Problems](#performance-problems)
6. [Authentication & Authorization](#authentication--authorization)
7. [JSON & Data Issues](#json--data-issues)
8. [Memory & Resource Issues](#memory--resource-issues)
9. [Logging & Monitoring](#logging--monitoring)
10. [Production Debugging](#production-debugging)

## Debug Environment Setup

### Prerequisites

Before debugging, ensure you have the necessary tools installed:

```bash
# Install debugging tools
sudo apt-get update
sudo apt-get install -y \
    gdb \
    valgrind \
    strace \
    tcpdump \
    netstat-nat \
    lsof \
    htop \
    curl \
    jq

# Install build tools if not already present
sudo apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    nlohmann-json3-dev

# Verify httplib installation
ls -la /usr/include/httplib.h
```

### Debug Build Configuration

Create a debug build configuration:

```bash
# Clean previous builds
cd /home/pi/openauto
rm -rf build

# Configure for debug build
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-g -O0 -DDEBUG -fsanitize=address" \
    -DENABLE_MODERN_API=ON \
    -DNOPI=ON

# Build with debug symbols
cmake --build build -- -j$(nproc)
```

### Enable Comprehensive Logging

```cpp
// In your main application or test
#include "modern/Logger.hpp"

// Set maximum logging level
Logger::getInstance().setLevel(LogLevel::DEBUG);

// Enable all categories
Logger::getInstance().enableCategory(LogCategory::GENERAL);
Logger::getInstance().enableCategory(LogCategory::API);
Logger::getInstance().enableCategory(LogCategory::NETWORK);
Logger::getInstance().enableCategory(LogCategory::AUTH);
```

## Build-Time Issues

### CMake Configuration Failures

#### Issue: "Could not find nlohmann/json"

**Diagnosis:**
```bash
# Check if nlohmann-json is installed
dpkg -l | grep nlohmann
find /usr -name "json.hpp" 2>/dev/null

# Check CMake module path
ls -la /home/pi/openauto/cmake_modules/FindNlohmannJson.cmake
```

**Solutions:**
```bash
# Install nlohmann-json
sudo apt-get install nlohmann-json3-dev

# Or install manually
wget https://github.com/nlohmann/json/releases/latest/download/json.hpp
sudo mkdir -p /usr/include/nlohmann
sudo cp json.hpp /usr/include/nlohmann/

# Update CMake cache
cd /home/pi/openauto
rm -rf build/CMakeCache.txt
cmake -S . -B build -DNOPI=ON
```

#### Issue: "Could not find cpp-httplib"

**Diagnosis:**
```bash
# Check httplib installation
ls -la /usr/include/httplib.h
file /usr/include/httplib.h

# Check CMake find module
cat /home/pi/openauto/cmake_modules/FindHttplib.cmake
```

**Solutions:**
```bash
# Download latest httplib
cd /tmp
wget https://github.com/yhirose/cpp-httplib/releases/latest/download/httplib.h
sudo cp httplib.h /usr/include/

# Verify installation
head -20 /usr/include/httplib.h | grep -i version
```

### Compilation Errors

#### Issue: "Undefined reference to httplib symbols"

**Diagnosis:**
```bash
# Check if HTTPLIB_IMPLEMENTATION is defined correctly
grep -n "HTTPLIB_IMPLEMENTATION" /home/pi/openauto/src/modern/RestApiServer.cpp

# Check for multiple definitions
grep -r "HTTPLIB_IMPLEMENTATION" /home/pi/openauto/src/
```

**Solutions:**
```cpp
// Ensure HTTPLIB_IMPLEMENTATION is defined exactly once
// In RestApiServer.cpp (NOT in header file):
#define HTTPLIB_IMPLEMENTATION
#include <httplib.h>
#include "modern/RestApiServer.hpp"
```

#### Issue: Logger method signature mismatches

**Diagnosis:**
```bash
# Check logger interface
grep -A 5 -B 5 "void info" /home/pi/openauto/include/modern/Logger.hpp
```

**Solutions:**
```cpp
// Use correct logger signature
Logger::getInstance().info(
    LogCategory::API,           // Category
    "ComponentName",            // Component
    __FUNCTION__,              // Function
    __FILE__,                  // File
    __LINE__,                  // Line
    "Your message here"        // Message
);
```

## Runtime Debugging

### GDB Debugging

#### Basic GDB Session

```bash
# Start with GDB
cd /home/pi/openauto
gdb ./bin/autoapp

# Set breakpoints
(gdb) break RestApiServer::start
(gdb) break RestApiServer::addRoute
(gdb) break HttpRequest::getJsonBody

# Run with arguments
(gdb) run --enable-rest-api --port 8080

# When breakpoint hits:
(gdb) info locals
(gdb) print server_
(gdb) print this->port_
(gdb) continue
```

#### Debug Specific Issues

```bash
# Debug server startup issues
(gdb) break RestApiServer::start
(gdb) run
(gdb) step
(gdb) print running_
(gdb) print port_

# Debug request handling
(gdb) break HttpRequest::getJsonBody
(gdb) continue
# Make a request in another terminal
(gdb) print req_.body
(gdb) step
```

### Runtime Logging

#### Add Debug Middleware

```cpp
// Add comprehensive debug middleware
server->addGlobalMiddleware([](HttpRequest& req, HttpResponse& res) {
    auto timestamp = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    
    // Log request details
    Logger::getInstance().debug(LogCategory::API, "Debug", "request", __FILE__, __LINE__,
        "=== REQUEST START ===");
    Logger::getInstance().debug(LogCategory::API, "Debug", "request", __FILE__, __LINE__,
        "Method: " + std::to_string(static_cast<int>(req.getMethod())));
    Logger::getInstance().debug(LogCategory::API, "Debug", "request", __FILE__, __LINE__,
        "Path: " + req.getPath());
    Logger::getInstance().debug(LogCategory::API, "Debug", "request", __FILE__, __LINE__,
        "Client: " + req.getClientAddress());
    Logger::getInstance().debug(LogCategory::API, "Debug", "request", __FILE__, __LINE__,
        "User-Agent: " + req.getHeader("User-Agent"));
    Logger::getInstance().debug(LogCategory::API, "Debug", "request", __FILE__, __LINE__,
        "Content-Type: " + req.getHeader("Content-Type"));
    Logger::getInstance().debug(LogCategory::API, "Debug", "request", __FILE__, __LINE__,
        "Content-Length: " + req.getHeader("Content-Length"));
    Logger::getInstance().debug(LogCategory::API, "Debug", "request", __FILE__, __LINE__,
        "Authorization: " + (req.hasHeader("Authorization") ? "[PRESENT]" : "[ABSENT]"));
    
    std::string body = req.getBody();
    if (!body.empty()) {
        Logger::getInstance().debug(LogCategory::API, "Debug", "request", __FILE__, __LINE__,
            "Body: " + body.substr(0, 500) + (body.length() > 500 ? "..." : ""));
    }
    
    Logger::getInstance().debug(LogCategory::API, "Debug", "request", __FILE__, __LINE__,
        "=== REQUEST END ===");
    
    return true;
});
```

#### Route Registration Debug

```cpp
// Add debug route to list all registered routes
server->addRoute(HttpMethod::GET, "/debug/routes", [this](const HttpRequest& req) {
    HttpResponse res;
    nlohmann::json routes = nlohmann::json::array();
    
    // This would need to be implemented in RestApiServer class
    for (const auto& route : routes_) {
        routes.push_back({
            {"method", methodToString(route.method)},
            {"path", route.path},
            {"operationId", route.operation.operationId},
            {"summary", route.operation.summary},
            {"hasHandler", route.handler != nullptr}
        });
    }
    
    res.setJson({
        {"totalRoutes", routes.size()},
        {"routes", routes}
    });
    return res;
});
```

### Memory Debugging with Valgrind

```bash
# Run with valgrind
cd /home/pi/openauto
valgrind --tool=memcheck \
         --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind.log \
         ./bin/autoapp --enable-rest-api

# Analyze results
grep "definitely lost" valgrind.log
grep "ERROR SUMMARY" valgrind.log
```

## Network & Connectivity Issues

### Port and Binding Issues

#### Check Port Availability

```bash
# Check if port is in use
netstat -tulpn | grep :8080
ss -tulpn | grep :8080
sudo lsof -i :8080

# Check which process is using the port
sudo fuser -v 8080/tcp

# Kill process using port (if needed)
sudo fuser -k 8080/tcp
```

#### Firewall Configuration

```bash
# Check firewall status
sudo ufw status verbose
sudo iptables -L -n | grep 8080

# Allow port through firewall
sudo ufw allow 8080/tcp
sudo iptables -I INPUT -p tcp --dport 8080 -j ACCEPT
```

#### Network Interface Binding

```cpp
// Debug binding configuration
Logger::getInstance().info(LogCategory::API, "Server", "config", __FILE__, __LINE__,
    "Binding to address: " + bindAddress_ + ":" + std::to_string(port_));

// Test different bind addresses
server->setBindAddress("127.0.0.1");  // Local only
server->setBindAddress("0.0.0.0");    // All interfaces
```

### Network Debugging Tools

#### Use tcpdump to Monitor Traffic

```bash
# Monitor HTTP traffic on port 8080
sudo tcpdump -i any -A -n port 8080

# Monitor specific interface
sudo tcpdump -i eth0 -A -n port 8080

# Save to file for analysis
sudo tcpdump -i any -w api_traffic.pcap port 8080
```

#### Test Connectivity

```bash
# Test basic connectivity
telnet localhost 8080

# Test HTTP connectivity
curl -v http://localhost:8080/api/health

# Test from external host
curl -v http://your_ip_address:8080/api/health

# Test with specific headers
curl -v \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer test_token" \
  -d '{"test": "data"}' \
  http://localhost:8080/api/test
```

## Performance Problems

### Response Time Analysis

#### Add Timing Middleware

```cpp
server->addGlobalMiddleware([](HttpRequest& req, HttpResponse& res) {
    // Record start time
    auto start = std::chrono::high_resolution_clock::now();
    
    // Set a custom property (if supported) or use thread-local storage
    static thread_local std::chrono::high_resolution_clock::time_point request_start;
    request_start = start;
    
    return true;
});

// Add post-processing middleware (if supported)
server->addPostMiddleware([](HttpRequest& req, HttpResponse& res) {
    static thread_local std::chrono::high_resolution_clock::time_point request_start;
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - request_start);
    
    Logger::getInstance().info(LogCategory::API, "Performance", "timing", __FILE__, __LINE__,
        "Request " + req.getPath() + " took " + std::to_string(duration.count()) + " microseconds");
    
    // Log slow requests
    if (duration.count() > 100000) { // > 100ms
        Logger::getInstance().warn(LogCategory::API, "Performance", "slow", __FILE__, __LINE__,
            "SLOW REQUEST: " + req.getPath() + " took " + std::to_string(duration.count() / 1000) + "ms");
    }
});
```

#### CPU Profiling

```bash
# Install profiling tools
sudo apt-get install perf linux-tools-generic

# Profile CPU usage
sudo perf record -g ./bin/autoapp --enable-rest-api
# ... run some requests ...
sudo perf report

# Use gprof
g++ -pg -o autoapp_profile [your_sources]
./autoapp_profile --enable-rest-api
gprof autoapp_profile gmon.out > profile_analysis.txt
```

### Memory Usage Monitoring

```cpp
// Add memory monitoring middleware
server->addGlobalMiddleware([](HttpRequest& req, HttpResponse& res) {
    static int request_count = 0;
    request_count++;
    
    if (request_count % 100 == 0) { // Check every 100 requests
        std::ifstream status("/proc/self/status");
        std::string line;
        while (std::getline(status, line)) {
            if (line.find("VmRSS:") == 0 || line.find("VmSize:") == 0) {
                Logger::getInstance().info(LogCategory::API, "Memory", "usage", __FILE__, __LINE__, line);
            }
        }
    }
    
    return true;
});
```

## Authentication & Authorization

### Authentication Debugging

#### Debug Authentication Handler

```cpp
server->setAuthenticationHandler([](const HttpRequest& req) {
    std::string authHeader = req.getHeader("Authorization");
    
    Logger::getInstance().debug(LogCategory::AUTH, "Handler", "check", __FILE__, __LINE__,
        "Checking authentication for: " + req.getPath());
    Logger::getInstance().debug(LogCategory::AUTH, "Handler", "check", __FILE__, __LINE__,
        "Auth header present: " + std::to_string(!authHeader.empty()));
    
    if (authHeader.empty()) {
        Logger::getInstance().debug(LogCategory::AUTH, "Handler", "check", __FILE__, __LINE__,
            "No Authorization header");
        return false;
    }
    
    if (!authHeader.starts_with("Bearer ")) {
        Logger::getInstance().debug(LogCategory::AUTH, "Handler", "check", __FILE__, __LINE__,
            "Invalid Authorization header format");
        return false;
    }
    
    std::string token = authHeader.substr(7);
    Logger::getInstance().debug(LogCategory::AUTH, "Handler", "check", __FILE__, __LINE__,
        "Extracted token: " + token.substr(0, 10) + "...");
    
    bool isValid = validateToken(token);
    Logger::getInstance().debug(LogCategory::AUTH, "Handler", "check", __FILE__, __LINE__,
        "Token validation result: " + std::to_string(isValid));
    
    return isValid;
});
```

#### Test Authentication

```bash
# Test without authentication
curl -v http://localhost:8080/api/protected

# Test with invalid token
curl -v -H "Authorization: Bearer invalid_token" http://localhost:8080/api/protected

# Test with valid token
curl -v -H "Authorization: Bearer valid_token_here" http://localhost:8080/api/protected

# Test different token formats
curl -v -H "Authorization: invalid_format" http://localhost:8080/api/protected
```

## JSON & Data Issues

### JSON Parsing Debug

```cpp
// Add JSON parsing debug middleware
server->addGlobalMiddleware([](HttpRequest& req, HttpResponse& res) {
    std::string body = req.getBody();
    if (!body.empty()) {
        Logger::getInstance().debug(LogCategory::API, "JSON", "parse", __FILE__, __LINE__,
            "Attempting to parse JSON body: " + body);
        
        try {
            auto json = nlohmann::json::parse(body);
            Logger::getInstance().debug(LogCategory::API, "JSON", "parse", __FILE__, __LINE__,
                "JSON parsed successfully");
            Logger::getInstance().debug(LogCategory::API, "JSON", "parse", __FILE__, __LINE__,
                "JSON structure: " + json.dump(2));
        } catch (const nlohmann::json::parse_error& e) {
            Logger::getInstance().error(LogCategory::API, "JSON", "parse", __FILE__, __LINE__,
                "JSON parse error: " + std::string(e.what()));
            Logger::getInstance().error(LogCategory::API, "JSON", "parse", __FILE__, __LINE__,
                "Error at byte position: " + std::to_string(e.byte));
        }
    }
    return true;
});
```

### Content-Type Validation

```cpp
// Add content-type validation
server->addGlobalMiddleware([](HttpRequest& req, HttpResponse& res) {
    if (req.getMethod() == HttpMethod::POST || req.getMethod() == HttpMethod::PUT) {
        std::string contentType = req.getHeader("Content-Type");
        Logger::getInstance().debug(LogCategory::API, "ContentType", "check", __FILE__, __LINE__,
            "Content-Type: " + contentType);
        
        if (!req.getBody().empty() && contentType.find("application/json") == std::string::npos) {
            Logger::getInstance().warn(LogCategory::API, "ContentType", "check", __FILE__, __LINE__,
                "Missing or incorrect Content-Type for JSON request");
        }
    }
    return true;
});
```

## Memory & Resource Issues

### Memory Leak Detection

```bash
# Use AddressSanitizer
cd /home/pi/openauto
rm -rf build
cmake -S . -B build -DCMAKE_CXX_FLAGS="-fsanitize=address -g" -DNOPI=ON
cmake --build build
./bin/autoapp --enable-rest-api

# Use Valgrind for detailed analysis
valgrind --tool=memcheck \
         --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         ./bin/autoapp --enable-rest-api
```

### Resource Monitoring

```cpp
// Add resource monitoring endpoint
server->addRoute(HttpMethod::GET, "/debug/resources", [](const HttpRequest& req) {
    HttpResponse res;
    
    // Get memory info
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    nlohmann::json memory;
    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") == 0 || 
            line.find("MemFree:") == 0 || 
            line.find("MemAvailable:") == 0) {
            // Parse and add to JSON
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                memory[key] = value;
            }
        }
    }
    
    // Get process info
    std::ifstream status("/proc/self/status");
    nlohmann::json process;
    while (std::getline(status, line)) {
        if (line.find("VmPeak:") == 0 || 
            line.find("VmSize:") == 0 || 
            line.find("VmRSS:") == 0) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                process[key] = value;
            }
        }
    }
    
    // Get load average
    std::ifstream loadavg("/proc/loadavg");
    std::string load;
    std::getline(loadavg, load);
    
    res.setJson({
        {"timestamp", std::time(nullptr)},
        {"memory", memory},
        {"process", process},
        {"load_average", load}
    });
    
    return res;
});
```

## Logging & Monitoring

### Structured Logging

```cpp
// Create structured log entries
class ApiLogger {
public:
    static void logRequest(const HttpRequest& req, int responseStatus, 
                          std::chrono::microseconds duration) {
        nlohmann::json logEntry;
        logEntry["timestamp"] = std::time(nullptr);
        logEntry["method"] = static_cast<int>(req.getMethod());
        logEntry["path"] = req.getPath();
        logEntry["client"] = req.getClientAddress();
        logEntry["status"] = responseStatus;
        logEntry["duration_us"] = duration.count();
        logEntry["user_agent"] = req.getHeader("User-Agent");
        logEntry["has_auth"] = req.hasHeader("Authorization");
        
        Logger::getInstance().info(LogCategory::API, "RequestLog", "structured", 
                                  __FILE__, __LINE__, logEntry.dump());
    }
};
```

### Log Analysis Tools

```bash
# Create log analysis script
cat > analyze_logs.sh << 'EOF'
#!/bin/bash
LOG_FILE="/var/log/openauto/api.log"

echo "=== API Request Analysis ==="
echo "Total requests:"
grep "RequestLog" "$LOG_FILE" | wc -l

echo "Requests by status code:"
grep "RequestLog" "$LOG_FILE" | grep -o '"status":[0-9]*' | sort | uniq -c

echo "Slowest requests (>1s):"
grep "RequestLog" "$LOG_FILE" | jq 'select(.duration_us > 1000000)' 2>/dev/null

echo "Most frequent paths:"
grep "RequestLog" "$LOG_FILE" | grep -o '"path":"[^"]*"' | sort | uniq -c | sort -nr | head -10

echo "Error analysis:"
grep "ERROR\|WARN" "$LOG_FILE" | tail -20
EOF

chmod +x analyze_logs.sh
./analyze_logs.sh
```

## Production Debugging

### Health Check Endpoint

```cpp
// Comprehensive health check
server->addRoute(HttpMethod::GET, "/health/detailed", [](const HttpRequest& req) {
    HttpResponse res;
    
    auto now = std::chrono::system_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
        now - getStartTime()).count();
    
    // Check dependencies
    bool eventBusHealthy = eventBus_ && eventBus_->isRunning();
    bool stateMachineHealthy = stateMachine_ && stateMachine_->isReady();
    bool configHealthy = configManager_ && configManager_->isLoaded();
    
    // Check resources
    bool memoryOk = getMemoryUsage() < 90; // Less than 90% memory usage
    bool diskOk = getDiskUsage() < 85;     // Less than 85% disk usage
    
    bool healthy = eventBusHealthy && stateMachineHealthy && 
                  configHealthy && memoryOk && diskOk;
    
    res.setStatus(healthy ? 200 : 503);
    res.setJson({
        {"status", healthy ? "healthy" : "unhealthy"},
        {"timestamp", std::chrono::system_clock::to_time_t(now)},
        {"uptime_seconds", uptime},
        {"checks", {
            {"event_bus", eventBusHealthy},
            {"state_machine", stateMachineHealthy},
            {"configuration", configHealthy},
            {"memory", memoryOk},
            {"disk", diskOk}
        }},
        {"metrics", {
            {"total_requests", getTotalRequestCount()},
            {"error_rate", getErrorRate()},
            {"avg_response_time", getAverageResponseTime()}
        }}
    });
    
    return res;
});
```

### Metrics Collection

```cpp
// Metrics collection class
class ApiMetrics {
private:
    std::atomic<uint64_t> totalRequests_{0};
    std::atomic<uint64_t> errorCount_{0};
    std::vector<std::chrono::microseconds> responseTimes_;
    std::mutex responseTimesMutex_;

public:
    void recordRequest(std::chrono::microseconds duration, bool isError) {
        totalRequests_++;
        if (isError) errorCount_++;
        
        std::lock_guard<std::mutex> lock(responseTimesMutex_);
        responseTimes_.push_back(duration);
        
        // Keep only last 1000 measurements
        if (responseTimes_.size() > 1000) {
            responseTimes_.erase(responseTimes_.begin());
        }
    }
    
    nlohmann::json getMetrics() const {
        std::lock_guard<std::mutex> lock(responseTimesMutex_);
        
        auto avg = std::accumulate(responseTimes_.begin(), responseTimes_.end(), 
                                  std::chrono::microseconds{0}) / responseTimes_.size();
        
        return {
            {"total_requests", totalRequests_.load()},
            {"error_count", errorCount_.load()},
            {"error_rate", static_cast<double>(errorCount_) / totalRequests_},
            {"avg_response_time_us", avg.count()},
            {"sample_size", responseTimes_.size()}
        };
    }
};
```

### Remote Debugging Support

```cpp
// Add remote debugging endpoints
server->addRoute(HttpMethod::GET, "/debug/config", [](const HttpRequest& req) {
    HttpResponse res;
    
    // Return current configuration (sanitized)
    nlohmann::json config;
    config["server"] = {
        {"port", getPort()},
        {"bind_address", getBindAddress()},
        {"cors_enabled", corsEnabled_},
        {"auth_enabled", authHandler_ != nullptr}
    };
    
    res.setJson(config);
    return res;
});

server->addRoute(HttpMethod::POST, "/debug/log-level", [](const HttpRequest& req) {
    HttpResponse res;
    
    try {
        auto body = req.getJsonBody();
        std::string level = body["level"];
        
        LogLevel logLevel = LogLevel::INFO;
        if (level == "DEBUG") logLevel = LogLevel::DEBUG;
        else if (level == "WARN") logLevel = LogLevel::WARN;
        else if (level == "ERROR") logLevel = LogLevel::ERROR;
        
        Logger::getInstance().setLevel(logLevel);
        
        res.setJson({{"status", "Log level updated to " + level}});
    } catch (const std::exception& e) {
        res.setStatus(400);
        res.setJson({{"error", "Invalid request: " + std::string(e.what())}});
    }
    
    return res;
});
```

This comprehensive debugging and troubleshooting guide provides systematic approaches to identifying and resolving issues with the OpenAuto REST API server, from build-time problems to production debugging scenarios.
