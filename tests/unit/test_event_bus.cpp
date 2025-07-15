#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <modern/EventBus.hpp>
#include <modern/Event.hpp>
#include <thread>
#include <chrono>
#include <atomic>

using namespace openauto::modern;
using namespace testing;

class MockEventHandler {
public:
    MOCK_METHOD(void, onEvent, (std::shared_ptr<Event> event));
};

class EventBusTest : public ::testing::Test {
protected:
    void SetUp() override {
        eventBus = std::make_shared<EventBus>();
        mockHandler = std::make_shared<MockEventHandler>();
    }

    void TearDown() override {
        eventBus.reset();
        mockHandler.reset();
    }

    std::shared_ptr<EventBus> eventBus;
    std::shared_ptr<MockEventHandler> mockHandler;
};

// Test basic event subscription and publishing
TEST_F(EventBusTest, BasicSubscriptionAndPublishing) {
    // Subscribe to CONFIG_CHANGED events
    std::shared_ptr<Event> receivedEvent;
    eventBus->subscribe(EventType::CONFIG_CHANGED, [&receivedEvent](std::shared_ptr<Event> event) {
        receivedEvent = event;
    });

    // Create and publish event
    auto event = std::make_shared<Event>(EventType::CONFIG_CHANGED, "test_source");
    event->setData("key", std::string("test_value"));
    
    eventBus->publish(event);

    // Give some time for async processing
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Verify event was received
    ASSERT_NE(receivedEvent, nullptr);
    EXPECT_EQ(receivedEvent->getType(), EventType::CONFIG_CHANGED);
    EXPECT_EQ(receivedEvent->getSource(), "test_source");
    
    auto value = receivedEvent->getData<std::string>("key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), "test_value");
}

// Test multiple subscribers
TEST_F(EventBusTest, MultipleSubscribers) {
    std::atomic<int> callCount1{0};
    std::atomic<int> callCount2{0};
    std::atomic<int> callCount3{0};

    // Subscribe multiple handlers to the same event type
    eventBus->subscribe(EventType::STATE_CHANGED, [&callCount1](std::shared_ptr<Event> event) {
        callCount1++;
    });

    eventBus->subscribe(EventType::STATE_CHANGED, [&callCount2](std::shared_ptr<Event> event) {
        callCount2++;
    });

    eventBus->subscribe(EventType::STATE_CHANGED, [&callCount3](std::shared_ptr<Event> event) {
        callCount3++;
    });

    // Publish event
    auto event = std::make_shared<Event>(EventType::STATE_CHANGED, "test");
    eventBus->publish(event);

    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // All subscribers should have been called
    EXPECT_EQ(callCount1.load(), 1);
    EXPECT_EQ(callCount2.load(), 1);
    EXPECT_EQ(callCount3.load(), 1);
}

// Test different event types don't interfere
TEST_F(EventBusTest, EventTypeFiltering) {
    std::atomic<int> configChangedCount{0};
    std::atomic<int> stateChangedCount{0};
    std::atomic<int> connectionStatusCount{0};

    // Subscribe to different event types
    eventBus->subscribe(EventType::CONFIG_CHANGED, [&configChangedCount](std::shared_ptr<Event> event) {
        configChangedCount++;
    });

    eventBus->subscribe(EventType::STATE_CHANGED, [&stateChangedCount](std::shared_ptr<Event> event) {
        stateChangedCount++;
    });

    eventBus->subscribe(EventType::CONNECTION_STATUS, [&connectionStatusCount](std::shared_ptr<Event> event) {
        connectionStatusCount++;
    });

    // Publish different event types
    eventBus->publish(std::make_shared<Event>(EventType::CONFIG_CHANGED, "test"));
    eventBus->publish(std::make_shared<Event>(EventType::STATE_CHANGED, "test"));
    eventBus->publish(std::make_shared<Event>(EventType::CONFIG_CHANGED, "test"));

    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Each handler should only receive its subscribed event type
    EXPECT_EQ(configChangedCount.load(), 2);
    EXPECT_EQ(stateChangedCount.load(), 1);
    EXPECT_EQ(connectionStatusCount.load(), 0);
}

// Test unsubscribe functionality
TEST_F(EventBusTest, UnsubscribeTest) {
    std::atomic<int> callCount{0};

    // Subscribe and get subscription ID
    auto subscriptionId = eventBus->subscribe(EventType::CONFIG_CHANGED, [&callCount](std::shared_ptr<Event> event) {
        callCount++;
    });

    // Publish event - should be received
    eventBus->publish(std::make_shared<Event>(EventType::CONFIG_CHANGED, "test"));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(callCount.load(), 1);

    // Unsubscribe
    eventBus->unsubscribe(subscriptionId);

    // Publish another event - should not be received
    eventBus->publish(std::make_shared<Event>(EventType::CONFIG_CHANGED, "test"));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(callCount.load(), 1); // Should still be 1
}

// Test event data types
TEST_F(EventBusTest, EventDataTypes) {
    std::shared_ptr<Event> receivedEvent;
    
    eventBus->subscribe(EventType::CONFIG_CHANGED, [&receivedEvent](std::shared_ptr<Event> event) {
        receivedEvent = event;
    });

    // Create event with different data types
    auto event = std::make_shared<Event>(EventType::CONFIG_CHANGED, "test");
    event->setData("string_key", std::string("test_string"));
    event->setData("int_key", 42);
    event->setData("double_key", 3.14);
    event->setData("bool_key", true);

    eventBus->publish(event);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_NE(receivedEvent, nullptr);

    // Verify all data types
    auto stringVal = receivedEvent->getData<std::string>("string_key");
    auto intVal = receivedEvent->getData<int>("int_key");
    auto doubleVal = receivedEvent->getData<double>("double_key");
    auto boolVal = receivedEvent->getData<bool>("bool_key");

    ASSERT_TRUE(stringVal.has_value());
    ASSERT_TRUE(intVal.has_value());
    ASSERT_TRUE(doubleVal.has_value());
    ASSERT_TRUE(boolVal.has_value());

    EXPECT_EQ(stringVal.value(), "test_string");
    EXPECT_EQ(intVal.value(), 42);
    EXPECT_DOUBLE_EQ(doubleVal.value(), 3.14);
    EXPECT_TRUE(boolVal.value());
}

// Test thread safety
TEST_F(EventBusTest, ThreadSafetyTest) {
    std::atomic<int> totalEvents{0};
    
    // Subscribe to events
    eventBus->subscribe(EventType::CONFIG_CHANGED, [&totalEvents](std::shared_ptr<Event> event) {
        totalEvents++;
    });

    std::vector<std::thread> publisherThreads;
    std::vector<std::thread> subscriberThreads;

    // Create multiple publisher threads
    for (int i = 0; i < 5; ++i) {
        publisherThreads.emplace_back([this, i]() {
            for (int j = 0; j < 10; ++j) {
                auto event = std::make_shared<Event>(EventType::CONFIG_CHANGED, "thread_" + std::to_string(i));
                event->setData("value", j);
                eventBus->publish(event);
            }
        });
    }

    // Create multiple subscriber threads
    for (int i = 0; i < 3; ++i) {
        subscriberThreads.emplace_back([this, i]() {
            eventBus->subscribe(EventType::STATE_CHANGED, [](std::shared_ptr<Event> event) {
                // Just consume the event
            });
        });
    }

    // Wait for all threads to complete
    for (auto& thread : publisherThreads) {
        thread.join();
    }
    for (auto& thread : subscriberThreads) {
        thread.join();
    }

    // Wait for event processing
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Should have received all published events
    EXPECT_EQ(totalEvents.load(), 50); // 5 threads * 10 events each
}

// Test event with metadata
TEST_F(EventBusTest, EventMetadataTest) {
    std::shared_ptr<Event> receivedEvent;
    
    eventBus->subscribe(EventType::CONFIG_CHANGED, [&receivedEvent](std::shared_ptr<Event> event) {
        receivedEvent = event;
    });

    auto event = std::make_shared<Event>(EventType::CONFIG_CHANGED, "metadata_test");
    event->setData("priority", std::string("high"));
    event->setData("category", std::string("system"));

    eventBus->publish(event);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_NE(receivedEvent, nullptr);
    EXPECT_EQ(receivedEvent->getSource(), "metadata_test");
    
    auto priority = receivedEvent->getData<std::string>("priority");
    auto category = receivedEvent->getData<std::string>("category");
    
    ASSERT_TRUE(priority.has_value());
    ASSERT_TRUE(category.has_value());
    EXPECT_EQ(priority.value(), "high");
    EXPECT_EQ(category.value(), "system");
}

// Test large number of events
TEST_F(EventBusTest, HighVolumeTest) {
    std::atomic<int> receivedCount{0};
    
    eventBus->subscribe(EventType::CONFIG_CHANGED, [&receivedCount](std::shared_ptr<Event> event) {
        receivedCount++;
    });

    const int NUM_EVENTS = 1000;
    
    // Publish many events
    for (int i = 0; i < NUM_EVENTS; ++i) {
        auto event = std::make_shared<Event>(EventType::CONFIG_CHANGED, "volume_test");
        event->setData("index", i);
        eventBus->publish(event);
    }

    // Wait for processing (longer timeout for high volume)
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Should have received all events
    EXPECT_EQ(receivedCount.load(), NUM_EVENTS);
}
