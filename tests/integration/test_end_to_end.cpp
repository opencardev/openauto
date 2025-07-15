#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <modern/EventBus.hpp>
#include <modern/ConfigurationManager.hpp>
#include <modern/StateMachine.hpp>
#include <modern/RestApiServer.hpp>
#include <modern/ModernIntegration.hpp>
#include <thread>
#include <chrono>
#include <atomic>

using namespace openauto::modern;
using namespace testing;

// End-to-end tests that simulate complete user scenarios
class EndToEndTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up complete system stack
        eventBus = std::make_shared<EventBus>();
        configManager = std::make_shared<ConfigurationManager>("test_e2e_config.json");
        stateMachine = std::make_shared<StateMachine>(eventBus);
        
        configManager->loadDefaults();
        configManager->setEventBus(eventBus);
        
        apiServer = std::make_unique<RestApiServer>(eventBus, configManager, stateMachine, "127.0.0.1", 18083);
        modernIntegration = std::make_unique<ModernIntegration>(eventBus, configManager, stateMachine);
        
        // Initialize system
        modernIntegration->initialize();
        apiServer->start();
        
        // Wait for initialization
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
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
        
        // Clean up
        std::filesystem::remove("test_e2e_config.json");
    }

    std::shared_ptr<EventBus> eventBus;
    std::shared_ptr<ConfigurationManager> configManager;
    std::shared_ptr<StateMachine> stateMachine;
    std::unique_ptr<RestApiServer> apiServer;
    std::unique_ptr<ModernIntegration> modernIntegration;
};

// Test complete system startup scenario
TEST_F(EndToEndTest, SystemStartupScenarioTest) {
    // Verify initial state
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::INITIALIZING);
    EXPECT_TRUE(apiServer->isRunning());
    
    // Simulate system startup sequence
    EXPECT_TRUE(stateMachine->transitionTo(SystemState::READY));
    
    // Verify default configuration is loaded
    EXPECT_TRUE(configManager->hasValue("audio.volume"));
    EXPECT_TRUE(configManager->hasValue("video.brightness"));
    EXPECT_TRUE(configManager->hasValue("system.language"));
    
    // Verify API is functional
    auto serverInfo = apiServer->getServerInfo();
    EXPECT_TRUE(serverInfo["running"].get<bool>());
}

// Test Android Auto connection simulation
TEST_F(EndToEndTest, AndroidAutoConnectionScenarioTest) {
    // Start in ready state
    stateMachine->transitionTo(SystemState::READY);
    
    // Simulate Android Auto device connection
    auto connectionEvent = std::make_shared<Event>(EventType::CONNECTION_STATUS, "usb_manager");
    connectionEvent->setData("status", std::string("connected"));
    connectionEvent->setData("device_type", std::string("android_auto"));
    eventBus->publish(connectionEvent);
    
    // Wait for event processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Should be able to transition to connected state
    EXPECT_TRUE(stateMachine->transitionTo(SystemState::CONNECTED));
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::CONNECTED);
    
    // Simulate successful handshake
    auto handshakeEvent = std::make_shared<Event>(EventType::CONNECTION_STATUS, "android_auto_entity");
    handshakeEvent->setData("status", std::string("handshake_complete"));
    eventBus->publish(handshakeEvent);
    
    // Should be able to start projection
    EXPECT_TRUE(stateMachine->transitionTo(SystemState::PROJECTING));
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::PROJECTING);
}

// Test configuration change during operation
TEST_F(EndToEndTest, RuntimeConfigurationChangeScenarioTest) {
    std::atomic<int> configChangeEvents{0};
    
    // Subscribe to configuration changes
    eventBus->subscribe(EventType::CONFIG_CHANGED, [&configChangeEvents](std::shared_ptr<Event> event) {
        configChangeEvents++;
    });
    
    // Move to operational state
    stateMachine->transitionTo(SystemState::READY);
    stateMachine->transitionTo(SystemState::CONNECTED);
    
    // Change various settings during operation
    configManager->setValue("audio.volume", 80);
    configManager->setValue("video.brightness", 90);
    configManager->setValue("video.day_mode", false);
    
    // Wait for events
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    // Should have received configuration change events
    EXPECT_GE(configChangeEvents.load(), 3);
    
    // Verify changes persisted
    EXPECT_EQ(configManager->getValue<int>("audio.volume", 0), 80);
    EXPECT_EQ(configManager->getValue<int>("video.brightness", 0), 90);
    EXPECT_FALSE(configManager->getValue<bool>("video.day_mode", true));
    
    // Save and verify persistence
    EXPECT_TRUE(configManager->save());
}

// Test error recovery scenario
TEST_F(EndToEndTest, ErrorRecoveryScenarioTest) {
    std::vector<std::shared_ptr<Event>> stateEvents;
    
    eventBus->subscribe(EventType::STATE_CHANGED, [&stateEvents](std::shared_ptr<Event> event) {
        stateEvents.push_back(event);
    });
    
    // Move to operational state
    stateMachine->transitionTo(SystemState::READY);
    stateMachine->transitionTo(SystemState::CONNECTED);
    stateMachine->transitionTo(SystemState::PROJECTING);
    
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::PROJECTING);
    
    // Simulate error condition
    auto errorEvent = std::make_shared<Event>(EventType::CONNECTION_STATUS, "android_auto_entity");
    errorEvent->setData("status", std::string("connection_lost"));
    errorEvent->setData("error", std::string("network_timeout"));
    eventBus->publish(errorEvent);
    
    // Transition to error state
    stateMachine->transitionTo(SystemState::ERROR);
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::ERROR);
    
    // Simulate recovery
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Should be able to recover to ready state
    stateMachine->transitionTo(SystemState::READY);
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::READY);
    
    // Wait for events
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Should have recorded all state transitions
    EXPECT_GE(stateEvents.size(), 5); // READY, CONNECTED, PROJECTING, ERROR, READY
}

