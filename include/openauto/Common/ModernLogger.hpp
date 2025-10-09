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

#include <memory>
#include <string>
#include <ostream>
#include <fstream>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <functional>
#include <vector>

namespace f1x {
namespace openauto {
namespace common {

/**
 * @brief Modern logging levels with detailed categorisation
 */
enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

/**
 * @brief Log categories specific to OpenAuto components
 */
enum class LogCategory {
    GENERAL,
    SYSTEM,
    ANDROID_AUTO,
    UI,
    AUDIO,
    VIDEO,
    BLUETOOTH,
    CAMERA,
    NETWORK,
    CONFIG,
    PROJECTION,
    INPUT,
    SERVICE,
    SETTINGS,
    MEDIA,
    NAVIGATION,
    PHONE,
    WIFI,
    USB,
    SECURITY
};

/**
 * @brief Log entry structure containing all relevant information
 */
struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    LogCategory category;
    std::string component;
    std::string function;
    std::string file;
    int line;
    std::thread::id threadId;
    std::string message;
    std::map<std::string, std::string> context;
};

/**
 * @brief Log formatter interface for customisable output formats
 */
class LogFormatter {
public:
    virtual ~LogFormatter() = default;
    virtual std::string format(const LogEntry& entry) = 0;
};

/**
 * @brief Log sink interface for customisable output destinations
 */
class LogSink {
public:
    virtual ~LogSink() = default;
    virtual void write(const std::string& formatted_message) = 0;
    virtual void flush() = 0;
};

/**
 * @brief Modern logger for OpenAuto with comprehensive features
 */
class ModernLogger {
public:
    using Pointer = std::shared_ptr<ModernLogger>;
    
    static ModernLogger& getInstance();
    
    ~ModernLogger();
    
    // Configuration
    void setLevel(LogLevel level);
    void setCategoryLevel(LogCategory category, LogLevel level);
    void addSink(std::shared_ptr<LogSink> sink);
    void setFormatter(std::shared_ptr<LogFormatter> formatter);
    void setAsync(bool async);
    void setMaxQueueSize(size_t maxSize);
    
    // Logging methods
    void log(LogLevel level, LogCategory category, const std::string& component,
             const std::string& function, const std::string& file, int line,
             const std::string& message);
    
    void logWithContext(LogLevel level, LogCategory category, const std::string& component,
                       const std::string& function, const std::string& file, int line,
                       const std::string& message, const std::map<std::string, std::string>& context);
    
    // Convenience methods
    void trace(LogCategory category, const std::string& component, const std::string& function,
               const std::string& file, int line, const std::string& message);
    void debug(LogCategory category, const std::string& component, const std::string& function,
               const std::string& file, int line, const std::string& message);
    void info(LogCategory category, const std::string& component, const std::string& function,
              const std::string& file, int line, const std::string& message);
    void warn(LogCategory category, const std::string& component, const std::string& function,
              const std::string& file, int line, const std::string& message);
    void error(LogCategory category, const std::string& component, const std::string& function,
               const std::string& file, int line, const std::string& message);
    void fatal(LogCategory category, const std::string& component, const std::string& function,
               const std::string& file, int line, const std::string& message);
    
    // Control
    void flush();
    void shutdown();
    
    // Status
    size_t getQueueSize() const;
    size_t getDroppedMessages() const;
    
    // Public method for checking if logging should happen
    bool shouldLog(LogLevel level, LogCategory category) const;
    
    // Utility methods
    static std::string levelToString(LogLevel level);
    static std::string categoryToString(LogCategory category);
    static LogLevel stringToLevel(const std::string& level);
    static LogCategory stringToCategory(const std::string& category);

private:
    ModernLogger();
    ModernLogger(const ModernLogger&) = delete;
    ModernLogger& operator=(const ModernLogger&) = delete;
    
    void processLogs();
    
    mutable std::mutex mutex_;
    LogLevel globalLevel_;
    std::map<LogCategory, LogLevel> categoryLevels_;
    std::vector<std::shared_ptr<LogSink>> sinks_;
    std::shared_ptr<LogFormatter> formatter_;
    
    // Async logging
    std::atomic<bool> async_;
    std::atomic<bool> running_;
    std::queue<LogEntry> logQueue_;
    std::condition_variable condition_;
    std::thread workerThread_;
    size_t maxQueueSize_;
    std::atomic<size_t> droppedMessages_;
};

/**
 * @brief Default console formatter
 */
class ConsoleFormatter : public LogFormatter {
public:
    std::string format(const LogEntry& entry) override;
};

/**
 * @brief JSON formatter for structured logging
 */
class JsonFormatter : public LogFormatter {
public:
    std::string format(const LogEntry& entry) override;
};

/**
 * @brief Detailed formatter with full context
 */
class DetailedFormatter : public LogFormatter {
public:
    std::string format(const LogEntry& entry) override;
};

/**
 * @brief Console sink for stdout/stderr output
 */
class ConsoleSink : public LogSink {
public:
    ConsoleSink(bool useStderr = false);
    
