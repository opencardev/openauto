# Test Guide

This document provides comprehensive guidance for testing OpenAuto, including unit tests, integration tests, and end-to-end testing scenarios.

## Table of Contents
- [Overview](#overview)
- [Testing Framework](#testing-framework)
- [Running Tests](#running-tests)
- [Unit Testing](#unit-testing)
- [Integration Testing](#integration-testing)
- [End-to-End Testing](#end-to-end-testing)
- [Test Coverage](#test-coverage)
- [Performance Testing](#performance-testing)
- [Test Data Management](#test-data-management)
- [Continuous Testing](#continuous-testing)
- [Troubleshooting](#troubleshooting)

## Overview

OpenAuto uses a comprehensive testing strategy that covers:
- **Unit Tests**: Individual component testing with GoogleTest/GoogleMock
- **Integration Tests**: Component interaction testing
- **End-to-End Tests**: Complete system workflow testing
- **Performance Tests**: Memory, CPU, and throughput testing
- **Security Tests**: Vulnerability and security validation

### Testing Philosophy
1. **Test Early, Test Often**: Continuous testing throughout development
2. **Test Pyramid**: More unit tests, fewer integration/E2E tests
3. **Fast Feedback**: Quick test execution for rapid development
4. **Comprehensive Coverage**: All critical paths and edge cases covered
5. **Realistic Testing**: Tests mirror real-world usage scenarios

## Testing Framework

### GoogleTest/GoogleMock
- **Framework**: GoogleTest 1.12.1+ with GoogleMock
- **Features**: Assertions, fixtures, parameterized tests, death tests
- **Integration**: CMake CTest integration for test discovery

### Test Organization
```
tests/
├── unit/                 # Unit tests for individual components
│   ├── test_logger.cpp
│   ├── test_event_bus.cpp
│   ├── test_configuration_manager.cpp
│   └── ...
├── integration/          # Integration tests
│   ├── test_api_integration.cpp
│   ├── test_android_auto_integration.cpp
│   └── ...
├── mocks/               # Mock objects and stubs
│   ├── MockEventBus.hpp
│   ├── MockLogger.hpp
│   └── ...
├── data/                # Test data files
└── CMakeLists.txt       # Test configuration
```

## Running Tests

### Command Line
```bash
# Build tests
cmake --build build --target all

# Run all tests
cd build
ctest

# Run tests with output
ctest --output-on-failure

# Run tests in parallel
ctest --parallel $(nproc)

# Run specific test suite
ctest -R "unit_tests"

# Run specific test
ctest -R "test_logger"

# Run tests with verbose output
ctest --verbose
```

### VS Code Integration
```json
// .vscode/tasks.json
{
    "label": "Run Tests",
    "type": "shell",
    "command": "ctest",
    "args": ["--test-dir", "build", "--output-on-failure"],
    "group": "test"
}
```

### Test Filtering
```bash
# GoogleTest filtering
./build/tests/unit/unit_tests --gtest_filter="LoggerTest.*"
./build/tests/unit/unit_tests --gtest_filter="*EventBus*"
./build/tests/unit/unit_tests --gtest_filter="-*SlowTest*"

# Run only failed tests
./build/tests/unit/unit_tests --gtest_repeat=10 --gtest_break_on_failure
```

## Unit Testing

### Test Structure
```cpp
// tests/unit/test_logger.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <modern/Logger.hpp>

namespace openauto {
namespace modern {
namespace test {

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        logger = std::make_unique<Logger>();
    }
    
    void TearDown() override {
        logger.reset();
    }
    
    std::unique_ptr<Logger> logger;
};

TEST_F(LoggerTest, ShouldLogAtCorrectLevel) {
    logger->setLogLevel(LogLevel::INFO);
    
    // Test implementation
    EXPECT_NO_THROW(logger->info("Test message", "TEST"));
    EXPECT_NO_THROW(logger->warning("Warning message", "TEST"));
    
    // Debug should be filtered out
    EXPECT_NO_THROW(logger->debug("Debug message", "TEST"));
}

} // namespace test
} // namespace modern
} // namespace openauto
```

### Parameterized Tests
```cpp
class LogLevelTest : public ::testing::TestWithParam<LogLevel> {
protected:
    Logger logger;
};

TEST_P(LogLevelTest, ShouldFilterCorrectly) {
    LogLevel level = GetParam();
    logger.setLogLevel(level);
    
    // Test level filtering logic
    EXPECT_TRUE(logger.shouldLog(level));
    EXPECT_FALSE(logger.shouldLog(static_cast<LogLevel>(level - 1)));
}

INSTANTIATE_TEST_SUITE_P(
    LogLevels,
    LogLevelTest,
    ::testing::Values(LogLevel::DEBUG, LogLevel::INFO, LogLevel::WARNING, LogLevel::ERROR)
);
```

### Mock Usage
```cpp
using ::testing::StrictMock;
using ::testing::Return;
using ::testing::_;

TEST_F(EventBusTest, ShouldNotifySubscribers) {
    auto mockHandler = std::make_shared<StrictMock<MockEventHandler>>();
    
    EXPECT_CALL(*mockHandler, handleEvent(_))
        .Times(1)
        .WillOnce(Return(true));
    
    eventBus.subscribe(EventType::STATE_CHANGED, mockHandler);
    eventBus.publish(std::make_shared<StateChangedEvent>(State::CONNECTED));
}
```

## Integration Testing

### Component Integration
```cpp
// tests/integration/test_configuration_integration.cpp
class ConfigurationIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        eventBus = std::make_shared<EventBus>();
        logger = std::make_shared<Logger>();
        configManager = std::make_unique<ConfigurationManager>(eventBus, logger);
        
        // Setup test configuration file
        testConfigPath = "/tmp/test_config.json";
        configManager->setConfigurationFile(testConfigPath);
    }
    
    void TearDown() override {
        std::filesystem::remove(testConfigPath);
    }
    
    std::shared_ptr<EventBus> eventBus;
    std::shared_ptr<Logger> logger;
    std::unique_ptr<ConfigurationManager> configManager;
    std::string testConfigPath;
};

TEST_F(ConfigurationIntegrationTest, ShouldPersistAndNotifyChanges) {
    // Setup event listener
    bool eventReceived = false;
    std::string subscription = eventBus->subscribe(
        EventType::CONFIG_CHANGED,
        [&eventReceived](std::shared_ptr<Event> event) {
            eventReceived = true;
        }
    );
    
    // Make configuration change
    configManager->setValue("test.setting", "test_value");
    configManager->save();
    
    // Verify persistence
    configManager->load();
    EXPECT_EQ(configManager->getValue("test.setting"), "test_value");
    
    // Verify event notification
    EXPECT_TRUE(eventReceived);
    
    eventBus->unsubscribe(subscription);
}
```

### System Integration
```cpp
// tests/integration/test_android_auto_integration.cpp
class AndroidAutoIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup complete system stack
        eventBus = std::make_shared<EventBus>();
        logger = std::make_shared<Logger>();
        configManager = std::make_unique<ConfigurationManager>(eventBus, logger);
        stateMachine = std::make_unique<StateMachine>(eventBus, logger);
        
        // Initialize with test configuration
        setupTestConfiguration();
    }
    
    void setupTestConfiguration() {
        nlohmann::json config = {
            {"android_auto", {
                {"video_fps", 30},
                {"video_resolution", "1280x720"},
                {"audio_sample_rate", 48000}
            }}
        };
        configManager->loadFromJson(config);
    }
};

TEST_F(AndroidAutoIntegrationTest, ShouldCompleteConnectionFlow) {
    // Simulate device connection
    auto connectEvent = std::make_shared<DeviceConnectedEvent>("test_device");
    eventBus->publish(connectEvent);
    
    // Wait for state transitions
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Verify state machine progression
    EXPECT_EQ(stateMachine->getCurrentState(), State::CONNECTING);
    
    // Simulate successful handshake
    auto handshakeEvent = std::make_shared<HandshakeCompleteEvent>();
    eventBus->publish(handshakeEvent);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(stateMachine->getCurrentState(), State::CONNECTED);
}
```

## End-to-End Testing

### Complete System Tests
```cpp
// tests/integration/test_end_to_end.cpp
class EndToEndTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup complete OpenAuto system
        system = std::make_unique<OpenAutoSystem>();
        system->initialize();
        
        // Setup test environment
        setupTestEnvironment();
    }
    
    void setupTestEnvironment() {
        // Create test configuration
        // Setup mock USB devices
        // Initialize test data
    }
    
    std::unique_ptr<OpenAutoSystem> system;
};

TEST_F(EndToEndTest, CompleteAndroidAutoSession) {
    // Test complete workflow from startup to shutdown
    
    // 1. System startup
    ASSERT_TRUE(system->startup());
    EXPECT_EQ(system->getState(), SystemState::READY);
    
    // 2. Device connection simulation
    auto mockDevice = createMockAndroidDevice();
    system->connectDevice(mockDevice);
    
    // 3. Wait for connection establishment
    waitForState(SystemState::CONNECTED, std::chrono::seconds(5));
    EXPECT_EQ(system->getState(), SystemState::CONNECTED);
    
    // 4. Simulate Android Auto session
    system->startAndroidAutoSession();
    waitForState(SystemState::ACTIVE, std::chrono::seconds(5));
    
    // 5. Test media streaming
    EXPECT_TRUE(system->isVideoStreaming());
    EXPECT_TRUE(system->isAudioStreaming());
    
    // 6. Test input handling
    system->sendTouchInput(TouchEvent(100, 200, TouchAction::PRESS));
    system->sendKeyInput(KeyEvent(KeyCode::HOME, KeyAction::PRESS));
    
    // 7. Graceful disconnection
    system->disconnectDevice();
    waitForState(SystemState::READY, std::chrono::seconds(5));
    
    // 8. System shutdown
    system->shutdown();
    EXPECT_EQ(system->getState(), SystemState::STOPPED);
}
```

### Performance Testing
```cpp
TEST_F(EndToEndTest, HighVolumeEventProcessing) {
    const int EVENT_COUNT = 10000;
    const auto TEST_DURATION = std::chrono::seconds(10);
    
    auto startTime = std::chrono::steady_clock::now();
    
    // Generate high volume of events
    for (int i = 0; i < EVENT_COUNT; ++i) {
        auto event = std::make_shared<TestEvent>(i);
        system->getEventBus()->publish(event);
    }
    
    // Wait for processing completion
    system->waitForEventProcessingComplete();
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Performance assertions
    EXPECT_LT(duration, TEST_DURATION);
    EXPECT_GT(EVENT_COUNT / duration.count() * 1000, 1000); // > 1000 events/sec
}
```

## Test Coverage

### Coverage Configuration
```cmake
# CMakeLists.txt - Coverage setup
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()
```

### Generating Coverage Reports
```bash
# Build with coverage
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
cmake --build build

# Run tests
cd build
ctest

# Generate coverage report
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/build/_deps/*' '*/tests/*' --output-file coverage_filtered.info
genhtml coverage_filtered.info --output-directory coverage_html

# View coverage report
xdg-open coverage_html/index.html
```

### Coverage Targets
- **Overall Coverage**: > 80%
- **Critical Components**: > 95%
- **Modern Components**: > 90%
- **Legacy Components**: > 70%

## Performance Testing

### Memory Testing
```bash
# Memory leak detection
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
  ./build/bin/autoapp --config config/test.json

# Memory usage profiling
valgrind --tool=massif ./build/bin/autoapp --config config/test.json
ms_print massif.out.* > memory_profile.txt
```

### CPU Profiling
```bash
# CPU profiling with perf
perf record -g ./build/bin/autoapp --config config/test.json
perf report > cpu_profile.txt

# CPU usage monitoring
top -p $(pgrep autoapp) -d 1 > cpu_usage.log &
```

### Load Testing
```cpp
TEST_F(PerformanceTest, ConcurrentEventProcessing) {
    const int THREAD_COUNT = 10;
    const int EVENTS_PER_THREAD = 1000;
    
    std::vector<std::thread> threads;
    std::atomic<int> eventsProcessed{0};
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Start concurrent event generation
    for (int t = 0; t < THREAD_COUNT; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < EVENTS_PER_THREAD; ++i) {
                auto event = std::make_shared<LoadTestEvent>(t, i);
                eventBus->publish(event);
                eventsProcessed++;
            }
        });
    }
    
    // Wait for completion
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    EXPECT_EQ(eventsProcessed.load(), THREAD_COUNT * EVENTS_PER_THREAD);
    EXPECT_LT(duration.count(), 5000); // Complete within 5 seconds
}
```

## Test Data Management

### Test Data Organization
```
tests/data/
├── configurations/
│   ├── minimal.json
│   ├── development.json
│   └── production.json
├── media/
│   ├── test_audio.wav
│   └── test_video.mp4
├── usb_devices/
│   ├── android_pixel.json
│   └── android_samsung.json
└── expected_outputs/
    ├── api_responses/
    └── log_patterns/
```

### Test Data Fixtures
```cpp
class TestDataFixture : public ::testing::Test {
protected:
    void SetUp() override {
        testDataPath = std::filesystem::current_path() / "tests" / "data";
        ASSERT_TRUE(std::filesystem::exists(testDataPath));
    }
    
    nlohmann::json loadTestConfig(const std::string& name) {
        auto configPath = testDataPath / "configurations" / (name + ".json");
        std::ifstream file(configPath);
        EXPECT_TRUE(file.is_open()) << "Failed to open: " << configPath;
        
        nlohmann::json config;
        file >> config;
        return config;
    }
    
    std::filesystem::path testDataPath;
};
```

## Continuous Testing

### Pre-commit Testing
```yaml
# .pre-commit-config.yaml
repos:
  - repo: local
    hooks:
      - id: unit-tests
        name: Run unit tests
        entry: bash -c 'cd build && ctest -R unit_tests'
        language: system
        pass_filenames: false
        always_run: true
```

### CI/CD Integration
```yaml
# GitHub Actions test job
- name: Run Tests
  run: |
    cd build
    ctest --output-on-failure --parallel $(nproc)
    
- name: Upload Test Results
  uses: actions/upload-artifact@v3
  if: always()
  with:
    name: test-results
    path: build/Testing/
```

### Automated Test Reporting
```bash
# Generate test report
cd build
ctest --output-junit test_results.xml

# Convert to different formats
python3 ../scripts/convert_test_results.py test_results.xml
```

## Troubleshooting

### Common Issues

#### Test Failures
```bash
# Run failed tests only
ctest --rerun-failed --output-on-failure

# Debug specific test
gdb ./build/tests/unit/unit_tests
(gdb) set args --gtest_filter="FailingTest.*"
(gdb) run
```

#### Memory Issues in Tests
```bash
# Check for memory leaks in tests
valgrind --tool=memcheck ./build/tests/unit/unit_tests

# Check for thread safety issues
valgrind --tool=helgrind ./build/tests/integration/integration_tests
```

#### Performance Issues
```bash
# Profile test execution
perf record ./build/tests/unit/unit_tests
perf report

# Monitor resource usage
htop & ./build/tests/integration/integration_tests
```

### Test Environment Issues
```bash
# Clean test environment
rm -rf build/Testing/
rm -rf /tmp/openauto_test_*

# Reset test database/files
rm -rf tests/data/temp/
mkdir -p tests/data/temp/

# Check test dependencies
ldd ./build/tests/unit/unit_tests
```

### Debugging Test Code
```cpp
// Add debug output to tests
TEST_F(DebugTest, SomeTest) {
    SCOPED_TRACE("Debug test execution");
    
    // Debug output (only in debug builds)
    #ifdef DEBUG
    std::cout << "Debug: variable value = " << value << std::endl;
    #endif
    
    // Add detailed assertions
    ASSERT_TRUE(condition) << "Detailed failure message: " << details;
}
```

## Best Practices

### Test Design
1. **AAA Pattern**: Arrange, Act, Assert structure
2. **Single Responsibility**: One concept per test
3. **Deterministic**: Tests should always produce same results
4. **Fast Execution**: Unit tests < 1s, integration tests < 10s
5. **Clear Naming**: Test names should describe what they test

### Test Maintenance
1. **Regular Review**: Update tests with code changes
2. **Refactor Tests**: Keep test code clean and maintainable
3. **Remove Obsolete Tests**: Clean up outdated test cases
4. **Monitor Coverage**: Maintain high test coverage
5. **Document Edge Cases**: Document complex test scenarios

### Test Organization
1. **Logical Grouping**: Group related tests together
2. **Shared Fixtures**: Reuse common test setup
3. **Test Data Management**: Centralized test data organization
4. **Environment Isolation**: Tests should not interfere with each other
5. **Platform Independence**: Tests should work across platforms

## Related Documentation

- [Unit Testing Guide](unit-testing.md) - Detailed unit testing practices
- [Integration Testing Guide](integration-testing.md) - Integration test strategies
- [Test Automation Guide](test-automation.md) - CI/CD test automation
- [Performance Guide](performance-guide.md) - Performance optimization
- [Troubleshooting Guide](troubleshooting-guide.md) - General troubleshooting
