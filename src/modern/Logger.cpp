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
#include <filesystem>
#include <regex>

#ifdef ENABLE_MODERN_API
#include <nlohmann/json.hpp>
#endif

namespace openauto {
namespace modern {

// Logger Implementation
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() 
    : globalLevel_(LogLevel::INFO)
    , async_(false)
    , maxQueueSize_(10000)
    , shutdown_(false)
    , droppedMessages_(0) {
    
    // Set up default console formatter and sink
    formatter_ = std::make_shared<ConsoleFormatter>();
    addSink(std::make_shared<ConsoleSink>());
}

Logger::~Logger() {
    shutdown();
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    globalLevel_ = level;
}

void Logger::setCategoryLevel(LogCategory category, LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    categoryLevels_[category] = level;
}

void Logger::addSink(std::shared_ptr<LogSink> sink) {
    std::lock_guard<std::mutex> lock(mutex_);
    sinks_.push_back(sink);
}

void Logger::setFormatter(std::shared_ptr<LogFormatter> formatter) {
    std::lock_guard<std::mutex> lock(mutex_);
    formatter_ = formatter;
}

void Logger::setAsync(bool async) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (async && !async_) {
        // Start async processing
        async_ = true;
        shutdown_ = false;
        workerThread_ = std::thread(&Logger::processLogs, this);
    } else if (!async && async_) {
        // Stop async processing
        async_ = false;
        shutdown_ = true;
        condition_.notify_all();
        
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }
}

void Logger::setMaxQueueSize(size_t maxSize) {
    std::lock_guard<std::mutex> lock(mutex_);
    maxQueueSize_ = maxSize;
}

void Logger::log(LogLevel level, LogCategory category, const std::string& component,
                const std::string& function, const std::string& file, int line,
                const std::string& message) {
    
    if (!shouldLog(level, category)) {
        return;
    }
    
    LogEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.level = level;
    entry.category = category;
    entry.component = component;
    entry.function = function;
    entry.file = file;
    entry.line = line;
    entry.threadId = std::this_thread::get_id();
    entry.message = message;
    
    if (async_) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (logQueue_.size() >= maxQueueSize_) {
            // Drop oldest message
            logQueue_.pop();
            droppedMessages_++;
        }
        
        logQueue_.push(entry);
        condition_.notify_one();
    } else {
        // Synchronous logging
        std::lock_guard<std::mutex> lock(mutex_);
        std::string formatted = formatter_->format(entry);
        
        for (auto& sink : sinks_) {
            sink->write(formatted);
        }
    }
}

void Logger::logWithContext(LogLevel level, LogCategory category, const std::string& component,
                           const std::string& function, const std::string& file, int line,
                           const std::string& message, const std::map<std::string, std::string>& context) {
    
    if (!shouldLog(level, category)) {
        return;
    }
    
    LogEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.level = level;
    entry.category = category;
    entry.component = component;
    entry.function = function;
    entry.file = file;
    entry.line = line;
    entry.threadId = std::this_thread::get_id();
    entry.message = message;
    entry.context = context;
    
    if (async_) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (logQueue_.size() >= maxQueueSize_) {
            logQueue_.pop();
            droppedMessages_++;
        }
        
        logQueue_.push(entry);
        condition_.notify_one();
    } else {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string formatted = formatter_->format(entry);
        
        for (auto& sink : sinks_) {
            sink->write(formatted);
        }
    }
}

void Logger::trace(LogCategory category, const std::string& component, const std::string& function,
                  const std::string& file, int line, const std::string& message) {
    log(LogLevel::TRACE, category, component, function, file, line, message);
}

void Logger::debug(LogCategory category, const std::string& component, const std::string& function,
                  const std::string& file, int line, const std::string& message) {
    log(LogLevel::DEBUG, category, component, function, file, line, message);
}

void Logger::info(LogCategory category, const std::string& component, const std::string& function,
                 const std::string& file, int line, const std::string& message) {
    log(LogLevel::INFO, category, component, function, file, line, message);
}

void Logger::warn(LogCategory category, const std::string& component, const std::string& function,
                 const std::string& file, int line, const std::string& message) {
    log(LogLevel::WARN, category, component, function, file, line, message);
}

void Logger::error(LogCategory category, const std::string& component, const std::string& function,
                  const std::string& file, int line, const std::string& message) {
    log(LogLevel::ERROR, category, component, function, file, line, message);
}

