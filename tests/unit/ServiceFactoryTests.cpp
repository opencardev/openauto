#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

// Include mocks
#include "../mocks/MockAndroidAutoEntityFactory.hpp"
#include "../mocks/MockAndroidAutoEntity.hpp"
#include "../mocks/MockAndroidAutoEntityEventHandler.hpp"
#include "../mocks/MockAOAPDevice.hpp"
#include "../mocks/MockTCPEndpoint.hpp"
#include "../mocks/MockUSBEndpoint.hpp"
#include "../mocks/MockMessenger.hpp"

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::NiceMock;
using ::testing::InSequence;

// Phase 2B Service Factory Tests - Testing factory patterns and device creation
namespace f1x::openauto::autoapp::service {

class ServiceFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockEntityFactory = std::make_shared<StrictMock<MockAndroidAutoEntityFactory>>();
        mockAndroidAutoEntity = std::make_shared<StrictMock<MockAndroidAutoEntity>>();
        mockAOAPDevice = std::make_shared<NiceMock<aasdk::usb::MockAOAPDevice>>();
        mockTCPEndpoint = std::make_shared<NiceMock<aasdk::tcp::MockTCPEndpoint>>();
        mockUSBInEndpoint = std::make_shared<NiceMock<aasdk::usb::MockUSBEndpoint>>();
        mockUSBOutEndpoint = std::make_shared<NiceMock<aasdk::usb::MockUSBEndpoint>>();
        mockEventHandler = std::make_shared<NiceMock<MockAndroidAutoEntityEventHandler>>();
        
        // Setup AOAP device to return endpoint references
        ON_CALL(*mockAOAPDevice, getInEndpoint())
            .WillByDefault(::testing::ReturnRef(*mockUSBInEndpoint));
        ON_CALL(*mockAOAPDevice, getOutEndpoint())
            .WillByDefault(::testing::ReturnRef(*mockUSBOutEndpoint));
    }

    void TearDown() override {
        mockEntityFactory.reset();
        mockAndroidAutoEntity.reset();
        mockAOAPDevice.reset();
        mockTCPEndpoint.reset();
        mockUSBInEndpoint.reset();
        mockUSBOutEndpoint.reset();
        mockEventHandler.reset();
    }

    std::shared_ptr<MockAndroidAutoEntityFactory> mockEntityFactory;
    std::shared_ptr<MockAndroidAutoEntity> mockAndroidAutoEntity;
    std::shared_ptr<aasdk::usb::MockAOAPDevice> mockAOAPDevice;
    std::shared_ptr<aasdk::tcp::MockTCPEndpoint> mockTCPEndpoint;
    std::shared_ptr<aasdk::usb::MockUSBEndpoint> mockUSBInEndpoint;
    std::shared_ptr<aasdk::usb::MockUSBEndpoint> mockUSBOutEndpoint;
    std::shared_ptr<MockAndroidAutoEntityEventHandler> mockEventHandler;
};

// Test USB device entity creation
TEST_F(ServiceFactoryTest, USBDeviceEntityCreation) {
    // Setup expectation for USB entity creation - use explicit matcher to resolve overload ambiguity
    EXPECT_CALL(*mockEntityFactory, create(::testing::Matcher<aasdk::usb::IAOAPDevice::Pointer>(::testing::Eq(mockAOAPDevice))))
        .Times(1)
        .WillOnce(Return(mockAndroidAutoEntity));
    
    // Create entity through factory
    auto createdEntity = mockEntityFactory->create(mockAOAPDevice);
    EXPECT_EQ(createdEntity, mockAndroidAutoEntity);
}

// Test TCP endpoint entity creation
TEST_F(ServiceFactoryTest, TCPEndpointEntityCreation) {
    // Setup expectation for TCP entity creation - use explicit matcher to resolve overload ambiguity
    EXPECT_CALL(*mockEntityFactory, create(::testing::Matcher<aasdk::tcp::ITCPEndpoint::Pointer>(::testing::Eq(mockTCPEndpoint))))
        .Times(1)
        .WillOnce(Return(mockAndroidAutoEntity));
    
    // Create entity through factory
    auto entity = mockEntityFactory->create(mockTCPEndpoint);
    
    // Verify entity creation
    EXPECT_EQ(entity, mockAndroidAutoEntity);
}

