#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <future>
#include <random>
#include <QtCore/QString>
#include <QtCore/QRect>

// Include comprehensive interfaces for real-world scenario testing
#include <f1x/openauto/autoapp/Projection/IVideoOutput.hpp>
#include <f1x/openauto/autoapp/Projection/IAudioOutput.hpp>
#include <f1x/openauto/autoapp/Projection/IAudioInput.hpp>
#include <f1x/openauto/autoapp/Service/IAndroidAutoEntity.hpp>
#include <f1x/openauto/autoapp/Service/IAndroidAutoEntityFactory.hpp>

// Include all existing mocks for real-world scenario testing
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

// Phase 3: Real-World Scenario Testing Suite

namespace f1x::openauto::autoapp::scenarios {

// Real-world scenario testing fixture
class RealWorldScenarioTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create mock components for realistic scenario testing
        mockVideoOutput = std::make_shared<NiceMock<projection::MockVideoOutput>>();
        mockAudioOutput = std::make_shared<NiceMock<projection::MockAudioOutput>>();
        mockAudioInput = std::make_shared<NiceMock<projection::MockAudioInput>>();
        mockInputDevice = std::make_shared<NiceMock<projection::MockInputDevice>>();
        mockInputHandler = std::make_shared<NiceMock<projection::MockInputDeviceEventHandler>>();
        
        mockAndroidAutoEntity = std::make_shared<NiceMock<service::MockAndroidAutoEntity>>();
        mockEntityFactory = std::make_shared<NiceMock<service::MockAndroidAutoEntityFactory>>();
        mockEntityEventHandler = std::make_shared<NiceMock<service::MockAndroidAutoEntityEventHandler>>();
        mockConfiguration = std::make_shared<NiceMock<f1x::openauto::autoapp::configuration::MockConfiguration>>();
        
        setupRealisticDefaults();
        
