#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <modern/Logger.hpp>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace openauto::modern;
using namespace testing;

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clean up any existing test files
        std::filesystem::remove("test_log.txt");
        std::filesystem::remove("test_json_log.txt");
    }

    void TearDown() override {
        // Clean up test files
        std::filesystem::remove("test_log.txt");
        std::filesystem::remove("test_json_log.txt");
    }
};

// Test basic logger initialization
TEST_F(LoggerTest, InitializationTest) {
    Logger& logger = Logger::getInstance();
    EXPECT_NO_THROW(logger.setLevel(LogLevel::INFO));
    EXPECT_NO_THROW(logger.setLevel(LogLevel::DEBUG));
    EXPECT_NO_THROW(logger.setLevel(LogLevel::ERROR));
}

// Test log level filtering
TEST_F(LoggerTest, LogLevelFiltering) {
    Logger& logger = Logger::getInstance();
    
    // Set level to WARNING - should filter out DEBUG and INFO
    logger.setLevel(LogLevel::WARNING);
    
    // These should not appear in output
    EXPECT_NO_THROW(LOG_DEBUG(SYSTEM, "Debug message"));
    EXPECT_NO_THROW(LOG_INFO(SYSTEM, "Info message"));
    
    // These should appear
    EXPECT_NO_THROW(LOG_WARNING(SYSTEM, "Warning message"));
    EXPECT_NO_THROW(LOG_ERROR(SYSTEM, "Error message"));
    EXPECT_NO_THROW(LOG_FATAL(SYSTEM, "Fatal message"));
}

// Test file sink functionality
TEST_F(LoggerTest, FileSinkTest) {
    Logger& logger = Logger::getInstance();
    
    // Add file sink
    EXPECT_NO_THROW(logger.addFileSink("test_log.txt"));
    
    // Log a test message
    LOG_INFO(SYSTEM, "Test file message");
    
    // Wait for async processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Check if file was created and contains message
    EXPECT_TRUE(std::filesystem::exists("test_log.txt"));
    
    std::ifstream file("test_log.txt");
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    EXPECT_THAT(content, HasSubstr("Test file message"));
    EXPECT_THAT(content, HasSubstr("SYSTEM"));
    EXPECT_THAT(content, HasSubstr("INFO"));
}

// Test JSON formatting
TEST_F(LoggerTest, JsonFormattingTest) {
    Logger& logger = Logger::getInstance();
    
    // Add JSON file sink
    EXPECT_NO_THROW(logger.addFileSink("test_json_log.txt", true));
    
    // Log a test message
    LOG_INFO(NETWORK, "JSON test message");
    
    // Wait for async processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Check if file was created and contains valid JSON
    EXPECT_TRUE(std::filesystem::exists("test_json_log.txt"));
    
    std::ifstream file("test_json_log.txt");
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    
    // Should contain JSON structure
    EXPECT_THAT(content, HasSubstr("\"level\""));
    EXPECT_THAT(content, HasSubstr("\"category\""));
    EXPECT_THAT(content, HasSubstr("\"message\""));
    EXPECT_THAT(content, HasSubstr("\"timestamp\""));
    EXPECT_THAT(content, HasSubstr("JSON test message"));
}

// Test stream macros
TEST_F(LoggerTest, StreamMacrosTest) {
    Logger& logger = Logger::getInstance();
    logger.setLevel(LogLevel::DEBUG);
    
    std::string testValue = "stream_test";
    int numValue = 42;
    
    // Test stream-style logging
    EXPECT_NO_THROW(LOG_DEBUG_STREAM(ANDROID_AUTO, "Stream test: " << testValue << " number: " << numValue));
    EXPECT_NO_THROW(LOG_INFO_STREAM(UI, "Info stream: " << testValue));
    EXPECT_NO_THROW(LOG_WARNING_STREAM(BLUETOOTH, "Warning stream: " << numValue));
    EXPECT_NO_THROW(LOG_ERROR_STREAM(NETWORK, "Error stream: " << testValue << " " << numValue));
}

// Test categories
TEST_F(LoggerTest, CategoriesTest) {
    Logger& logger = Logger::getInstance();
    
    // Test all defined categories
    EXPECT_NO_THROW(LOG_INFO(SYSTEM, "System message"));
    EXPECT_NO_THROW(LOG_INFO(ANDROID_AUTO, "Android Auto message"));
    EXPECT_NO_THROW(LOG_INFO(BLUETOOTH, "Bluetooth message"));
    EXPECT_NO_THROW(LOG_INFO(NETWORK, "Network message"));
    EXPECT_NO_THROW(LOG_INFO(UI, "UI message"));
    EXPECT_NO_THROW(LOG_INFO(AUDIO, "Audio message"));
    EXPECT_NO_THROW(LOG_INFO(VIDEO, "Video message"));
    EXPECT_NO_THROW(LOG_INFO(CONFIG, "Config message"));
    EXPECT_NO_THROW(LOG_INFO(API, "API message"));
}

// Test context functionality
TEST_F(LoggerTest, ContextTest) {
    Logger& logger = Logger::getInstance();
    
    // Test context setting and retrieval
    std::unordered_map<std::string, std::string> context = {
        {"session_id", "test_session_123"},
        {"user_id", "test_user"},
        {"component", "test_component"}
    };
    
    EXPECT_NO_THROW(logger.setContext(context));
    
    auto retrievedContext = logger.getContext();
    EXPECT_EQ(retrievedContext["session_id"], "test_session_123");
    EXPECT_EQ(retrievedContext["user_id"], "test_user");
    EXPECT_EQ(retrievedContext["component"], "test_component");
}

// Test async functionality doesn't crash
TEST_F(LoggerTest, AsyncFunctionalityTest) {
    Logger& logger = Logger::getInstance();
    
    // Log many messages quickly to test async processing
    for (int i = 0; i < 100; ++i) {
        LOG_INFO(SYSTEM, "Async test message " + std::to_string(i));
    }
    
    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Logger should still be functional
    EXPECT_NO_THROW(LOG_INFO(SYSTEM, "Final async test message"));
}

// Test error conditions
TEST_F(LoggerTest, ErrorConditionsTest) {
    Logger& logger = Logger::getInstance();
    
    // Test adding file sink with invalid path (should handle gracefully)
    EXPECT_NO_THROW(logger.addFileSink("/invalid/path/test.log"));
    
    // Logger should still function
    EXPECT_NO_THROW(LOG_INFO(SYSTEM, "After invalid path test"));
}

// Test thread safety
TEST_F(LoggerTest, ThreadSafetyTest) {
    Logger& logger = Logger::getInstance();
    
    std::vector<std::thread> threads;
    std::atomic<int> counter{0};
    
    // Create multiple threads logging simultaneously
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&logger, &counter, i]() {
            for (int j = 0; j < 10; ++j) {
                LOG_INFO(SYSTEM, "Thread " + std::to_string(i) + " message " + std::to_string(j));
                counter++;
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(counter.load(), 100);
}
