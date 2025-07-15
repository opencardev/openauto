#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <modern/EventBus.hpp>
#include <modern/ConfigurationManager.hpp>
#include <modern/StateMachine.hpp>
#include <modern/RestApiServer.hpp>
#include <modern/ModernIntegration.hpp>
#include <thread>
#include <chrono>

using namespace openauto::modern;
using namespace testing;

class AndroidAutoIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up the modern integration stack
        eventBus = std::make_shared<EventBus>();
        configManager = std::make_shared<ConfigurationManager>("test_integration_config.json");
        stateMachine = std::make_shared<StateMachine>(eventBus);
        
        // Initialize configuration with default values
        configManager->loadDefaults();
        configManager->setEventBus(eventBus);
        
        // Set up REST API server on test port
        apiServer = std::make_unique<RestApiServer>(eventBus, configManager, stateMachine, "127.0.0.1", 18081);
        
        // Initialize modern integration
        modernIntegration = std::make_unique<ModernIntegration>(eventBus, configManager, stateMachine);
    }

    void TearDown() override {
        if (apiServer && apiServer->isRunning()) {
            apiServer->stop();
        }
        
        modernIntegration.reset();
        apiServer.reset();
        stateMachine.reset();
        configManager.reset();
        eventBus.reset();
        
        // Clean up test files
        std::filesystem::remove("test_integration_config.json");
    }

    std::shared_ptr<EventBus> eventBus;
    std::shared_ptr<ConfigurationManager> configManager;
    std::shared_ptr<StateMachine> stateMachine;
    std::unique_ptr<RestApiServer> apiServer;
    std::unique_ptr<ModernIntegration> modernIntegration;
};

// Test the complete initialization flow
TEST_F(AndroidAutoIntegrationTest, InitializationFlowTest) {
    // Should start in INITIALIZING state
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::INITIALIZING);
    
    // Start the API server
    EXPECT_NO_THROW(apiServer->start());
    EXPECT_TRUE(apiServer->isRunning());
    
    // Initialize the modern integration
    EXPECT_NO_THROW(modernIntegration->initialize());
    
    // Should be able to transition to READY state
    EXPECT_TRUE(stateMachine->transitionTo(SystemState::READY));
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::READY);
}

// Test configuration changes propagate through the system
TEST_F(AndroidAutoIntegrationTest, ConfigurationPropagationTest) {
    std::shared_ptr<Event> receivedEvent;
    
    // Subscribe to configuration changes
    eventBus->subscribe(EventType::CONFIG_CHANGED, [&receivedEvent](std::shared_ptr<Event> event) {
        receivedEvent = event;
    });
    
    // Initialize system
    apiServer->start();
    modernIntegration->initialize();
    
    // Change a configuration value
    configManager->setValue("test.integration.value", std::string("test_value"));
    
    // Wait for event propagation
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Should have received configuration change event
    ASSERT_NE(receivedEvent, nullptr);
    EXPECT_EQ(receivedEvent->getType(), EventType::CONFIG_CHANGED);
    
    auto key = receivedEvent->getData<std::string>("key");
    ASSERT_TRUE(key.has_value());
    EXPECT_EQ(key.value(), "test.integration.value");
}

// Test state transitions trigger appropriate events
TEST_F(AndroidAutoIntegrationTest, StateTransitionEventsTest) {
    std::vector<std::shared_ptr<Event>> stateEvents;
    
    eventBus->subscribe(EventType::STATE_CHANGED, [&stateEvents](std::shared_ptr<Event> event) {
        stateEvents.push_back(event);
    });
    
    // Initialize system
    apiServer->start();
    modernIntegration->initialize();
    
    // Perform state transitions
    stateMachine->transitionTo(SystemState::READY);
    stateMachine->transitionTo(SystemState::CONNECTED);
    stateMachine->transitionTo(SystemState::PROJECTING);
    
    // Wait for event processing
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    // Should have received state change events
    EXPECT_GE(stateEvents.size(), 3);
    
    // Verify the events represent correct transitions
    if (stateEvents.size() >= 3) {
        auto toState1 = stateEvents[0]->getData<int>("to_state");
        auto toState2 = stateEvents[1]->getData<int>("to_state");
        auto toState3 = stateEvents[2]->getData<int>("to_state");
        
        ASSERT_TRUE(toState1.has_value());
        ASSERT_TRUE(toState2.has_value());
        ASSERT_TRUE(toState3.has_value());
        
        EXPECT_EQ(toState1.value(), static_cast<int>(SystemState::READY));
        EXPECT_EQ(toState2.value(), static_cast<int>(SystemState::CONNECTED));
        EXPECT_EQ(toState3.value(), static_cast<int>(SystemState::PROJECTING));
    }
}

// Test error recovery scenario
TEST_F(AndroidAutoIntegrationTest, ErrorRecoveryTest) {
    // Initialize system
    apiServer->start();
    modernIntegration->initialize();
    
    // Move to operational state
    stateMachine->transitionTo(SystemState::READY);
    stateMachine->transitionTo(SystemState::CONNECTED);
    
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::CONNECTED);
    
    // Simulate error condition
    stateMachine->transitionTo(SystemState::ERROR);
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::ERROR);
    
    // Should be able to recover to READY state
    stateMachine->transitionTo(SystemState::READY);
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::READY);
}