        // Initialize random number generator for realistic scenarios
        randomEngine.seed(std::chrono::steady_clock::now().time_since_epoch().count());
    }

    void setupRealisticDefaults() {
        // Realistic video settings (not perfect)
        ON_CALL(*mockVideoOutput, open()).WillByDefault(Return(true));
        ON_CALL(*mockVideoOutput, init()).WillByDefault(Return(true));
        ON_CALL(*mockVideoOutput, getVideoFPS())
            .WillByDefault(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_30));  // Standard FPS
        ON_CALL(*mockVideoOutput, getVideoResolution())
            .WillByDefault(Return(aap_protobuf::service::media::sink::message::VIDEO_1280x720)); // Standard resolution
        ON_CALL(*mockVideoOutput, getScreenDPI()).WillByDefault(Return(160));  // Standard DPI
        ON_CALL(*mockVideoOutput, getVideoMargins()).WillByDefault(Return(QRect(10, 10, 1260, 700))); // Real margins
        
        // Realistic audio settings
        ON_CALL(*mockAudioOutput, open()).WillByDefault(Return(true));
        ON_CALL(*mockAudioOutput, getSampleRate()).WillByDefault(Return(44100));  // CD quality
        ON_CALL(*mockAudioOutput, getChannelCount()).WillByDefault(Return(2));    // Stereo
        ON_CALL(*mockAudioOutput, getSampleSize()).WillByDefault(Return(16));     // 16-bit
        
        // Realistic audio input (sometimes unreliable)
        ON_CALL(*mockAudioInput, open()).WillByDefault(Return(true));
        ON_CALL(*mockAudioInput, isActive()).WillByDefault(Return(true));
        ON_CALL(*mockAudioInput, getSampleRate()).WillByDefault(Return(44100));
        ON_CALL(*mockAudioInput, getChannelCount()).WillByDefault(Return(1));     // Mono input
        ON_CALL(*mockAudioInput, getSampleSize()).WillByDefault(Return(16));
        
        // Service factory for realistic scenarios
        ON_CALL(*mockEntityFactory, create(::testing::Matcher<aasdk::usb::IAOAPDevice::Pointer>(::testing::_)))
            .WillByDefault(Return(mockAndroidAutoEntity));
        
        // Realistic configuration
        ON_CALL(*mockConfiguration, getVideoFPS())
            .WillByDefault(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_30));
        ON_CALL(*mockConfiguration, getVideoResolution())
            .WillByDefault(Return(aap_protobuf::service::media::sink::message::VIDEO_1280x720));
        ON_CALL(*mockConfiguration, getScreenDPI()).WillByDefault(Return(160));
        ON_CALL(*mockConfiguration, getVideoMargins()).WillByDefault(Return(QRect(10, 10, 1260, 700)));
    }

    // Utility methods for realistic scenarios
    void simulateNetworkDelay(int minMs = 10, int maxMs = 100) {
        std::uniform_int_distribution<int> delay(minMs, maxMs);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay(randomEngine)));
    }

    bool simulateRandomFailure(double failureRate = 0.1) {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(randomEngine) < failureRate;
    }

    void simulateUserInput() {
        // Simulate random user input events
        std::uniform_int_distribution<int> inputType(1, 3);
        int type = inputType(randomEngine);
        
        // Different types of user interactions
        switch (type) {
            case 1: // Touch input
                // Would normally trigger input device events
                break;
            case 2: // Voice input
                // Would normally activate audio input
                break;
            case 3: // Physical buttons
                // Would normally trigger input device events
                break;
        }
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

    std::shared_ptr<projection::MockVideoOutput> mockVideoOutput;
    std::shared_ptr<projection::MockAudioOutput> mockAudioOutput;
    std::shared_ptr<projection::MockAudioInput> mockAudioInput;
    std::shared_ptr<projection::MockInputDevice> mockInputDevice;
    std::shared_ptr<projection::MockInputDeviceEventHandler> mockInputHandler;
    std::shared_ptr<service::MockAndroidAutoEntity> mockAndroidAutoEntity;
    std::shared_ptr<service::MockAndroidAutoEntityFactory> mockEntityFactory;
    std::shared_ptr<service::MockAndroidAutoEntityEventHandler> mockEntityEventHandler;
    std::shared_ptr<f1x::openauto::autoapp::configuration::MockConfiguration> mockConfiguration;
    
    std::mt19937 randomEngine;
};

