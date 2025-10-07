#pragma once

#include <gmock/gmock.h>
#include <f1x/openauto/autoapp/Projection/IAudioInput.hpp>

namespace f1x::openauto::autoapp::projection {

class MockAudioInput : public IAudioInput {
public:
    MOCK_METHOD(bool, open, (), (override));
    MOCK_METHOD(bool, isActive, (), (const, override));
    MOCK_METHOD(void, read, (ReadPromise::Pointer promise), (override));
    MOCK_METHOD(void, start, (StartPromise::Pointer promise), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(uint32_t, getSampleSize, (), (const, override));
    MOCK_METHOD(uint32_t, getChannelCount, (), (const, override));
    MOCK_METHOD(uint32_t, getSampleRate, (), (const, override));
};

}  // namespace f1x::openauto::autoapp::projection