void Logger::fatal(LogCategory category, const std::string& component, const std::string& function,
                  const std::string& file, int line, const std::string& message) {
    log(LogLevel::FATAL, category, component, function, file, line, message);
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& sink : sinks_) {
        sink->flush();
    }
}

void Logger::shutdown() {
    if (async_ && !shutdown_.load()) {
        shutdown_ = true;
        condition_.notify_all();
        
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }
    
    flush();
}

size_t Logger::getQueueSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return logQueue_.size();
}

size_t Logger::getDroppedMessages() const {
    return droppedMessages_.load();
}

void Logger::processLogs() {
    while (!shutdown_.load()) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        condition_.wait(lock, [this] {
            return !logQueue_.empty() || shutdown_.load();
        });
        
        while (!logQueue_.empty()) {
            LogEntry entry = logQueue_.front();
            logQueue_.pop();
            
            std::string formatted = formatter_->format(entry);
            
            for (auto& sink : sinks_) {
                sink->write(formatted);
            }
        }
    }
}

bool Logger::shouldLog(LogLevel level, LogCategory category) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check category-specific level first
    auto it = categoryLevels_.find(category);
    if (it != categoryLevels_.end()) {
        return level >= it->second;
    }
    
    // Fall back to global level
    return level >= globalLevel_;
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

std::string Logger::categoryToString(LogCategory category) {
    switch (category) {
        case LogCategory::SYSTEM:      return "SYSTEM";
        case LogCategory::ANDROID_AUTO: return "ANDROID_AUTO";
        case LogCategory::UI:          return "UI";
        case LogCategory::CAMERA:      return "CAMERA";
        case LogCategory::NETWORK:     return "NETWORK";
        case LogCategory::BLUETOOTH:   return "BLUETOOTH";
        case LogCategory::AUDIO:       return "AUDIO";
        case LogCategory::VIDEO:       return "VIDEO";
        case LogCategory::CONFIG:      return "CONFIG";
        case LogCategory::API:         return "API";
        case LogCategory::EVENT:       return "EVENT";
        case LogCategory::STATE:       return "STATE";
        case LogCategory::GENERAL:     return "GENERAL";
        default: return "UNKNOWN";
    }
}

LogLevel Logger::stringToLevel(const std::string& level) {
    if (level == "TRACE") return LogLevel::TRACE;
    if (level == "DEBUG") return LogLevel::DEBUG;
    if (level == "INFO")  return LogLevel::INFO;
    if (level == "WARN")  return LogLevel::WARN;
    if (level == "ERROR") return LogLevel::ERROR;
    if (level == "FATAL") return LogLevel::FATAL;
    return LogLevel::INFO; // Default
}

LogCategory Logger::stringToCategory(const std::string& category) {
    if (category == "SYSTEM")       return LogCategory::SYSTEM;
    if (category == "ANDROID_AUTO") return LogCategory::ANDROID_AUTO;
    if (category == "UI")           return LogCategory::UI;
    if (category == "CAMERA")       return LogCategory::CAMERA;
    if (category == "NETWORK")      return LogCategory::NETWORK;
    if (category == "BLUETOOTH")    return LogCategory::BLUETOOTH;
    if (category == "AUDIO")        return LogCategory::AUDIO;
    if (category == "VIDEO")        return LogCategory::VIDEO;
    if (category == "CONFIG")       return LogCategory::CONFIG;
    if (category == "API")          return LogCategory::API;
    if (category == "EVENT")        return LogCategory::EVENT;
    if (category == "STATE")        return LogCategory::STATE;
    return LogCategory::GENERAL; // Default
}

// ConsoleFormatter Implementation
ConsoleFormatter::ConsoleFormatter(bool useColors, bool showThreadId, bool showLocation)
    : useColors_(useColors), showThreadId_(showThreadId), showLocation_(showLocation) {
}