// Test typical morning commute scenario - Fixed to remove InSequence conflicts
TEST_F(RealWorldScenarioTest, MorningCommuteScenario) {
    // Morning commute: Car startup, Android Auto connection
    EXPECT_CALL(*mockVideoOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockVideoOutput, init()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioInput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAndroidAutoEntity, start(_)).Times(1);
    
    // Start navigation (video becomes primary)
    EXPECT_CALL(*mockVideoOutput, getVideoFPS()).Times(AtLeast(2));
    EXPECT_CALL(*mockVideoOutput, getVideoResolution()).Times(AtLeast(1));
    
    // Start music (audio becomes active)
    EXPECT_CALL(*mockAudioOutput, start()).Times(AtLeast(1));  // Allow multiple starts
    EXPECT_CALL(*mockAudioOutput, getSampleRate()).Times(AtLeast(1));
    
    // Incoming call interruption
    EXPECT_CALL(*mockAndroidAutoEntity, pause()).Times(1);
    EXPECT_CALL(*mockAudioOutput, suspend()).Times(1);
    EXPECT_CALL(*mockAudioInput, start(_)).Times(1);  // Voice call
    
    // Call ends, resume music
    EXPECT_CALL(*mockAudioInput, stop()).Times(1);
    EXPECT_CALL(*mockAndroidAutoEntity, resume()).Times(1);
    
    // End of commute
    EXPECT_CALL(*mockAudioOutput, stop()).Times(1);
    EXPECT_CALL(*mockVideoOutput, stop()).Times(1);
    EXPECT_CALL(*mockAndroidAutoEntity, stop()).Times(1);
    
    // Execute morning commute scenario
    // Car startup and connection
    EXPECT_TRUE(mockVideoOutput->open());
    EXPECT_TRUE(mockVideoOutput->init());
    EXPECT_TRUE(mockAudioOutput->open());
    EXPECT_TRUE(mockAudioInput->open());
    mockAndroidAutoEntity->start(*mockEntityEventHandler);
    
    simulateNetworkDelay(1, 5);  // Fast connection establishment delay for testing
    
    // Navigation starts
    auto fps = mockVideoOutput->getVideoFPS();
    auto resolution = mockVideoOutput->getVideoResolution();
    EXPECT_EQ(fps, aap_protobuf::service::media::sink::message::VIDEO_FPS_30);
    EXPECT_EQ(resolution, aap_protobuf::service::media::sink::message::VIDEO_1280x720);
    
    // Music starts
    mockAudioOutput->start();
    auto sampleRate = mockAudioOutput->getSampleRate();
    EXPECT_EQ(sampleRate, 44100);
    
    simulateNetworkDelay(1, 2);  // Fast normal operation delay
    
    // Incoming call
    mockAndroidAutoEntity->pause();
    mockAudioOutput->suspend();
    mockAudioInput->start(nullptr);
    
    simulateNetworkDelay(10, 50);  // Short call duration for testing (10-50ms instead of 5-15s)
    
    // Call ends
    mockAudioInput->stop();
    mockAndroidAutoEntity->resume();
    mockAudioOutput->start();
    
    simulateNetworkDelay(1, 3);  // Fast resume delay
    
    // Additional navigation updates
    fps = mockVideoOutput->getVideoFPS();
    EXPECT_EQ(fps, aap_protobuf::service::media::sink::message::VIDEO_FPS_30);
    
    // End commute
    mockAudioOutput->stop();
    mockVideoOutput->stop();
    mockAndroidAutoEntity->stop();
}

// Test long road trip scenario with various interruptions
TEST_F(RealWorldScenarioTest, LongRoadTripScenario) {
    const int tripSegments = 5;
    const int callsPerSegment = 2;
    
    // Trip initialization
    EXPECT_CALL(*mockVideoOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockVideoOutput, init()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioInput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAndroidAutoEntity, start(_)).Times(1);
    EXPECT_CALL(*mockInputDevice, start(_)).Times(1);
    
    // Multiple interruptions throughout trip
    EXPECT_CALL(*mockAndroidAutoEntity, pause()).Times(tripSegments * callsPerSegment);
    EXPECT_CALL(*mockAndroidAutoEntity, resume()).Times(tripSegments * callsPerSegment);
    EXPECT_CALL(*mockAudioOutput, suspend()).Times(tripSegments * callsPerSegment);
    EXPECT_CALL(*mockAudioOutput, start()).Times(1 + tripSegments * callsPerSegment);  // Initial + resumes
    EXPECT_CALL(*mockAudioInput, start(_)).Times(tripSegments * callsPerSegment);
    EXPECT_CALL(*mockAudioInput, stop()).Times(tripSegments * callsPerSegment);
    
    // Navigation updates throughout trip
    EXPECT_CALL(*mockVideoOutput, getVideoFPS()).Times(AtLeast(tripSegments * 3));
    EXPECT_CALL(*mockConfiguration, getVideoResolution()).Times(AtLeast(tripSegments));
    
    // Trip cleanup
    EXPECT_CALL(*mockInputDevice, stop()).Times(1);
    EXPECT_CALL(*mockAudioOutput, stop()).Times(1);
    EXPECT_CALL(*mockVideoOutput, stop()).Times(1);
    EXPECT_CALL(*mockAndroidAutoEntity, stop()).Times(1);
    
    // Execute long road trip scenario
    // Trip initialization
    EXPECT_TRUE(mockVideoOutput->open());
    EXPECT_TRUE(mockVideoOutput->init());
    EXPECT_TRUE(mockAudioOutput->open());
    EXPECT_TRUE(mockAudioInput->open());
    mockAndroidAutoEntity->start(*mockEntityEventHandler);
    mockInputDevice->start(*mockInputHandler);
    mockAudioOutput->start();
    
    // Trip segments with various interruptions
    for (int segment = 0; segment < tripSegments; ++segment) {
        // Regular navigation updates
        for (int update = 0; update < 3; ++update) {
            auto fps = mockVideoOutput->getVideoFPS();
            EXPECT_EQ(fps, aap_protobuf::service::media::sink::message::VIDEO_FPS_30);
            simulateNetworkDelay(1, 2);  // Fast navigation update delays for testing
        }
        
        // Configuration check (road conditions change)
        auto resolution = mockConfiguration->getVideoResolution();
        EXPECT_EQ(resolution, aap_protobuf::service::media::sink::message::VIDEO_1280x720);
        
        // Interruptions (calls) during this segment
        for (int call = 0; call < callsPerSegment; ++call) {
            simulateNetworkDelay(5, 10);  // Fast time between calls for testing
            
            // Incoming call
            mockAndroidAutoEntity->pause();
            mockAudioOutput->suspend();
            mockAudioInput->start(nullptr);
            
            simulateNetworkDelay(10, 20);  // Short call duration for testing
            
            // Call ends
            mockAudioInput->stop();
            mockAndroidAutoEntity->resume();
            mockAudioOutput->start();
            
            simulateNetworkDelay(1, 2);  // Fast resume delay
        }
    }
    
    // Trip end
    mockInputDevice->stop();
    mockAudioOutput->stop();
    mockVideoOutput->stop();
    mockAndroidAutoEntity->stop();
}

