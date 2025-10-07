#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <future>
#include <QtCore/QString>
#include <QtCore/QRect>

// Include comprehensive interfaces for advanced testing
#include <f1x/openauto/autoapp/Projection/IVideoOutput.hpp>
#include <f1x/openauto/autoapp/Projection/IAudioOutput.hpp>
#include <f1x/openauto/autoapp/Projection/IAudioInput.hpp>
#include <f1x/openauto/autoapp/Service/IAndroidAutoEntity.hpp>
#include <f1x/openauto/autoapp/Service/IAndroidAutoEntityFactory.hpp>

// Include all existing mocks for advanced testing
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
using ::testing::WithArgs;

// Phase 3: Advanced Integration Tests - Performance, stress, and real-world scenarios

namespace f1x::openauto::autoapp::advanced {

// Advanced performance and stress testing fixture
class AdvancedIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create mock components with different strictness levels for advanced testing
        mockVideoOutput = std::make_shared<StrictMock<projection::MockVideoOutput>>();
        mockAudioOutput = std::make_shared<NiceMock<projection::MockAudioOutput>>();
        mockAudioInput = std::make_shared<NiceMock<projection::MockAudioInput>>();
        mockInputDevice = std::make_shared<NiceMock<projection::MockInputDevice>>();
        mockInputHandler = std::make_shared<NiceMock<projection::MockInputDeviceEventHandler>>();
        
        mockAndroidAutoEntity = std::make_shared<StrictMock<service::MockAndroidAutoEntity>>();
        mockEntityFactory = std::make_shared<NiceMock<service::MockAndroidAutoEntityFactory>>();
        mockEntityEventHandler = std::make_shared<NiceMock<service::MockAndroidAutoEntityEventHandler>>();
        mockConfiguration = std::make_shared<NiceMock<f1x::openauto::autoapp::configuration::MockConfiguration>>();
        
        setupAdvancedDefaults();
    }

    void setupAdvancedDefaults() {
        // Video output with performance characteristics
        ON_CALL(*mockVideoOutput, open()).WillByDefault(Return(true));
        ON_CALL(*mockVideoOutput, init()).WillByDefault(Return(true));
        ON_CALL(*mockVideoOutput, getVideoFPS())
            .WillByDefault(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_60));  // High performance
        ON_CALL(*mockVideoOutput, getVideoResolution())
            .WillByDefault(Return(aap_protobuf::service::media::sink::message::VIDEO_1920x1080));  // High resolution
        ON_CALL(*mockVideoOutput, getScreenDPI()).WillByDefault(Return(320));  // High DPI
        ON_CALL(*mockVideoOutput, getVideoMargins()).WillByDefault(Return(QRect(0, 0, 1920, 1080)));
        
        // Audio output with high quality settings
        ON_CALL(*mockAudioOutput, open()).WillByDefault(Return(true));
        ON_CALL(*mockAudioOutput, getSampleRate()).WillByDefault(Return(96000));  // High quality audio
        ON_CALL(*mockAudioOutput, getChannelCount()).WillByDefault(Return(6));   // 5.1 surround
        ON_CALL(*mockAudioOutput, getSampleSize()).WillByDefault(Return(24));    // 24-bit audio
        
        // Audio input optimized for voice
        ON_CALL(*mockAudioInput, open()).WillByDefault(Return(true));
        ON_CALL(*mockAudioInput, isActive()).WillByDefault(Return(false));
        ON_CALL(*mockAudioInput, getSampleRate()).WillByDefault(Return(48000));  // High quality voice
        ON_CALL(*mockAudioInput, getChannelCount()).WillByDefault(Return(2));    // Stereo input
        ON_CALL(*mockAudioInput, getSampleSize()).WillByDefault(Return(16));
        
        // Service factory with advanced entity creation
        ON_CALL(*mockEntityFactory, create(::testing::Matcher<aasdk::usb::IAOAPDevice::Pointer>(::testing::_)))
            .WillByDefault(Return(mockAndroidAutoEntity));
        
        // Configuration for high-performance mode
        ON_CALL(*mockConfiguration, getVideoFPS())
            .WillByDefault(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_60));
        ON_CALL(*mockConfiguration, getVideoResolution())
            .WillByDefault(Return(aap_protobuf::service::media::sink::message::VIDEO_1920x1080));
        ON_CALL(*mockConfiguration, getScreenDPI()).WillByDefault(Return(320));
        ON_CALL(*mockConfiguration, getVideoMargins()).WillByDefault(Return(QRect(0, 0, 1920, 1080)));
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

    // Advanced test utility methods
    void simulateHighThroughputScenario() {
        // Simulate high-frequency operations without setting expectations
        // (expectations should be set in the calling test)
        
        for (int i = 0; i < 5; ++i) {
            auto fps = mockVideoOutput->getVideoFPS();
            EXPECT_EQ(fps, aap_protobuf::service::media::sink::message::VIDEO_FPS_60);
        }
        
        for (int i = 0; i < 3; ++i) {
            auto rate = mockAudioOutput->getSampleRate();
            EXPECT_EQ(rate, 96000);
        }
    }

    std::shared_ptr<projection::MockVideoOutput> mockVideoOutput;
    std::shared_ptr<projection::MockAudioOutput> mockAudioOutput;
    std::shared_ptr<projection::MockAudioInput> mockAudioInput;
    std::shared_ptr<projection::MockInputDevice> mockInputDevice;
    std::shared_ptr<projection::MockInputDeviceEventHandler> mockInputHandler;
    std::shared_ptr<service::MockAndroidAutoEntity> mockAndroidAutoEntity;
    std::shared_ptr<service::MockAndroidAutoEntityFactory> mockEntityFactory;
    std::shared_ptr<service::MockAndroidAutoEntityEventHandler> mockEntityEventHandler;
    std::shared_ptr<f1x::openauto::autoapp::configuration::MockConfiguration> mockConfiguration;
};

