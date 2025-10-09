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
#include <algorithm>
#include <filesystem>
#include <regex>

namespace f1x {
namespace openauto {
namespace common {

// ModernLogger Implementation
ModernLogger& ModernLogger::getInstance() {
    static ModernLogger instance;
    return instance;
}

ModernLogger::ModernLogger() 
    : globalLevel_(LogLevel::INFO)
    , async_(false)
    , running_(false)
    , maxQueueSize_(1000)
    , droppedMessages_(0) {
    
    // Set up default console formatter and sink
    formatter_ = std::make_shared<ConsoleFormatter>();
    auto consoleSink = std::make_shared<ConsoleSink>();
    sinks_.push_back(consoleSink);
}

ModernLogger::~ModernLogger() {
    shutdown();
}

void ModernLogger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    globalLevel_ = level;
}

void ModernLogger::setCategoryLevel(LogCategory category, LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    categoryLevels_[category] = level;
}

void ModernLogger::addSink(std::shared_ptr<LogSink> sink) {
    std::lock_guard<std::mutex> lock(mutex_);
    sinks_.push_back(sink);
}

void ModernLogger::setFormatter(std::shared_ptr<LogFormatter> formatter) {
    std::lock_guard<std::mutex> lock(mutex_);
    formatter_ = formatter;
}

void ModernLogger::setAsync(bool async) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (async_ == async) return;
    
    async_ = async;
    
    if (async && !running_) {
        running_ = true;
        workerThread_ = std::thread(&ModernLogger::processLogs, this);
    } else if (!async && running_) {
        running_ = false;
        condition_.notify_all();
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }
}

void ModernLogger::setMaxQueueSize(size_t maxSize) {
    std::lock_guard<std::mutex> lock(mutex_);
    maxQueueSize_ = maxSize;
}

bool ModernLogger::shouldLog(LogLevel level, LogCategory category) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check category-specific level first
    auto it = categoryLevels_.find(category);
    if (it != categoryLevels_.end()) {
        return level >= it->second;
    }
    
    // Fall back to global level
    return level >= globalLevel_;
}

void ModernLogger::log(LogLevel level, LogCategory category, const std::string& component,
                      const std::string& function, const std::string& file, int line,
                      const std::string& message) {
    
    if (!shouldLog(level, category)) {
        return;
    }
    
    LogEntry entry{
        std::chrono::system_clock::now(),
        level,
        category,
        component,
        function,
        file,
        line,
        std::this_thread::get_id(),
        message,
        {}
    };
    
    if (async_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (logQueue_.size() >= maxQueueSize_) {
            droppedMessages_++;
            return;
        }
        logQueue_.push(entry);
        condition_.notify_one();
    } else {
        // Synchronous logging
        std::lock_guard<std::mutex> lock(mutex_);
        if (formatter_) {
            std::string formatted = formatter_->format(entry);
            for (auto& sink : sinks_) {
                sink->write(formatted);
            }
        }
    }
}

void ModernLogger::logWithContext(LogLevel level, LogCategory category, const std::string& component,
                                 const std::string& function, const std::string& file, int line,
                                 const std::string& message, const std::map<std::string, std::string>& context) {
    
    if (!shouldLog(level, category)) {
        return;
    }
    
    LogEntry entry{
        std::chrono::system_clock::now(),
        level,
        category,
        component,
        function,
        file,
        line,
        std::this_thread::get_id(),
        message,
        context
    };
    
    if (async_) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (logQueue_.size() >= maxQueueSize_) {
            droppedMessages_++;
            return;
        }
        logQueue_.push(entry);
        condition_.notify_one();
    } else {
        // Synchronous logging
        std::lock_guard<std::mutex> lock(mutex_);
        if (formatter_) {
            std::string formatted = formatter_->format(entry);
            for (auto& sink : sinks_) {
                sink->write(formatted);
            }
        }
    }
}

void ModernLogger::trace(LogCategory category, const std::string& component, const std::string& function,
                        const std::string& file, int line, const std::string& message) {
    log(LogLevel::TRACE, category, component, function, file, line, message);
}

void ModernLogger::debug(LogCategory category, const std::string& component, const std::string& function,
                        const std::string& file, int line, const std::string& message) {
    log(LogLevel::DEBUG, category, component, function, file, line, message);
}

void ModernLogger::info(LogCategory category, const std::string& component, const std::string& function,
                       const std::string& file, int line, const std::string& message) {
    log(LogLevel::INFO, category, component, function, file, line, message);
}

void ModernLogger::warn(LogCategory category, const std::string& component, const std::string& function,
                       const std::string& file, int line, const std::string& message) {
    log(LogLevel::WARN, category, component, function, file, line, message);
}

