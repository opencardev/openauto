#pragma once

#include <gmock/gmock.h>
#include <modern/Logger.hpp>

namespace openauto {
namespace modern {
namespace test {

class MockLogger {
  public:
    MOCK_METHOD(void, log,
                (LogLevel level, const std::string& message, const std::string& component));
    MOCK_METHOD(void, debug, (const std::string& message, const std::string& component));
    MOCK_METHOD(void, info, (const std::string& message, const std::string& component));
    MOCK_METHOD(void, warning, (const std::string& message, const std::string& component));
    MOCK_METHOD(void, error, (const std::string& message, const std::string& component));
    MOCK_METHOD(void, setLogLevel, (LogLevel level));
    MOCK_METHOD(LogLevel, getLogLevel, (), (const));
    MOCK_METHOD(void, addFileSink, (const std::string& filename));
    MOCK_METHOD(void, enableConsoleOutput, (bool enable));
    MOCK_METHOD(void, enableAsyncLogging, (bool enable));
    MOCK_METHOD(void, flush, ());
};

}  // namespace test
}  // namespace modern
}  // namespace openauto