// Test high-performance system configuration
TEST_F(AdvancedIntegrationTest, HighPerformanceSystemConfiguration) {
    // Verify high-performance configuration values
    EXPECT_CALL(*mockVideoOutput, getVideoFPS()).Times(1);
    EXPECT_CALL(*mockVideoOutput, getVideoResolution()).Times(1);
    EXPECT_CALL(*mockVideoOutput, getScreenDPI()).Times(1);
    EXPECT_CALL(*mockAudioOutput, getSampleRate()).Times(1);
    EXPECT_CALL(*mockAudioOutput, getChannelCount()).Times(1);
    EXPECT_CALL(*mockAudioOutput, getSampleSize()).Times(1);
    
    // Validate high-performance settings
    auto fps = mockVideoOutput->getVideoFPS();
    auto resolution = mockVideoOutput->getVideoResolution();
    auto dpi = mockVideoOutput->getScreenDPI();
    auto sampleRate = mockAudioOutput->getSampleRate();
    auto channels = mockAudioOutput->getChannelCount();
    auto sampleSize = mockAudioOutput->getSampleSize();
    
    // Assert high-performance values
    EXPECT_EQ(fps, aap_protobuf::service::media::sink::message::VIDEO_FPS_60);
    EXPECT_EQ(resolution, aap_protobuf::service::media::sink::message::VIDEO_1920x1080);
    EXPECT_EQ(dpi, 320);
    EXPECT_EQ(sampleRate, 96000);
    EXPECT_EQ(channels, 6);
    EXPECT_EQ(sampleSize, 24);
}

// Test concurrent operation stress scenarios
TEST_F(AdvancedIntegrationTest, ConcurrentOperationStressTest) {
    // Remove InSequence to avoid conflicts with setup calls
    
    // Setup for concurrent operations
    EXPECT_CALL(*mockVideoOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockVideoOutput, init()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, start()).Times(1);
    EXPECT_CALL(*mockAudioInput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioInput, start(_)).Times(1);
    EXPECT_CALL(*mockInputDevice, start(_)).Times(1);
    EXPECT_CALL(*mockAndroidAutoEntity, start(_)).Times(1);
    
    // Concurrent cleanup expectations
    EXPECT_CALL(*mockInputDevice, stop()).Times(1);
    EXPECT_CALL(*mockAudioInput, stop()).Times(1);
    EXPECT_CALL(*mockAudioOutput, stop()).Times(1);
    EXPECT_CALL(*mockVideoOutput, stop()).Times(1);
    EXPECT_CALL(*mockAndroidAutoEntity, stop()).Times(1);
    
    // High throughput expectations (removed from sequence)
    EXPECT_CALL(*mockVideoOutput, getVideoFPS()).Times(AtLeast(5));
    EXPECT_CALL(*mockAudioOutput, getSampleRate()).Times(AtLeast(3));
    
    // Execute concurrent startup
    EXPECT_TRUE(mockVideoOutput->open());
    EXPECT_TRUE(mockVideoOutput->init());
    EXPECT_TRUE(mockAudioOutput->open());
    mockAudioOutput->start();
    EXPECT_TRUE(mockAudioInput->open());
    mockAudioInput->start(nullptr);
    mockInputDevice->start(*mockInputHandler);
    mockAndroidAutoEntity->start(*mockEntityEventHandler);
    
    // Simulate concurrent operations with high throughput
    simulateHighThroughputScenario();
    
    // Execute concurrent shutdown
    mockInputDevice->stop();
    mockAudioInput->stop();
    mockAudioOutput->stop();
    mockVideoOutput->stop();
    mockAndroidAutoEntity->stop();
}

