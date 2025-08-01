#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <modern/ConfigurationManager.hpp>
#include <modern/EventBus.hpp>

using namespace openauto::modern;
using namespace testing;

class ConfigurationManagerTest : public ::testing::Test {
  protected:
    void SetUp() override {
        testConfigPath = "test_config.json";

        // Clean up any existing test files
        std::filesystem::remove(testConfigPath);

        configManager = std::make_unique<ConfigurationManager>(testConfigPath);
        eventBus = std::make_shared<EventBus>();
        configManager->setEventBus(eventBus);
    }

    void TearDown() override {
        configManager.reset();
        eventBus.reset();

        // Clean up test files
        std::filesystem::remove(testConfigPath);
    }

    std::unique_ptr<ConfigurationManager> configManager;
    std::shared_ptr<EventBus> eventBus;
    std::string testConfigPath;
};

// Test basic value setting and getting
TEST_F(ConfigurationManagerTest, BasicValueOperations) {
    // Test string value
    configManager->setValue("test_string", std::string("hello world"));
    auto stringVal = configManager->getValue<std::string>("test_string", "default");
    EXPECT_EQ(stringVal, "hello world");

    // Test integer value
    configManager->setValue("test_int", 42);
    auto intVal = configManager->getValue<int>("test_int", 0);
    EXPECT_EQ(intVal, 42);

    // Test double value
    configManager->setValue("test_double", 3.14);
    auto doubleVal = configManager->getValue<double>("test_double", 0.0);
    EXPECT_DOUBLE_EQ(doubleVal, 3.14);

    // Test boolean value
    configManager->setValue("test_bool", true);
    auto boolVal = configManager->getValue<bool>("test_bool", false);
    EXPECT_TRUE(boolVal);
}

// Test default values
TEST_F(ConfigurationManagerTest, DefaultValues) {
    // Test getting non-existent key returns default
    auto stringVal = configManager->getValue<std::string>("nonexistent", "default_value");
    EXPECT_EQ(stringVal, "default_value");

    auto intVal = configManager->getValue<int>("nonexistent", 999);
    EXPECT_EQ(intVal, 999);

    auto boolVal = configManager->getValue<bool>("nonexistent", false);
    EXPECT_FALSE(boolVal);
}

// Test hasValue functionality
TEST_F(ConfigurationManagerTest, HasValueTest) {
    EXPECT_FALSE(configManager->hasValue("nonexistent"));

    configManager->setValue("existing_key", std::string("value"));
    EXPECT_TRUE(configManager->hasValue("existing_key"));
}

// Test removeValue functionality
TEST_F(ConfigurationManagerTest, RemoveValueTest) {
    configManager->setValue("to_remove", std::string("value"));
    EXPECT_TRUE(configManager->hasValue("to_remove"));

    configManager->removeValue("to_remove");
    EXPECT_FALSE(configManager->hasValue("to_remove"));
}

// Test save and load functionality
TEST_F(ConfigurationManagerTest, SaveLoadTest) {
    // Set some values
    configManager->setValue("string_key", std::string("test_value"));
    configManager->setValue("int_key", 123);
    configManager->setValue("double_key", 2.71);
    configManager->setValue("bool_key", true);

    // Save to file
    EXPECT_TRUE(configManager->save());
    EXPECT_TRUE(std::filesystem::exists(testConfigPath));

    // Create new configuration manager and load
    auto newConfigManager = std::make_unique<ConfigurationManager>(testConfigPath);
    EXPECT_TRUE(newConfigManager->load());

    // Verify values were loaded correctly
    EXPECT_EQ(newConfigManager->getValue<std::string>("string_key", ""), "test_value");
    EXPECT_EQ(newConfigManager->getValue<int>("int_key", 0), 123);
    EXPECT_DOUBLE_EQ(newConfigManager->getValue<double>("double_key", 0.0), 2.71);
    EXPECT_TRUE(newConfigManager->getValue<bool>("bool_key", false));
}

// Test JSON serialization
TEST_F(ConfigurationManagerTest, JsonSerializationTest) {
    // Set some values
    configManager->setValue("key1", std::string("value1"));
    configManager->setValue("key2", 42);
    configManager->setValue("key3", true);

    // Get as JSON
    auto json = configManager->toJson();

    // Verify JSON structure
    EXPECT_TRUE(json.contains("key1"));
    EXPECT_TRUE(json.contains("key2"));
    EXPECT_TRUE(json.contains("key3"));

    EXPECT_EQ(json["key1"], "value1");
    EXPECT_EQ(json["key2"], 42);
    EXPECT_EQ(json["key3"], true);

    // Test fromJson
    nlohmann::json newJson;
    newJson["new_key1"] = "new_value1";
    newJson["new_key2"] = 999;

    configManager->fromJson(newJson);

    EXPECT_EQ(configManager->getValue<std::string>("new_key1", ""), "new_value1");
    EXPECT_EQ(configManager->getValue<int>("new_key2", 0), 999);
}

