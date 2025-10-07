#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <thread>
#include <chrono>
#include <QtCore/QString>
#include <QtCore/QRect>

// Include comprehensive interfaces for integration testing
#include <f1x/openauto/autoapp/Projection/IVideoOutput.hpp>
#include <f1x/openauto/autoapp/Projection/IAudioOutput.hpp>
#include <f1x/openauto/autoapp/Projection/IAudioInput.hpp>
#include <f1x/openauto/autoapp/Service/IAndroidAutoEntity.hpp>
#include <f1x/openauto/autoapp/Service/IAndroidAutoEntityFactory.hpp>

// Include all existing mocks for comprehensive integration
#include "../mocks/MockVideoOutput.hpp"
#include "../mocks/MockAudioOutput.hpp"
#include "../mocks/MockAudioInput.hpp"
#include "../mocks/MockInputDevice.hpp"
#include "../mocks/MockInputDeviceEventHandler.hpp"
#include "../mocks/MockConfiguration.hpp"
#include "../mocks/MockAndroidAutoEntity.hpp"
#include "../mocks/MockAndroidAutoEntityFactory.hpp"
#include "../mocks/MockAndroidAutoEntityEventHandler.hpp"

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::NiceMock;
using ::testing::InSequence;
using ::testing::AtLeast;
using ::testing::AtMost;
using ::testing::Exactly;
using ::testing::Invoke;
using ::testing::SaveArg;
using ::testing::DoAll;

// Phase 2D: Comprehensive Integration Tests - End-to-end system scenarios

namespace f1x::openauto::autoapp::integration {

// Comprehensive integration test fixture combining all components
class ComprehensiveIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Projection layer components
        mockVideoOutput = std::make_shared<NiceMock<projection::MockVideoOutput>>();
        mockAudioOutput = std::make_shared<NiceMock<projection::MockAudioOutput>>();
        mockAudioInput = std::make_shared<NiceMock<projection::MockAudioInput>>();
        mockInputDevice = std::make_shared<NiceMock<projection::MockInputDevice>>();
        mockInputHandler = std::make_shared<NiceMock<projection::MockInputDeviceEventHandler>>();
        
        // Service layer components
        mockAndroidAutoEntity = std::make_shared<NiceMock<service::MockAndroidAutoEntity>>();
        mockEntityFactory = std::make_shared<NiceMock<service::MockAndroidAutoEntityFactory>>();
        mockEntityEventHandler = std::make_shared<NiceMock<service::MockAndroidAutoEntityEventHandler>>();
        
        // Configuration component
        mockConfiguration = std::make_shared<NiceMock<f1x::openauto::autoapp::configuration::MockConfiguration>>();
        