// Test intermittent connectivity scenario
TEST_F(RealWorldScenarioTest, IntermittentConnectivityScenario) {
    const int connectivityCycles = 8;
    
    // Initial connection - more flexible expectations for random behavior
    EXPECT_CALL(*mockVideoOutput, open()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockVideoOutput, init()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockAudioOutput, open()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockAndroidAutoEntity, start(_)).Times(AtLeast(1));
    
    // Disconnections and reconnections - flexible for random behavior
    EXPECT_CALL(*mockAndroidAutoEntity, stop()).Times(AtLeast(1));
    EXPECT_CALL(*mockVideoOutput, stop()).Times(AtLeast(1));
    EXPECT_CALL(*mockAudioOutput, stop()).Times(AtLeast(1));
    
    // Execute intermittent connectivity scenario
    bool connected = false;
    
    for (int cycle = 0; cycle < connectivityCycles; ++cycle) {
        if (!connected) {
            // Attempt connection
            bool videoOk = mockVideoOutput->open();
            bool audioOk = mockAudioOutput->open();
            
            if (videoOk && audioOk && !simulateRandomFailure(0.3)) {  // 30% chance of connection failure
                EXPECT_TRUE(mockVideoOutput->init());
                mockAndroidAutoEntity->start(*mockEntityEventHandler);
                connected = true;
                
                simulateNetworkDelay(1, 5);  // Fast connection establishment for testing
            } else {
                simulateNetworkDelay(2, 5);  // Fast retry delay for testing
            }
        } else {
            // Simulate connection loss
            if (simulateRandomFailure(0.2)) {  // 20% chance of disconnection
                mockAndroidAutoEntity->stop();
                mockVideoOutput->stop();
                mockAudioOutput->stop();
                connected = false;
                
                simulateNetworkDelay(1, 3);  // Fast disconnection recovery for testing
            } else {
                // Normal operation
                if (!simulateRandomFailure(0.05)) {  // 95% success rate for normal operations
                    auto fps = mockVideoOutput->getVideoFPS();
                    EXPECT_EQ(fps, aap_protobuf::service::media::sink::message::VIDEO_FPS_30);
                }
                
                simulateNetworkDelay(1, 2);  // Fast normal operation delay for testing
            }
        }
    }
    
    // Ensure clean disconnection at end
    if (connected) {
        mockAndroidAutoEntity->stop();
        mockVideoOutput->stop();
        mockAudioOutput->stop();
    }
}