void ModernLogger::error(LogCategory category, const std::string& component, const std::string& function,
                        const std::string& file, int line, const std::string& message) {
    log(LogLevel::ERROR, category, component, function, file, line, message);
}

void ModernLogger::fatal(LogCategory category, const std::string& component, const std::string& function,
                        const std::string& file, int line, const std::string& message) {
    log(LogLevel::FATAL, category, component, function, file, line, message);
}

void ModernLogger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& sink : sinks_) {
        sink->flush();
    }
}

void ModernLogger::shutdown() {
    if (running_) {
        running_ = false;
        condition_.notify_all();
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }
    flush();
}

size_t ModernLogger::getQueueSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return logQueue_.size();
}

size_t ModernLogger::getDroppedMessages() const {
    return droppedMessages_.load();
}

void ModernLogger::processLogs() {
    while (running_) {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return !logQueue_.empty() || !running_; });
        
        while (!logQueue_.empty()) {
            LogEntry entry = logQueue_.front();
            logQueue_.pop();
            
            if (formatter_) {
                std::string formatted = formatter_->format(entry);
                for (auto& sink : sinks_) {
                    sink->write(formatted);
                }
            }
        }
    }
}

// Utility methods
std::string ModernLogger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

std::string ModernLogger::categoryToString(LogCategory category) {
    switch (category) {
        case LogCategory::GENERAL: return "GENERAL";
        case LogCategory::SYSTEM: return "SYSTEM";
        case LogCategory::ANDROID_AUTO: return "ANDROID_AUTO";
        case LogCategory::UI: return "UI";
        case LogCategory::AUDIO: return "AUDIO";
        case LogCategory::VIDEO: return "VIDEO";
        case LogCategory::BLUETOOTH: return "BLUETOOTH";
        case LogCategory::CAMERA: return "CAMERA";
        case LogCategory::NETWORK: return "NETWORK";
        case LogCategory::CONFIG: return "CONFIG";
        case LogCategory::PROJECTION: return "PROJECTION";
        case LogCategory::INPUT: return "INPUT";
        case LogCategory::SERVICE: return "SERVICE";
        case LogCategory::SETTINGS: return "SETTINGS";
        case LogCategory::MEDIA: return "MEDIA";
        case LogCategory::NAVIGATION: return "NAVIGATION";
        case LogCategory::PHONE: return "PHONE";
        case LogCategory::WIFI: return "WIFI";
        case LogCategory::USB: return "USB";
        case LogCategory::SECURITY: return "SECURITY";
        default: return "UNKNOWN";
    }
}

LogLevel ModernLogger::stringToLevel(const std::string& level) {
    std::string upper = level;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    
    if (upper == "TRACE") return LogLevel::TRACE;
    if (upper == "DEBUG") return LogLevel::DEBUG;
    if (upper == "INFO") return LogLevel::INFO;
    if (upper == "WARN" || upper == "WARNING") return LogLevel::WARN;
    if (upper == "ERROR") return LogLevel::ERROR;
    if (upper == "FATAL") return LogLevel::FATAL;
    
    return LogLevel::INFO; // Default
}

LogCategory ModernLogger::stringToCategory(const std::string& category) {
    std::string upper = category;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    
    if (upper == "GENERAL") return LogCategory::GENERAL;
    if (upper == "SYSTEM") return LogCategory::SYSTEM;
    if (upper == "ANDROID_AUTO") return LogCategory::ANDROID_AUTO;
    if (upper == "UI") return LogCategory::UI;
    if (upper == "AUDIO") return LogCategory::AUDIO;
    if (upper == "VIDEO") return LogCategory::VIDEO;
    if (upper == "BLUETOOTH") return LogCategory::BLUETOOTH;
    if (upper == "CAMERA") return LogCategory::CAMERA;
    if (upper == "NETWORK") return LogCategory::NETWORK;
    if (upper == "CONFIG") return LogCategory::CONFIG;
    if (upper == "PROJECTION") return LogCategory::PROJECTION;
    if (upper == "INPUT") return LogCategory::INPUT;
    if (upper == "SERVICE") return LogCategory::SERVICE;
    if (upper == "SETTINGS") return LogCategory::SETTINGS;
    if (upper == "MEDIA") return LogCategory::MEDIA;
    if (upper == "NAVIGATION") return LogCategory::NAVIGATION;
    if (upper == "PHONE") return LogCategory::PHONE;
    if (upper == "WIFI") return LogCategory::WIFI;
    if (upper == "USB") return LogCategory::USB;
    if (upper == "SECURITY") return LogCategory::SECURITY;
    
    return LogCategory::GENERAL; // Default
}

// Formatter implementations
std::string ConsoleFormatter::format(const LogEntry& entry) {
    auto time = std::chrono::system_clock::to_time_t(entry.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        entry.timestamp.time_since_epoch()) % 1000;
    
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    ss << " [" << ModernLogger::levelToString(entry.level) << "]";
    ss << " [" << ModernLogger::categoryToString(entry.category) << "]";
    ss << " " << entry.message << std::endl;
    
    return ss.str();
}