std::string ConsoleFormatter::format(const LogEntry& entry) {
    std::ostringstream oss;
    
    // Timestamp
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        entry.timestamp.time_since_epoch()) % 1000;
    
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    // Level with color
    if (useColors_) {
        oss << " " << getLevelColor(entry.level) << "[" << Logger::levelToString(entry.level) << "]" << resetColor();
    } else {
        oss << " [" << Logger::levelToString(entry.level) << "]";
    }
    
    // Category with color
    if (useColors_) {
        oss << " " << getCategoryColor(entry.category) << "[" << Logger::categoryToString(entry.category) << "]" << resetColor();
    } else {
        oss << " [" << Logger::categoryToString(entry.category) << "]";
    }
    
    // Thread ID
    if (showThreadId_) {
        oss << " [" << entry.threadId << "]";
    }
    
    // Component and function
    std::string component = entry.component;
    // Clean up C++ mangled names
    if (component.find("class ") == 0) {
        component = component.substr(6);
    }
    size_t lastColon = component.find_last_of(':');
    if (lastColon != std::string::npos) {
        component = component.substr(lastColon + 1);
    }
    
    oss << " [" << component;
    if (!entry.function.empty()) {
        oss << "::" << entry.function;
    }
    oss << "]";
    
    // Location
    if (showLocation_ && !entry.file.empty()) {
        std::string filename = entry.file;
        size_t lastSlash = filename.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            filename = filename.substr(lastSlash + 1);
        }
        oss << " (" << filename << ":" << entry.line << ")";
    }
    
    // Message
    oss << " - " << entry.message;
    
    // Context
    if (!entry.context.empty()) {
        oss << " {";
        bool first = true;
        for (const auto& [key, value] : entry.context) {
            if (!first) oss << ", ";
            oss << key << "=" << value;
            first = false;
        }
        oss << "}";
    }
    
    oss << std::endl;
    return oss.str();
}

std::string ConsoleFormatter::getLevelColor(LogLevel level) const {
    if (!useColors_) return "";
    
    switch (level) {
        case LogLevel::TRACE: return "\033[37m";      // White
        case LogLevel::DEBUG: return "\033[36m";      // Cyan
        case LogLevel::INFO:  return "\033[32m";      // Green
        case LogLevel::WARN:  return "\033[33m";      // Yellow
        case LogLevel::ERROR: return "\033[31m";      // Red
        case LogLevel::FATAL: return "\033[35m";      // Magenta
        default: return "";
    }
}

std::string ConsoleFormatter::getCategoryColor(LogCategory category) const {
    if (!useColors_) return "";
    
    switch (category) {
        case LogCategory::SYSTEM:      return "\033[1;34m";  // Bold Blue
        case LogCategory::ANDROID_AUTO: return "\033[1;32m";  // Bold Green
        case LogCategory::UI:          return "\033[1;36m";  // Bold Cyan
        case LogCategory::CAMERA:      return "\033[1;35m";  // Bold Magenta
        case LogCategory::NETWORK:     return "\033[1;33m";  // Bold Yellow
        case LogCategory::BLUETOOTH:   return "\033[1;34m";  // Bold Blue
        case LogCategory::AUDIO:       return "\033[1;32m";  // Bold Green
        case LogCategory::VIDEO:       return "\033[1;35m";  // Bold Magenta
        case LogCategory::CONFIG:      return "\033[1;37m";  // Bold White
        case LogCategory::API:         return "\033[1;31m";  // Bold Red
        case LogCategory::EVENT:       return "\033[1;36m";  // Bold Cyan
        case LogCategory::STATE:       return "\033[1;33m";  // Bold Yellow
        default: return "";
    }
}

std::string ConsoleFormatter::resetColor() const {
    return useColors_ ? "\033[0m" : "";
}

// JsonFormatter Implementation
JsonFormatter::JsonFormatter(bool prettyPrint) : prettyPrint_(prettyPrint) {
}

std::string JsonFormatter::format(const LogEntry& entry) {
#ifdef ENABLE_MODERN_API
    nlohmann::json json;
    
    // Convert timestamp to ISO 8601
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        entry.timestamp.time_since_epoch()) % 1000;
    
    std::ostringstream timeStr;
    timeStr << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    timeStr << "." << std::setfill('0') << std::setw(3) << ms.count() << "Z";
    
    json["timestamp"] = timeStr.str();
    json["level"] = Logger::levelToString(entry.level);
    json["category"] = Logger::categoryToString(entry.category);
    json["component"] = entry.component;
    json["function"] = entry.function;
    json["file"] = entry.file;
    json["line"] = entry.line;
    
    std::ostringstream threadStr;
    threadStr << entry.threadId;
    json["thread_id"] = threadStr.str();
    
    json["message"] = entry.message;
    
    if (!entry.context.empty()) {
        json["context"] = entry.context;
    }
    
    return json.dump(prettyPrint_ ? 2 : -1) + "\n";