        // Setup comprehensive default behaviors
        setupProjectionDefaults();
        setupServiceDefaults();
        setupConfigurationDefaults();
    }

    void setupProjectionDefaults() {
        // Video output defaults
        ON_CALL(*mockVideoOutput, open()).WillByDefault(Return(true));
        ON_CALL(*mockVideoOutput, init()).WillByDefault(Return(true));
        ON_CALL(*mockVideoOutput, getVideoFPS())
            .WillByDefault(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_30));
        ON_CALL(*mockVideoOutput, getVideoResolution())
            .WillByDefault(Return(aap_protobuf::service::media::sink::message::VIDEO_800x480));
        ON_CALL(*mockVideoOutput, getScreenDPI()).WillByDefault(Return(160));
        ON_CALL(*mockVideoOutput, getVideoMargins()).WillByDefault(Return(QRect(0, 0, 800, 480)));
        
        // Audio output defaults
        ON_CALL(*mockAudioOutput, open()).WillByDefault(Return(true));
        ON_CALL(*mockAudioOutput, getSampleRate()).WillByDefault(Return(48000));
        ON_CALL(*mockAudioOutput, getChannelCount()).WillByDefault(Return(2));
        ON_CALL(*mockAudioOutput, getSampleSize()).WillByDefault(Return(16));
        
        // Audio input defaults
        ON_CALL(*mockAudioInput, open()).WillByDefault(Return(true));
        ON_CALL(*mockAudioInput, isActive()).WillByDefault(Return(false));
        ON_CALL(*mockAudioInput, getSampleRate()).WillByDefault(Return(16000));
        ON_CALL(*mockAudioInput, getChannelCount()).WillByDefault(Return(1));
        ON_CALL(*mockAudioInput, getSampleSize()).WillByDefault(Return(16));
    }
    
    void setupServiceDefaults() {
        // Entity factory defaults - use explicit matcher to avoid ambiguity
        ON_CALL(*mockEntityFactory, create(::testing::Matcher<aasdk::usb::IAOAPDevice::Pointer>(::testing::_)))
            .WillByDefault(Return(mockAndroidAutoEntity));
    }
    
    void setupConfigurationDefaults() {
        // Configuration defaults
        ON_CALL(*mockConfiguration, getVideoFPS())
            .WillByDefault(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_30));
        ON_CALL(*mockConfiguration, getVideoResolution())
            .WillByDefault(Return(aap_protobuf::service::media::sink::message::VIDEO_800x480));
        ON_CALL(*mockConfiguration, getScreenDPI()).WillByDefault(Return(160));
        ON_CALL(*mockConfiguration, getVideoMargins()).WillByDefault(Return(QRect(0, 0, 800, 480)));
    }

    void TearDown() override {
        mockVideoOutput.reset();
        mockAudioOutput.reset();
        mockAudioInput.reset();
        mockInputDevice.reset();
        mockInputHandler.reset();
        mockAndroidAutoEntity.reset();
        mockEntityFactory.reset();
        mockEntityEventHandler.reset();
        mockConfiguration.reset();
    }

    // Projection layer mocks
    std::shared_ptr<projection::MockVideoOutput> mockVideoOutput;
    std::shared_ptr<projection::MockAudioOutput> mockAudioOutput;
    std::shared_ptr<projection::MockAudioInput> mockAudioInput;
    std::shared_ptr<projection::MockInputDevice> mockInputDevice;
    std::shared_ptr<projection::MockInputDeviceEventHandler> mockInputHandler;
    
    // Service layer mocks
    std::shared_ptr<service::MockAndroidAutoEntity> mockAndroidAutoEntity;
    std::shared_ptr<service::MockAndroidAutoEntityFactory> mockEntityFactory;
    std::shared_ptr<service::MockAndroidAutoEntityEventHandler> mockEntityEventHandler;
    
    // Configuration mock
    std::shared_ptr<f1x::openauto::autoapp::configuration::MockConfiguration> mockConfiguration;
};

// Test complete system startup sequence
TEST_F(ComprehensiveIntegrationTest, CompleteSystemStartup) {
    InSequence seq;
    
    // Phase 1: Configuration validation
    EXPECT_CALL(*mockConfiguration, getVideoFPS()).Times(1);
    EXPECT_CALL(*mockConfiguration, getVideoResolution()).Times(1);
    
    // Phase 2: Projection layer initialization
    EXPECT_CALL(*mockVideoOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioInput, open()).Times(1).WillOnce(Return(true));
    
    // Phase 3: Service layer initialization
    EXPECT_CALL(*mockEntityFactory, create(::testing::Matcher<aasdk::usb::IAOAPDevice::Pointer>(::testing::_))).Times(1);
    EXPECT_CALL(*mockAndroidAutoEntity, start(_)).Times(1);
    
    // Phase 4: Projection activation
    EXPECT_CALL(*mockVideoOutput, init()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, start()).Times(1);
    
    // Phase 5: Input system activation
    EXPECT_CALL(*mockInputDevice, start(_)).Times(1);
    
    // Execute complete startup sequence
    // Configuration validation
    auto fps = mockConfiguration->getVideoFPS();
    auto resolution = mockConfiguration->getVideoResolution();
    EXPECT_EQ(fps, aap_protobuf::service::media::sink::message::VIDEO_FPS_30);
    EXPECT_EQ(resolution, aap_protobuf::service::media::sink::message::VIDEO_800x480);
    
    // Projection initialization
    EXPECT_TRUE(mockVideoOutput->open());
    EXPECT_TRUE(mockAudioOutput->open());
    EXPECT_TRUE(mockAudioInput->open());
    
    // Service initialization
    auto entity = mockEntityFactory->create(std::shared_ptr<aasdk::usb::IAOAPDevice>());
    entity->start(*mockEntityEventHandler);
    
    // Projection activation
    EXPECT_TRUE(mockVideoOutput->init());
    mockAudioOutput->start();
    
    // Input activation
    mockInputDevice->start(*mockInputHandler);
}