std::string JsonFormatter::format(const LogEntry& entry) {
    auto time = std::chrono::system_clock::to_time_t(entry.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        entry.timestamp.time_since_epoch()) % 1000;
    
    std::ostringstream ss;
    ss << "{";
    ss << "\"timestamp\":\"" << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count() << "Z\",";
    ss << "\"level\":\"" << ModernLogger::levelToString(entry.level) << "\",";
    ss << "\"category\":\"" << ModernLogger::categoryToString(entry.category) << "\",";
    ss << "\"message\":\"" << entry.message << "\",";
    ss << "\"file\":\"" << entry.file << "\",";
    ss << "\"function\":\"" << entry.function << "\",";
    ss << "\"line\":" << entry.line;
    
    if (!entry.context.empty()) {
        ss << ",\"context\":{";
        bool first = true;
        for (const auto& [key, value] : entry.context) {
            if (!first) ss << ",";
            ss << "\"" << key << "\":\"" << value << "\"";
            first = false;
        }
        ss << "}";
    }
    
    ss << "}" << std::endl;
    return ss.str();
}

std::string DetailedFormatter::format(const LogEntry& entry) {
    auto time = std::chrono::system_clock::to_time_t(entry.timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        entry.timestamp.time_since_epoch()) % 1000;
    
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    ss << " [" << ModernLogger::levelToString(entry.level) << "]";
    ss << " [" << ModernLogger::categoryToString(entry.category) << "]";
    ss << " [" << entry.threadId << "]";
    ss << " [" << std::filesystem::path(entry.file).filename().string() << ":" << entry.line << "]";
    ss << " [" << entry.function << "]";
    ss << " " << entry.message;
    
    if (!entry.context.empty()) {
        ss << " {";
        bool first = true;
        for (const auto& [key, value] : entry.context) {
            if (!first) ss << ", ";
            ss << key << "=" << value;
            first = false;
        }
        ss << "}";
    }
    
    ss << std::endl;
    return ss.str();
}

// Sink implementations
ConsoleSink::ConsoleSink(bool useStderr) : useStderr_(useStderr) {}

void ConsoleSink::write(const std::string& message) {
    std::lock_guard<std::mutex> lock(consoleMutex_);
    if (useStderr_) {
        std::cerr << message;
    } else {
        std::cout << message;
    }
}

void ConsoleSink::flush() {
    std::lock_guard<std::mutex> lock(consoleMutex_);
    if (useStderr_) {
        std::cerr.flush();
    } else {
        std::cout.flush();
    }
}

FileSink::FileSink(const std::string& filename, size_t maxSize, size_t maxFiles)
    : filename_(filename), maxSize_(maxSize), maxFiles_(maxFiles), currentSize_(0) {
    
    file_.open(filename_, std::ios::app);
    if (file_.is_open()) {
        file_.seekp(0, std::ios::end);
        currentSize_ = file_.tellp();
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
    for (size_t i = maxFiles_; i > 1; --i) {
        std::string oldFile = filename_ + "." + std::to_string(i - 1);
        std::string newFile = filename_ + "." + std::to_string(i);
        
        if (std::filesystem::exists(oldFile)) {
            std::filesystem::rename(oldFile, newFile);
        }
    }
    
    // Move current log file to .1
    if (std::filesystem::exists(filename_)) {
        std::filesystem::rename(filename_, filename_ + ".1");
    }
    
    // Open new file
    file_.open(filename_, std::ios::out | std::ios::trunc);
    currentSize_ = 0;
}

RemoteSink::RemoteSink(const std::string& endpoint) : endpoint_(endpoint) {
    // Implementation would depend on specific network protocol
    // For now, this is a placeholder
}

void RemoteSink::write(const std::string& message) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    pendingMessages_.push(message);
    // In a real implementation, this would send over network
}

void RemoteSink::flush() {
    std::lock_guard<std::mutex> lock(queueMutex_);
    // In a real implementation, this would ensure all pending messages are sent
    while (!pendingMessages_.empty()) {
        pendingMessages_.pop();
    }
}

// Legacy log stream implementation
LegacyLogStream::LegacyLogStream(LogLevel level, LogCategory category, const std::string& component,
                                const std::string& function, const std::string& file, int line)
    : level_(level), category_(category), component_(component), function_(function), file_(file), line_(line) {
}

LegacyLogStream::~LegacyLogStream() {
    auto& logger = ModernLogger::getInstance();
    logger.log(level_, category_, component_, function_, file_, line_, stream_.str());
}

} // namespace common
} // namespace openauto
} // namespace f1x