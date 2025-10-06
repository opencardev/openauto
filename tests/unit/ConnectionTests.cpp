#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <chrono>

#include <f1x/openauto/autoapp/App.hpp>
#include "../../mocks/MockAndroidAutoEntity.hpp"
#include "../../mocks/MockAndroidAutoEntityFactory.hpp"
#include "../../mocks/MockConfiguration.hpp"

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::AtLeast;

namespace f1x::openauto::autoapp {

class ConnectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockUsbWrapper = std::make_shared<NiceMock<aasdk::usb::MockUSBWrapper>>();
        mockTcpWrapper = std::make_shared<NiceMock<aasdk::tcp::MockTCPWrapper>>();
        mockUsbHub = std::make_shared<NiceMock<aasdk::usb::MockUSBHub>>();
        mockConnectedAccessoriesEnumerator = std::make_shared<NiceMock<aasdk::usb::MockConnectedAccessoriesEnumerator>>();
        mockAndroidAutoEntityFactory = std::make_shared<NiceMock<service::MockAndroidAutoEntityFactory>>();
        mockAndroidAutoEntity = std::make_shared<NiceMock<service::MockAndroidAutoEntity>>();
        configuration = std::make_shared<NiceMock<configuration::MockConfiguration>>();

        ON_CALL(*mockAndroidAutoEntityFactory, create(testing::An<aasdk::usb::IAOAPDevice::Pointer>()))
            .WillByDefault(Return(mockAndroidAutoEntity));
            
        ON_CALL(*mockAndroidAutoEntityFactory, create(testing::An<aasdk::tcp::ITCPEndpoint::Pointer>()))
            .WillByDefault(Return(mockAndroidAutoEntity));

        app = std::make_unique<App>(
            ioService, 
            *mockUsbWrapper, 
            *mockTcpWrapper, 
            *mockAndroidAutoEntityFactory,
            mockUsbHub, 
            mockConnectedAccessoriesEnumerator);
    }

    void TearDown() override {
        if (workThread.joinable()) {
            workThread.join();
        }
    }

    void runIoServiceInBackground() {
        workThread = std::thread([this]() {
            boost::asio::io_service::work work(ioService);
            ioService.run();
        });
    }

    boost::asio::io_service ioService;
    std::thread workThread;
    std::shared_ptr<NiceMock<aasdk::usb::MockUSBWrapper>> mockUsbWrapper;
    std::shared_ptr<NiceMock<aasdk::tcp::MockTCPWrapper>> mockTcpWrapper;
    std::shared_ptr<NiceMock<aasdk::usb::MockUSBHub>> mockUsbHub;
    std::shared_ptr<NiceMock<aasdk::usb::MockConnectedAccessoriesEnumerator>> mockConnectedAccessoriesEnumerator;
    std::shared_ptr<NiceMock<service::MockAndroidAutoEntityFactory>> mockAndroidAutoEntityFactory;
    std::shared_ptr<NiceMock<service::MockAndroidAutoEntity>> mockAndroidAutoEntity;
    std::shared_ptr<NiceMock<configuration::MockConfiguration>> configuration;
    std::unique_ptr<App> app;
};

// TC-CONN-001 - USB Device Connection
TEST_F(ConnectionTest, USBDeviceConnection) {
    runIoServiceInBackground();
    
    // Expect the ConnectedAccessoriesEnumerator to be asked to enumerate devices
    EXPECT_CALL(*mockConnectedAccessoriesEnumerator, enumerate());

    // Expect AndroidAutoEntity to be started after connection
    EXPECT_CALL(*mockAndroidAutoEntity, start(_)).Times(1);

    // Trigger waitForUSBDevice
    app->waitForUSBDevice();
    
    // Simulate USB device connection by calling the registered handler
    app->aoapDeviceHandler(123); // 123 is a mock device handle
    
    // Sleep to allow async operations to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Stop I/O service and app
    app->stop();
    ioService.stop();
}

// TC-CONN-002 - WiFi Connection
TEST_F(ConnectionTest, WiFiConnection) {
    runIoServiceInBackground();
    
    // Create mock socket for TCP connection
    auto mockSocket = std::make_shared<boost::asio::ip::tcp::socket>(ioService);
    
    // Expect AndroidAutoEntity to be started once WiFi connection is established
    EXPECT_CALL(*mockAndroidAutoEntity, start(_)).Times(1);

    // Simulate WiFi connection
    app->start(std::move(mockSocket));
    
    // Sleep to allow async operations to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Stop I/O service and app
    app->stop();
    ioService.stop();
}

// TC-CONN-003 - Connection Lost Recovery
TEST_F(ConnectionTest, ConnectionLostRecovery) {
    runIoServiceInBackground();

    // First connect via USB
    EXPECT_CALL(*mockConnectedAccessoriesEnumerator, enumerate()).Times(AtLeast(1));
    EXPECT_CALL(*mockAndroidAutoEntity, start(_)).Times(AtLeast(1));
    
    app->waitForUSBDevice();
    app->aoapDeviceHandler(123); // Connect device first
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Simulate disconnection and expect onAndroidAutoQuit to be called
    app->onAndroidAutoQuit();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Verify that the app waits for a new device after disconnect
    EXPECT_CALL(*mockConnectedAccessoriesEnumerator, enumerate()).Times(AtLeast(1));
    
    // Stop I/O service and app
    app->stop();
    ioService.stop();
}

} // namespace f1x::openauto::autoapp