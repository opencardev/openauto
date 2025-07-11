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

#include <string>
#include <unordered_map>
#include <variant>
#include <mutex>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "Event.hpp"
#include "EventBus.hpp"

namespace openauto {
namespace modern {

using ConfigValue = std::variant<std::string, int, double, bool>;

class ConfigurationManager {
public:
    ConfigurationManager(const std::string& configPath = "config.json");
    ~ConfigurationManager();
    
    // Configuration management
    bool load();
    bool save();
    void reset();
    
    // Value access
    template<typename T>
    T getValue(const std::string& key, const T& defaultValue = T{}) const;
    
    void setValue(const std::string& key, const ConfigValue& value);
    bool hasValue(const std::string& key) const;
    void removeValue(const std::string& key);
    
    // Bulk operations
    std::unordered_map<std::string, ConfigValue> getAllValues() const;
    void setValues(const std::unordered_map<std::string, ConfigValue>& values);
    
    // JSON operations
    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& json);
    
    // Event integration
    void setEventBus(std::shared_ptr<EventBus> eventBus);
    
    // Path management
    void setConfigPath(const std::string& path);
    std::string getConfigPath() const;
    
    // Validation
    bool isValid() const;
    std::vector<std::string> validate() const;
    
    // Default configuration
    void loadDefaults();

private:
    mutable std::mutex mutex_;
    std::string configPath_;
    std::unordered_map<std::string, ConfigValue> values_;
    std::shared_ptr<EventBus> eventBus_;
    
    // Helper methods
    void notifyConfigChanged(const std::string& key, const ConfigValue& value);
    ConfigValue jsonToConfigValue(const nlohmann::json& value);
    nlohmann::json configValueToJson(const ConfigValue& value);
    
    // Default values
    void setDefaultValues();
};

// Template implementation
template<typename T>
T ConfigurationManager::getValue(const std::string& key, const T& defaultValue) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = values_.find(key);
    if (it != values_.end()) {
        try {
            return std::get<T>(it->second);
        } catch (const std::bad_variant_access&) {
            // Type mismatch, return default
            return defaultValue;
        }
    }
    
    return defaultValue;
}

} // namespace modern
} // namespace openauto