    void write(const std::string& message) override;
    void flush() override;

private:
    bool useStderr_;
    std::mutex consoleMutex_;
};

/**
 * @brief File sink with rotation support
 */
class FileSink : public LogSink {
public:
    FileSink(const std::string& filename, size_t maxSize = 10 * 1024 * 1024, size_t maxFiles = 5);
    
    void write(const std::string& message) override;
    void flush() override;

private:
    void rotateFile();
    
    std::string filename_;
    std::ofstream file_;
    std::mutex fileMutex_;
    size_t maxSize_;
    size_t maxFiles_;
    size_t currentSize_;
};

/**
 * @brief Remote sink for network logging
 */
class RemoteSink : public LogSink {
public:
    RemoteSink(const std::string& endpoint);
    
    void write(const std::string& message) override;
    void flush() override;

private:
    std::string endpoint_;
    std::queue<std::string> pendingMessages_;
    std::mutex queueMutex_;
};

// Legacy log stream for backward compatibility
class LegacyLogStream {
public:
    LegacyLogStream(LogLevel level, LogCategory category, const std::string& component,
                   const std::string& function, const std::string& file, int line);
    ~LegacyLogStream();
    
    template<typename T>
    LegacyLogStream& operator<<(const T& value) {
        stream_ << value;
        return *this;
    }

private:
    LogLevel level_;
    LogCategory category_;
    std::string component_;
    std::string function_;
    std::string file_;
    int line_;
    std::ostringstream stream_;
};

} // namespace common
} // namespace openauto
} // namespace f1x

// Convenience macros for OpenAuto logging
#define OPENAUTO_LOG_TRACE(category, message) \
    do { \
        auto& logger = f1x::openauto::common::ModernLogger::getInstance(); \
        if (logger.shouldLog(f1x::openauto::common::LogLevel::TRACE, f1x::openauto::common::LogCategory::category)) { \
            logger.trace( \
                f1x::openauto::common::LogCategory::category, \
                __PRETTY_FUNCTION__, \
                __FUNCTION__, \
                __FILE__, \
                __LINE__, \
                message); \
        } \
    } while(0)

#define OPENAUTO_LOG_DEBUG(category, message) \
    do { \
        auto& logger = f1x::openauto::common::ModernLogger::getInstance(); \
        if (logger.shouldLog(f1x::openauto::common::LogLevel::DEBUG, f1x::openauto::common::LogCategory::category)) { \
            logger.debug( \
                f1x::openauto::common::LogCategory::category, \
                __PRETTY_FUNCTION__, \
                __FUNCTION__, \
                __FILE__, \
                __LINE__, \
                message); \
        } \
    } while(0)

#define OPENAUTO_LOG_INFO(category, message) \
    do { \
        auto& logger = f1x::openauto::common::ModernLogger::getInstance(); \
        if (logger.shouldLog(f1x::openauto::common::LogLevel::INFO, f1x::openauto::common::LogCategory::category)) { \
            logger.info( \
                f1x::openauto::common::LogCategory::category, \
                __PRETTY_FUNCTION__, \
                __FUNCTION__, \
                __FILE__, \
                __LINE__, \
                message); \
        } \
    } while(0)

#define OPENAUTO_LOG_WARN(category, message) \
    do { \
        auto& logger = f1x::openauto::common::ModernLogger::getInstance(); \
        if (logger.shouldLog(f1x::openauto::common::LogLevel::WARN, f1x::openauto::common::LogCategory::category)) { \
            logger.warn( \
                f1x::openauto::common::LogCategory::category, \
                __PRETTY_FUNCTION__, \
                __FUNCTION__, \
                __FILE__, \
                __LINE__, \
                message); \
        } \
    } while(0)

#define OPENAUTO_LOG_ERROR(category, message) \
    do { \
        auto& logger = f1x::openauto::common::ModernLogger::getInstance(); \
        if (logger.shouldLog(f1x::openauto::common::LogLevel::ERROR, f1x::openauto::common::LogCategory::category)) { \
            logger.error( \
                f1x::openauto::common::LogCategory::category, \
                __PRETTY_FUNCTION__, \
                __FUNCTION__, \
                __FILE__, \
                __LINE__, \
                message); \
        } \
    } while(0)

#define OPENAUTO_LOG_FATAL(category, message) \
    do { \
        auto& logger = f1x::openauto::common::ModernLogger::getInstance(); \
        if (logger.shouldLog(f1x::openauto::common::LogLevel::FATAL, f1x::openauto::common::LogCategory::category)) { \
            logger.fatal( \
                f1x::openauto::common::LogCategory::category, \
                __PRETTY_FUNCTION__, \
                __FUNCTION__, \
                __FILE__, \
                __LINE__, \
                message); \
        } \
    } while(0)

// Legacy compatibility macros - backward compatible with existing code
#define OPENAUTO_LOG(severity) \
    f1x::openauto::common::LegacyLogStream(f1x::openauto::common::LogLevel::severity, \
                                          f1x::openauto::common::LogCategory::GENERAL, \
                                          __PRETTY_FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
