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

namespace openauto {
namespace modern {

/**
 * @brief Modern logging levels with detailed categorization
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
 * @brief Log categories for better organization
 */
enum class LogCategory {
    SYSTEM,
    ANDROID_AUTO,
    UI,
    CAMERA,
    NETWORK,
    BLUETOOTH,
    AUDIO,
    VIDEO,
    CONFIG,
    API,
    EVENT,
    STATE,
    GENERAL
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
 * @brief Log formatter interface for customizable output formats
 */
class LogFormatter {
public:
    virtual ~LogFormatter() = default;
    virtual std::string format(const LogEntry& entry) = 0;
};

/**
 * @brief Log sink interface for customizable output destinations
 */
class LogSink {
public:
    virtual ~LogSink() = default;
    virtual void write(const std::string& formatted_message) = 0;
    virtual void flush() = 0;
};

/**
 * @brief Modern logger with comprehensive features
 */
class Logger {
public:
    using Pointer = std::shared_ptr<Logger>;
    
    static Logger& getInstance();
    
    ~Logger();
    
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
    
    // Statistics
    size_t getQueueSize() const;
    size_t getDroppedMessages() const;
    
    // Utility methods
    static std::string levelToString(LogLevel level);
    static std::string categoryToString(LogCategory category);
    static LogLevel stringToLevel(const std::string& level);
    static LogCategory stringToCategory(const std::string& category);

private:
    Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    void processLogs();
    bool shouldLog(LogLevel level, LogCategory category) const;
    
    mutable std::mutex mutex_;
    LogLevel globalLevel_;
    std::map<LogCategory, LogLevel> categoryLevels_;
    std::vector<std::shared_ptr<LogSink>> sinks_;
    std::shared_ptr<LogFormatter> formatter_;
    
    // Async processing
    bool async_;
    size_t maxQueueSize_;
    std::queue<LogEntry> logQueue_;
    std::condition_variable condition_;
    std::thread workerThread_;
    std::atomic<bool> shutdown_;
    std::atomic<size_t> droppedMessages_;
};

/**
 * @brief Default console formatter with colors and detailed information
 */
class ConsoleFormatter : public LogFormatter {
public:
    ConsoleFormatter(bool useColors = true, bool showThreadId = true, bool showLocation = true);
    std::string format(const LogEntry& entry) override;

private:
    bool useColors_;
    bool showThreadId_;
    bool showLocation_;
    
    std::string getLevelColor(LogLevel level) const;
    std::string getCategoryColor(LogCategory category) const;
    std::string resetColor() const;
};

/**
 * @brief JSON formatter for structured logging
 */
class JsonFormatter : public LogFormatter {
public:
    JsonFormatter(bool prettyPrint = false);
    std::string format(const LogEntry& entry) override;

private:
    bool prettyPrint_;
};

/**
 * @brief File formatter with detailed information
 */
class FileFormatter : public LogFormatter {
public:
    FileFormatter();
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
};

/**
 * @brief File sink with rotation support
 */
class FileSink : public LogSink {
public:
    FileSink(const std::string& filename, size_t maxSize = 10 * 1024 * 1024, size_t maxFiles = 5);
    ~FileSink();
    
    void write(const std::string& message) override;
    void flush() override;

private:
    void rotateFile();
    
