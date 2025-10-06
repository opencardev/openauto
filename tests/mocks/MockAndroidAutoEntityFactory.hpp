#pragma once

#include <gmock/gmock.h>
#include <f1x/openauto/autoapp/Service/IAndroidAutoEntityFactory.hpp>

namespace f1x::openauto::autoapp::service {

class MockAndroidAutoEntityFactory : public IAndroidAutoEntityFactory {
public:
    MOCK_METHOD(IAndroidAutoEntity::Pointer, create, (aasdk::usb::IAOAPDevice::Pointer aoapDevice), (override));
    MOCK_METHOD(IAndroidAutoEntity::Pointer, create, (aasdk::tcp::ITCPEndpoint::Pointer tcpEndpoint), (override));
};

}  // namespace f1x::openauto::autoapp::service