#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <boost/asio.hpp>

#include <f1x/openauto/autoapp/Service/AndroidAutoEntity.hpp>
#include <f1x/openauto/autoapp/Service/ServiceFactory.hpp>
#include <f1x/openauto/autoapp/Service/Pinger.hpp>
#include "../../mocks/MockConfiguration.hpp"
#include "../../mocks/MockAndroidAutoEntity.hpp"

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::InSequence;

namespace f1x::openauto::autoapp::service {

// Mock classes needed for testing
class MockMessenger : public aasdk::messenger::IMessenger {
public:
    MOCK_METHOD(void, enqueueReceive, (aasdk::messenger::ReceivePromise::Pointer promise), (override));
    MOCK_METHOD(void, enqueueSend, (aasdk::messenger::Message message, aasdk::messenger::SendPromise::Pointer promise), (override));
    MOCK_METHOD(void, enqueueSend, (aasdk::common::Data data, aasdk::messenger::SendPromise::Pointer promise), (override));
    MOCK_METHOD(void, cancelActiveTransfers, (), (override));
};

class MockControlServiceChannel : public aasdk::channel::control::IControlServiceChannel {
public:
    MOCK_METHOD(void, receive, (aasdk::channel::control::IControlServiceChannelEventHandler::Pointer eventHandler), (override));
    MOCK_METHOD(void, sendVersionRequest, (aasdk::messenger::SendPromise::Pointer promise), (override));
    MOCK_METHOD(void, sendHandshake, (const aasdk::common::DataConstBuffer& buffer, aasdk::messenger::SendPromise::Pointer promise), (override));
    MOCK_METHOD(void, sendAuthComplete, (const aap_protobuf::service::control::message::AuthCompleteIndication& authCompleteIndication, aasdk::messenger::SendPromise::Pointer promise), (override));
    MOCK_METHOD(void, sendServiceDiscoveryResponse, (const aap_protobuf::service::control::message::ServiceDiscoveryResponse& serviceDiscoveryResponse, aasdk::messenger::SendPromise::Pointer promise), (override));
    MOCK_METHOD(void, sendAudioFocusResponse, (const aap_protobuf::service::control::message::AudioFocusNotification& response, aasdk::messenger::SendPromise::Pointer promise), (override));
    MOCK_METHOD(void, sendShutdownResponse, (const aap_protobuf::service::control::message::ByeByeResponse& response, aasdk::messenger::SendPromise::Pointer promise), (override));
    MOCK_METHOD(void, sendNavigationFocusResponse, (const aap_protobuf::service::control::message::NavFocusNotification& response, aasdk::messenger::SendPromise::Pointer promise), (override));
    MOCK_METHOD(void, sendPingRequest, (const aap_protobuf::service::control::message::PingRequest& request, aasdk::messenger::SendPromise::Pointer promise), (override));
};

class MockCryptor : public aasdk::messenger::ICryptor {
public:
    MOCK_METHOD(void, init, (), (override));
    MOCK_METHOD(bool, doHandshake, (), (override));
    MOCK_METHOD(void, encrypt, (aasdk::common::Data& output, const aasdk::common::DataConstBuffer& buffer), (override));
    MOCK_METHOD(void, decrypt, (aasdk::common::Data& output, const aasdk::common::DataConstBuffer& buffer), (override));
    MOCK_METHOD(void, readHandshakeBuffer, (aasdk::common::DataConstBuffer& buffer), (override));
    MOCK_METHOD(void, writeHandshakeBuffer, (const aasdk::common::DataConstBuffer& buffer), (override));
    MOCK_METHOD(const aasdk::common::Data&, readHandshakeBuffer, (), (const, override));
};

class MockTransport : public aasdk::transport::ITransport {
public:
    MOCK_METHOD(void, receive, (size_t size, ReceivePromise::Pointer promise), (override));
    MOCK_METHOD(void, send, (common::Data data, SendPromise::Pointer promise), (override));
    MOCK_METHOD(void, stop, (), (override));
};

class MockIAndroidAutoEntityEventHandler : public IAndroidAutoEntityEventHandler {
public:
    MOCK_METHOD(void, onAndroidAutoQuit, (), (override));
};

class ServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        ioService = std::make_unique<boost::asio::io_service>();
        mockMessenger = std::make_shared<NiceMock<MockMessenger>>();
        mockConfiguration = std::make_shared<NiceMock<configuration::MockConfiguration>>();
        mockControlServiceChannel = std::make_shared<NiceMock<MockControlServiceChannel>>();
        mockCryptor = std::make_shared<NiceMock<MockCryptor>>();
        mockTransport = std::make_shared<NiceMock<MockTransport>>();
        serviceFactory = std::make_shared<ServiceFactory>(*ioService, mockConfiguration);
        mockEventHandler = std::make_shared<NiceMock<MockIAndroidAutoEntityEventHandler>>();
        pinger = std::make_shared<Pinger>(*ioService, 100);
    }

    void TearDown() override {
        if (workThread.joinable()) {
            workThread.join();
        }
    }

    void runIoServiceInBackground() {
        workThread = std::thread([this]() {
            boost::asio::io_service::work work(*ioService);
            ioService->run();
        });
    }

    std::unique_ptr<boost::asio::io_service> ioService;
    std::thread workThread;
    std::shared_ptr<NiceMock<MockMessenger>> mockMessenger;
    std::shared_ptr<NiceMock<configuration::MockConfiguration>> mockConfiguration;
    std::shared_ptr<NiceMock<MockControlServiceChannel>> mockControlServiceChannel;
    std::shared_ptr<NiceMock<MockCryptor>> mockCryptor;
    std::shared_ptr<NiceMock<MockTransport>> mockTransport;
    std::shared_ptr<ServiceFactory> serviceFactory;
    std::shared_ptr<NiceMock<MockIAndroidAutoEntityEventHandler>> mockEventHandler;
    std::shared_ptr<Pinger> pinger;
};