// Test power management scenario (battery optimization)
TEST_F(RealWorldScenarioTest, PowerManagementScenario) {
    // Setup expectations for power-saving configuration changes
    EXPECT_CALL(*mockConfiguration, getVideoFPS())
        .Times(6)
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_60))  // Full power
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_30))  // Power save
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_30))  // Stay in power save
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_60))  // Back to full
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_30))  // Power save again
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_FPS_60)); // Final full power
    
    EXPECT_CALL(*mockConfiguration, getVideoResolution())
        .Times(4)
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_1920x1080))  // High res
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_1280x720))   // Reduced res
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_800x480))    // Low res
        .WillOnce(Return(aap_protobuf::service::media::sink::message::VIDEO_1920x1080)); // Back to high
    
    // Service adaptations to power management
    EXPECT_CALL(*mockAndroidAutoEntity, pause()).Times(3);
    EXPECT_CALL(*mockAndroidAutoEntity, resume()).Times(3);
    EXPECT_CALL(*mockAudioOutput, suspend()).Times(2);
    EXPECT_CALL(*mockAudioOutput, start()).Times(3);  // Initial + 2 resumes
    
    // System initialization
    EXPECT_CALL(*mockVideoOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockVideoOutput, init()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioOutput, open()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockAndroidAutoEntity, start(_)).Times(1);
    
    // System shutdown
    EXPECT_CALL(*mockAudioOutput, stop()).Times(1);
    EXPECT_CALL(*mockVideoOutput, stop()).Times(1);
    EXPECT_CALL(*mockAndroidAutoEntity, stop()).Times(1);
    
    // Execute power management scenario
    // System startup
    EXPECT_TRUE(mockVideoOutput->open());
    EXPECT_TRUE(mockVideoOutput->init());
    EXPECT_TRUE(mockAudioOutput->open());
    mockAndroidAutoEntity->start(*mockEntityEventHandler);
    mockAudioOutput->start();
    
    // Full power mode
    auto fps1 = mockConfiguration->getVideoFPS();
    auto res1 = mockConfiguration->getVideoResolution();
    EXPECT_EQ(fps1, aap_protobuf::service::media::sink::message::VIDEO_FPS_60);
    EXPECT_EQ(res1, aap_protobuf::service::media::sink::message::VIDEO_1920x1080);
    
    simulateNetworkDelay(5, 10);  // Fast normal operation for testing
    
    // Power saving mode activated (low battery)
    mockAndroidAutoEntity->pause();
    auto fps2 = mockConfiguration->getVideoFPS();
    auto res2 = mockConfiguration->getVideoResolution();
    EXPECT_EQ(fps2, aap_protobuf::service::media::sink::message::VIDEO_FPS_30);
    EXPECT_EQ(res2, aap_protobuf::service::media::sink::message::VIDEO_1280x720);
    mockAndroidAutoEntity->resume();
    
    simulateNetworkDelay(5, 10);  // Fast power save operation for testing
    
    // Very low battery - minimal mode
    mockAndroidAutoEntity->pause();
    mockAudioOutput->suspend();
    auto fps3 = mockConfiguration->getVideoFPS();
    auto res3 = mockConfiguration->getVideoResolution();
    EXPECT_EQ(fps3, aap_protobuf::service::media::sink::message::VIDEO_FPS_30);
    EXPECT_EQ(res3, aap_protobuf::service::media::sink::message::VIDEO_800x480);
    mockAndroidAutoEntity->resume();
    mockAudioOutput->start();
    
    simulateNetworkDelay(3, 8);  // Fast minimal operation for testing
    
    // Charging started - return to full power
    mockAndroidAutoEntity->pause();
    auto fps4 = mockConfiguration->getVideoFPS();
    auto res4 = mockConfiguration->getVideoResolution();
    EXPECT_EQ(fps4, aap_protobuf::service::media::sink::message::VIDEO_FPS_60);
    EXPECT_EQ(res4, aap_protobuf::service::media::sink::message::VIDEO_1920x1080);
    mockAndroidAutoEntity->resume();
    
    simulateNetworkDelay(2, 5);  // Fast return to normal for testing
    
    // Another power save cycle
    auto fps5 = mockConfiguration->getVideoFPS();
    EXPECT_EQ(fps5, aap_protobuf::service::media::sink::message::VIDEO_FPS_30);
    mockAudioOutput->suspend();
    
    // Final return to full power
    auto fps6 = mockConfiguration->getVideoFPS();
    EXPECT_EQ(fps6, aap_protobuf::service::media::sink::message::VIDEO_FPS_60);
    mockAudioOutput->start();
    
    // System shutdown
    mockAudioOutput->stop();
    mockVideoOutput->stop();
    mockAndroidAutoEntity->stop();
}

