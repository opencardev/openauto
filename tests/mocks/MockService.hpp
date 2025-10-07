#pragma once

#include <gmock/gmock.h>
#include <f1x/openauto/autoapp/Service/IService.hpp>

namespace f1x::openauto::autoapp::service {

class MockService : public IService {
public:
    MOCK_METHOD(void, start, (), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(void, pause, (), (override));
    MOCK_METHOD(void, resume, (), (override));
    MOCK_METHOD(void, fillFeatures, (aap_protobuf::service::control::message::ServiceDiscoveryResponse& response), (override));
};

}  // namespace f1x::openauto::autoapp::service