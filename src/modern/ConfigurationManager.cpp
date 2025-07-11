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

#include "modern/ConfigurationManager.hpp"
#include <fstream>
#include <iostream>

namespace openauto {
namespace modern {

ConfigurationManager::ConfigurationManager(const std::string& configPath)
    : configPath_(configPath) {
    setDefaultValues();
}

ConfigurationManager::~ConfigurationManager() = default;

bool ConfigurationManager::load() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!std::filesystem::exists(configPath_)) {
        // Config file doesn't exist, use defaults
        return true;
    }
    
    try {
        std::ifstream file(configPath_);
        if (!file.is_open()) {
            return false;
        }
        
        nlohmann::json json;
        file >> json;
        
        values_.clear();
        for (const auto& [key, value] : json.items()) {
            values_[key] = jsonToConfigValue(value);
        }
        
        return true;
    } catch (const std::exception& e) {
        // Log error and keep current values
        return false;
    }
}

bool ConfigurationManager::save() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // Create directory if it doesn't exist
        std::filesystem::path configDir = std::filesystem::path(configPath_).parent_path();
        if (!configDir.empty()) {
            std::filesystem::create_directories(configDir);
        }
        
        std::ofstream file(configPath_);
        if (!file.is_open()) {
            return false;
        }
        
        nlohmann::json json;
        for (const auto& [key, value] : values_) {
            json[key] = configValueToJson(value);
        }
        
        file << json.dump(2);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void ConfigurationManager::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    values_.clear();
    setDefaultValues();
    
    if (eventBus_) {
        auto event = std::make_shared<Event>(EventType::CONFIG_CHANGED, "config_manager");
        event->setData("action", std::string("reset"));
        eventBus_->publish(event);
    }
}

void ConfigurationManager::setValue(const std::string& key, const ConfigValue& value) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        values_[key] = value;
    }
    
    notifyConfigChanged(key, value);
}

bool ConfigurationManager::hasValue(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return values_.find(key) != values_.end();
}

void ConfigurationManager::removeValue(const std::string& key) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        values_.erase(key);
    }
    
    if (eventBus_) {
        auto event = std::make_shared<Event>(EventType::CONFIG_CHANGED, "config_manager");
        event->setData("key", key);
        event->setData("action", std::string("removed"));
        eventBus_->publish(event);
    }
}

std::unordered_map<std::string, ConfigValue> ConfigurationManager::getAllValues() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return values_;
}

void ConfigurationManager::setValues(const std::unordered_map<std::string, ConfigValue>& values) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        values_ = values;
    }
    
    if (eventBus_) {
        auto event = std::make_shared<Event>(EventType::CONFIG_CHANGED, "config_manager");
        event->setData("action", std::string("bulk_update"));
        event->setData("count", static_cast<int>(values.size()));
        eventBus_->publish(event);
    }
}

nlohmann::json ConfigurationManager::toJson() const {
    std::lock_guard<std::mutex> lock(mutex_);
    nlohmann::json json;
    
    for (const auto& [key, value] : values_) {
        json[key] = configValueToJson(value);
    }
    
    return json;
}

void ConfigurationManager::fromJson(const nlohmann::json& json) {
    std::unordered_map<std::string, ConfigValue> newValues;
    
    for (const auto& [key, value] : json.items()) {
        newValues[key] = jsonToConfigValue(value);
    }
    
    setValues(newValues);
}

void ConfigurationManager::setEventBus(std::shared_ptr<EventBus> eventBus) {
    eventBus_ = eventBus;
}

void ConfigurationManager::setConfigPath(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    configPath_ = path;
}

std::string ConfigurationManager::getConfigPath() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return configPath_;
}

bool ConfigurationManager::isValid() const {
    return validate().empty();
}

std::vector<std::string> ConfigurationManager::validate() const {
    std::vector<std::string> errors;
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Add validation rules here
    // Example: Check required keys
    std::vector<std::string> requiredKeys = {
        "audio.volume",
        "video.brightness",
        "system.language"
    };
    
    for (const auto& key : requiredKeys) {
        if (values_.find(key) == values_.end()) {
            errors.push_back("Missing required configuration key: " + key);
        }
    }
    
    return errors;
}

void ConfigurationManager::loadDefaults() {
    std::lock_guard<std::mutex> lock(mutex_);
    setDefaultValues();
}

void ConfigurationManager::notifyConfigChanged(const std::string& key, const ConfigValue& value) {
    if (eventBus_) {
        auto event = std::make_shared<Event>(EventType::CONFIG_CHANGED, "config_manager");
        event->setData("key", key);
        event->setData("action", std::string("changed"));
        
        // Add the actual value based on its type
        std::visit([&event](const auto& v) {
            event->setData("value", EventValue(v));
        }, value);
        
        eventBus_->publish(event);
    }
}

ConfigValue ConfigurationManager::jsonToConfigValue(const nlohmann::json& value) {
    if (value.is_string()) {
        return value.get<std::string>();
    } else if (value.is_number_integer()) {
        return value.get<int>();
    } else if (value.is_number_float()) {
        return value.get<double>();
    } else if (value.is_boolean()) {
        return value.get<bool>();
    } else {
        // Default to string representation
        return value.dump();
    }
}

nlohmann::json ConfigurationManager::configValueToJson(const ConfigValue& value) {
    return std::visit([](const auto& v) -> nlohmann::json {
        return v;
    }, value);
}

void ConfigurationManager::setDefaultValues() {
    // Audio settings
    values_["audio.volume"] = 50;
    values_["audio.muted"] = false;
    
    // Video settings
    values_["video.brightness"] = 75;
    values_["video.day_mode"] = true;
    values_["video.resolution"] = std::string("1920x1080");
    
    // System settings
    values_["system.language"] = std::string("en_US");
    values_["system.timezone"] = std::string("UTC");
    values_["system.auto_start_android_auto"] = true;
    
    // Network settings
    values_["network.wifi_enabled"] = true;
    values_["network.hotspot_enabled"] = false;
    values_["network.bluetooth_enabled"] = true;
    
    // Camera settings
    values_["camera.enabled"] = true;
    values_["camera.auto_record"] = false;
    values_["camera.quality"] = std::string("high");
    
    // API settings
    values_["api.enabled"] = true;
    values_["api.port"] = 8080;
    values_["api.bind_address"] = std::string("127.0.0.1");
    
    // Debug settings
    values_["debug.log_level"] = std::string("info");
    values_["debug.enable_event_logging"] = false;
}

} // namespace modern
} // namespace openauto
