#pragma once

#include <gmock/gmock.h>
#include <f1x/openauto/autoapp/Service/IAndroidAutoEntity.hpp>

namespace f1x::openauto::autoapp::service {

class MockAndroidAutoEntity : public IAndroidAutoEntity {
public:
    MOCK_METHOD(void, start, (IAndroidAutoEntityEventHandler& eventHandler), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(void, pause, (), (override));
    MOCK_METHOD(void, resume, (), (override));
};

}  // namespace f1x::openauto::autoapp::service