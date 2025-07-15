#pragma once

#include <gmock/gmock.h>
#include <modern/EventBus.hpp>

namespace openauto {
namespace modern {
namespace test {

class MockEventBus {
public:
    MOCK_METHOD(std::string, subscribe, (EventType type, std::function<void(std::shared_ptr<Event>)> handler));
    MOCK_METHOD(void, unsubscribe, (const std::string& subscriptionId));
    MOCK_METHOD(void, publish, (std::shared_ptr<Event> event));
    MOCK_METHOD(size_t, getSubscriberCount, (EventType type), (const));
    MOCK_METHOD(void, clear, ());
};

} // namespace test
} // namespace modern
} // namespace openauto
