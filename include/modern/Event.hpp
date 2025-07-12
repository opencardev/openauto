/*
*  This file is part of openauto project.
*  Copyright (C) 2025 OpenCarDev Team
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

namespace openauto {
namespace modern {

using EventValue = std::variant<std::string, int, double, bool>;
using EventData = std::unordered_map<std::string, EventValue>;

enum class EventType {
    // System Events
    SYSTEM_STARTUP,
    SYSTEM_SHUTDOWN,
    SYSTEM_REBOOT,
    SYSTEM_ERROR,
    SYSTEM_CONFIG_CHANGED,
    
    // Android Auto Events
    ANDROID_AUTO_CONNECTED,
    ANDROID_AUTO_DISCONNECTED,
    ANDROID_AUTO_START,
    ANDROID_AUTO_STOP,
    ANDROID_AUTO_PAUSE,
    ANDROID_AUTO_RESUME,
    ANDROID_AUTO_ERROR,
    
    // UI Events
    UI_BUTTON_PRESSED,
    UI_BRIGHTNESS_CHANGED,
    UI_VOLUME_CHANGED,
    UI_MODE_CHANGED,
    UI_SCREEN_TOUCH,
    UI_WINDOW_SHOW,
    UI_WINDOW_HIDE,
    
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
    WIFI_SCAN_COMPLETE,
    HOTSPOT_ENABLED,
    HOTSPOT_DISABLED,
    BLUETOOTH_CONNECTED,
    BLUETOOTH_DISCONNECTED,
    BLUETOOTH_PAIRING_REQUEST,
    NETWORK_STATUS_CHANGED,
    
    // Media Events
    MEDIA_PLAY,
    MEDIA_PAUSE,
    MEDIA_STOP,
    MEDIA_NEXT,
    MEDIA_PREVIOUS,
    MEDIA_TRACK_CHANGED,
    MEDIA_VOLUME_CHANGED,
    
    // Configuration Events
    CONFIG_CHANGED,
    CONFIG_SAVED,
    CONFIG_LOADED,
    CONFIG_RESET,
    
    // Update Events
    UPDATE_AVAILABLE,
    UPDATE_STARTED,
    UPDATE_PROGRESS,
    UPDATE_COMPLETED,
    UPDATE_FAILED,
    
    // State Machine Events
    STATE_CHANGED,
    STATE_ENTERED,
    STATE_EXITED,
    STATE_TRANSITION_FAILED,
    
    // UI Mode Events
    DAY_MODE_ENABLED,
    NIGHT_MODE_ENABLED,
    
    // Custom Events
    CUSTOM_BUTTON_1,
    CUSTOM_BUTTON_2,
    CUSTOM_BUTTON_3,
    CUSTOM_BUTTON_4,
    CUSTOM_BUTTON_5,
    CUSTOM_BUTTON_6,
    CUSTOM_EVENT
};

class Event {
public:
    using Pointer = std::shared_ptr<Event>;
    
    Event(EventType type, const std::string& source = "unknown");
    Event(EventType type, const EventData& data, const std::string& source = "unknown");
    
    // Getters
    EventType getType() const { return type_; }
    const EventData& getData() const { return data_; }
    const std::string& getSource() const { return source_; }
    std::chrono::system_clock::time_point getTimestamp() const { return timestamp_; }
    const std::string& getId() const { return id_; }
    
    // Data manipulation
    void setData(const std::string& key, const EventValue& value);
    EventValue getData(const std::string& key) const;
    bool hasData(const std::string& key) const;
    
    // Serialization
    std::string toString() const;
    nlohmann::json toJson() const;
    static Event::Pointer fromJson(const nlohmann::json& json);
    
    // Type conversion utilities
    static std::string eventTypeToString(EventType type);
    static EventType stringToEventType(const std::string& typeStr);
    
    // Factory methods
    static Event::Pointer create(EventType type, const std::string& source = "system");
    static Event::Pointer create(EventType type, const EventData& data, const std::string& source = "system");

private:
    std::string generateEventId() const;
    
    EventType type_;
    EventData data_;
    std::string source_;
    std::string id_;
    std::chrono::system_clock::time_point timestamp_;
};

} // namespace modern
} // namespace openauto