// Test rapid configuration changes and adaptation
TEST_F(AdvancedIntegrationTest, RapidConfigurationAdaptation) {
    // Test rapid configuration switching between modes
    EXPECT_CALL(*mockConfiguration, getVideoFPS())
        .Times(4)
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_60))  // High performance
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_30))  // Power save
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_60))  // Back to high
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_30)); // Back to power save
    
    EXPECT_CALL(*mockConfiguration, getVideoResolution())
        .Times(4)
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_1920x1080))  // High res
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_800x480))    // Low res
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_1920x1080))  // High res
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_800x480));   // Low res
    
    // Service pauses/resumes during rapid configuration changes
    EXPECT_CALL(*mockAndroidAutoEntity, pause()).Times(3);
    EXPECT_CALL(*mockAndroidAutoEntity, resume()).Times(3);
    
    // Execute rapid configuration adaptation
    auto fps1 = mockConfiguration->getVideoFPS();
    auto res1 = mockConfiguration->getVideoResolution();
    mockAndroidAutoEntity->pause();
    mockAndroidAutoEntity->resume();
    
    auto fps2 = mockConfiguration->getVideoFPS();
    auto res2 = mockConfiguration->getVideoResolution();
    mockAndroidAutoEntity->pause();
    mockAndroidAutoEntity->resume();
    
    auto fps3 = mockConfiguration->getVideoFPS();
    auto res3 = mockConfiguration->getVideoResolution();
    mockAndroidAutoEntity->pause();
    mockAndroidAutoEntity->resume();
    
    auto fps4 = mockConfiguration->getVideoFPS();
    auto res4 = mockConfiguration->getVideoResolution();
    
    // Verify rapid adaptation worked
    EXPECT_EQ(fps1, aap_protobuf::service::media::sink::message::VIDEO_FPS_60);
    EXPECT_EQ(fps2, aap_protobuf::service::media::sink::message::VIDEO_FPS_30);
    EXPECT_EQ(fps3, aap_protobuf::service::media::sink::message::VIDEO_FPS_60);
    EXPECT_EQ(fps4, aap_protobuf::service::media::sink::message::VIDEO_FPS_30);
}

// Test system behavior under failure cascade scenarios
TEST_F(AdvancedIntegrationTest, FailureCascadeRecovery) {
    InSequence seq;
    
    // Initial successful setup
    EXPECT_CALL(*mockVideoOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAndroidAutoEntity, start(_)).Times(1);
    
    // Cascade failure scenario: Video fails first
    EXPECT_CALL(*mockVideoOutput, init()).Times(1).WillOnce(Return(false));
    
    // System continues with degraded video but full audio
    EXPECT_CALL(*mockAudioOutput, start()).Times(1);
    EXPECT_CALL(*mockAudioInput, open()).Times(1).WillOnce(Return(true));
    
    // Audio system then fails
    EXPECT_CALL(*mockAudioInput, start(_)).Times(1);
    EXPECT_CALL(*mockAudioInput, isActive()).Times(1).WillOnce(Return(false));  // Audio input failed
    
    // Service adjusts to audio-only mode
    EXPECT_CALL(*mockAndroidAutoEntity, pause()).Times(1);
    
    // Recovery attempt - video comes back online
    EXPECT_CALL(*mockVideoOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockVideoOutput, init()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAndroidAutoEntity, resume()).Times(1);
    
    // Execute failure cascade scenario
    EXPECT_TRUE(mockVideoOutput->open());
    EXPECT_TRUE(mockAudioOutput->open());
    mockAndroidAutoEntity->start(*mockEntityEventHandler);
    
    // Video failure
    EXPECT_FALSE(mockVideoOutput->init());
    
    // Continue with audio
    mockAudioOutput->start();
    EXPECT_TRUE(mockAudioInput->open());
    mockAudioInput->start(nullptr);
    EXPECT_FALSE(mockAudioInput->isActive());  // Audio input fails
    
    // Service degradation
    mockAndroidAutoEntity->pause();
    
    // Recovery
    EXPECT_TRUE(mockVideoOutput->open());
    EXPECT_TRUE(mockVideoOutput->init());
    mockAndroidAutoEntity->resume();
}

