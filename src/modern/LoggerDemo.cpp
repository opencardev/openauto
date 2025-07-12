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

#include "modern/Logger.hpp"
#include <iostream>
#include <thread>
#include <chrono>

/**
 * @brief Example demonstrating modern logging features
 */
void demonstrateModernLogging() {
    using namespace openauto::modern;
    
    std::cout << "=== Modern Logger Demonstration ===" << std::endl;
    
    // Get logger instance
    auto& logger = Logger::getInstance();
    
    // Configure logger
    logger.setLevel(LogLevel::TRACE);
    logger.setAsync(true);
    
    // Add colored console output
    auto consoleFormatter = std::make_shared<ConsoleFormatter>(true, true, true);
    logger.setFormatter(consoleFormatter);
    
    // Add file output with JSON format
    auto fileSink = std::make_shared<FileSink>("demo.log", 1024 * 1024, 3);
    logger.addSink(fileSink);
    
    // Demonstrate different log levels
    std::cout << "\n--- Log Levels ---" << std::endl;
    SLOG_TRACE(SYSTEM, "DemoApp", "This is a trace message");
    SLOG_DEBUG(SYSTEM, "DemoApp", "This is a debug message");
    SLOG_INFO(SYSTEM, "DemoApp", "This is an info message");
    SLOG_WARN(SYSTEM, "DemoApp", "This is a warning message");
    SLOG_ERROR(SYSTEM, "DemoApp", "This is an error message");
    SLOG_FATAL(SYSTEM, "DemoApp", "This is a fatal message");
    
    // Demonstrate categories
    std::cout << "\n--- Categories ---" << std::endl;
    SLOG_INFO(ANDROID_AUTO, "DemoApp", "Android Auto device detected");
    SLOG_INFO(UI, "DemoApp", "User interface initialized");
    SLOG_INFO(CAMERA, "DemoApp", "Camera system started");
    SLOG_INFO(NETWORK, "DemoApp", "WiFi connection established");
    SLOG_INFO(BLUETOOTH, "DemoApp", "Bluetooth adapter ready");
    SLOG_INFO(AUDIO, "DemoApp", "Audio system configured");
    SLOG_INFO(VIDEO, "DemoApp", "Video output initialized");
    SLOG_INFO(CONFIG, "DemoApp", "Configuration loaded");
    SLOG_INFO(API, "DemoApp", "REST API server started");
    SLOG_INFO(EVENT, "DemoApp", "Event bus initialized");
    SLOG_INFO(STATE, "DemoApp", "State machine configured");
    
    // Demonstrate context logging
    std::cout << "\n--- Context Logging ---" << std::endl;
    std::map<std::string, std::string> context = {
        {"device_id", "ABC123"},
        {"vendor", "Google"},
        {"api_version", "2.0"},
        {"connection_type", "USB"}
    };
    
    logger.logWithContext(LogLevel::INFO, LogCategory::ANDROID_AUTO, "DemoApp",
                         "demonstrateModernLogging", __FILE__, __LINE__,
                         "Android Auto device connected with details", context);
    
    // Demonstrate performance logging
    std::cout << "\n--- Performance Logging ---" << std::endl;
    // Performance measurement example (simplified)
    SLOG_INFO(SYSTEM, "LoggerDemo", "Starting performance measurement");
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
    SLOG_INFO(SYSTEM, "LoggerDemo", "Initialization complete");
    
    SLOG_INFO(SYSTEM, "LoggerDemo", "Starting data processing");
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Simulate work
    SLOG_INFO(SYSTEM, "LoggerDemo", "Data processing complete");
    
    // Demonstrate different formatters
    std::cout << "\n--- JSON Formatter ---" << std::endl;
    auto jsonFormatter = std::make_shared<JsonFormatter>(true);
    logger.setFormatter(jsonFormatter);
    
    SLOG_INFO(SYSTEM, "DemoApp", "This message will be in JSON format");
    
    // Switch back to console formatter
    logger.setFormatter(consoleFormatter);
    
    // Demonstrate category-specific log levels
    std::cout << "\n--- Category-Specific Levels ---" << std::endl;
    logger.setCategoryLevel(LogCategory::CAMERA, LogLevel::ERROR);
    
    SLOG_DEBUG(CAMERA, "DemoApp", "This debug message won't appear");
    SLOG_ERROR(CAMERA, "DemoApp", "This error message will appear");
    SLOG_DEBUG(SYSTEM, "DemoApp", "This debug message will appear");
    
    // Reset camera category level
    logger.setCategoryLevel(LogCategory::CAMERA, LogLevel::TRACE);
    
    // Demonstrate multi-threaded logging
    std::cout << "\n--- Multi-threaded Logging ---" << std::endl;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([i]() {
            for (int j = 0; j < 5; ++j) {
                SLOG_INFO(SYSTEM, "Thread" + std::to_string(i), 
                         "Message " + std::to_string(j) + " from thread " + std::to_string(i));
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Show logger statistics
    std::cout << "\n--- Logger Statistics ---" << std::endl;
    SLOG_INFO(SYSTEM, "DemoApp", "Queue size: " + std::to_string(logger.getQueueSize()));
    SLOG_INFO(SYSTEM, "DemoApp", "Dropped messages: " + std::to_string(logger.getDroppedMessages()));
    
    // Flush and finish
    logger.flush();
    std::cout << "\n--- Demo Complete ---" << std::endl;
    std::cout << "Check demo.log for JSON formatted output" << std::endl;
}

/**
 * @brief Example class showing modern logging integration
 */
class ExampleComponent {
public:
    ExampleComponent() {
        LOG_INFO(SYSTEM, "ExampleComponent initialized");
    }
    
    void processData(const std::string& data) {
        LOG_DEBUG(SYSTEM, "Processing data: " + data);
        
        LOG_PERF_START(data_validation);
        if (validateData(data)) {
            LOG_PERF_END(SYSTEM, data_validation);
            LOG_INFO(SYSTEM, "Data validation successful");
            
            std::map<std::string, std::string> context = {
                {"data_size", std::to_string(data.size())},
                {"data_type", "user_input"}
            };
            LOG_INFO_CTX(SYSTEM, "Data processed successfully", context);
        } else {
            LOG_PERF_END(SYSTEM, data_validation);
            LOG_ERROR(SYSTEM, "Data validation failed");
        }
    }
    
    void handleError(const std::string& error) {
        std::map<std::string, std::string> context = {
            {"error_code", "E001"},
            {"recovery_action", "retry"}
        };
        LOG_ERROR_CTX(SYSTEM, "Error occurred: " + error, context);
    }

private:
    bool validateData(const std::string& data) {
        // Simulate validation
        return !data.empty() && data.size() < 1000;
    }
};

#ifdef LOGGER_DEMO_STANDALONE
int main() {
    // Initialize modern logger
    demonstrateModernLogging();
    
    // Show component usage
    std::cout << "\n=== Component Usage Example ===" << std::endl;
    
    ExampleComponent component;
    component.processData("Valid data string");
    component.processData(""); // This will fail validation
    component.handleError("Network timeout");
    
    return 0;
}
#endif
