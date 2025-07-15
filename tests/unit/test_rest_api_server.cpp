#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <modern/RestApiServer.hpp>
#include <modern/EventBus.hpp>
#include <modern/ConfigurationManager.hpp>
#include <modern/StateMachine.hpp>
#include <thread>
#include <chrono>

using namespace openauto::modern;
using namespace testing;

class RestApiServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        eventBus = std::make_shared<EventBus>();
        configManager = std::make_shared<ConfigurationManager>();
        stateMachine = std::make_shared<StateMachine>(eventBus);
        
        // Use a test port to avoid conflicts
        testPort = 18080;
        apiServer = std::make_unique<RestApiServer>(eventBus, configManager, stateMachine, "127.0.0.1", testPort);
    }

    void TearDown() override {
        if (apiServer) {
            apiServer->stop();
        }
        apiServer.reset();
        stateMachine.reset();
        configManager.reset();
        eventBus.reset();
    }

    std::unique_ptr<RestApiServer> apiServer;
    std::shared_ptr<EventBus> eventBus;
    std::shared_ptr<ConfigurationManager> configManager;
    std::shared_ptr<StateMachine> stateMachine;
    int testPort;
};

// Test server initialization
TEST_F(RestApiServerTest, InitializationTest) {
    EXPECT_NO_THROW(apiServer->start());
    EXPECT_TRUE(apiServer->isRunning());
    
    // Wait a moment for server to fully start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_NO_THROW(apiServer->stop());
    EXPECT_FALSE(apiServer->isRunning());
}

// Test server configuration
TEST_F(RestApiServerTest, ConfigurationTest) {
    // Test getting server info
    auto info = apiServer->getServerInfo();
    EXPECT_EQ(info["port"], testPort);
    EXPECT_EQ(info["host"], "127.0.0.1");
    EXPECT_FALSE(info["running"].get<bool>());
    
    // Start server and check again
    apiServer->start();
    info = apiServer->getServerInfo();
    EXPECT_TRUE(info["running"].get<bool>());
}

// Test event publishing through API
TEST_F(RestApiServerTest, EventPublishingTest) {
    std::shared_ptr<Event> receivedEvent;
    
    // Subscribe to events
    eventBus->subscribe(EventType::CONFIG_CHANGED, [&receivedEvent](std::shared_ptr<Event> event) {
        receivedEvent = event;
    });
    
    // Start server
    apiServer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // The actual HTTP testing would require more complex setup
    // For now, test the event publishing mechanism directly
    auto event = std::make_shared<Event>(EventType::CONFIG_CHANGED, "api_test");
    event->setData("test_key", std::string("test_value"));
    
    eventBus->publish(event);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    ASSERT_NE(receivedEvent, nullptr);
    EXPECT_EQ(receivedEvent->getType(), EventType::CONFIG_CHANGED);
    EXPECT_EQ(receivedEvent->getSource(), "api_test");
}

// Test configuration integration
TEST_F(RestApiServerTest, ConfigurationIntegrationTest) {
    // Set some configuration values
    configManager->setValue("test_api_key", std::string("test_api_value"));
    configManager->setValue("test_int_key", 123);
    
    // Start server
    apiServer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Verify configuration is accessible
    EXPECT_TRUE(configManager->hasValue("test_api_key"));
    EXPECT_EQ(configManager->getValue<std::string>("test_api_key", ""), "test_api_value");
    EXPECT_EQ(configManager->getValue<int>("test_int_key", 0), 123);
}

// Test state machine integration
TEST_F(RestApiServerTest, StateMachineIntegrationTest) {
    // Start server
    apiServer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Test state transitions
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::INITIALIZING);
    
    stateMachine->transitionTo(SystemState::READY);
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::READY);
    
    stateMachine->transitionTo(SystemState::CONNECTED);
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::CONNECTED);
}

// Test concurrent access
TEST_F(RestApiServerTest, ConcurrentAccessTest) {
    apiServer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::atomic<int> successCount{0};
    std::vector<std::thread> threads;
    
    // Create multiple threads accessing configuration
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([this, &successCount, i]() {
            try {
                configManager->setValue("thread_key_" + std::to_string(i), i);
                auto value = configManager->getValue<int>("thread_key_" + std::to_string(i), -1);
                if (value == i) {
                    successCount++;
                }
            } catch (...) {
                // Handle any exceptions
            }
        });
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(successCount.load(), 10);
}

// Test error handling
TEST_F(RestApiServerTest, ErrorHandlingTest) {
    // Test starting on invalid port
    auto invalidApiServer = std::make_unique<RestApiServer>(eventBus, configManager, stateMachine, "127.0.0.1", -1);
    
    // Should handle gracefully (implementation dependent)
    EXPECT_NO_THROW(invalidApiServer->start());
    
    invalidApiServer.reset();
}

// Test server restart
TEST_F(RestApiServerTest, RestartTest) {
    // Start server
    EXPECT_NO_THROW(apiServer->start());
    EXPECT_TRUE(apiServer->isRunning());
    
    // Stop server
    EXPECT_NO_THROW(apiServer->stop());
    EXPECT_FALSE(apiServer->isRunning());
    
    // Start again
    EXPECT_NO_THROW(apiServer->start());
    EXPECT_TRUE(apiServer->isRunning());
}

// Test event subscription through API
TEST_F(RestApiServerTest, EventSubscriptionTest) {
    std::atomic<int> eventCount{0};
    
    // Subscribe to state changes
    eventBus->subscribe(EventType::STATE_CHANGED, [&eventCount](std::shared_ptr<Event> event) {
        eventCount++;
    });
    
    apiServer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Trigger state changes
    stateMachine->transitionTo(SystemState::READY);
    stateMachine->transitionTo(SystemState::CONNECTED);
    stateMachine->transitionTo(SystemState::READY);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_EQ(eventCount.load(), 3);
}

// Test API server metrics
TEST_F(RestApiServerTest, MetricsTest) {
    apiServer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // The server should track basic metrics
    auto info = apiServer->getServerInfo();
    EXPECT_TRUE(info.contains("uptime"));
    EXPECT_TRUE(info.contains("requests_count"));
    
    // After starting, uptime should be > 0
    EXPECT_GT(info["uptime"].get<int>(), 0);
}
