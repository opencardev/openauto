#pragma once

#include <gmock/gmock.h>
#include <f1x/openauto/autoapp/Projection/IInputDevice.hpp>

namespace f1x::openauto::autoapp::projection {

class MockInputDevice : public IInputDevice {
public:
    MOCK_METHOD(void, start, (IInputDeviceEventHandler& eventHandler), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(ButtonCodes, getSupportedButtonCodes, (), (const, override));
    MOCK_METHOD(bool, hasTouchscreen, (), (const, override));
    MOCK_METHOD(QRect, getTouchscreenGeometry, (), (const, override));
};

}  // namespace f1x::openauto::autoapp::projection