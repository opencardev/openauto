#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <QTemporaryFile>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <f1x/openauto/autoapp/Configuration/Configuration.hpp>

namespace f1x::openauto::autoapp::configuration {

class ConfigurationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary configuration file for testing
        configFile = std::make_unique<QTemporaryFile>();
        configFile->open();
        configFileName = configFile->fileName();
        configFile->close(); // Close so Configuration can open it
        
        // Create a clean configuration object
        configuration = std::make_shared<Configuration>();
    }
    
    void TearDown() override {
        // Temporary file will automatically be removed when the object is destroyed
    }
    
    void createTestConfigFile() {
        QFile file(configFileName);
        if (file.open(QIODevice::WriteOnly)) {
            QJsonObject configObject;
            
            // General settings
            configObject["HandednessOfTrafficType"] = static_cast<int>(HandednessOfTrafficType::LEFT_HAND_DRIVE);
            configObject["ShowClock"] = true;
            configObject["ShowBigClock"] = false;
            configObject["OldGUI"] = false;
            configObject["AlphaTrans"] = 50;
            configObject["HideMenuToggle"] = false;
            configObject["HideAlpha"] = false;
            
            // Video settings
            configObject["VideoResolution"] = 1080;
            configObject["VideoFPS"] = 60;
            configObject["ScreenDPI"] = 140;
            configObject["TouchscreenEnabled"] = true;
            
            QJsonDocument document(configObject);
            file.write(document.toJson());
            file.close();
        }
    }

    std::unique_ptr<QTemporaryFile> configFile;
    QString configFileName;
    std::shared_ptr<Configuration> configuration;
};

// TC-CONF-001 - Configuration Save/Load
TEST_F(ConfigurationTest, SaveAndLoadConfiguration) {
    // Set configuration values
    configuration->setHandednessOfTrafficType(HandednessOfTrafficType::LEFT_HAND_DRIVE);
    configuration->showClock(true);
    configuration->showBigClock(false);
    configuration->oldGUI(false);
    configuration->setAlphaTrans(50);
    configuration->hideMenuToggle(false);
    configuration->hideAlpha(false);
    
    // Save configuration
    // Use a specific known path for the test
    configuration->save();
    
    // Create a new configuration object and load
    auto newConfiguration = std::make_shared<Configuration>();
    newConfiguration->load();
    
    // Verify the loaded configuration matches what was saved
    EXPECT_EQ(configuration->getHandednessOfTrafficType(), newConfiguration->getHandednessOfTrafficType());
    EXPECT_EQ(configuration->showClock(), newConfiguration->showClock());
    EXPECT_EQ(configuration->showBigClock(), newConfiguration->showBigClock());
    EXPECT_EQ(configuration->oldGUI(), newConfiguration->oldGUI());
    EXPECT_EQ(configuration->getAlphaTrans(), newConfiguration->getAlphaTrans());
    EXPECT_EQ(configuration->hideMenuToggle(), newConfiguration->hideMenuToggle());
    EXPECT_EQ(configuration->hideAlpha(), newConfiguration->hideAlpha());
}

// TC-CONF-002 - Audio Output Configuration
TEST_F(ConfigurationTest, AudioOutputConfiguration) {
    // Test default audio settings
    auto musicAudioChannelEnabled = configuration->musicAudioChannelEnabled();
    auto speechAudioChannelEnabled = configuration->speechAudioChannelEnabled();
    auto audioOutputBackendType = configuration->getAudioOutputBackendType();
    
    // Modify settings
    configuration->setMusicAudioChannelEnabled(!musicAudioChannelEnabled);
    configuration->setSpeechAudioChannelEnabled(!speechAudioChannelEnabled);
    configuration->setAudioOutputBackendType(
        audioOutputBackendType == AudioOutputBackendType::RTAUDIO ? 
        AudioOutputBackendType::QT : AudioOutputBackendType::RTAUDIO);
    
    // Save and reload
    configuration->save();
    configuration->load();
    
    // Verify changes were persisted
    EXPECT_EQ(!musicAudioChannelEnabled, configuration->musicAudioChannelEnabled());
    EXPECT_EQ(!speechAudioChannelEnabled, configuration->speechAudioChannelEnabled());
    EXPECT_EQ(audioOutputBackendType == AudioOutputBackendType::RTAUDIO ? 
              AudioOutputBackendType::QT : AudioOutputBackendType::RTAUDIO, 
              configuration->getAudioOutputBackendType());
}

// TC-CONF-003 - Video Output Configuration
TEST_F(ConfigurationTest, VideoOutputConfiguration) {
    // Get current video settings
    auto videoResolution = configuration->getVideoResolution();
    auto fps = configuration->getVideoFPS();
    
    // Change video settings
    configuration->setVideoResolution(videoResolution == VideoResolution::_480 ? 
                                      VideoResolution::_720 : VideoResolution::_480);
    configuration->setVideoFPS(fps == VideoFPS::_30 ? VideoFPS::_60 : VideoFPS::_30);
    
    // Save and reload
    configuration->save();
    configuration->load();
    
    // Verify changes were persisted
    EXPECT_EQ(videoResolution == VideoResolution::_480 ? VideoResolution::_720 : VideoResolution::_480, 
              configuration->getVideoResolution());
    EXPECT_EQ(fps == VideoFPS::_30 ? VideoFPS::_60 : VideoFPS::_30, 
              configuration->getVideoFPS());
}

// TC-CONF-004 - Input Configuration
TEST_F(ConfigurationTest, InputConfiguration) {
    // Test touch input settings
    auto touchscreenEnabled = configuration->getTouchscreenEnabled();
    
    // Modify settings
    configuration->setTouchscreenEnabled(!touchscreenEnabled);
    
    // Save and reload
    configuration->save();
    configuration->load();
    
    // Verify changes were persisted
    EXPECT_EQ(!touchscreenEnabled, configuration->getTouchscreenEnabled());
}

} // namespace f1x::openauto::autoapp::configuration