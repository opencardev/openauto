#pragma once

#include <gmock/gmock.h>
#include <QtCore/QString>
#include <QtCore/QRect>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>

namespace f1x::openauto::autoapp::configuration {

class MockConfiguration : public IConfiguration {
public:
    // Service management methods
    MOCK_METHOD(void, load, (), (override));
    MOCK_METHOD(void, reset, (), (override));
    MOCK_METHOD(void, save, (), (override));
    
    // Display methods
    MOCK_METHOD(bool, hasTouchScreen, (), (const, override));
    MOCK_METHOD(void, setHandednessOfTrafficType, (HandednessOfTrafficType value), (override));
    MOCK_METHOD(HandednessOfTrafficType, getHandednessOfTrafficType, (), (const, override));
    MOCK_METHOD(void, showClock, (bool value), (override));
    MOCK_METHOD(bool, showClock, (), (const, override));
    MOCK_METHOD(void, showBigClock, (bool value), (override));
    MOCK_METHOD(bool, showBigClock, (), (const, override));
    MOCK_METHOD(void, oldGUI, (bool value), (override));
    MOCK_METHOD(bool, oldGUI, (), (const, override));
    MOCK_METHOD(void, setAlphaTrans, (size_t value), (override));
    MOCK_METHOD(size_t, getAlphaTrans, (), (const, override));
    MOCK_METHOD(void, hideMenuToggle, (bool value), (override));
    MOCK_METHOD(bool, hideMenuToggle, (), (const, override));
    MOCK_METHOD(void, hideAlpha, (bool value), (override));
    MOCK_METHOD(bool, hideAlpha, (), (const, override));
    MOCK_METHOD(void, showLux, (bool value), (override));
    MOCK_METHOD(bool, showLux, (), (const, override));
    MOCK_METHOD(void, showCursor, (bool value), (override));
    MOCK_METHOD(bool, showCursor, (), (const, override));
    MOCK_METHOD(void, hideBrightnessControl, (bool value), (override));
    MOCK_METHOD(bool, hideBrightnessControl, (), (const, override));
    MOCK_METHOD(void, showNetworkinfo, (bool value), (override));
    MOCK_METHOD(bool, showNetworkinfo, (), (const, override));
    MOCK_METHOD(void, hideWarning, (bool value), (override));
    MOCK_METHOD(bool, hideWarning, (), (const, override));
    
    // MP3 methods
    MOCK_METHOD(std::string, getMp3MasterPath, (), (const, override));
    MOCK_METHOD(void, setMp3MasterPath, (const std::string& value), (override));
    MOCK_METHOD(std::string, getMp3SubFolder, (), (const, override));
    MOCK_METHOD(void, setMp3SubFolder, (const std::string& value), (override));
    MOCK_METHOD(int32_t, getMp3Track, (), (const, override));
    MOCK_METHOD(void, setMp3Track, (int32_t value), (override));
    MOCK_METHOD(bool, mp3AutoPlay, (), (const, override));
    MOCK_METHOD(void, mp3AutoPlay, (bool value), (override));
    MOCK_METHOD(bool, showAutoPlay, (), (const, override));
    MOCK_METHOD(void, showAutoPlay, (bool value), (override));
    MOCK_METHOD(bool, instantPlay, (), (const, override));
    MOCK_METHOD(void, instantPlay, (bool value), (override));
    
    // Configuration access methods
    MOCK_METHOD(QString, getCSValue, (QString searchString), (const, override));
    MOCK_METHOD(QString, readFileContent, (QString fileName), (const, override));
    MOCK_METHOD(QString, getParamFromFile, (QString fileName, QString searchString), (const, override));
    
    // Video methods
    MOCK_METHOD(aap_protobuf::service::media::sink::message::VideoFrameRateType, getVideoFPS, (), (const, override));
    MOCK_METHOD(void, setVideoFPS, (aap_protobuf::service::media::sink::message::VideoFrameRateType value), (override));
    MOCK_METHOD(aap_protobuf::service::media::sink::message::VideoCodecResolutionType, getVideoResolution, (), (const, override));
    MOCK_METHOD(void, setVideoResolution, (aap_protobuf::service::media::sink::message::VideoCodecResolutionType value), (override));
    MOCK_METHOD(size_t, getScreenDPI, (), (const, override));
    MOCK_METHOD(void, setScreenDPI, (size_t value), (override));
    MOCK_METHOD(void, setOMXLayerIndex, (int32_t value), (override));
    MOCK_METHOD(int32_t, getOMXLayerIndex, (), (const, override));
    MOCK_METHOD(void, setVideoMargins, (QRect value), (override));
    MOCK_METHOD(QRect, getVideoMargins, (), (const, override));
    
    // Touchscreen methods
    MOCK_METHOD(bool, getTouchscreenEnabled, (), (const, override));
    MOCK_METHOD(void, setTouchscreenEnabled, (bool value), (override));
    MOCK_METHOD(bool, playerButtonControl, (), (const, override));
    MOCK_METHOD(void, playerButtonControl, (bool value), (override));
    MOCK_METHOD(ButtonCodes, getButtonCodes, (), (const, override));
    MOCK_METHOD(void, setButtonCodes, (const ButtonCodes& value), (override));
    
    // Bluetooth methods
    MOCK_METHOD(BluetoothAdapterType, getBluetoothAdapterType, (), (const, override));
    MOCK_METHOD(void, setBluetoothAdapterType, (BluetoothAdapterType value), (override));
    MOCK_METHOD(std::string, getBluetoothAdapterAddress, (), (const, override));
    MOCK_METHOD(void, setBluetoothAdapterAddress, (const std::string& value), (override));
    MOCK_METHOD(bool, getWirelessProjectionEnabled, (), (const, override));
    MOCK_METHOD(void, setWirelessProjectionEnabled, (bool value), (override));
    
    // Audio methods
    MOCK_METHOD(bool, musicAudioChannelEnabled, (), (const, override));
    MOCK_METHOD(void, setMusicAudioChannelEnabled, (bool value), (override));
    MOCK_METHOD(bool, guidanceAudioChannelEnabled, (), (const, override));
    MOCK_METHOD(void, setGuidanceAudioChannelEnabled, (bool value), (override));
    MOCK_METHOD(bool, systemAudioChannelEnabled, (), (const, override));
    MOCK_METHOD(void, setSystemAudioChannelEnabled, (bool value), (override));
    MOCK_METHOD(bool, telephonyAudioChannelEnabled, (), (const, override));
    MOCK_METHOD(void, setTelephonyAudioChannelEnabled, (bool value), (override));
    MOCK_METHOD(AudioOutputBackendType, getAudioOutputBackendType, (), (const, override));
    MOCK_METHOD(void, setAudioOutputBackendType, (AudioOutputBackendType value), (override));
};

}  // namespace f1x::openauto::autoapp::configuration