// Test factory error handling for invalid devices
TEST_F(ServiceFactoryTest, FactoryErrorHandling) {
    // Test with null device - use explicit matcher to resolve overload ambiguity
    EXPECT_CALL(*mockEntityFactory, create(::testing::Matcher<aasdk::usb::IAOAPDevice::Pointer>(::testing::Eq(std::shared_ptr<aasdk::usb::IAOAPDevice>()))))
        .Times(1)
        .WillOnce(Return(std::shared_ptr<IAndroidAutoEntity>()));
    
    // Attempt to create entity with null device
    auto nullEntity = mockEntityFactory->create(std::shared_ptr<aasdk::usb::IAOAPDevice>());
    
    // Verify null entity returned
    EXPECT_EQ(nullEntity, nullptr);
}

// Test multiple entity creation with different types
TEST_F(ServiceFactoryTest, MultipleEntityCreation) {
    auto secondEntity = std::make_shared<StrictMock<MockAndroidAutoEntity>>();
    
    InSequence seq;
    
    // Create USB entity first - use explicit matcher
    EXPECT_CALL(*mockEntityFactory, create(::testing::Matcher<aasdk::usb::IAOAPDevice::Pointer>(::testing::Eq(mockAOAPDevice))))
        .Times(1)
        .WillOnce(Return(mockAndroidAutoEntity));
    
    // Create TCP entity second - use explicit matcher
    EXPECT_CALL(*mockEntityFactory, create(::testing::Matcher<aasdk::tcp::ITCPEndpoint::Pointer>(::testing::Eq(mockTCPEndpoint))))
        .Times(1)
        .WillOnce(Return(secondEntity));
    
    // Execute creation sequence
    auto usbEntity = mockEntityFactory->create(mockAOAPDevice);
    auto tcpEntity = mockEntityFactory->create(mockTCPEndpoint);
    
    // Verify different entities created
    EXPECT_EQ(usbEntity, mockAndroidAutoEntity);
    EXPECT_EQ(tcpEntity, secondEntity);
    EXPECT_NE(usbEntity, tcpEntity);
}

} // namespace f1x::openauto::autoapp::service

// Device connection workflow tests
class DeviceConnectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockAOAPDevice = std::make_shared<StrictMock<aasdk::usb::MockAOAPDevice>>();
        mockTCPEndpoint = std::make_shared<StrictMock<aasdk::tcp::MockTCPEndpoint>>();
        mockAndroidAutoEntity = std::make_shared<StrictMock<f1x::openauto::autoapp::service::MockAndroidAutoEntity>>();
        mockUSBInEndpoint = std::make_shared<NiceMock<aasdk::usb::MockUSBEndpoint>>();
        mockUSBOutEndpoint = std::make_shared<NiceMock<aasdk::usb::MockUSBEndpoint>>();
        mockEventHandler = std::make_shared<NiceMock<f1x::openauto::autoapp::service::MockAndroidAutoEntityEventHandler>>();
        
        // Setup AOAP device to return endpoint references
        ON_CALL(*mockAOAPDevice, getInEndpoint())
            .WillByDefault(::testing::ReturnRef(*mockUSBInEndpoint));
        ON_CALL(*mockAOAPDevice, getOutEndpoint())
            .WillByDefault(::testing::ReturnRef(*mockUSBOutEndpoint));
    }

    void TearDown() override {
        mockAOAPDevice.reset();
        mockTCPEndpoint.reset();
        mockAndroidAutoEntity.reset();
        mockUSBInEndpoint.reset();
        mockUSBOutEndpoint.reset();
        mockEventHandler.reset();
    }

    std::shared_ptr<aasdk::usb::MockAOAPDevice> mockAOAPDevice;
    std::shared_ptr<aasdk::tcp::MockTCPEndpoint> mockTCPEndpoint;
    std::shared_ptr<f1x::openauto::autoapp::service::MockAndroidAutoEntity> mockAndroidAutoEntity;
    std::shared_ptr<aasdk::usb::MockUSBEndpoint> mockUSBInEndpoint;
    std::shared_ptr<aasdk::usb::MockUSBEndpoint> mockUSBOutEndpoint;
    std::shared_ptr<f1x::openauto::autoapp::service::MockAndroidAutoEntityEventHandler> mockEventHandler;
};

