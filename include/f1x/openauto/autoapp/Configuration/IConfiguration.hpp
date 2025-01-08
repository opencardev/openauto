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

#include <string>
#include <QRect>
#include <aap_protobuf/service/media/sink/message/VideoFrameRateType.pb.h>
#include <aap_protobuf/service/media/sink/message/VideoCodecResolutionType.pb.h>
#include <aap_protobuf/service/media/sink/message/KeyCode.pb.h>
#include <f1x/openauto/autoapp/Configuration/BluetoothAdapterType.hpp>
#include <f1x/openauto/autoapp/Configuration/HandednessOfTrafficType.hpp>
#include <f1x/openauto/autoapp/Configuration/AudioOutputBackendType.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace configuration
{

class IConfiguration
{
public:
    typedef std::shared_ptr<IConfiguration> Pointer;
    typedef std::vector<aap_protobuf::service::media::sink::message::KeyCode> ButtonCodes;

    virtual ~IConfiguration() = default;

    virtual void load() = 0;
    virtual void reset() = 0;
    virtual void save() = 0;

    virtual bool hasTouchScreen() const = 0;

    virtual void setHandednessOfTrafficType(HandednessOfTrafficType value) = 0;
    virtual HandednessOfTrafficType getHandednessOfTrafficType() const = 0;
    virtual void showClock(bool value) = 0;
    virtual bool showClock() const = 0;
    virtual void showBigClock(bool value) = 0;
    virtual bool showBigClock() const = 0;
    virtual void oldGUI(bool value) = 0;
    virtual bool oldGUI() const = 0;
    virtual void setAlphaTrans(size_t value) = 0;
    virtual size_t getAlphaTrans() const = 0;
    virtual void hideMenuToggle(bool value) = 0;
    virtual bool hideMenuToggle() const = 0;
    virtual void hideAlpha(bool value) = 0;
    virtual bool hideAlpha() const = 0;
    virtual void showLux(bool value) = 0;
    virtual bool showLux() const = 0;
    virtual void showCursor(bool value) = 0;
    virtual bool showCursor() const = 0;
    virtual void hideBrightnessControl(bool value) = 0;
    virtual bool hideBrightnessControl() const = 0;
    virtual void showNetworkinfo(bool value) = 0;
    virtual bool showNetworkinfo() const = 0;
    virtual void hideWarning(bool value) = 0;
    virtual bool hideWarning() const = 0;

    virtual std::string getMp3MasterPath() const = 0;
    virtual void setMp3MasterPath(const std::string& value) = 0;
    virtual std::string getMp3SubFolder() const = 0;
    virtual void setMp3SubFolder(const std::string& value) = 0;
    virtual int32_t getMp3Track() const = 0;
    virtual void setMp3Track(int32_t value) = 0;
    virtual bool mp3AutoPlay() const = 0;
    virtual void mp3AutoPlay(bool value) = 0;
    virtual bool showAutoPlay() const = 0;
    virtual void showAutoPlay(bool value) = 0;
    virtual bool instantPlay() const = 0;
    virtual void instantPlay(bool value) = 0;

    virtual QString getCSValue(QString searchString) const = 0;
    virtual QString readFileContent(QString fileName) const = 0;
    virtual QString getParamFromFile(QString fileName, QString searchString) const = 0;

    virtual aap_protobuf::service::media::sink::message::VideoFrameRateType getVideoFPS() const = 0;
    virtual void setVideoFPS(aap_protobuf::service::media::sink::message::VideoFrameRateType value) = 0;
    virtual aap_protobuf::service::media::sink::message::VideoCodecResolutionType getVideoResolution() const = 0;
    virtual void setVideoResolution(aap_protobuf::service::media::sink::message::VideoCodecResolutionType value) = 0;
    virtual size_t getScreenDPI() const = 0;
    virtual void setScreenDPI(size_t value) = 0;
    virtual void setOMXLayerIndex(int32_t value) = 0;
    virtual int32_t getOMXLayerIndex() const = 0;
    virtual void setVideoMargins(QRect value) = 0;
    virtual QRect getVideoMargins() const = 0;

    virtual bool getTouchscreenEnabled() const = 0;
    virtual void setTouchscreenEnabled(bool value) = 0;
    virtual bool playerButtonControl() const = 0;
    virtual void playerButtonControl(bool value) = 0;
    virtual ButtonCodes getButtonCodes() const = 0;
    virtual void setButtonCodes(const ButtonCodes& value) = 0;

    virtual BluetoothAdapterType getBluetoothAdapterType() const = 0;
    virtual void setBluetoothAdapterType(BluetoothAdapterType value) = 0;
    virtual std::string getBluetoothAdapterAddress() const = 0;
    virtual void setBluetoothAdapterAddress(const std::string& value) = 0;
    virtual bool getWirelessProjectionEnabled() const = 0;
    virtual void setWirelessProjectionEnabled(bool value) = 0;

    virtual bool musicAudioChannelEnabled() const = 0;
    virtual void setMusicAudioChannelEnabled(bool value) = 0;
    virtual bool guidanceAudioChannelEnabled() const = 0;
    virtual void setGuidanceAudioChannelEnabled(bool value) = 0;
    virtual bool systemAudioChannelEnabled() const = 0;
    virtual void setSystemAudioChannelEnabled(bool value) = 0;
    virtual bool telephonyAudioChannelEnabled() const = 0;
    virtual void setTelephonyAudioChannelEnabled(bool value) = 0;
    virtual AudioOutputBackendType getAudioOutputBackendType() const = 0;
    virtual void setAudioOutputBackendType(AudioOutputBackendType value) = 0;
};

}
}
}
}
