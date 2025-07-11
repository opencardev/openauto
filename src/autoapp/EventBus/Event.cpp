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

#include <f1x/openauto/autoapp/EventBus/Event.hpp>
#include <sstream>
#include <stdexcept>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace eventbus
{

Event::Event(EventType type, const std::string& source)
    : type_(type), source_(source), timestamp_(std::chrono::system_clock::now())
{
}

Event::Event(EventType type, const EventData& data, const std::string& source)
    : type_(type), data_(data), source_(source), timestamp_(std::chrono::system_clock::now())
{
}

void Event::setData(const std::string& key, const EventValue& value)
{
    data_[key] = value;
}

EventValue Event::getData(const std::string& key) const
{
    auto it = data_.find(key);
    if (it != data_.end()) {
        return it->second;
    }
    return std::string(""); // Return empty string as default
}

bool Event::hasData(const std::string& key) const
{
    return data_.find(key) != data_.end();
}

std::string Event::toString() const
{
    std::ostringstream oss;
    oss << "Event{type:" << eventTypeToString(type_) 
        << ", source:" << source_ 
        << ", timestamp:" << std::chrono::duration_cast<std::chrono::milliseconds>(timestamp_.time_since_epoch()).count()
        << ", data:{";
    
    bool first = true;
    for (const auto& [key, value] : data_) {
        if (!first) oss << ", ";
        oss << key << ":";
        
        std::visit([&oss](const auto& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::string>) {
                oss << "\"" << v << "\"";
            } else {
                oss << v;
            }
        }, value);
        
        first = false;
    }
    
    oss << "}}";
    return oss.str();
}

