#pragma once

#include <gmock/gmock.h>
#include <aasdk/Messenger/ICryptor.hpp>

namespace aasdk::messenger {

class MockCryptor : public ICryptor {
public:
    MOCK_METHOD(void, init, (), (override));
    MOCK_METHOD(bool, doHandshake, (), (override));
    MOCK_METHOD(void, encrypt, (common::Data& output, const common::DataConstBuffer& buffer), (override));
    MOCK_METHOD(void, decrypt, (common::Data& output, const common::DataConstBuffer& buffer), (override));
    MOCK_METHOD(void, readHandshakeBuffer, (common::DataConstBuffer& buffer), (override));
    MOCK_METHOD(void, writeHandshakeBuffer, (const common::DataConstBuffer& buffer), (override));
    MOCK_METHOD(const common::Data&, readHandshakeBuffer, (), (const, override));
};

}  // namespace aasdk::messenger