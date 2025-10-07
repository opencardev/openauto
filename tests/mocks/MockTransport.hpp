#pragma once

#include <gmock/gmock.h>
#include <aasdk/Transport/ITransport.hpp>

namespace aasdk::transport {

class MockTransport : public ITransport {
public:
    MOCK_METHOD(void, receive, (size_t size, ReceivePromise::Pointer promise), (override));
    MOCK_METHOD(void, send, (common::Data data, SendPromise::Pointer promise), (override));
    MOCK_METHOD(void, stop, (), (override));
};

}  // namespace aasdk::transport