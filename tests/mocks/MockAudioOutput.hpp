#pragma once

#include <gmock/gmock.h>
#include <f1x/openauto/autoapp/Projection/IAudioOutput.hpp>

namespace f1x::openauto::autoapp::projection {

class MockAudioOutput : public IAudioOutput {
public:
    MOCK_METHOD(bool, open, (), (override));
    MOCK_METHOD(void, write, (aasdk::messenger::Timestamp::ValueType timestamp, const aasdk::common::DataConstBuffer& buffer), (override));
    MOCK_METHOD(void, start, (), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(void, suspend, (), (override));
    MOCK_METHOD(uint32_t, getSampleSize, (), (const, override));
    MOCK_METHOD(uint32_t, getChannelCount, (), (const, override));
    MOCK_METHOD(uint32_t, getSampleRate, (), (const, override));
};

}  // namespace f1x::openauto::autoapp::projection