    std::string filename_;
    std::ofstream file_;
    size_t maxSize_;
    size_t maxFiles_;
    size_t currentSize_;
    std::mutex fileMutex_;
};

/**
 * @brief Remote sink for sending logs to external systems
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

} // namespace modern
} // namespace openauto

// Modern logging macros with detailed context
#define LOG_TRACE(category, message) \
    ::openauto::modern::Logger::getInstance().trace( \
        ::openauto::modern::LogCategory::category, \
        typeid(*this).name(), __FUNCTION__, __FILE__, __LINE__, message)

#define LOG_DEBUG(category, message) \
    ::openauto::modern::Logger::getInstance().debug( \
        ::openauto::modern::LogCategory::category, \
        typeid(*this).name(), __FUNCTION__, __FILE__, __LINE__, message)

#define LOG_INFO(category, message) \
    ::openauto::modern::Logger::getInstance().info( \
        ::openauto::modern::LogCategory::category, \
        typeid(*this).name(), __FUNCTION__, __FILE__, __LINE__, message)

#define LOG_WARN(category, message) \
    ::openauto::modern::Logger::getInstance().warn( \
        ::openauto::modern::LogCategory::category, \
        typeid(*this).name(), __FUNCTION__, __FILE__, __LINE__, message)

#define LOG_ERROR(category, message) \
    ::openauto::modern::Logger::getInstance().error( \
        ::openauto::modern::LogCategory::category, \
        typeid(*this).name(), __FUNCTION__, __FILE__, __LINE__, message)

#define LOG_FATAL(category, message) \
    ::openauto::modern::Logger::getInstance().fatal( \
        ::openauto::modern::LogCategory::category, \
        typeid(*this).name(), __FUNCTION__, __FILE__, __LINE__, message)

// Static logging macros (for use in static functions or C-style code)
#define SLOG_TRACE(category, component, message) \
    ::openauto::modern::Logger::getInstance().trace( \
        ::openauto::modern::LogCategory::category, \
        component, __FUNCTION__, __FILE__, __LINE__, message)

#define SLOG_DEBUG(category, component, message) \
    ::openauto::modern::Logger::getInstance().debug( \
        ::openauto::modern::LogCategory::category, \
        component, __FUNCTION__, __FILE__, __LINE__, message)

#define SLOG_INFO(category, component, message) \
    ::openauto::modern::Logger::getInstance().info( \
        ::openauto::modern::LogCategory::category, \
        component, __FUNCTION__, __FILE__, __LINE__, message)

#define SLOG_WARN(category, component, message) \
    ::openauto::modern::Logger::getInstance().warn( \
        ::openauto::modern::LogCategory::category, \
        component, __FUNCTION__, __FILE__, __LINE__, message)

#define SLOG_ERROR(category, component, message) \
    ::openauto::modern::Logger::getInstance().error( \
        ::openauto::modern::LogCategory::category, \
        component, __FUNCTION__, __FILE__, __LINE__, message)

#define SLOG_FATAL(category, component, message) \
    ::openauto::modern::Logger::getInstance().fatal( \
        ::openauto::modern::LogCategory::category, \
        component, __FUNCTION__, __FILE__, __LINE__, message)

// Stream-based logging macros (for use with << operators)
#define LOG_TRACE_STREAM(category, stream) \
    do { \
        std::ostringstream oss_; \
        oss_ << stream; \
        ::openauto::modern::Logger::getInstance().trace( \
            ::openauto::modern::LogCategory::category, \
            "Component", __FUNCTION__, __FILE__, __LINE__, oss_.str()); \
    } while(0)

#define LOG_DEBUG_STREAM(category, stream) \
    do { \
        std::ostringstream oss_; \
        oss_ << stream; \
        ::openauto::modern::Logger::getInstance().debug( \
            ::openauto::modern::LogCategory::category, \
            "Component", __FUNCTION__, __FILE__, __LINE__, oss_.str()); \
    } while(0)

#define LOG_INFO_STREAM(category, stream) \
    do { \
        std::ostringstream oss_; \
        oss_ << stream; \
        ::openauto::modern::Logger::getInstance().info( \
            ::openauto::modern::LogCategory::category, \
            "Component", __FUNCTION__, __FILE__, __LINE__, oss_.str()); \
    } while(0)

#define LOG_WARN_STREAM(category, stream) \
    do { \
        std::ostringstream oss_; \
        oss_ << stream; \
        ::openauto::modern::Logger::getInstance().warn( \
            ::openauto::modern::LogCategory::category, \
            "Component", __FUNCTION__, __FILE__, __LINE__, oss_.str()); \
    } while(0)

#define LOG_ERROR_STREAM(category, stream) \
    do { \
        std::ostringstream oss_; \
        oss_ << stream; \
        ::openauto::modern::Logger::getInstance().error( \
            ::openauto::modern::LogCategory::category, \
            "Component", __FUNCTION__, __FILE__, __LINE__, oss_.str()); \
    } while(0)

#define LOG_FATAL_STREAM(category, stream) \
    do { \
        std::ostringstream oss_; \
        oss_ << stream; \
        ::openauto::modern::Logger::getInstance().fatal( \
            ::openauto::modern::LogCategory::category, \
            "Component", __FUNCTION__, __FILE__, __LINE__, oss_.str()); \
    } while(0)

// Static stream-based logging macros (for use in static functions or C-style code)
#define SLOG_TRACE_STREAM(category, component, stream) \
    do { \
        std::ostringstream oss_; \
        oss_ << stream; \
        ::openauto::modern::Logger::getInstance().trace( \
            ::openauto::modern::LogCategory::category, \
            component, __FUNCTION__, __FILE__, __LINE__, oss_.str()); \
    } while(0)

#define SLOG_DEBUG_STREAM(category, component, stream) \
    do { \
        std::ostringstream oss_; \
        oss_ << stream; \
        ::openauto::modern::Logger::getInstance().debug( \
            ::openauto::modern::LogCategory::category, \
            component, __FUNCTION__, __FILE__, __LINE__, oss_.str()); \
    } while(0)

#define SLOG_INFO_STREAM(category, component, stream) \
    do { \
        std::ostringstream oss_; \
        oss_ << stream; \
        ::openauto::modern::Logger::getInstance().info( \
            ::openauto::modern::LogCategory::category, \
            component, __FUNCTION__, __FILE__, __LINE__, oss_.str()); \
    } while(0)

#define SLOG_WARN_STREAM(category, component, stream) \
    do { \
        std::ostringstream oss_; \
        oss_ << stream; \
        ::openauto::modern::Logger::getInstance().warn( \
            ::openauto::modern::LogCategory::category, \
            component, __FUNCTION__, __FILE__, __LINE__, oss_.str()); \
    } while(0)

#define SLOG_ERROR_STREAM(category, component, stream) \
    do { \
        std::ostringstream oss_; \
        oss_ << stream; \
        ::openauto::modern::Logger::getInstance().error( \
            ::openauto::modern::LogCategory::category, \
            component, __FUNCTION__, __FILE__, __LINE__, oss_.str()); \
    } while(0)

#define SLOG_FATAL_STREAM(category, component, stream) \
    do { \
        std::ostringstream oss_; \
        oss_ << stream; \
        ::openauto::modern::Logger::getInstance().fatal( \
            ::openauto::modern::LogCategory::category, \
            component, __FUNCTION__, __FILE__, __LINE__, oss_.str()); \
    } while(0)

// Context logging macros
#define LOG_DEBUG_CTX(category, message, context) \
    ::openauto::modern::Logger::getInstance().logWithContext( \
        ::openauto::modern::LogLevel::DEBUG, ::openauto::modern::LogCategory::category, \
        typeid(*this).name(), __FUNCTION__, __FILE__, __LINE__, message, context)

#define LOG_INFO_CTX(category, message, context) \
    ::openauto::modern::Logger::getInstance().logWithContext( \
        ::openauto::modern::LogLevel::INFO, ::openauto::modern::LogCategory::category, \
        typeid(*this).name(), __FUNCTION__, __FILE__, __LINE__, message, context)

#define LOG_ERROR_CTX(category, message, context) \
    ::openauto::modern::Logger::getInstance().logWithContext( \
        ::openauto::modern::LogLevel::ERROR, ::openauto::modern::LogCategory::category, \
        typeid(*this).name(), __FUNCTION__, __FILE__, __LINE__, message, context)

// Performance logging macros
#define LOG_PERF_START(name) \
    auto perf_start_##name = std::chrono::high_resolution_clock::now()

#define LOG_PERF_END(category, name) \
    do { \
        auto perf_end_##name = std::chrono::high_resolution_clock::now(); \
        auto perf_duration_##name = std::chrono::duration_cast<std::chrono::microseconds>( \
            perf_end_##name - perf_start_##name).count(); \
        std::ostringstream perf_oss; \
        perf_oss << "Performance [" << #name << "]: " << perf_duration_##name << "μs"; \
        LOG_DEBUG(category, perf_oss.str()); \
    } while(0)

// Backward compatibility macros for existing code
#ifdef ENABLE_MODERN_API
    // Replace old OPENAUTO_LOG with modern equivalent
    #define OPENAUTO_LOG(severity) \
        ::openauto::modern::Logger::getInstance().info( \
            ::openauto::modern::LogCategory::GENERAL, "OpenAuto", __FUNCTION__, __FILE__, __LINE__, "")
    
    // Add stream operator support for backward compatibility
    class LogStream {
    public:
        LogStream(::openauto::modern::LogLevel level, ::openauto::modern::LogCategory category,
                 const std::string& component, const std::string& function,
                 const std::string& file, int line)
            : level_(level), category_(category), component_(component),
              function_(function), file_(file), line_(line) {}
        
        ~LogStream() {
            openauto::modern::Logger::getInstance().log(
                level_, category_, component_, function_, file_, line_, oss_.str());
        }
        
        template<typename T>
        LogStream& operator<<(const T& value) {
            oss_ << value;
            return *this;
        }
        
    private:
        ::openauto::modern::LogLevel level_;
        ::openauto::modern::LogCategory category_;
        std::string component_;
        std::string function_;
        std::string file_;
        int line_;
        std::ostringstream oss_;
    };
    
    #define OPENAUTO_LOG_STREAM(severity, category) \
        ::openauto::modern::LogStream( \
            ::openauto::modern::LogLevel::severity, \
            ::openauto::modern::LogCategory::category, \
            "OpenAuto", __FUNCTION__, __FILE__, __LINE__)

#else
    // Fallback to boost logging when modern API is disabled
    #include <boost/log/trivial.hpp>
    #define OPENAUTO_LOG_CONTEXT ""
    #define OPENAUTO_LOG(severity) BOOST_LOG_TRIVIAL(severity) << "[OpenAuto] " << OPENAUTO_LOG_CONTEXT
    
    // No-op macros for modern logging when disabled
    #define LOG_TRACE(category, message)
    #define LOG_DEBUG(category, message)
    #define LOG_INFO(category, message)
    #define LOG_WARN(category, message)
    #define LOG_ERROR(category, message)
    #define LOG_FATAL(category, message)
    #define SLOG_TRACE(category, component, message)
    #define SLOG_DEBUG(category, component, message)
    #define SLOG_INFO(category, component, message)
    #define SLOG_WARN(category, component, message)
    #define SLOG_ERROR(category, component, message)
    #define SLOG_FATAL(category, component, message)
    #define LOG_DEBUG_CTX(category, message, context)
    #define LOG_INFO_CTX(category, message, context)
    #define LOG_ERROR_CTX(category, message, context)
    #define LOG_PERF_START(name)
    #define LOG_PERF_END(category, name)
#endif