// Test high-load scenario
TEST_F(EndToEndTest, HighLoadScenarioTest) {
    std::atomic<int> totalEventsProcessed{0};
    
    // Subscribe to all event types
    eventBus->subscribe(EventType::CONFIG_CHANGED, [&totalEventsProcessed](std::shared_ptr<Event> event) {
        totalEventsProcessed++;
    });
    eventBus->subscribe(EventType::STATE_CHANGED, [&totalEventsProcessed](std::shared_ptr<Event> event) {
        totalEventsProcessed++;
    });
    eventBus->subscribe(EventType::CONNECTION_STATUS, [&totalEventsProcessed](std::shared_ptr<Event> event) {
        totalEventsProcessed++;
    });
    
    // Move to operational state
    stateMachine->transitionTo(SystemState::READY);
    stateMachine->transitionTo(SystemState::CONNECTED);
    
    // Generate high load of events and configuration changes
    std::vector<std::thread> loadThreads;
    
    // Configuration change thread
    loadThreads.emplace_back([this]() {
        for (int i = 0; i < 100; ++i) {
            configManager->setValue("load.test.config." + std::to_string(i), i);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    
    // Event generation thread
    loadThreads.emplace_back([this]() {
        for (int i = 0; i < 100; ++i) {
            auto event = std::make_shared<Event>(EventType::CONNECTION_STATUS, "load_test");
            event->setData("sequence", i);
            eventBus->publish(event);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    
    // State transition thread
    loadThreads.emplace_back([this]() {
        for (int i = 0; i < 20; ++i) {
            stateMachine->transitionTo(SystemState::PROJECTING);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            stateMachine->transitionTo(SystemState::CONNECTED);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });
    
    // Wait for all load threads
    for (auto& thread : loadThreads) {
        thread.join();
    }
    
    // Wait for event processing
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // System should have processed most events
    EXPECT_GE(totalEventsProcessed.load(), 200); // 100 config + 100 connection + state changes
    
    // System should still be functional
    EXPECT_TRUE(apiServer->isRunning());
    EXPECT_TRUE(configManager->isValid());
}

// Test graceful shutdown scenario
TEST_F(EndToEndTest, GracefulShutdownScenarioTest) {
    // Move to operational state
    stateMachine->transitionTo(SystemState::READY);
    stateMachine->transitionTo(SystemState::CONNECTED);
    stateMachine->transitionTo(SystemState::PROJECTING);
    
    // Set some configuration that should be saved
    configManager->setValue("shutdown.test.value", std::string("before_shutdown"));
    configManager->setValue("shutdown.test.number", 42);
    
    // Save configuration
    EXPECT_TRUE(configManager->save());
    
    // Initiate graceful shutdown
    stateMachine->transitionTo(SystemState::SHUTDOWN);
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::SHUTDOWN);
    
    // Stop API server
    apiServer->stop();
    EXPECT_FALSE(apiServer->isRunning());
    
    // Verify configuration was saved and can be reloaded
    auto shutdownConfigManager = std::make_unique<ConfigurationManager>("test_e2e_config.json");
    EXPECT_TRUE(shutdownConfigManager->load());
    
    EXPECT_EQ(shutdownConfigManager->getValue<std::string>("shutdown.test.value", ""), "before_shutdown");
    EXPECT_EQ(shutdownConfigManager->getValue<int>("shutdown.test.number", 0), 42);
}

// Test system restart scenario
TEST_F(EndToEndTest, SystemRestartScenarioTest) {
    // Set initial configuration
    configManager->setValue("restart.test.persist", std::string("persistent_value"));
    configManager->setValue("restart.test.number", 999);
    configManager->save();
    
    // Move to operational state
    stateMachine->transitionTo(SystemState::READY);
    stateMachine->transitionTo(SystemState::CONNECTED);
    
    // Stop API server (simulating shutdown)
    apiServer->stop();
    
    // Reset state machine (simulating restart)
    stateMachine->reset();
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::INITIALIZING);
    
    // Create new configuration manager (simulating restart)
    auto newConfigManager = std::make_shared<ConfigurationManager>("test_e2e_config.json");
    newConfigManager->setEventBus(eventBus);
    EXPECT_TRUE(newConfigManager->load());
    
    // Verify persistent values survived restart
    EXPECT_EQ(newConfigManager->getValue<std::string>("restart.test.persist", ""), "persistent_value");
    EXPECT_EQ(newConfigManager->getValue<int>("restart.test.number", 0), 999);
    
    // Start new API server
    auto newApiServer = std::make_unique<RestApiServer>(eventBus, newConfigManager, stateMachine, "127.0.0.1", 18084);
    newApiServer->start();
    
    EXPECT_TRUE(newApiServer->isRunning());
    
    // Should be able to return to operational state
    stateMachine->transitionTo(SystemState::READY);
    EXPECT_EQ(stateMachine->getCurrentState(), SystemState::READY);
    
    // Clean up
    newApiServer->stop();
}