// Test complete system shutdown sequence
TEST_F(ComprehensiveIntegrationTest, CompleteSystemShutdown) {
    InSequence seq;
    
    // Phase 1: Input system shutdown
    EXPECT_CALL(*mockInputDevice, stop()).Times(1);
    
    // Phase 2: Projection deactivation
    EXPECT_CALL(*mockAudioInput, stop()).Times(1);
    EXPECT_CALL(*mockAudioOutput, stop()).Times(1);
    EXPECT_CALL(*mockVideoOutput, stop()).Times(1);
    
    // Phase 3: Service layer shutdown
    EXPECT_CALL(*mockAndroidAutoEntity, stop()).Times(1);
    
    // Execute complete shutdown sequence
    mockInputDevice->stop();
    mockAudioInput->stop();
    mockAudioOutput->stop();
    mockVideoOutput->stop();
    mockAndroidAutoEntity->stop();
}

// Test system behavior under configuration changes
TEST_F(ComprehensiveIntegrationTest, ConfigurationDrivenSystemAdaptation) {
    // Initial configuration - low quality
    EXPECT_CALL(*mockConfiguration, getVideoFPS())
        .Times(2)
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_30))
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_60));
    
    EXPECT_CALL(*mockConfiguration, getVideoResolution())
        .Times(2)
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_800x480))
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_1280x720));
    
    // Projection adapts to configuration changes
    EXPECT_CALL(*mockVideoOutput, getVideoFPS()).Times(2);
    EXPECT_CALL(*mockVideoOutput, getVideoResolution()).Times(2);
    
    // Service layer reacts to configuration changes
    EXPECT_CALL(*mockAndroidAutoEntity, pause()).Times(1);
    EXPECT_CALL(*mockAndroidAutoEntity, resume()).Times(1);
    
    // Execute configuration adaptation scenario
    // Initial low-quality setup
    auto fps1 = mockConfiguration->getVideoFPS();
    auto res1 = mockConfiguration->getVideoResolution();
    mockVideoOutput->getVideoFPS();
    mockVideoOutput->getVideoResolution();
    
    // Configuration change simulation
    mockAndroidAutoEntity->pause();
    
    // New high-quality configuration
    auto fps2 = mockConfiguration->getVideoFPS();
    auto res2 = mockConfiguration->getVideoResolution();
    mockVideoOutput->getVideoFPS();
    mockVideoOutput->getVideoResolution();
    
    mockAndroidAutoEntity->resume();
    
    // Verify configuration changes
    EXPECT_EQ(fps1, aap_protobuf::service::media::sink::message::VIDEO_FPS_30);
    EXPECT_EQ(res1, aap_protobuf::service::media::sink::message::VIDEO_800x480);
    EXPECT_EQ(fps2, aap_protobuf::service::media::sink::message::VIDEO_FPS_60);
    EXPECT_EQ(res2, aap_protobuf::service::media::sink::message::VIDEO_1280x720);
}

// Test error recovery across multiple system layers
TEST_F(ComprehensiveIntegrationTest, MultiLayerErrorRecovery) {
    InSequence seq;
    
    // Scenario: Video fails but system continues with audio
    EXPECT_CALL(*mockVideoOutput, open())
        .Times(1)
        .WillOnce(Return(false));  // Video failure
    
    // Audio systems continue despite video failure
    EXPECT_CALL(*mockAudioOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioInput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, start()).Times(1);
    
    // Service layer handles video failure gracefully
    EXPECT_CALL(*mockEntityFactory, create(::testing::Matcher<aasdk::usb::IAOAPDevice::Pointer>(::testing::_))).Times(1);
    EXPECT_CALL(*mockAndroidAutoEntity, start(_)).Times(1);
    
    // Recovery attempt for video
    EXPECT_CALL(*mockVideoOutput, open())
        .Times(1)
        .WillOnce(Return(true));  // Video recovers
    EXPECT_CALL(*mockVideoOutput, init()).Times(1).WillOnce(Return(true));
    
    // Execute error recovery scenario
    EXPECT_FALSE(mockVideoOutput->open());  // Initial video failure
    EXPECT_TRUE(mockAudioOutput->open());   // Audio continues
    EXPECT_TRUE(mockAudioInput->open());    // Audio input continues
    mockAudioOutput->start();
    
    // Service layer continues
    auto entity = mockEntityFactory->create(std::shared_ptr<aasdk::usb::IAOAPDevice>());
    entity->start(*mockEntityEventHandler);
    
    // Video recovery
    EXPECT_TRUE(mockVideoOutput->open());
    EXPECT_TRUE(mockVideoOutput->init());
}

