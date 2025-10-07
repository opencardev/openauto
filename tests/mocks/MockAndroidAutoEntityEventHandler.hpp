#pragma once

#include <gmock/gmock.h>
#include <f1x/openauto/autoapp/Service/IAndroidAutoEntityEventHandler.hpp>

namespace f1x::openauto::autoapp::service {

class MockAndroidAutoEntityEventHandler : public IAndroidAutoEntityEventHandler {
public:
    MOCK_METHOD(void, onAndroidAutoQuit, (), (override));
};

}  // namespace f1x::openauto::autoapp::service