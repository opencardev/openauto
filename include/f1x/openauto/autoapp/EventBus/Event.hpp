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

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <chrono>
#include <nlohmann/json.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace eventbus
{

using EventValue = std::variant<std::string, int, double, bool>;
using EventData = std::unordered_map<std::string, EventValue>;

enum class EventType
{
    // System Events
    SYSTEM_STARTUP,
    SYSTEM_SHUTDOWN,
    SYSTEM_REBOOT,
    SYSTEM_ERROR,
    
    // Android Auto Events
    ANDROID_AUTO_CONNECTED,
    ANDROID_AUTO_DISCONNECTED,
    ANDROID_AUTO_START,
    ANDROID_AUTO_STOP,
    ANDROID_AUTO_PAUSE,
    ANDROID_AUTO_RESUME,
    
    // UI Events
    UI_BUTTON_PRESSED,
    UI_BRIGHTNESS_CHANGED,
    UI_VOLUME_CHANGED,
    UI_MODE_CHANGED,
    UI_SCREEN_TOUCH,
    
    // Camera Events
    CAMERA_SHOW,
    CAMERA_HIDE,
    CAMERA_RECORD_START,
    CAMERA_RECORD_STOP,
    CAMERA_SAVE,
    CAMERA_ZOOM_IN,
    CAMERA_ZOOM_OUT,
    CAMERA_MOVE_UP,
    CAMERA_MOVE_DOWN,
    
    // Network Events
    WIFI_CONNECTED,
    WIFI_DISCONNECTED,
    HOTSPOT_ENABLED,
    HOTSPOT_DISABLED,
    BLUETOOTH_CONNECTED,
    BLUETOOTH_DISCONNECTED,
    BLUETOOTH_PAIRING_REQUEST,
    
    // Media Events
    MEDIA_PLAY,
    MEDIA_PAUSE,
    MEDIA_STOP,
    MEDIA_NEXT,
    MEDIA_PREVIOUS,
    MEDIA_TRACK_CHANGED,
    
    // Configuration Events
    CONFIG_CHANGED,
    CONFIG_SAVED,
    
    // Custom Button Events
    CUSTOM_BUTTON_1,
    CUSTOM_BUTTON_2,
    CUSTOM_BUTTON_3,
    CUSTOM_BUTTON_4,
    CUSTOM_BUTTON_5,
    CUSTOM_BUTTON_6,
    
    // Day/Night Mode Events
    DAY_MODE_ENABLED,
    NIGHT_MODE_ENABLED,
    
    // Update Events
    UPDATE_AVAILABLE,
    UPDATE_STARTED,
    UPDATE_COMPLETED,
    UPDATE_FAILED,
    
    // Custom Events
    CUSTOM_EVENT
};

class Event
{
public:
    using Pointer = std::shared_ptr<Event>;
    
    Event(EventType type, const std::string& source = "unknown");
    Event(EventType type, const EventData& data, const std::string& source = "unknown");
    
    EventType getType() const { return type_; }
    const EventData& getData() const { return data_; }
    const std::string& getSource() const { return source_; }
    std::chrono::system_clock::time_point getTimestamp() const { return timestamp_; }
    
    void setData(const std::string& key, const EventValue& value);
    EventValue getData(const std::string& key) const;
    bool hasData(const std::string& key) const;
    
    std::string toString() const;
    nlohmann::json toJson() const;
    static Event::Pointer fromJson(const nlohmann::json& json);
    
    static std::string eventTypeToString(EventType type);
    static EventType stringToEventType(const std::string& typeStr);

private:
    EventType type_;
    EventData data_;
    std::string source_;
    std::chrono::system_clock::time_point timestamp_;
};

}
}
}
}
