#pragma once

#include <gmock/gmock.h>
#include <f1x/openauto/autoapp/Service/IServiceFactory.hpp>

namespace f1x::openauto::autoapp::service {

class MockServiceFactory : public IServiceFactory {
public:
    MOCK_METHOD(ServiceList, create, (aasdk::messenger::IMessenger::Pointer messenger), (override));
};

}  // namespace f1x::openauto::autoapp::service