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

#ifdef ENABLE_MODERN_API

#include <memory>
#include "modern/ConfigurationManager.hpp"
#include "modern/EventBus.hpp"
#include "modern/RestApiServer.hpp"
#include "modern/StateMachine.hpp"

namespace openauto {
namespace modern {

/**
 * @brief Modern architecture integration manager
 *
 * This class manages the integration of modern components with the existing
 * OpenAuto application, providing a bridge between legacy and modern code.
 */
class ModernIntegration {
  public:
    static ModernIntegration& getInstance();

    bool initialize();
    void shutdown();

    bool isInitialized() const { return initialized_; }

    // Component access
    std::shared_ptr<EventBus> getEventBus() const { return eventBus_; }
    std::shared_ptr<StateMachine> getStateMachine() const { return stateMachine_; }
    std::shared_ptr<ConfigurationManager> getConfigManager() const { return configManager_; }
    std::shared_ptr<RestApiServer> getApiServer() const { return apiServer_; }

    // Configuration
    void setApiPort(int port);
    void setConfigPath(const std::string& path);

    // Legacy integration helpers
    void publishLegacyEvent(const std::string& eventType, const std::string& source,
                            const std::string& data = "");
    void updateLegacyState(const std::string& newState);
    void setLegacyConfig(const std::string& key, const std::string& value);
    std::string getLegacyConfig(const std::string& key, const std::string& defaultValue = "");

  private:
    ModernIntegration() = default;
    ~ModernIntegration() = default;
    ModernIntegration(const ModernIntegration&) = delete;
    ModernIntegration& operator=(const ModernIntegration&) = delete;

    void setupEventBus();
    void setupStateMachine();
    void setupConfigManager();
    void setupApiServer();
    void setupIntegrationCallbacks();
    void setupLogger();

    bool initialized_ = false;
    int apiPort_ = 8080;
    std::string configPath_ = "openauto.conf";

    std::shared_ptr<EventBus> eventBus_;
    std::shared_ptr<StateMachine> stateMachine_;
    std::shared_ptr<ConfigurationManager> configManager_;
    std::shared_ptr<RestApiServer> apiServer_;
};

// Convenience macros for legacy code integration
#define OPENAUTO_PUBLISH_EVENT(type, source, data)                                                 \
    if (openauto::modern::ModernIntegration::getInstance().isInitialized()) {                      \
        openauto::modern::ModernIntegration::getInstance().publishLegacyEvent(type, source, data); \
    }

#define OPENAUTO_SET_STATE(state)                                                    \
    if (openauto::modern::ModernIntegration::getInstance().isInitialized()) {        \
        openauto::modern::ModernIntegration::getInstance().updateLegacyState(state); \
    }

#define OPENAUTO_SET_CONFIG(key, value)                                                 \
    if (openauto::modern::ModernIntegration::getInstance().isInitialized()) {           \
        openauto::modern::ModernIntegration::getInstance().setLegacyConfig(key, value); \
    }

#define OPENAUTO_GET_CONFIG(key, defaultValue)                                                   \
    (openauto::modern::ModernIntegration::getInstance().isInitialized()                          \
         ? openauto::modern::ModernIntegration::getInstance().getLegacyConfig(key, defaultValue) \
         : defaultValue)

}  // namespace modern
}  // namespace openauto

#else

// Empty macros when modern API is disabled
#define OPENAUTO_PUBLISH_EVENT(type, source, data)
#define OPENAUTO_SET_STATE(state)
#define OPENAUTO_SET_CONFIG(key, value)
#define OPENAUTO_GET_CONFIG(key, defaultValue) (defaultValue)

#endif  // ENABLE_MODERN_API