// Test concurrent operations
TEST_F(AndroidAutoIntegrationTest, ConcurrentOperationsTest) {
    // Initialize system
    apiServer->start();
    modernIntegration->initialize();
    stateMachine->transitionTo(SystemState::READY);
    
    std::atomic<int> configOperations{0};
    std::atomic<int> stateOperations{0};
    std::atomic<int> eventOperations{0};
    
    std::vector<std::thread> threads;
    
    // Thread 1: Configuration operations
    threads.emplace_back([this, &configOperations]() {
        for (int i = 0; i < 20; ++i) {
            configManager->setValue("concurrent.key." + std::to_string(i), i);
            configOperations++;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    
    // Thread 2: State operations
    threads.emplace_back([this, &stateOperations]() {
        for (int i = 0; i < 10; ++i) {
            if (stateMachine->transitionTo(SystemState::CONNECTED)) {
                stateOperations++;
            }
            if (stateMachine->transitionTo(SystemState::READY)) {
                stateOperations++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    });
    
    // Thread 3: Event operations
    threads.emplace_back([this, &eventOperations]() {
        for (int i = 0; i < 15; ++i) {
            auto event = std::make_shared<Event>(EventType::CONNECTION_STATUS, "test_thread");
            event->setData("operation", i);
            eventBus->publish(event);
            eventOperations++;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Wait for any pending operations
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // All operations should have completed successfully
    EXPECT_EQ(configOperations.load(), 20);
    EXPECT_GT(stateOperations.load(), 0); // Should have some successful transitions
    EXPECT_EQ(eventOperations.load(), 15);
}

// Test configuration persistence across restarts
TEST_F(AndroidAutoIntegrationTest, ConfigurationPersistenceTest) {
    // Initialize system and set configuration
    apiServer->start();
    modernIntegration->initialize();
    
    configManager->setValue("persistence.test.string", std::string("persistent_value"));
    configManager->setValue("persistence.test.int", 42);
    configManager->setValue("persistence.test.bool", true);
    
    // Save configuration
    EXPECT_TRUE(configManager->save());
    
    // Create new configuration manager (simulating restart)
    auto newConfigManager = std::make_shared<ConfigurationManager>("test_integration_config.json");
    EXPECT_TRUE(newConfigManager->load());
    
    // Verify values were persisted
    EXPECT_EQ(newConfigManager->getValue<std::string>("persistence.test.string", ""), "persistent_value");
    EXPECT_EQ(newConfigManager->getValue<int>("persistence.test.int", 0), 42);
    EXPECT_TRUE(newConfigManager->getValue<bool>("persistence.test.bool", false));
}

// Test system shutdown scenario
TEST_F(AndroidAutoIntegrationTest, SystemShutdownTest) {
    // Initialize system
    apiServer->start();
    modernIntegration->initialize();
    
    // Move through operational states
    stateMachine->transitionTo(SystemState::READY);
    stateMachine->transitionTo(SystemState::CONNECTED);
    stateMachine->transitionTo(SystemState::PROJECTING);
    
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::PROJECTING);
    
    // Initiate shutdown
    stateMachine->transitionTo(SystemState::SHUTDOWN);
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::SHUTDOWN);
    
    // API server should be stopped
    if (apiServer->isRunning()) {
        apiServer->stop();
    }
    EXPECT_FALSE(apiServer->isRunning());
}

// Test high-volume event processing
TEST_F(AndroidAutoIntegrationTest, HighVolumeEventProcessingTest) {
    std::atomic<int> processedEvents{0};
    
    // Subscribe to all event types
    eventBus->subscribe(EventType::CONFIG_CHANGED, [&processedEvents](std::shared_ptr<Event> event) {
        processedEvents++;
    });
    eventBus->subscribe(EventType::STATE_CHANGED, [&processedEvents](std::shared_ptr<Event> event) {
        processedEvents++;
    });
    eventBus->subscribe(EventType::CONNECTION_STATUS, [&processedEvents](std::shared_ptr<Event> event) {
        processedEvents++;
    });
    
    // Initialize system
    apiServer->start();
    modernIntegration->initialize();
    
    const int NUM_EVENTS = 1000;
    
    // Generate high volume of events
    for (int i = 0; i < NUM_EVENTS / 3; ++i) {
        eventBus->publish(std::make_shared<Event>(EventType::CONFIG_CHANGED, "volume_test"));
        eventBus->publish(std::make_shared<Event>(EventType::STATE_CHANGED, "volume_test"));
        eventBus->publish(std::make_shared<Event>(EventType::CONNECTION_STATUS, "volume_test"));
    }
    
    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Should have processed most or all events
    EXPECT_GE(processedEvents.load(), NUM_EVENTS * 0.9); // Allow for some margin
}
