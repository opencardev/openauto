/*
 * Project: OpenAuto
 * This file is part of openauto project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  openauto is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  openauto is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with openauto. If not, see <http://www.gnu.org/licenses/>.
 */

#include <openauto/Common/ModernLogger.hpp>
#include <openauto/Common/LoggerConfig.hpp>
#include <iostream>
#include <thread>
#include <chrono>

using namespace f1x::openauto::common;

int main() {
    std::cout << "Testing OpenAuto Modern Logger\n";
    std::cout << "===============================\n";
    
    // Initialize logger with development settings
    LoggerConfig::initializeDevelopment();
    
    std::cout << "\n1. Testing basic logging macros:\n";
    
    // Test all log levels
    OPENAUTO_LOG_TRACE(GENERAL, "This is a trace message");
    OPENAUTO_LOG_DEBUG(GENERAL, "This is a debug message");
    OPENAUTO_LOG_INFO(GENERAL, "This is an info message");
    OPENAUTO_LOG_WARN(GENERAL, "This is a warning message");
    OPENAUTO_LOG_ERROR(GENERAL, "This is an error message");
    
    std::cout << "\n2. Testing category-specific logging:\n";
    
    OPENAUTO_LOG_INFO(ANDROID_AUTO, "Android Auto connection established");
    OPENAUTO_LOG_DEBUG(UI, "UI component initialized");
    OPENAUTO_LOG_INFO(AUDIO, "Audio output device configured");
    OPENAUTO_LOG_WARN(BLUETOOTH, "Bluetooth pairing timeout");
    OPENAUTO_LOG_ERROR(NETWORK, "Network connection failed");
    OPENAUTO_LOG_INFO(PROJECTION, "Video projection started");
    
    std::cout << "\n3. Testing legacy compatibility:\n";
    
    // Test legacy macro compatibility
    OPENAUTO_LOG(INFO) << "This is a legacy-style log message";
    OPENAUTO_LOG(WARN) << "Legacy warning with " << "multiple" << " parts";
    OPENAUTO_LOG(ERROR) << "Legacy error: code=" << 42;
    
    std::cout << "\n4. Testing different formatters:\n";
    
    auto& logger = ModernLogger::getInstance();
    
    // Test JSON formatter
    std::cout << "\nSwitching to JSON formatter:\n";
    logger.setFormatter(std::make_shared<JsonFormatter>());
    OPENAUTO_LOG_INFO(SYSTEM, "JSON formatted message");
    
    // Test detailed formatter
    std::cout << "\nSwitching to detailed formatter:\n";
    logger.setFormatter(std::make_shared<DetailedFormatter>());
    OPENAUTO_LOG_INFO(SYSTEM, "Detailed formatted message");
    
    // Back to console formatter
    std::cout << "\nBack to console formatter:\n";
    logger.setFormatter(std::make_shared<ConsoleFormatter>());
    
    std::cout << "\n5. Testing async logging:\n";
    
    logger.setAsync(true);
    
    // Log multiple messages quickly
    for (int i = 0; i < 5; ++i) {
        OPENAUTO_LOG_INFO(GENERAL, "Async message " + std::to_string(i));
    }
    
    // Give async logger time to process
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "\n6. Testing log levels:\n";
    
    logger.setLevel(LogLevel::WARN);
    std::cout << "Set global level to WARN - should only see WARN, ERROR, FATAL:\n";
    
    OPENAUTO_LOG_TRACE(GENERAL, "This trace should not appear");
    OPENAUTO_LOG_DEBUG(GENERAL, "This debug should not appear");
    OPENAUTO_LOG_INFO(GENERAL, "This info should not appear");
    OPENAUTO_LOG_WARN(GENERAL, "This warning should appear");
    OPENAUTO_LOG_ERROR(GENERAL, "This error should appear");
    
    std::cout << "\n7. Testing category-specific levels:\n";
    
    logger.setLevel(LogLevel::INFO);  // Reset global level
    logger.setCategoryLevel(LogCategory::UI, LogLevel::ERROR);  // Only errors for UI
    
    std::cout << "Set UI category to ERROR level:\n";
    OPENAUTO_LOG_INFO(UI, "UI info should not appear");
    OPENAUTO_LOG_WARN(UI, "UI warning should not appear");
    OPENAUTO_LOG_ERROR(UI, "UI error should appear");
    
    OPENAUTO_LOG_INFO(GENERAL, "General info should appear");
    
    std::cout << "\n8. Testing logger status:\n";
    
    std::cout << "Queue size: " << logger.getQueueSize() << std::endl;
    std::cout << "Dropped messages: " << logger.getDroppedMessages() << std::endl;
    
    std::cout << "\n9. Testing flush and shutdown:\n";
    
    logger.flush();
    std::cout << "Logger flushed\n";
    
    // Final message
    OPENAUTO_LOG_INFO(GENERAL, "Modern logger test completed successfully");
    
    logger.shutdown();
    std::cout << "Logger shutdown complete\n";
    
    return 0;
}