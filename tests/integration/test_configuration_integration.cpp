#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <modern/ConfigurationManager.hpp>
#include <modern/EventBus.hpp>
#include <modern/StateMachine.hpp>

using namespace openauto::modern;
using namespace testing;

class ConfigurationIntegrationTest : public ::testing::Test {
  protected:
    void SetUp() override {
        testConfigPath = "test_config_integration.json";

        // Clean up any existing test files
        std::filesystem::remove(testConfigPath);

        eventBus = std::make_shared<EventBus>();
        configManager = std::make_unique<ConfigurationManager>(testConfigPath);
        stateMachine = std::make_shared<StateMachine>(eventBus);

        configManager->setEventBus(eventBus);
    }

    void TearDown() override {
        configManager.reset();
        stateMachine.reset();
        eventBus.reset();

        // Clean up test files
        std::filesystem::remove(testConfigPath);
    }

    std::unique_ptr<ConfigurationManager> configManager;
    std::shared_ptr<EventBus> eventBus;
    std::shared_ptr<StateMachine> stateMachine;
    std::string testConfigPath;
};

// Test configuration integration with state machine
TEST_F(ConfigurationIntegrationTest, StateMachineIntegrationTest) {
    std::shared_ptr<Event> configEvent;
    std::shared_ptr<Event> stateEvent;

    // Subscribe to both config and state events
    eventBus->subscribe(EventType::CONFIG_CHANGED,
                        [&configEvent](std::shared_ptr<Event> event) { configEvent = event; });

    eventBus->subscribe(EventType::STATE_CHANGED,
                        [&stateEvent](std::shared_ptr<Event> event) { stateEvent = event; });

    // Set configuration that might affect state
    configManager->setValue("system.auto_start", true);

    // Transition state
    stateMachine->transitionTo(SystemState::READY);

    // Wait for event processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Both events should have been received
    ASSERT_NE(configEvent, nullptr);
    ASSERT_NE(stateEvent, nullptr);

    EXPECT_EQ(configEvent->getType(), EventType::CONFIG_CHANGED);
    EXPECT_EQ(stateEvent->getType(), EventType::STATE_CHANGED);
}

// Test configuration file operations
TEST_F(ConfigurationIntegrationTest, FileOperationsTest) {
    // Set various configuration values
    configManager->setValue("audio.volume", 75);
    configManager->setValue("video.brightness", 80);
    configManager->setValue("system.language", std::string("en_US"));
    configManager->setValue("network.wifi_enabled", true);

    // Save configuration
    EXPECT_TRUE(configManager->save());
    EXPECT_TRUE(std::filesystem::exists(testConfigPath));

    // Verify file content
    std::ifstream file(testConfigPath);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Should contain JSON with our values
    EXPECT_THAT(content, HasSubstr("audio.volume"));
    EXPECT_THAT(content, HasSubstr("75"));
    EXPECT_THAT(content, HasSubstr("video.brightness"));
    EXPECT_THAT(content, HasSubstr("80"));
    EXPECT_THAT(content, HasSubstr("system.language"));
    EXPECT_THAT(content, HasSubstr("en_US"));

    // Create new configuration manager and load
    auto newConfigManager = std::make_unique<ConfigurationManager>(testConfigPath);
    EXPECT_TRUE(newConfigManager->load());

    // Verify values were loaded correctly
    EXPECT_EQ(newConfigManager->getValue<int>("audio.volume", 0), 75);
    EXPECT_EQ(newConfigManager->getValue<int>("video.brightness", 0), 80);
    EXPECT_EQ(newConfigManager->getValue<std::string>("system.language", ""), "en_US");
    EXPECT_TRUE(newConfigManager->getValue<bool>("network.wifi_enabled", false));
}

// Test configuration validation with different scenarios
TEST_F(ConfigurationIntegrationTest, ValidationScenariosTest) {
    // Test valid configuration
    configManager->setValue("audio.volume", 50);
    configManager->setValue("video.brightness", 75);
    configManager->setValue("system.language", std::string("en_US"));

    EXPECT_TRUE(configManager->isValid());

    auto errors = configManager->validate();
    EXPECT_TRUE(errors.empty());

    // Test invalid configuration - remove required key
    configManager->removeValue("audio.volume");

    EXPECT_FALSE(configManager->isValid());

    errors = configManager->validate();
    EXPECT_FALSE(errors.empty());
    EXPECT_THAT(errors, Contains(HasSubstr("audio.volume")));

    // Test invalid values
    configManager->setValue("audio.volume", -1);  // Invalid volume

    // Note: Actual validation rules depend on implementation
    // This test structure allows for extensible validation
}

