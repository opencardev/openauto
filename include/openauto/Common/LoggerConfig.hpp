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

#pragma once

#include <openauto/Common/ModernLogger.hpp>
#include <string>
#include <memory>

namespace f1x {
namespace openauto {
namespace common {

/**
 * @brief Logger configuration utility class
 */
class LoggerConfig {
public:
    /**
     * @brief Initialize logger with default configuration
     */
    static void initializeDefault();
    
    /**
     * @brief Initialize logger from configuration file
     * @param configPath Path to configuration file
     */
    static void initializeFromFile(const std::string& configPath);
    
    /**
     * @brief Initialize logger for development (verbose logging)
     */
    static void initializeDevelopment();
    
    /**
     * @brief Initialize logger for production (optimised logging)
     */
    static void initializeProduction();
    
    /**
     * @brief Initialize logger for debugging (detailed logging)
     */
    static void initializeDebug();
    
    /**
     * @brief Set up file logging with rotation
     * @param filename Base filename for log files
     * @param maxSize Maximum size per file (default: 10MB)
     * @param maxFiles Maximum number of rotated files (default: 5)
     */
    static void enableFileLogging(const std::string& filename, 
                                 size_t maxSize = 10 * 1024 * 1024, 
                                 size_t maxFiles = 5);
    
    /**
     * @brief Set up remote logging
     * @param endpoint Remote logging endpoint
     */
    static void enableRemoteLogging(const std::string& endpoint);
    
    /**
     * @brief Configure category-specific log levels
     */
    static void configureCategoryLevels();

private:
    LoggerConfig() = delete;
};

} // namespace common
} // namespace openauto
} // namespace f1x