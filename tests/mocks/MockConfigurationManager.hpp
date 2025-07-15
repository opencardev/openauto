#pragma once

#include <gmock/gmock.h>
#include <modern/ConfigurationManager.hpp>

namespace openauto {
namespace modern {
namespace test {

class MockConfigurationManager {
public:
    MOCK_METHOD(void, setValue, (const std::string& key, const nlohmann::json& value));
    MOCK_METHOD(nlohmann::json, getValue, (const std::string& key), (const));
    MOCK_METHOD(bool, hasKey, (const std::string& key), (const));
    MOCK_METHOD(void, removeKey, (const std::string& key));
    MOCK_METHOD(void, clear, ());
    MOCK_METHOD(void, save, ());
    MOCK_METHOD(void, load, ());
    MOCK_METHOD(nlohmann::json, getAllSettings, (), (const));
    MOCK_METHOD(void, setEventBus, (std::shared_ptr<EventBus> eventBus));
    MOCK_METHOD(void, notifyChange, (const std::string& key));
};

} // namespace test
} // namespace modern
} // namespace openauto
