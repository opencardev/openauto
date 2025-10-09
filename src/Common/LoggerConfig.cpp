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

#include <openauto/Common/LoggerConfig.hpp>
#include <iostream>
#include <fstream>

namespace f1x {
namespace openauto {
namespace common {

void LoggerConfig::initializeDefault() {
    auto& logger = ModernLogger::getInstance();
    
    // Set default log level to INFO
    logger.setLevel(LogLevel::INFO);
    
    // Configure default console formatter
    logger.setFormatter(std::make_shared<ConsoleFormatter>());
    
    // Configure category levels for better filtering
    configureCategoryLevels();
    
    // Enable asynchronous logging for better performance
    logger.setAsync(true);
    logger.setMaxQueueSize(1000);
}

void LoggerConfig::initializeFromFile(const std::string& configPath) {
    // For now, fall back to default initialization
    // In a full implementation, this would parse a config file
    initializeDefault();
    
    std::ifstream configFile(configPath);
    if (configFile.is_open()) {
        // Parse configuration file and apply settings
        // This is a placeholder for future implementation
        configFile.close();
    } else {
        std::cerr << "Warning: Could not open config file " << configPath 
                  << ", using default configuration" << std::endl;
    }
}

void LoggerConfig::initializeDevelopment() {
    auto& logger = ModernLogger::getInstance();
    
    // Set debug level for development
    logger.setLevel(LogLevel::DEBUG);
    
    // Use detailed formatter for development
    logger.setFormatter(std::make_shared<DetailedFormatter>());
    
    // Configure category levels
    configureCategoryLevels();
    
    // Enable file logging for development
    enableFileLogging("/tmp/openauto-dev.log");
    
    // Disable async for immediate feedback during development
    logger.setAsync(false);
}

void LoggerConfig::initializeProduction() {
    auto& logger = ModernLogger::getInstance();
    
    // Set INFO level for production (reduce noise)
    logger.setLevel(LogLevel::INFO);
    
    // Use console formatter for production
    logger.setFormatter(std::make_shared<ConsoleFormatter>());
    
    // Configure stricter category levels for production
    logger.setCategoryLevel(LogCategory::SYSTEM, LogLevel::WARN);
    logger.setCategoryLevel(LogCategory::CONFIG, LogLevel::WARN);
    logger.setCategoryLevel(LogCategory::GENERAL, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::ANDROID_AUTO, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::UI, LogLevel::WARN);
    logger.setCategoryLevel(LogCategory::AUDIO, LogLevel::WARN);
    logger.setCategoryLevel(LogCategory::VIDEO, LogLevel::WARN);
    logger.setCategoryLevel(LogCategory::BLUETOOTH, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::NETWORK, LogLevel::INFO);
    
    // Enable file logging for production
    enableFileLogging("/var/log/openauto.log", 50 * 1024 * 1024, 10);
    
    // Enable async for better performance
    logger.setAsync(true);
    logger.setMaxQueueSize(2000);
}

void LoggerConfig::initializeDebug() {
    auto& logger = ModernLogger::getInstance();
    
    // Set trace level for maximum verbosity
    logger.setLevel(LogLevel::TRACE);
    
    // Use detailed formatter for debugging
    logger.setFormatter(std::make_shared<DetailedFormatter>());
    
    // Enable all categories at trace level
    logger.setCategoryLevel(LogCategory::SYSTEM, LogLevel::TRACE);
    logger.setCategoryLevel(LogCategory::CONFIG, LogLevel::TRACE);
    logger.setCategoryLevel(LogCategory::GENERAL, LogLevel::TRACE);
    logger.setCategoryLevel(LogCategory::ANDROID_AUTO, LogLevel::TRACE);
    logger.setCategoryLevel(LogCategory::UI, LogLevel::TRACE);
    logger.setCategoryLevel(LogCategory::AUDIO, LogLevel::TRACE);
    logger.setCategoryLevel(LogCategory::VIDEO, LogLevel::TRACE);
    logger.setCategoryLevel(LogCategory::BLUETOOTH, LogLevel::TRACE);
    logger.setCategoryLevel(LogCategory::NETWORK, LogLevel::TRACE);
    logger.setCategoryLevel(LogCategory::PROJECTION, LogLevel::TRACE);
    logger.setCategoryLevel(LogCategory::INPUT, LogLevel::TRACE);
    
    // Enable detailed file logging
    enableFileLogging("/tmp/openauto-debug.log", 100 * 1024 * 1024, 3);
    
    // Disable async for immediate output during debugging
    logger.setAsync(false);
}

void LoggerConfig::enableFileLogging(const std::string& filename, size_t maxSize, size_t maxFiles) {
    auto& logger = ModernLogger::getInstance();
    auto fileSink = std::make_shared<FileSink>(filename, maxSize, maxFiles);
    logger.addSink(fileSink);
}

void LoggerConfig::enableRemoteLogging(const std::string& endpoint) {
    auto& logger = ModernLogger::getInstance();
    auto remoteSink = std::make_shared<RemoteSink>(endpoint);
    logger.addSink(remoteSink);
}

void LoggerConfig::configureCategoryLevels() {
    auto& logger = ModernLogger::getInstance();
    
    // Configure sensible default levels for each category
    logger.setCategoryLevel(LogCategory::SYSTEM, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::CONFIG, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::GENERAL, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::ANDROID_AUTO, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::UI, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::AUDIO, LogLevel::WARN);
    logger.setCategoryLevel(LogCategory::VIDEO, LogLevel::WARN);
    logger.setCategoryLevel(LogCategory::BLUETOOTH, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::CAMERA, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::NETWORK, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::PROJECTION, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::INPUT, LogLevel::DEBUG);
    logger.setCategoryLevel(LogCategory::SERVICE, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::SETTINGS, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::MEDIA, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::NAVIGATION, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::PHONE, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::WIFI, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::USB, LogLevel::INFO);
    logger.setCategoryLevel(LogCategory::SECURITY, LogLevel::WARN);
}

} // namespace common
} // namespace openauto
} // namespace f1x