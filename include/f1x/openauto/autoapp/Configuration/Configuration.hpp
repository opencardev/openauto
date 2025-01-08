/*
*  This file is part of openauto project.
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
*
*  openauto is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.

*  openauto is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with openauto. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <boost/property_tree/ini_parser.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <iostream>
#include <string>
#include <fstream>

#include <stdio.h>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace configuration
{

class Configuration: public IConfiguration
{
public:
    Configuration();

    void load() override;
    void reset() override;
    void save() override;

    bool hasTouchScreen() const override;

    void setHandednessOfTrafficType(HandednessOfTrafficType value) override;
    HandednessOfTrafficType getHandednessOfTrafficType() const override;
    void showClock(bool value) override;
    bool showClock() const override;

    void showBigClock(bool value) override;
    bool showBigClock() const override;
    void oldGUI(bool value) override;
    bool oldGUI() const override;
    void setAlphaTrans(size_t value) override;
    size_t getAlphaTrans() const override;
    void hideMenuToggle(bool value) override;
    bool hideMenuToggle() const override;
    void hideAlpha(bool value) override;
    bool hideAlpha() const override;
    void showLux(bool value) override;
    bool showLux() const override;
    void showCursor(bool value) override;
    bool showCursor() const override;
    void hideBrightnessControl(bool value) override;
    bool hideBrightnessControl() const override;
    void showNetworkinfo(bool value) override;
    bool showNetworkinfo() const override;
    void hideWarning(bool value) override;
    bool hideWarning() const override;

    std::string getMp3MasterPath() const override;
    void setMp3MasterPath(const std::string& value) override;
    std::string getMp3SubFolder() const override;
    void setMp3SubFolder(const std::string& value) override;
    int32_t getMp3Track() const override;
    void setMp3Track(int32_t value) override;
    bool mp3AutoPlay() const override;
    void mp3AutoPlay(bool value) override;
    bool showAutoPlay() const override;
    void showAutoPlay(bool value) override;
    bool instantPlay() const override;
    void instantPlay(bool value) override;

    QString getCSValue(QString searchString) const override;
    QString readFileContent(QString fileName) const override;
    QString getParamFromFile(QString fileName, QString searchString) const override;

    aap_protobuf::service::media::sink::message::VideoFrameRateType getVideoFPS() const override;
    void setVideoFPS(aap_protobuf::service::media::sink::message::VideoFrameRateType value) override;
    aap_protobuf::service::media::sink::message::VideoCodecResolutionType getVideoResolution() const override;
    void setVideoResolution(aap_protobuf::service::media::sink::message::VideoCodecResolutionType value) override;
    size_t getScreenDPI() const override;
    void setScreenDPI(size_t value) override;
    void setOMXLayerIndex(int32_t value) override;
    int32_t getOMXLayerIndex() const override;
    void setVideoMargins(QRect value) override;
    QRect getVideoMargins() const override;

    bool getTouchscreenEnabled() const override;
    void setTouchscreenEnabled(bool value) override;
    bool playerButtonControl() const override;
    void playerButtonControl(bool value) override;
    ButtonCodes getButtonCodes() const override;
    void setButtonCodes(const ButtonCodes& value) override;

    BluetoothAdapterType getBluetoothAdapterType() const override;
    void setBluetoothAdapterType(BluetoothAdapterType value) override;
    std::string getBluetoothAdapterAddress() const override;
    void setBluetoothAdapterAddress(const std::string& value) override;
    bool getWirelessProjectionEnabled() const override;
    void setWirelessProjectionEnabled(bool value) override;

    bool musicAudioChannelEnabled() const override;
    void setMusicAudioChannelEnabled(bool value) override;
    bool guidanceAudioChannelEnabled() const override;
    void setGuidanceAudioChannelEnabled(bool value) override;
    bool systemAudioChannelEnabled() const override;
    void setSystemAudioChannelEnabled(bool value) override;
    bool telephonyAudioChannelEnabled() const override;
    void setTelephonyAudioChannelEnabled(bool value) override;

    AudioOutputBackendType getAudioOutputBackendType() const override;
    void setAudioOutputBackendType(AudioOutputBackendType value) override;
private:
    void readButtonCodes(boost::property_tree::ptree& iniConfig);
    void insertButtonCode(boost::property_tree::ptree& iniConfig, const std::string& buttonCodeKey, aap_protobuf::service::media::sink::message::KeyCode buttonCode);
    void writeButtonCodes(boost::property_tree::ptree& iniConfig);

    HandednessOfTrafficType handednessOfTrafficType_;
    bool showClock_;

    bool showBigClock_;
    bool oldGUI_;
    size_t alphaTrans_;
    bool hideMenuToggle_;
    bool hideAlpha_;
    bool showLux_;
    bool showCursor_;
    bool hideBrightnessControl_;
    bool showNetworkinfo_;
    bool hideWarning_;
    std::string mp3MasterPath_;
    std::string mp3SubFolder_;
    int32_t mp3Track_;
    bool mp3AutoPlay_;
    bool showAutoPlay_;
    bool instantPlay_;

    aap_protobuf::service::media::sink::message::VideoFrameRateType videoFPS_;
    aap_protobuf::service::media::sink::message::VideoCodecResolutionType videoResolution_;
    size_t screenDPI_;
    int32_t omxLayerIndex_;
    QRect videoMargins_;
    bool enableTouchscreen_;
    bool enablePlayerControl_;
    ButtonCodes buttonCodes_;
    BluetoothAdapterType bluetoothAdapterType_;
    std::string bluetoothAdapterAddress_;
    bool wirelessProjectionEnabled_;

    bool _audioChannelEnabledMedia;
    bool _audioChannelEnabledGuidance;
    bool _audioChannelEnabledSystem;
    bool _audioChannelEnabledTelephony;

    AudioOutputBackendType audioOutputBackendType_;

    static const std::string cConfigFileName;

    static const std::string cGeneralShowClockKey;

    static const std::string cGeneralShowBigClockKey;
    static const std::string cGeneralOldGUIKey;
    static const std::string cGeneralAlphaTransKey;
    static const std::string cGeneralHideMenuToggleKey;
    static const std::string cGeneralHideAlphaKey;
    static const std::string cGeneralShowLuxKey;
    static const std::string cGeneralShowCursorKey;
    static const std::string cGeneralHideBrightnessControlKey;
    static const std::string cGeneralShowNetworkinfoKey;
    static const std::string cGeneralHideWarningKey;

    static const std::string cGeneralHandednessOfTrafficTypeKey;

    static const std::string cGeneralMp3MasterPathKey;
    static const std::string cGeneralMp3SubFolderKey;
    static const std::string cGeneralMp3TrackKey;
    static const std::string cGeneralMp3AutoPlayKey;
    static const std::string cGeneralShowAutoPlayKey;
    static const std::string cGeneralInstantPlayKey;

    static const std::string cVideoFPSKey;
    static const std::string cVideoResolutionKey;
    static const std::string cVideoScreenDPIKey;
    static const std::string cVideoOMXLayerIndexKey;
    static const std::string cVideoMarginWidth;
    static const std::string cVideoMarginHeight;

    static const std::string cAudioChannelMediaEnabled;
    static const std::string cAudioChannelGuidanceEnabled;
    static const std::string cAudioChannelSystemEnabled;
    static const std::string cAudioChannelTelephonyEnabled;

    static const std::string cAudioOutputBackendType;

    static const std::string cBluetoothAdapterTypeKey;
    static const std::string cBluetoothAdapterAddressKey;
    static const std::string cBluetoothWirelessProjectionEnabledKey;

    static const std::string cInputEnableTouchscreenKey;
    static const std::string cInputEnablePlayerControlKey;
    static const std::string cInputPlayButtonKey;
    static const std::string cInputPauseButtonKey;
    static const std::string cInputTogglePlayButtonKey;
    static const std::string cInputNextTrackButtonKey;
    static const std::string cInputPreviousTrackButtonKey;
    static const std::string cInputHomeButtonKey;
    static const std::string cInputPhoneButtonKey;
    static const std::string cInputCallEndButtonKey;
    static const std::string cInputVoiceCommandButtonKey;
    static const std::string cInputLeftButtonKey;
    static const std::string cInputRightButtonKey;
    static const std::string cInputUpButtonKey;
    static const std::string cInputDownButtonKey;
    static const std::string cInputScrollWheelButtonKey;
    static const std::string cInputBackButtonKey;
    static const std::string cInputEnterButtonKey;
    static const std::string cInputNavButtonKey;
};

}
}
}
}
