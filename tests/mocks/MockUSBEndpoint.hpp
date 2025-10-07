#pragma once

#include <gmock/gmock.h>
#include <aasdk/USB/IUSBEndpoint.hpp>

namespace aasdk::usb {

class MockUSBEndpoint : public IUSBEndpoint {
public:
    MOCK_METHOD(uint8_t, getAddress, (), (override));
    MOCK_METHOD(void, controlTransfer, (common::DataBuffer buffer, uint32_t timeout, Promise::Pointer promise), (override));
    MOCK_METHOD(void, bulkTransfer, (common::DataBuffer buffer, uint32_t timeout, Promise::Pointer promise), (override));
    MOCK_METHOD(void, interruptTransfer, (common::DataBuffer buffer, uint32_t timeout, Promise::Pointer promise), (override));
    MOCK_METHOD(void, cancelTransfers, (), (override));
    MOCK_METHOD(DeviceHandle, getDeviceHandle, (), (const, override));
};

}  // namespace aasdk::usb