// TC-AAP-001 - Android Auto Protocol Version Negotiation
TEST_F(ServiceTest, VersionNegotiation) {
    runIoServiceInBackground();

    // Create AndroidAutoEntity with mocks
    auto androidAutoEntity = std::make_shared<AndroidAutoEntity>(
        *ioService,
        mockCryptor,
        mockTransport,
        mockMessenger,
        mockConfiguration,
        serviceFactory->create(mockMessenger),
        pinger
    );

    // Expect version request to be sent during start
    EXPECT_CALL(*mockControlServiceChannel, sendVersionRequest(_)).Times(1);
    
    // Replace internal channel with our mock
    testing::internal::CaptureStdout(); // Suppress stdout during test
    androidAutoEntity->start(*mockEventHandler);
    testing::internal::GetCapturedStdout();

    // Stop I/O service
    ioService->stop();
}

// TC-AAP-002 - Service Discovery
TEST_F(ServiceTest, ServiceDiscovery) {
    runIoServiceInBackground();

    // Create AndroidAutoEntity with mocks
    auto androidAutoEntity = std::make_shared<AndroidAutoEntity>(
        *ioService,
        mockCryptor,
        mockTransport,
        mockMessenger,
        mockConfiguration,
        serviceFactory->create(mockMessenger),
        pinger
    );

    // Setup test sequence
    {
        InSequence seq;

        EXPECT_CALL(*mockControlServiceChannel, sendServiceDiscoveryResponse(_, _)).Times(1);
        EXPECT_CALL(*mockControlServiceChannel, receive(_)).Times(1);
    }

    // Create a ServiceDiscoveryRequest
    aap_protobuf::service::control::message::ServiceDiscoveryRequest request;
    request.set_device_name("TestDevice");
    request.set_label_text("TestLabel");
    
    // Call the service discovery handler
    androidAutoEntity->onServiceDiscoveryRequest(request);

    // Stop I/O service
    ioService->stop();
}

// TC-AAP-003 - Audio Focus Handling
TEST_F(ServiceTest, AudioFocusHandling) {
    runIoServiceInBackground();

    // Create AndroidAutoEntity with mocks
    auto androidAutoEntity = std::make_shared<AndroidAutoEntity>(
        *ioService,
        mockCryptor,
        mockTransport,
        mockMessenger,
        mockConfiguration,
        serviceFactory->create(mockMessenger),
        pinger
    );

    // Setup test sequence
    {
        InSequence seq;

        EXPECT_CALL(*mockControlServiceChannel, sendAudioFocusResponse(_, _)).Times(1);
        EXPECT_CALL(*mockControlServiceChannel, receive(_)).Times(1);
    }

    // Create an AudioFocusRequest for gain
    aap_protobuf::service::control::message::AudioFocusRequest request;
    request.set_audio_focus_type(aap_protobuf::service::control::message::AudioFocusRequestType::AUDIO_FOCUS_GAIN);
    
    // Call the audio focus handler
    androidAutoEntity->onAudioFocusRequest(request);

    // Now test with a release request
    request.set_audio_focus_type(aap_protobuf::service::control::message::AudioFocusRequestType::AUDIO_FOCUS_RELEASE);
    
    // Setup expectations for release
    {
        InSequence seq;

        EXPECT_CALL(*mockControlServiceChannel, sendAudioFocusResponse(_, _)).Times(1);
        EXPECT_CALL(*mockControlServiceChannel, receive(_)).Times(1);
    }
    
    androidAutoEntity->onAudioFocusRequest(request);

    // Stop I/O service
    ioService->stop();
}

// TC-AAP-004 - Navigation Focus Handling
TEST_F(ServiceTest, NavigationFocusHandling) {
    runIoServiceInBackground();

    // Create AndroidAutoEntity with mocks
    auto androidAutoEntity = std::make_shared<AndroidAutoEntity>(
        *ioService,
        mockCryptor,
        mockTransport,
        mockMessenger,
        mockConfiguration,
        serviceFactory->create(mockMessenger),
        pinger
    );

    // Setup test sequence
    {
        InSequence seq;

        EXPECT_CALL(*mockControlServiceChannel, sendNavigationFocusResponse(_, _)).Times(1);
        EXPECT_CALL(*mockControlServiceChannel, receive(_)).Times(1);
    }

    // Create a NavigationFocusRequest
    aap_protobuf::service::control::message::NavFocusRequestNotification request;
    request.set_focus_type(aap_protobuf::service::control::message::NavFocusType::NAV_FOCUS_PROJECTED);
    
    // Call the navigation focus handler
    androidAutoEntity->onNavigationFocusRequest(request);

    // Stop I/O service
    ioService->stop();
}

// TC-AAP-005 - Ping Mechanism
TEST_F(ServiceTest, PingMechanism) {
    // Create a custom pinger for testing with short intervals
    auto testPinger = std::make_shared<Pinger>(*ioService, 100); // 100ms interval
    
    // Mock expectations
    EXPECT_CALL(*mockControlServiceChannel, sendPingRequest(_, _)).Times(AtLeast(1));
    
    // Start the pinger
    testPinger->start();
    
    // Run I/O service for a short time to allow pings
    auto work = std::make_unique<boost::asio::io_service::work>(*ioService);
    std::thread ioThread([this]() { ioService->run(); });
    
    // Wait for some pings to be sent
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    
    // Stop pinger and I/O service
    testPinger->stop();
    work.reset();
    ioService->stop();
    
    if (ioThread.joinable()) {
        ioThread.join();
    }
}

} // namespace f1x::openauto::autoapp::service