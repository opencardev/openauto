#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

// Include the new mock classes
#include "../mocks/MockService.hpp"
#include "../mocks/MockServiceFactory.hpp"
#include "../mocks/MockVideoOutput.hpp"
#include "../mocks/MockInputDevice.hpp"
#include "../mocks/MockBluetoothDevice.hpp"
#include "../mocks/MockPinger.hpp"
#include "../mocks/MockInputDeviceEventHandler.hpp"

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::NiceMock;

// Phase 2 Mock-Based Tests - Testing component interactions with mocks
namespace f1x::openauto::autoapp::service {

class MockBasedServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockService = std::make_shared<StrictMock<MockService>>();
        mockServiceFactory = std::make_shared<StrictMock<MockServiceFactory>>();
        mockPinger = std::make_shared<StrictMock<MockPinger>>();
    }

    void TearDown() override {
        mockService.reset();
        mockServiceFactory.reset();
        mockPinger.reset();
    }

    std::shared_ptr<MockService> mockService;
    std::shared_ptr<MockServiceFactory> mockServiceFactory;
    std::shared_ptr<MockPinger> mockPinger;
};

// Test service lifecycle with mocks
TEST_F(MockBasedServiceTest, ServiceLifecycle) {
    // Setup expectations
    EXPECT_CALL(*mockService, start())
        .Times(1);
    EXPECT_CALL(*mockService, stop())
        .Times(1);

    // Execute
    mockService->start();
    mockService->stop();
}

// Test service factory creation
TEST_F(MockBasedServiceTest, ServiceFactoryCreation) {
    // Mock messenger (simplified for Phase 2A)
    auto mockMessenger = std::shared_ptr<aasdk::messenger::IMessenger>();
    
    // Setup expectations
    EXPECT_CALL(*mockServiceFactory, create(_))
        .Times(1)
        .WillOnce(Return(ServiceList{}));

    // Execute
    auto services = mockServiceFactory->create(mockMessenger);
    
    // Verify
    EXPECT_TRUE(services.empty());
}

// Test pinger functionality
TEST_F(MockBasedServiceTest, PingerOperations) {
    auto mockPromise = std::shared_ptr<IPinger::Promise>();
    
    // Setup expectations
    EXPECT_CALL(*mockPinger, ping(_))
        .Times(1);
    EXPECT_CALL(*mockPinger, pong())
        .Times(1);
    EXPECT_CALL(*mockPinger, cancel())
        .Times(1);

    // Execute
    mockPinger->ping(mockPromise);
    mockPinger->pong();
    mockPinger->cancel();
}

} // namespace f1x::openauto::autoapp::service

namespace f1x::openauto::autoapp::projection {

class MockBasedProjectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockVideoOutput = std::make_shared<StrictMock<MockVideoOutput>>();
        mockInputDevice = std::make_shared<StrictMock<MockInputDevice>>();
        mockBluetoothDevice = std::make_shared<StrictMock<MockBluetoothDevice>>();
        mockInputEventHandler = std::make_shared<StrictMock<MockInputDeviceEventHandler>>();
    }

    void TearDown() override {
        mockVideoOutput.reset();
        mockInputDevice.reset();
        mockBluetoothDevice.reset();
        mockInputEventHandler.reset();
    }

    std::shared_ptr<MockVideoOutput> mockVideoOutput;
    std::shared_ptr<MockInputDevice> mockInputDevice;
    std::shared_ptr<MockBluetoothDevice> mockBluetoothDevice;
    std::shared_ptr<MockInputDeviceEventHandler> mockInputEventHandler;
};

// Test video output lifecycle
TEST_F(MockBasedProjectionTest, VideoOutputLifecycle) {
    // Setup expectations
    EXPECT_CALL(*mockVideoOutput, open())
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mockVideoOutput, init())
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mockVideoOutput, stop())
        .Times(1);

    // Execute
    EXPECT_TRUE(mockVideoOutput->open());
    EXPECT_TRUE(mockVideoOutput->init());
    mockVideoOutput->stop();
}

// Test input device functionality
TEST_F(MockBasedProjectionTest, InputDeviceOperations) {
    // Setup expectations
    EXPECT_CALL(*mockInputDevice, hasTouchscreen())
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mockInputDevice, start(_))
        .Times(1);
    EXPECT_CALL(*mockInputDevice, stop())
        .Times(1);

    // Execute
    EXPECT_TRUE(mockInputDevice->hasTouchscreen());
    mockInputDevice->start(*mockInputEventHandler);
    mockInputDevice->stop();
}

// Test Bluetooth device functionality
TEST_F(MockBasedProjectionTest, BluetoothDeviceOperations) {
    const std::string testAddress = "00:11:22:33:44:55";
    const std::string adapterAddress = "AA:BB:CC:DD:EE:FF";

    // Setup expectations
    EXPECT_CALL(*mockBluetoothDevice, isAvailable())
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*mockBluetoothDevice, isPaired(testAddress))
        .Times(1)
        .WillOnce(Return(false));
    EXPECT_CALL(*mockBluetoothDevice, getAdapterAddress())
        .Times(1)
        .WillOnce(Return(adapterAddress));
    EXPECT_CALL(*mockBluetoothDevice, stop())
        .Times(1);

    // Execute
    EXPECT_TRUE(mockBluetoothDevice->isAvailable());
    EXPECT_FALSE(mockBluetoothDevice->isPaired(testAddress));
    EXPECT_EQ(mockBluetoothDevice->getAdapterAddress(), adapterAddress);
    mockBluetoothDevice->stop();
}

} // namespace f1x::openauto::autoapp::projection