// Test bulk operations
TEST_F(ConfigurationManagerTest, BulkOperationsTest) {
    std::unordered_map<std::string, ConfigValue> values = {
        {"bulk_key1", std::string("bulk_value1")},
        {"bulk_key2", 100},
        {"bulk_key3", false},
        {"bulk_key4", 9.99}};

    configManager->setValues(values);

    // Verify all values were set
    EXPECT_EQ(configManager->getValue<std::string>("bulk_key1", ""), "bulk_value1");
    EXPECT_EQ(configManager->getValue<int>("bulk_key2", 0), 100);
    EXPECT_FALSE(configManager->getValue<bool>("bulk_key3", true));
    EXPECT_DOUBLE_EQ(configManager->getValue<double>("bulk_key4", 0.0), 9.99);

    // Test getAllValues
    auto allValues = configManager->getAllValues();
    EXPECT_GE(allValues.size(), 4);
    EXPECT_TRUE(allValues.find("bulk_key1") != allValues.end());
    EXPECT_TRUE(allValues.find("bulk_key2") != allValues.end());
    EXPECT_TRUE(allValues.find("bulk_key3") != allValues.end());
    EXPECT_TRUE(allValues.find("bulk_key4") != allValues.end());
}

// Test reset functionality
TEST_F(ConfigurationManagerTest, ResetTest) {
    // Set some custom values
    configManager->setValue("custom_key", std::string("custom_value"));
    EXPECT_TRUE(configManager->hasValue("custom_key"));

    // Reset should restore defaults and remove custom values
    configManager->reset();

    // Custom key should be gone
    EXPECT_FALSE(configManager->hasValue("custom_key"));

    // Default values should be present
    EXPECT_TRUE(configManager->hasValue("audio.volume"));
    EXPECT_TRUE(configManager->hasValue("video.brightness"));
    EXPECT_TRUE(configManager->hasValue("system.language"));
}

// Test default configuration
TEST_F(ConfigurationManagerTest, DefaultConfigurationTest) {
    configManager->loadDefaults();

    // Verify default values are set
    EXPECT_EQ(configManager->getValue<int>("audio.volume", -1), 50);
    EXPECT_FALSE(configManager->getValue<bool>("audio.muted", true));
    EXPECT_EQ(configManager->getValue<int>("video.brightness", -1), 75);
    EXPECT_TRUE(configManager->getValue<bool>("video.day_mode", false));
    EXPECT_EQ(configManager->getValue<std::string>("system.language", ""), "en_US");
    EXPECT_TRUE(configManager->getValue<bool>("network.wifi_enabled", false));
    EXPECT_EQ(configManager->getValue<int>("api.port", -1), 8080);
}

// Test validation
TEST_F(ConfigurationManagerTest, ValidationTest) {
    // Set required values
    configManager->setValue("audio.volume", 75);
    configManager->setValue("video.brightness", 80);
    configManager->setValue("system.language", std::string("en_US"));

    // Should be valid
    EXPECT_TRUE(configManager->isValid());

    auto errors = configManager->validate();
    EXPECT_TRUE(errors.empty());

    // Remove a required value
    configManager->removeValue("audio.volume");

    // Should not be valid
    EXPECT_FALSE(configManager->isValid());

    errors = configManager->validate();
    EXPECT_FALSE(errors.empty());
    EXPECT_THAT(errors, Contains(HasSubstr("audio.volume")));
}

// Test event notifications
TEST_F(ConfigurationManagerTest, EventNotificationTest) {
    std::shared_ptr<Event> lastEvent;

    eventBus->subscribe(EventType::CONFIG_CHANGED,
                        [&lastEvent](std::shared_ptr<Event> event) { lastEvent = event; });

    // Setting a value should trigger an event
    configManager->setValue("test_key", std::string("test_value"));

    // Wait for event processing
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_NE(lastEvent, nullptr);
    EXPECT_EQ(lastEvent->getType(), EventType::CONFIG_CHANGED);
    EXPECT_EQ(lastEvent->getSource(), "config_manager");

    auto key = lastEvent->getData<std::string>("key");
    auto action = lastEvent->getData<std::string>("action");

    ASSERT_TRUE(key.has_value());
    ASSERT_TRUE(action.has_value());
    EXPECT_EQ(key.value(), "test_key");
    EXPECT_EQ(action.value(), "changed");
}

// Test type safety
TEST_F(ConfigurationManagerTest, TypeSafetyTest) {
    // Set an integer value
    configManager->setValue("type_test", 42);

    // Try to get as string - should return default
    auto stringVal = configManager->getValue<std::string>("type_test", "default");
    EXPECT_EQ(stringVal, "default");

    // Get as correct type should work
    auto intVal = configManager->getValue<int>("type_test", 0);
    EXPECT_EQ(intVal, 42);
}

// Test configuration path management
TEST_F(ConfigurationManagerTest, ConfigPathTest) {
    EXPECT_EQ(configManager->getConfigPath(), testConfigPath);

    std::string newPath = "new_test_config.json";
    configManager->setConfigPath(newPath);
    EXPECT_EQ(configManager->getConfigPath(), newPath);

    // Clean up
    std::filesystem::remove(newPath);
}