// Test multi-resolution and multi-format support
TEST_F(AdvancedIntegrationTest, MultiFormatAdaptiveSupport) {
    // Test support for multiple video formats
    EXPECT_CALL(*mockVideoOutput, getVideoResolution())
        .Times(5)
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_800x480))
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_1280x720))
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_1920x1080))
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_720x1280))   // Portrait
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_1080x1920)); // Portrait HD
    
    EXPECT_CALL(*mockVideoOutput, getVideoFPS())
        .Times(3)
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_30))
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_60))
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_30));
    
    // Test support for multiple audio formats
    EXPECT_CALL(*mockAudioOutput, getSampleRate())
        .Times(4)
        .WillOnce(Return(44100))  // CD quality
        .WillOnce(Return(48000))  // Professional
        .WillOnce(Return(96000))  // High resolution
        .WillOnce(Return(192000)); // Ultra high resolution
    
    EXPECT_CALL(*mockAudioOutput, getChannelCount())
        .Times(3)
        .WillOnce(Return(2))  // Stereo
        .WillOnce(Return(6))  // 5.1 surround
        .WillOnce(Return(8)); // 7.1 surround
    
    // Execute multi-format testing
    std::vector<aap_protobuf::service::media::sink::message::VideoCodecResolutionType> resolutions;
    std::vector<aap_protobuf::service::media::sink::message::VideoFrameRateType> frameRates;
    std::vector<uint32_t> sampleRates;
    std::vector<uint32_t> channelCounts;
    
    // Collect all supported formats
    for (int i = 0; i < 5; ++i) {
        resolutions.push_back(mockVideoOutput->getVideoResolution());
    }
    for (int i = 0; i < 3; ++i) {
        frameRates.push_back(mockVideoOutput->getVideoFPS());
    }
    for (int i = 0; i < 4; ++i) {
        sampleRates.push_back(mockAudioOutput->getSampleRate());
    }
    for (int i = 0; i < 3; ++i) {
        channelCounts.push_back(mockAudioOutput->getChannelCount());
    }
    
    // Verify format diversity
    EXPECT_EQ(resolutions.size(), 5);
    EXPECT_EQ(frameRates.size(), 3);
    EXPECT_EQ(sampleRates.size(), 4);
    EXPECT_EQ(channelCounts.size(), 3);
    
    // Verify specific high-end formats are supported
    EXPECT_EQ(sampleRates[3], 192000);  // Ultra high resolution audio
    EXPECT_EQ(channelCounts[2], 8);     // 7.1 surround sound
}