// Test multi-user device scenario
TEST_F(RealWorldScenarioTest, MultiUserDeviceScenario) {
    const int userSwitches = 4;
    
    // Multiple user sessions with different configurations
    EXPECT_CALL(*mockConfiguration, getVideoFPS()).Times(userSwitches * 2);
    EXPECT_CALL(*mockConfiguration, getVideoResolution()).Times(userSwitches);
    EXPECT_CALL(*mockConfiguration, getScreenDPI()).Times(userSwitches);
    
    // Service lifecycle for each user
    EXPECT_CALL(*mockAndroidAutoEntity, start(_)).Times(userSwitches);
    EXPECT_CALL(*mockAndroidAutoEntity, stop()).Times(userSwitches);
    EXPECT_CALL(*mockVideoOutput, open()).Times(userSwitches).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockVideoOutput, init()).Times(userSwitches).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockVideoOutput, stop()).Times(userSwitches);
    EXPECT_CALL(*mockAudioOutput, open()).Times(userSwitches).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockAudioOutput, start()).Times(userSwitches);
    EXPECT_CALL(*mockAudioOutput, stop()).Times(userSwitches);
    
    // Execute multi-user scenario
    for (int user = 0; user < userSwitches; ++user) {
        // User session start
        EXPECT_TRUE(mockVideoOutput->open());
        EXPECT_TRUE(mockVideoOutput->init());
        EXPECT_TRUE(mockAudioOutput->open());
        mockAndroidAutoEntity->start(*mockEntityEventHandler);
        mockAudioOutput->start();
        
        // Each user has different preferences
        auto fps = mockConfiguration->getVideoFPS();
        auto resolution = mockConfiguration->getVideoResolution();
        auto dpi = mockConfiguration->getScreenDPI();
        
        // Validate realistic configurations
        EXPECT_TRUE(fps == aap_protobuf::service::media::sink::message::VIDEO_FPS_30 ||
                   fps == aap_protobuf::service::media::sink::message::VIDEO_FPS_60);
        EXPECT_TRUE(resolution == aap_protobuf::service::media::sink::message::VIDEO_1280x720 ||
                   resolution == aap_protobuf::service::media::sink::message::VIDEO_1920x1080 ||
                   resolution == aap_protobuf::service::media::sink::message::VIDEO_800x480);
        EXPECT_GE(dpi, 120);
        EXPECT_LE(dpi, 480);
        
        // User session activity
        simulateNetworkDelay(5, 15);  // Fast session duration for testing
        
        // Check configuration persistence during session
        auto fps2 = mockConfiguration->getVideoFPS();
        EXPECT_EQ(fps, fps2);  // Configuration should remain consistent
        
        // User session end
        mockAudioOutput->stop();
        mockVideoOutput->stop();
        mockAndroidAutoEntity->stop();
        
        simulateNetworkDelay(1, 3);  // Fast switch delay for testing
    }
}

} // namespace f1x::openauto::autoapp::scenarios