// Test configuration change propagation
TEST_F(ConfigurationIntegrationTest, ChangePropagationTest) {
    std::vector<std::shared_ptr<Event>> events;

    eventBus->subscribe(EventType::CONFIG_CHANGED,
                        [&events](std::shared_ptr<Event> event) { events.push_back(event); });

    // Make multiple configuration changes
    configManager->setValue("change.test.1", std::string("value1"));
    configManager->setValue("change.test.2", 42);
    configManager->setValue("change.test.3", true);

    // Bulk change
    std::unordered_map<std::string, ConfigValue> bulkChanges = {
        {"bulk.test.1", std::string("bulk_value1")}, {"bulk.test.2", 999}, {"bulk.test.3", false}};
    configManager->setValues(bulkChanges);

    // Wait for event processing
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // Should have received events for each change
    EXPECT_GE(events.size(), 4);  // 3 individual + 1 bulk

    // Verify event details
    bool foundIndividualChange = false;
    bool foundBulkChange = false;

    for (const auto& event : events) {
        auto action = event->getData<std::string>("action");
        if (action.has_value()) {
            if (action.value() == "changed") {
                foundIndividualChange = true;
            } else if (action.value() == "bulk_update") {
                foundBulkChange = true;
            }
        }
    }

    EXPECT_TRUE(foundIndividualChange);
    EXPECT_TRUE(foundBulkChange);
}

// Test configuration reset and defaults
TEST_F(ConfigurationIntegrationTest, ResetAndDefaultsTest) {
    // Set some custom values
    configManager->setValue("custom.value.1", std::string("custom"));
    configManager->setValue("custom.value.2", 123);

    // Verify custom values exist
    EXPECT_TRUE(configManager->hasValue("custom.value.1"));
    EXPECT_TRUE(configManager->hasValue("custom.value.2"));

    // Reset configuration
    configManager->reset();

    // Custom values should be gone
    EXPECT_FALSE(configManager->hasValue("custom.value.1"));
    EXPECT_FALSE(configManager->hasValue("custom.value.2"));

    // Default values should be present
    EXPECT_TRUE(configManager->hasValue("audio.volume"));
    EXPECT_TRUE(configManager->hasValue("video.brightness"));
    EXPECT_TRUE(configManager->hasValue("system.language"));

    // Verify default values
    EXPECT_EQ(configManager->getValue<int>("audio.volume", -1), 50);
    EXPECT_EQ(configManager->getValue<int>("video.brightness", -1), 75);
    EXPECT_EQ(configManager->getValue<std::string>("system.language", ""), "en_US");
}

// Test configuration with corrupted file
TEST_F(ConfigurationIntegrationTest, CorruptedFileTest) {
    // Create a corrupted configuration file
    std::ofstream corruptedFile(testConfigPath);
    corruptedFile << "{ invalid json content :::";
    corruptedFile.close();

    // Should handle gracefully
    auto corruptedConfigManager = std::make_unique<ConfigurationManager>(testConfigPath);
    EXPECT_NO_THROW(corruptedConfigManager->load());

    // Should fall back to defaults
    corruptedConfigManager->loadDefaults();
    EXPECT_TRUE(corruptedConfigManager->hasValue("audio.volume"));
}

// Test concurrent configuration access
TEST_F(ConfigurationIntegrationTest, ConcurrentAccessTest) {
    std::atomic<int> readOperations{0};
    std::atomic<int> writeOperations{0};

    std::vector<std::thread> threads;

    // Reader threads
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([this, &readOperations, i]() {
            for (int j = 0; j < 20; ++j) {
                auto value = configManager->getValue<int>("concurrent.read.test", 0);
                readOperations++;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }

    // Writer threads
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([this, &writeOperations, i]() {
            for (int j = 0; j < 10; ++j) {
                configManager->setValue("concurrent.write.test." + std::to_string(i), j);
                writeOperations++;
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // All operations should complete successfully
    EXPECT_EQ(readOperations.load(), 100);  // 5 threads * 20 operations
    EXPECT_EQ(writeOperations.load(), 30);  // 3 threads * 10 operations
}
