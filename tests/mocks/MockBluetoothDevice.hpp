#pragma once

#include <gmock/gmock.h>
#include <f1x/openauto/autoapp/Projection/IBluetoothDevice.hpp>

namespace f1x::openauto::autoapp::projection {

class MockBluetoothDevice : public IBluetoothDevice {
public:
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(bool, isPaired, (const std::string& address), (const, override));
    MOCK_METHOD(std::string, getAdapterAddress, (), (const, override));
    MOCK_METHOD(bool, isAvailable, (), (const, override));
};

}  // namespace f1x::openauto::autoapp::projection