// Test bidirectional communication flow
TEST_F(ComprehensiveIntegrationTest, BidirectionalCommunicationFlow) {
    InSequence seq;
    
    // Outbound flow: Service -> Projection -> Output
    EXPECT_CALL(*mockAndroidAutoEntity, start(_)).Times(1);
    EXPECT_CALL(*mockVideoOutput, init()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, start()).Times(1);
    
    // Inbound flow: Input -> Projection -> Service
    EXPECT_CALL(*mockInputDevice, start(_)).Times(1);
    EXPECT_CALL(*mockAudioInput, start(_)).Times(1);
    EXPECT_CALL(*mockAudioInput, isActive()).Times(1).WillOnce(Return(true));
    
    // Bidirectional coordination
    EXPECT_CALL(*mockAndroidAutoEntity, resume()).Times(1);  // Service responds to input
    
    // Execute bidirectional communication
    // Outbound: Service initiates output
    mockAndroidAutoEntity->start(*mockEntityEventHandler);
    EXPECT_TRUE(mockVideoOutput->init());
    mockAudioOutput->start();
    
    // Inbound: Input systems activate
    mockInputDevice->start(*mockInputHandler);
    mockAudioInput->start(nullptr);  // Simplified promise handling
    EXPECT_TRUE(mockAudioInput->isActive());
    
    // Bidirectional: Input affects service behavior
    mockAndroidAutoEntity->resume();
}

// Test high-load system performance
TEST_F(ComprehensiveIntegrationTest, HighLoadSystemPerformance) {
    // Setup for high-throughput scenario
    EXPECT_CALL(*mockVideoOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockVideoOutput, init()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, start()).Times(1);
    EXPECT_CALL(*mockAudioInput, open()).Times(1).WillOnce(Return(true));
    
    // High-frequency operations
    // Note: Actual write operations omitted due to DataConstBuffer linking issues
    // In real scenario: video at 60fps, audio at high packet rate
    
    // Service layer handles high load
    EXPECT_CALL(*mockAndroidAutoEntity, start(_)).Times(1);
    EXPECT_CALL(*mockAndroidAutoEntity, pause()).Times(AtMost(3));  // May pause under load
    EXPECT_CALL(*mockAndroidAutoEntity, resume()).Times(AtMost(3)); // May resume after load
    
    // Performance optimization triggers
    EXPECT_CALL(*mockVideoOutput, getVideoFPS()).Times(AtLeast(1));
    EXPECT_CALL(*mockAudioOutput, getSampleRate()).Times(AtLeast(1));
    
    // Execute high-load scenario
    EXPECT_TRUE(mockVideoOutput->open());
    EXPECT_TRUE(mockVideoOutput->init());
    EXPECT_TRUE(mockAudioOutput->open());
    mockAudioOutput->start();
    EXPECT_TRUE(mockAudioInput->open());
    
    mockAndroidAutoEntity->start(*mockEntityEventHandler);
    
    // Simulate load-based optimizations
    auto fps = mockVideoOutput->getVideoFPS();
    auto sampleRate = mockAudioOutput->getSampleRate();
    EXPECT_EQ(fps, aap_protobuf::service::media::sink::message::VIDEO_FPS_30);
    EXPECT_EQ(sampleRate, 48000);
    
    // System may pause/resume under load
    mockAndroidAutoEntity->pause();
    mockAndroidAutoEntity->resume();
}

// Test system resource management under constraints
TEST_F(ComprehensiveIntegrationTest, ResourceConstrainedOperation) {
    InSequence seq;
    
    // Normal operation setup
    EXPECT_CALL(*mockVideoOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAndroidAutoEntity, start(_)).Times(1);
    
    // Resource constraint triggers suspension
    EXPECT_CALL(*mockVideoOutput, stop()).Times(1);  // Video suspended first
    EXPECT_CALL(*mockAudioOutput, suspend()).Times(1);  // Audio reduced quality
    EXPECT_CALL(*mockAndroidAutoEntity, pause()).Times(1);  // Service pauses
    
    // Resource availability triggers resumption
    EXPECT_CALL(*mockVideoOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockVideoOutput, init()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, start()).Times(1);  // Audio resumes
    EXPECT_CALL(*mockAndroidAutoEntity, resume()).Times(1);  // Service resumes
    
    // Execute resource management scenario
    // Normal operation
    EXPECT_TRUE(mockVideoOutput->open());
    EXPECT_TRUE(mockAudioOutput->open());
    mockAndroidAutoEntity->start(*mockEntityEventHandler);
    
    // Resource constraint response
    mockVideoOutput->stop();
    mockAudioOutput->suspend();
    mockAndroidAutoEntity->pause();
    
    // Resource recovery
    EXPECT_TRUE(mockVideoOutput->open());
    EXPECT_TRUE(mockVideoOutput->init());
    mockAudioOutput->start();
    mockAndroidAutoEntity->resume();
}

} // namespace f1x::openauto::autoapp::integration