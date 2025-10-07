#pragma once

#include <gmock/gmock.h>
#include <aasdk/TCP/ITCPEndpoint.hpp>

namespace aasdk::tcp {

class MockTCPEndpoint : public ITCPEndpoint {
public:
    MOCK_METHOD(void, send, (common::DataConstBuffer buffer, Promise::Pointer promise), (override));
    MOCK_METHOD(void, receive, (common::DataBuffer buffer, Promise::Pointer promise), (override));
    MOCK_METHOD(void, stop, (), (override));
};

}  // namespace aasdk::tcp