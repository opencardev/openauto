#pragma once

#include <gmock/gmock.h>
#include <aasdk/Messenger/IMessenger.hpp>

namespace aasdk::messenger {

class MockMessenger : public IMessenger {
public:
    MOCK_METHOD(void, enqueueReceive, (ChannelId channelId, ReceivePromise::Pointer promise), (override));
    MOCK_METHOD(void, enqueueSend, (Message::Pointer message, SendPromise::Pointer promise), (override));
    MOCK_METHOD(void, stop, (), (override));
};

}  // namespace aasdk::messenger