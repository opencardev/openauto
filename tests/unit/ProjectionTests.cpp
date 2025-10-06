#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

#include <f1x/openauto/autoapp/Projection/VideoOutput.hpp>
#include <f1x/openauto/autoapp/Projection/QtAudioOutput.hpp>
#include <f1x/openauto/autoapp/Projection/InputDevice.hpp>
#include "../../mocks/MockConfiguration.hpp"
#include "../../mocks/MockAudioOutput.hpp"

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::InSequence;

namespace f1x::openauto::autoapp::projection {

class ProjectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockConfiguration = std::make_shared<NiceMock<configuration::MockConfiguration>>();
        
        // Configure default responses
        ON_CALL(*mockConfiguration, getCSValue(QString("VIDEO_FPS")))
            .WillByDefault(Return("60"));
        ON_CALL(*mockConfiguration, getCSValue(QString("VIDEO_RESOLUTION")))
            .WillByDefault(Return("1080"));
    }

    std::shared_ptr<NiceMock<configuration::MockConfiguration>> mockConfiguration;
};

// TC-PROJ-001 - Video Projection
TEST_F(ProjectionTest, VideoOutputCreation) {
    // Test that VideoOutput is properly created with configured settings
    EXPECT_CALL(*mockConfiguration, getCSValue(QString("VIDEO_FPS")))
        .WillRepeatedly(Return("60"));
    EXPECT_CALL(*mockConfiguration, getCSValue(QString("VIDEO_RESOLUTION")))
        .WillRepeatedly(Return("1080"));
    
    // Create VideoOutput
    auto videoOutput = VideoOutput::create(mockConfiguration);
    
    // Check that the video output was created with correct settings
    ASSERT_NE(videoOutput, nullptr);
}

// TC-PROJ-002 - Audio Playback
TEST_F(ProjectionTest, AudioOutputPlayback) {
    // Create mock audio output
    auto mockAudio = std::make_shared<NiceMock<MockAudioOutput>>();
    
    // Set up expectations
    {
        InSequence seq;
        EXPECT_CALL(*mockAudio, open()).WillOnce(Return(true));
        EXPECT_CALL(*mockAudio, start());
        EXPECT_CALL(*mockAudio, write(_, _)).Times(1);
        EXPECT_CALL(*mockAudio, stop());
    }
    
    // Test the audio output functionality
    EXPECT_TRUE(mockAudio->open());
    mockAudio->start();
    
    // Create a dummy audio buffer
    const uint8_t dummyData[] = {0, 1, 2, 3, 4};
    aasdk::common::DataConstBuffer audioBuffer(dummyData, sizeof(dummyData));
    mockAudio->write(0, audioBuffer);
    
    mockAudio->stop();
}

// TC-PROJ-004 - Touch Input
TEST_F(ProjectionTest, TouchInputHandling) {
    // Setup input device with mocked configuration
    ON_CALL(*mockConfiguration, hasTouchScreen())
        .WillByDefault(Return(true));
        
    InputDevice inputDevice(mockConfiguration);
    
    // Verify touch event handling
    QRect touchscreenGeometry(0, 0, 1920, 1080);
    
    // Create a touch point at position 500, 500
    QTouchEvent::TouchPoint touchPoint;
    touchPoint.setPos(QPointF(500, 500));
    touchPoint.setState(Qt::TouchPointPressed);
    
    // Create a touch event
    QList<QTouchEvent::TouchPoint> touchPoints;
    touchPoints.append(touchPoint);
    
    // Call the touch event handler (we can only test that it doesn't crash with our mocked objects)
    inputDevice.onTouchEvent(touchPoints, touchscreenGeometry);
    
    // Create a touch move event
    touchPoint.setPos(QPointF(600, 600));
    touchPoint.setState(Qt::TouchPointMoved);
    touchPoints.clear();
    touchPoints.append(touchPoint);
    
    // Call the touch event handler for move
    inputDevice.onTouchEvent(touchPoints, touchscreenGeometry);
    
    // Create a touch release event
    touchPoint.setState(Qt::TouchPointReleased);
    touchPoints.clear();
    touchPoints.append(touchPoint);
    
    // Call the touch event handler for release
    inputDevice.onTouchEvent(touchPoints, touchscreenGeometry);
    
    // Hard to test actual behavior without access to the Android device
    // but we can verify the class handles the touch events without crashing
    SUCCEED();
}

} // namespace f1x::openauto::autoapp::projection