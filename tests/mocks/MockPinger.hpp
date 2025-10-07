#pragma once

#include <gmock/gmock.h>
#include <f1x/openauto/autoapp/Service/IPinger.hpp>

namespace f1x::openauto::autoapp::service {

class MockPinger : public IPinger {
public:
    MOCK_METHOD(void, ping, (Promise::Pointer promise), (override));
    MOCK_METHOD(void, pong, (), (override));
    MOCK_METHOD(void, cancel, (), (override));
};

}  // namespace f1x::openauto::autoapp::service