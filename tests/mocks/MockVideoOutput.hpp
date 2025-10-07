#pragma once

#include <gmock/gmock.h>
#include <f1x/openauto/autoapp/Projection/IVideoOutput.hpp>

namespace f1x::openauto::autoapp::projection {

class MockVideoOutput : public IVideoOutput {
public:
    MOCK_METHOD(bool, open, (), (override));
    MOCK_METHOD(bool, init, (), (override));
    MOCK_METHOD(void, write, (aasdk::messenger::Timestamp::ValueType timestamp, const aasdk::common::DataConstBuffer& buffer), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(aap_protobuf::service::media::sink::message::VideoFrameRateType, getVideoFPS, (), (const, override));
    MOCK_METHOD(aap_protobuf::service::media::sink::message::VideoCodecResolutionType, getVideoResolution, (), (const, override));
    MOCK_METHOD(size_t, getScreenDPI, (), (const, override));
    MOCK_METHOD(QRect, getVideoMargins, (), (const, override));
};

}  // namespace f1x::openauto::autoapp::projection