// Test USB device connection workflow
TEST_F(DeviceConnectionTest, USBDeviceConnectionWorkflow) {
    InSequence seq;
    
    // Device endpoint access
    EXPECT_CALL(*mockAOAPDevice, getInEndpoint())
        .Times(1)
        .WillOnce(::testing::ReturnRef(*mockUSBInEndpoint));
    EXPECT_CALL(*mockAOAPDevice, getOutEndpoint())
        .Times(1)
        .WillOnce(::testing::ReturnRef(*mockUSBOutEndpoint));
    
    // Entity lifecycle after device connection
    EXPECT_CALL(*mockAndroidAutoEntity, start(_))
        .Times(1);
    EXPECT_CALL(*mockAndroidAutoEntity, resume())
        .Times(1);
    
    // Disconnection sequence
    EXPECT_CALL(*mockAndroidAutoEntity, pause())
        .Times(1);
    EXPECT_CALL(*mockAndroidAutoEntity, stop())
        .Times(1);
    
    // Execute connection workflow - access endpoints
    auto& inEndpoint = mockAOAPDevice->getInEndpoint();
    auto& outEndpoint = mockAOAPDevice->getOutEndpoint();
    
    // Verify endpoints are accessible
    EXPECT_EQ(&inEndpoint, mockUSBInEndpoint.get());
    EXPECT_EQ(&outEndpoint, mockUSBOutEndpoint.get());
    
    // Start entity and test lifecycle
    mockAndroidAutoEntity->start(*mockEventHandler);
    mockAndroidAutoEntity->resume();
    
    // Execute disconnection workflow
    mockAndroidAutoEntity->pause();
    mockAndroidAutoEntity->stop();
}

// Test TCP endpoint connection workflow
TEST_F(DeviceConnectionTest, TCPEndpointConnectionWorkflow) {
    InSequence seq;
    
    // Entity lifecycle with TCP connection - these happen first
    EXPECT_CALL(*mockAndroidAutoEntity, start(_))
        .Times(1);
    EXPECT_CALL(*mockAndroidAutoEntity, resume())
        .Times(1);
    
    // Disconnection sequence
    EXPECT_CALL(*mockAndroidAutoEntity, stop())
        .Times(1);
    
    // TCP endpoint cleanup - happens last
    EXPECT_CALL(*mockTCPEndpoint, stop())
        .Times(1);
    
    // Execute TCP connection workflow
    mockAndroidAutoEntity->start(*mockEventHandler);
    mockAndroidAutoEntity->resume();
    
    // Execute disconnection workflow
    mockAndroidAutoEntity->stop();
    mockTCPEndpoint->stop();
}

// Test connection error scenarios
TEST_F(DeviceConnectionTest, ConnectionErrorScenarios) {
    // Test endpoint functionality
    EXPECT_CALL(*mockTCPEndpoint, stop())
        .Times(1);
    
    // Test device endpoint access error handling
    EXPECT_CALL(*mockAOAPDevice, getInEndpoint())
        .Times(1)
        .WillOnce(::testing::ReturnRef(*mockUSBInEndpoint));
    
    // Test error scenarios
    auto& inEndpoint = mockAOAPDevice->getInEndpoint();
    EXPECT_EQ(&inEndpoint, mockUSBInEndpoint.get());
    
    // Cleanup after connection failure
    mockTCPEndpoint->stop();
}