#pragma once

#include <gmock/gmock.h>
#include <aasdk/USB/IAOAPDevice.hpp>

namespace aasdk::usb {

class MockAOAPDevice : public IAOAPDevice {
public:
    MOCK_METHOD(IUSBEndpoint&, getInEndpoint, (), (override));
    MOCK_METHOD(IUSBEndpoint&, getOutEndpoint, (), (override));
};

}  // namespace aasdk::usb