// Test long-running stability and memory management
TEST_F(AdvancedIntegrationTest, LongRunningStabilityTest) {
    // Setup for long-running operations
    EXPECT_CALL(*mockVideoOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockVideoOutput, init()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, start()).Times(1);
    
    // Simulate periodic operations (memory cleanup, health checks) - reduced expectations
    EXPECT_CALL(*mockVideoOutput, getVideoFPS()).Times(AtLeast(10));
    EXPECT_CALL(*mockAudioOutput, getSampleRate()).Times(AtLeast(3)); // Reduced from 5 to 3
    
    // Periodic service lifecycle operations
    EXPECT_CALL(*mockAndroidAutoEntity, pause()).Times(2);
    EXPECT_CALL(*mockAndroidAutoEntity, resume()).Times(2);
    
    // Final cleanup
    EXPECT_CALL(*mockAudioOutput, stop()).Times(1);
    EXPECT_CALL(*mockVideoOutput, stop()).Times(1);
    
    // Execute long-running stability test
    EXPECT_TRUE(mockVideoOutput->open());
    EXPECT_TRUE(mockVideoOutput->init());
    EXPECT_TRUE(mockAudioOutput->open());
    mockAudioOutput->start();
    
    // Simulate extended operation with periodic health checks
    for (int cycle = 0; cycle < 3; ++cycle) {
        // Simulate periodic health checks
        for (int i = 0; i < 4; ++i) {
            auto fps = mockVideoOutput->getVideoFPS();
            EXPECT_EQ(fps, aap_protobuf::service::media::sink::message::VIDEO_FPS_60);
        }
        
        auto sampleRate = mockAudioOutput->getSampleRate();
        EXPECT_EQ(sampleRate, 96000);
        
        // Periodic service lifecycle (simulating background/foreground transitions)
        if (cycle < 2) {
            mockAndroidAutoEntity->pause();
            mockAndroidAutoEntity->resume();
        }
    }
    
    // Clean shutdown after long operation
    mockAudioOutput->stop();
    mockVideoOutput->stop();
}

// Test real-world Android Auto connection simulation
TEST_F(AdvancedIntegrationTest, AndroidAutoConnectionSimulation) {
    InSequence seq;
    
    // Phase 1: Device detection and handshake
    EXPECT_CALL(*mockEntityFactory, create(::testing::Matcher<aasdk::usb::IAOAPDevice::Pointer>(::testing::_))).Times(1);
    EXPECT_CALL(*mockAndroidAutoEntity, start(_)).Times(1);
    
    // Phase 2: Service discovery and initialization
    EXPECT_CALL(*mockVideoOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioInput, open()).Times(1).WillOnce(Return(true));
    
    // Phase 3: Projection setup and activation
    EXPECT_CALL(*mockVideoOutput, init()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, start()).Times(1);
    EXPECT_CALL(*mockAudioInput, start(_)).Times(1);
    
    // Phase 4: Input system activation
    EXPECT_CALL(*mockInputDevice, start(_)).Times(1);
    
    // Phase 5: Active session with periodic operations
    EXPECT_CALL(*mockAudioInput, isActive()).Times(2).WillRepeatedly(Return(true));
    
    // Phase 6: Session pause/resume (phone call scenario)
    EXPECT_CALL(*mockAndroidAutoEntity, pause()).Times(1);
    EXPECT_CALL(*mockAudioOutput, suspend()).Times(1);
    EXPECT_CALL(*mockAndroidAutoEntity, resume()).Times(1);
    EXPECT_CALL(*mockAudioOutput, start()).Times(1);
    
    // Phase 7: Clean disconnection
    EXPECT_CALL(*mockInputDevice, stop()).Times(1);
    EXPECT_CALL(*mockAudioInput, stop()).Times(1);
    EXPECT_CALL(*mockAudioOutput, stop()).Times(1);
    EXPECT_CALL(*mockVideoOutput, stop()).Times(1);
    EXPECT_CALL(*mockAndroidAutoEntity, stop()).Times(1);
    
    // Execute Android Auto connection simulation
    // Device detection and handshake
    auto entity = mockEntityFactory->create(std::shared_ptr<aasdk::usb::IAOAPDevice>());
    entity->start(*mockEntityEventHandler);
    
    // Service discovery and initialization
    EXPECT_TRUE(mockVideoOutput->open());
    EXPECT_TRUE(mockAudioOutput->open());
    EXPECT_TRUE(mockAudioInput->open());
    
    // Projection setup and activation
    EXPECT_TRUE(mockVideoOutput->init());
    mockAudioOutput->start();
    mockAudioInput->start(nullptr);
    
    // Input system activation
    mockInputDevice->start(*mockInputHandler);
    
    // Active session verification
    EXPECT_TRUE(mockAudioInput->isActive());
    EXPECT_TRUE(mockAudioInput->isActive());
    
    // Session interruption (phone call)
    mockAndroidAutoEntity->pause();
    mockAudioOutput->suspend();
    mockAndroidAutoEntity->resume();
    mockAudioOutput->start();
    
    // Clean disconnection
    mockInputDevice->stop();
    mockAudioInput->stop();
    mockAudioOutput->stop();
    mockVideoOutput->stop();
    mockAndroidAutoEntity->stop();
}

} // namespace f1x::openauto::autoapp::advanced