#else
    // Fallback simple format when nlohmann/json is not available
    std::ostringstream oss;
    oss << "{\"level\":\"" << Logger::levelToString(entry.level) 
        << "\",\"category\":\"" << Logger::categoryToString(entry.category)
        << "\",\"message\":\"" << entry.message << "\"}\n";
    return oss.str();
#endif
}

// FileFormatter Implementation
FileFormatter::FileFormatter() {
}

std::string FileFormatter::format(const LogEntry& entry) {
    std::ostringstream oss;
    
    // Timestamp
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        entry.timestamp.time_since_epoch()) % 1000;
    
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    // Level and category
    oss << " [" << Logger::levelToString(entry.level) << "]";
    oss << " [" << Logger::categoryToString(entry.category) << "]";
    
    // Thread ID
    oss << " [" << entry.threadId << "]";
    
    // Component and function
    oss << " [" << entry.component << "::" << entry.function << "]";
    
    // Location
    if (!entry.file.empty()) {
        std::string filename = entry.file;
        size_t lastSlash = filename.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            filename = filename.substr(lastSlash + 1);
        }
        oss << " (" << filename << ":" << entry.line << ")";
    }
    
    // Message
    oss << " - " << entry.message;
    
    // Context
    if (!entry.context.empty()) {
        oss << " {";
        bool first = true;
        for (const auto& [key, value] : entry.context) {
            if (!first) oss << ", ";
            oss << key << "=" << value;
            first = false;
        }
        oss << "}";
    }
    
    oss << std::endl;
    return oss.str();
}

// ConsoleSink Implementation
ConsoleSink::ConsoleSink(bool useStderr) : useStderr_(useStderr) {
}

void ConsoleSink::write(const std::string& message) {
    if (useStderr_) {
        std::cerr << message;
    } else {
        std::cout << message;
    }
}

void ConsoleSink::flush() {
    if (useStderr_) {
        std::cerr.flush();
    } else {
        std::cout.flush();
    }
}

// FileSink Implementation
FileSink::FileSink(const std::string& filename, size_t maxSize, size_t maxFiles)
    : filename_(filename), maxSize_(maxSize), maxFiles_(maxFiles), currentSize_(0) {
    
    file_.open(filename_, std::ios::app);
    if (file_.is_open()) {
        file_.seekp(0, std::ios::end);
        currentSize_ = file_.tellp();
    }
}

FileSink::~FileSink() {
    if (file_.is_open()) {
        file_.close();
    }
}

void FileSink::write(const std::string& message) {
    std::lock_guard<std::mutex> lock(fileMutex_);
    
    if (!file_.is_open()) {
        return;
    }
    
    if (currentSize_ + message.size() > maxSize_) {
        rotateFile();
    }
    
    file_ << message;
    currentSize_ += message.size();
}

void FileSink::flush() {
    std::lock_guard<std::mutex> lock(fileMutex_);
    if (file_.is_open()) {
        file_.flush();
    }
}

void FileSink::rotateFile() {
    if (file_.is_open()) {
        file_.close();
    }
    
    // Rotate existing files
    for (size_t i = maxFiles_ - 1; i > 0; --i) {
        std::string oldFile = filename_ + "." + std::to_string(i);
        std::string newFile = filename_ + "." + std::to_string(i + 1);
        
        if (std::filesystem::exists(oldFile)) {
            if (i == maxFiles_ - 1) {
                std::filesystem::remove(newFile); // Remove oldest
            }
            std::filesystem::rename(oldFile, newFile);
        }
    }
    
    // Move current file to .1
    if (std::filesystem::exists(filename_)) {
        std::filesystem::rename(filename_, filename_ + ".1");
    }
    
    // Open new file
    file_.open(filename_, std::ios::out | std::ios::trunc);
    currentSize_ = 0;
}

// RemoteSink Implementation
RemoteSink::RemoteSink(const std::string& endpoint) : endpoint_(endpoint) {
}

void RemoteSink::write(const std::string& message) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    pendingMessages_.push(message);
    
    // TODO: Implement actual remote sending (HTTP POST, TCP, etc.)
    // For now, just queue the messages
}

void RemoteSink::flush() {
    std::lock_guard<std::mutex> lock(queueMutex_);
    
    // TODO: Implement batch sending of pending messages
    while (!pendingMessages_.empty()) {
        // Send pendingMessages_.front() to endpoint_
        pendingMessages_.pop();
    }
}

} // namespace modern
} // namespace openauto
