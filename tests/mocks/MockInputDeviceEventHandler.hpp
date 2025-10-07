#pragma once

#include <gmock/gmock.h>
#include <f1x/openauto/autoapp/Projection/IInputDeviceEventHandler.hpp>

namespace f1x::openauto::autoapp::projection {

class MockInputDeviceEventHandler : public IInputDeviceEventHandler {
public:
    MOCK_METHOD(void, onButtonEvent, (const ButtonEvent& event), (override));
    MOCK_METHOD(void, onTouchEvent, (const TouchEvent& event), (override));
};

}  // namespace f1x::openauto::autoapp::projection