nlohmann::json Event::toJson() const
{
    nlohmann::json json;
    json["type"] = eventTypeToString(type_);
    json["source"] = source_;
    json["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp_.time_since_epoch()).count();
    
    nlohmann::json dataJson;
    for (const auto& [key, value] : data_) {
        std::visit([&dataJson, &key](const auto& v) {
            dataJson[key] = v;
        }, value);
    }
    json["data"] = dataJson;
    
    return json;
}

Event::Pointer Event::fromJson(const nlohmann::json& json)
{
    EventType type = stringToEventType(json["type"]);
    std::string source = json.value("source", "unknown");
    
    EventData data;
    if (json.contains("data")) {
        const auto& dataJson = json["data"];
        for (auto& [key, value] : dataJson.items()) {
            if (value.is_string()) {
                data[key] = value.get<std::string>();
            } else if (value.is_number_integer()) {
                data[key] = value.get<int>();
            } else if (value.is_number_float()) {
                data[key] = value.get<double>();
            } else if (value.is_boolean()) {
                data[key] = value.get<bool>();
            }
        }
    }
    
    return std::make_shared<Event>(type, data, source);
}

std::string Event::eventTypeToString(EventType type)
{
    switch (type) {
        case EventType::SYSTEM_STARTUP: return "SYSTEM_STARTUP";
        case EventType::SYSTEM_SHUTDOWN: return "SYSTEM_SHUTDOWN";
        case EventType::SYSTEM_REBOOT: return "SYSTEM_REBOOT";
        case EventType::SYSTEM_ERROR: return "SYSTEM_ERROR";
        case EventType::ANDROID_AUTO_CONNECTED: return "ANDROID_AUTO_CONNECTED";
        case EventType::ANDROID_AUTO_DISCONNECTED: return "ANDROID_AUTO_DISCONNECTED";
        case EventType::ANDROID_AUTO_START: return "ANDROID_AUTO_START";
        case EventType::ANDROID_AUTO_STOP: return "ANDROID_AUTO_STOP";
        case EventType::ANDROID_AUTO_PAUSE: return "ANDROID_AUTO_PAUSE";
        case EventType::ANDROID_AUTO_RESUME: return "ANDROID_AUTO_RESUME";
        case EventType::UI_BUTTON_PRESSED: return "UI_BUTTON_PRESSED";
        case EventType::UI_BRIGHTNESS_CHANGED: return "UI_BRIGHTNESS_CHANGED";
        case EventType::UI_VOLUME_CHANGED: return "UI_VOLUME_CHANGED";
        case EventType::UI_MODE_CHANGED: return "UI_MODE_CHANGED";
        case EventType::UI_SCREEN_TOUCH: return "UI_SCREEN_TOUCH";
        case EventType::CAMERA_SHOW: return "CAMERA_SHOW";
        case EventType::CAMERA_HIDE: return "CAMERA_HIDE";
        case EventType::CAMERA_RECORD_START: return "CAMERA_RECORD_START";
        case EventType::CAMERA_RECORD_STOP: return "CAMERA_RECORD_STOP";
        case EventType::CAMERA_SAVE: return "CAMERA_SAVE";
        case EventType::CAMERA_ZOOM_IN: return "CAMERA_ZOOM_IN";
        case EventType::CAMERA_ZOOM_OUT: return "CAMERA_ZOOM_OUT";
        case EventType::CAMERA_MOVE_UP: return "CAMERA_MOVE_UP";
        case EventType::CAMERA_MOVE_DOWN: return "CAMERA_MOVE_DOWN";
        case EventType::WIFI_CONNECTED: return "WIFI_CONNECTED";
        case EventType::WIFI_DISCONNECTED: return "WIFI_DISCONNECTED";
        case EventType::HOTSPOT_ENABLED: return "HOTSPOT_ENABLED";
        case EventType::HOTSPOT_DISABLED: return "HOTSPOT_DISABLED";
        case EventType::BLUETOOTH_CONNECTED: return "BLUETOOTH_CONNECTED";
        case EventType::BLUETOOTH_DISCONNECTED: return "BLUETOOTH_DISCONNECTED";
        case EventType::BLUETOOTH_PAIRING_REQUEST: return "BLUETOOTH_PAIRING_REQUEST";
        case EventType::MEDIA_PLAY: return "MEDIA_PLAY";
        case EventType::MEDIA_PAUSE: return "MEDIA_PAUSE";
        case EventType::MEDIA_STOP: return "MEDIA_STOP";
        case EventType::MEDIA_NEXT: return "MEDIA_NEXT";
        case EventType::MEDIA_PREVIOUS: return "MEDIA_PREVIOUS";
        case EventType::MEDIA_TRACK_CHANGED: return "MEDIA_TRACK_CHANGED";
        case EventType::CONFIG_CHANGED: return "CONFIG_CHANGED";
        case EventType::CONFIG_SAVED: return "CONFIG_SAVED";
        case EventType::CUSTOM_BUTTON_1: return "CUSTOM_BUTTON_1";
        case EventType::CUSTOM_BUTTON_2: return "CUSTOM_BUTTON_2";
        case EventType::CUSTOM_BUTTON_3: return "CUSTOM_BUTTON_3";
        case EventType::CUSTOM_BUTTON_4: return "CUSTOM_BUTTON_4";
        case EventType::CUSTOM_BUTTON_5: return "CUSTOM_BUTTON_5";
        case EventType::CUSTOM_BUTTON_6: return "CUSTOM_BUTTON_6";
        case EventType::DAY_MODE_ENABLED: return "DAY_MODE_ENABLED";
        case EventType::NIGHT_MODE_ENABLED: return "NIGHT_MODE_ENABLED";
        case EventType::UPDATE_AVAILABLE: return "UPDATE_AVAILABLE";
        case EventType::UPDATE_STARTED: return "UPDATE_STARTED";
        case EventType::UPDATE_COMPLETED: return "UPDATE_COMPLETED";
        case EventType::UPDATE_FAILED: return "UPDATE_FAILED";
        case EventType::CUSTOM_EVENT: return "CUSTOM_EVENT";
        default: return "UNKNOWN";
    }
}

EventType Event::stringToEventType(const std::string& typeStr)
{
    if (typeStr == "SYSTEM_STARTUP") return EventType::SYSTEM_STARTUP;
    if (typeStr == "SYSTEM_SHUTDOWN") return EventType::SYSTEM_SHUTDOWN;
    if (typeStr == "SYSTEM_REBOOT") return EventType::SYSTEM_REBOOT;
    if (typeStr == "SYSTEM_ERROR") return EventType::SYSTEM_ERROR;
    if (typeStr == "ANDROID_AUTO_CONNECTED") return EventType::ANDROID_AUTO_CONNECTED;
    if (typeStr == "ANDROID_AUTO_DISCONNECTED") return EventType::ANDROID_AUTO_DISCONNECTED;
    if (typeStr == "ANDROID_AUTO_START") return EventType::ANDROID_AUTO_START;
    if (typeStr == "ANDROID_AUTO_STOP") return EventType::ANDROID_AUTO_STOP;
    if (typeStr == "ANDROID_AUTO_PAUSE") return EventType::ANDROID_AUTO_PAUSE;
    if (typeStr == "ANDROID_AUTO_RESUME") return EventType::ANDROID_AUTO_RESUME;
    if (typeStr == "UI_BUTTON_PRESSED") return EventType::UI_BUTTON_PRESSED;
    if (typeStr == "UI_BRIGHTNESS_CHANGED") return EventType::UI_BRIGHTNESS_CHANGED;
    if (typeStr == "UI_VOLUME_CHANGED") return EventType::UI_VOLUME_CHANGED;
    if (typeStr == "UI_MODE_CHANGED") return EventType::UI_MODE_CHANGED;
    if (typeStr == "UI_SCREEN_TOUCH") return EventType::UI_SCREEN_TOUCH;
    if (typeStr == "CAMERA_SHOW") return EventType::CAMERA_SHOW;
    if (typeStr == "CAMERA_HIDE") return EventType::CAMERA_HIDE;
    if (typeStr == "CAMERA_RECORD_START") return EventType::CAMERA_RECORD_START;
    if (typeStr == "CAMERA_RECORD_STOP") return EventType::CAMERA_RECORD_STOP;
    if (typeStr == "CAMERA_SAVE") return EventType::CAMERA_SAVE;
    if (typeStr == "CAMERA_ZOOM_IN") return EventType::CAMERA_ZOOM_IN;
    if (typeStr == "CAMERA_ZOOM_OUT") return EventType::CAMERA_ZOOM_OUT;
    if (typeStr == "CAMERA_MOVE_UP") return EventType::CAMERA_MOVE_UP;
    if (typeStr == "CAMERA_MOVE_DOWN") return EventType::CAMERA_MOVE_DOWN;
    if (typeStr == "WIFI_CONNECTED") return EventType::WIFI_CONNECTED;
    if (typeStr == "WIFI_DISCONNECTED") return EventType::WIFI_DISCONNECTED;
    if (typeStr == "HOTSPOT_ENABLED") return EventType::HOTSPOT_ENABLED;
    if (typeStr == "HOTSPOT_DISABLED") return EventType::HOTSPOT_DISABLED;
    if (typeStr == "BLUETOOTH_CONNECTED") return EventType::BLUETOOTH_CONNECTED;
    if (typeStr == "BLUETOOTH_DISCONNECTED") return EventType::BLUETOOTH_DISCONNECTED;
    if (typeStr == "BLUETOOTH_PAIRING_REQUEST") return EventType::BLUETOOTH_PAIRING_REQUEST;
    if (typeStr == "MEDIA_PLAY") return EventType::MEDIA_PLAY;
    if (typeStr == "MEDIA_PAUSE") return EventType::MEDIA_PAUSE;
    if (typeStr == "MEDIA_STOP") return EventType::MEDIA_STOP;
    if (typeStr == "MEDIA_NEXT") return EventType::MEDIA_NEXT;
    if (typeStr == "MEDIA_PREVIOUS") return EventType::MEDIA_PREVIOUS;
    if (typeStr == "MEDIA_TRACK_CHANGED") return EventType::MEDIA_TRACK_CHANGED;
    if (typeStr == "CONFIG_CHANGED") return EventType::CONFIG_CHANGED;
    if (typeStr == "CONFIG_SAVED") return EventType::CONFIG_SAVED;
    if (typeStr == "CUSTOM_BUTTON_1") return EventType::CUSTOM_BUTTON_1;
    if (typeStr == "CUSTOM_BUTTON_2") return EventType::CUSTOM_BUTTON_2;
    if (typeStr == "CUSTOM_BUTTON_3") return EventType::CUSTOM_BUTTON_3;
    if (typeStr == "CUSTOM_BUTTON_4") return EventType::CUSTOM_BUTTON_4;
    if (typeStr == "CUSTOM_BUTTON_5") return EventType::CUSTOM_BUTTON_5;
    if (typeStr == "CUSTOM_BUTTON_6") return EventType::CUSTOM_BUTTON_6;
    if (typeStr == "DAY_MODE_ENABLED") return EventType::DAY_MODE_ENABLED;
    if (typeStr == "NIGHT_MODE_ENABLED") return EventType::NIGHT_MODE_ENABLED;
    if (typeStr == "UPDATE_AVAILABLE") return EventType::UPDATE_AVAILABLE;
    if (typeStr == "UPDATE_STARTED") return EventType::UPDATE_STARTED;
    if (typeStr == "UPDATE_COMPLETED") return EventType::UPDATE_COMPLETED;
    if (typeStr == "UPDATE_FAILED") return EventType::UPDATE_FAILED;
    if (typeStr == "CUSTOM_EVENT") return EventType::CUSTOM_EVENT;
    throw std::invalid_argument("Unknown event type: " + typeStr);
}

}
}
}
}
