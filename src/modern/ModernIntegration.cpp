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

#ifdef ENABLE_MODERN_API

#include "modern/ModernIntegration.hpp"
#include <iostream>
#include <stdexcept>

namespace openauto {
namespace modern {

ModernIntegration& ModernIntegration::getInstance() {
    static ModernIntegration instance;
    return instance;
}

bool ModernIntegration::initialize() {
    if (initialized_) {
        return true;
    }
    
    try {
        std::cout << "Initializing Modern OpenAuto Architecture..." << std::endl;
        
        setupEventBus();
        setupStateMachine();
        setupConfigManager();
        setupApiServer();
        setupIntegrationCallbacks();
        
        initialized_ = true;
        std::cout << "Modern OpenAuto Architecture initialized successfully" << std::endl;
        
        // Publish initialization event
        publishLegacyEvent("SYSTEM_STARTUP", "modern_integration", "Modern architecture initialized");
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize Modern OpenAuto Architecture: " << e.what() << std::endl;
        shutdown();
        return false;
    }
}

void ModernIntegration::shutdown() {
    if (!initialized_) {
        return;
    }
    
    std::cout << "Shutting down Modern OpenAuto Architecture..." << std::endl;
    
    if (apiServer_) {
        apiServer_->stop();
    }
    
    if (configManager_) {
        configManager_->save();
    }
    
    publishLegacyEvent("SYSTEM_SHUTDOWN", "modern_integration", "Modern architecture shutting down");
    
    apiServer_.reset();
    configManager_.reset();
    stateMachine_.reset();
    eventBus_.reset();
    
    initialized_ = false;
    std::cout << "Modern OpenAuto Architecture shutdown complete" << std::endl;
}

void ModernIntegration::setApiPort(int port) {
    apiPort_ = port;
    if (apiServer_) {
        apiServer_->setPort(port);
    }
}

void ModernIntegration::setConfigPath(const std::string& path) {
    configPath_ = path;
}

void ModernIntegration::publishLegacyEvent(const std::string& eventType, const std::string& source, const std::string& data) {
    if (!eventBus_) {
        return;
    }
    
    try {
        auto eventTypeEnum = Event::stringToEventType(eventType);
        auto event = std::make_shared<Event>(eventTypeEnum, source);
        
        if (!data.empty()) {
            event->setData("legacy_data", data);
        }
        
        eventBus_->publish(event);
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to publish legacy event: " << e.what() << std::endl;
    }
}

void ModernIntegration::updateLegacyState(const std::string& newState) {
    if (!stateMachine_) {
        return;
    }
    
    try {
        stateMachine_->transitionTo(newState);
    } catch (const std::exception& e) {
        std::cerr << "Failed to update legacy state: " << e.what() << std::endl;
    }
}

void ModernIntegration::setLegacyConfig(const std::string& key, const std::string& value) {
    if (!configManager_) {
        return;
    }
    
    try {
        configManager_->set(key, value);
    } catch (const std::exception& e) {
        std::cerr << "Failed to set legacy config: " << e.what() << std::endl;
    }
}

std::string ModernIntegration::getLegacyConfig(const std::string& key, const std::string& defaultValue) {
    if (!configManager_) {
        return defaultValue;
    }
    
    try {
        auto value = configManager_->get(key);
        if (value.is_null()) {
            return defaultValue;
        }
        
        if (value.is_string()) {
            return value.get<std::string>();
        } else {
            return value.dump();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to get legacy config: " << e.what() << std::endl;
        return defaultValue;
    }
}

void ModernIntegration::setupEventBus() {
    eventBus_ = std::make_shared<EventBus>();
    
    // Subscribe to all events for logging
    eventBus_->subscribeToAll([](const Event::Pointer& event) {
        std::cout << "Event: " << event->toString() << std::endl;
    });
}

void ModernIntegration::setupStateMachine() {
    stateMachine_ = std::make_shared<StateMachine>();
    
    // Define common OpenAuto states
    stateMachine_->addState("initializing", "System is initializing");
    stateMachine_->addState("idle", "Waiting for device connection");
    stateMachine_->addState("connected", "Device connected, ready for projection");
    stateMachine_->addState("projection", "Android Auto projection active");
    stateMachine_->addState("camera", "Camera view active");
    stateMachine_->addState("settings", "Settings menu open");
    stateMachine_->addState("media", "Media player active");
    stateMachine_->addState("navigation", "Navigation active");
    stateMachine_->addState("error", "Error state");
    stateMachine_->addState("shutdown", "System shutting down");
    
    // Define transitions
    stateMachine_->addTransition("initializing", "idle", "init_complete");
    stateMachine_->addTransition("idle", "connected", "device_connected");
    stateMachine_->addTransition("connected", "projection", "start_projection");
    stateMachine_->addTransition("projection", "connected", "stop_projection");
    stateMachine_->addTransition("connected", "idle", "device_disconnected");
    
    // Camera can be accessed from any state
    stateMachine_->addTransition("*", "camera", "camera_requested");
    stateMachine_->addTransition("camera", "*", "camera_exit");
    
    // Settings can be accessed from any state except error
    stateMachine_->addTransition("idle", "settings", "settings_opened");
    stateMachine_->addTransition("connected", "settings", "settings_opened");
    stateMachine_->addTransition("projection", "settings", "settings_opened");
    stateMachine_->addTransition("settings", "*", "settings_closed");
    
    // Media and navigation states during projection
    stateMachine_->addTransition("projection", "media", "media_started");
    stateMachine_->addTransition("projection", "navigation", "navigation_started");
    stateMachine_->addTransition("media", "projection", "media_stopped");
    stateMachine_->addTransition("navigation", "projection", "navigation_stopped");
    
    // Error state can be reached from anywhere
    stateMachine_->addTransition("*", "error", "error_occurred");
    stateMachine_->addTransition("error", "idle", "error_recovered");
    
    // Shutdown from any state
    stateMachine_->addTransition("*", "shutdown", "shutdown_requested");
    
    // Set initial state
    stateMachine_->transitionTo("initializing");
    
    // Listen for state changes and publish events
    stateMachine_->onStateChanged([this](const std::string& from, const std::string& to) {
        auto event = std::make_shared<Event>(EventType::CUSTOM_EVENT, "state_machine");
        event->setData("event_type", std::string("STATE_CHANGED"));
        event->setData("from_state", from);
        event->setData("to_state", to);
        eventBus_->publish(event);
    });
}

void ModernIntegration::setupConfigManager() {
    configManager_ = std::make_shared<ConfigurationManager>();
    configManager_->setConfigPath(configPath_);
    
    // Set default configuration values
    configManager_->setDefault("ui.brightness", 75);
    configManager_->setDefault("ui.volume", 50);
    configManager_->setDefault("ui.theme", "dark");
    configManager_->setDefault("ui.language", "en");
    configManager_->setDefault("ui.auto_launch", true);
    
    configManager_->setDefault("audio.enabled", true);
    configManager_->setDefault("audio.sample_rate", 48000);
    configManager_->setDefault("audio.channels", 2);
    
    configManager_->setDefault("video.enabled", true);
    configManager_->setDefault("video.width", 1920);
    configManager_->setDefault("video.height", 1080);
    configManager_->setDefault("video.fps", 60);
    
    configManager_->setDefault("network.wifi.enabled", true);
    configManager_->setDefault("network.bluetooth.enabled", true);
    configManager_->setDefault("network.hotspot.enabled", false);
    
    configManager_->setDefault("camera.enabled", true);
    configManager_->setDefault("camera.rear.device", "/dev/video0");
    configManager_->setDefault("camera.front.device", "/dev/video1");
    
    configManager_->setDefault("api.enabled", true);
    configManager_->setDefault("api.port", apiPort_);
    configManager_->setDefault("api.cors.enabled", true);
    
    // Load existing configuration
    configManager_->load();
    
    // Update API port from configuration
    apiPort_ = configManager_->get("api.port").get<int>();
}

void ModernIntegration::setupApiServer() {
    apiServer_ = std::make_shared<RestApiServer>(
        apiPort_, eventBus_, stateMachine_, configManager_);
    
    // Start the API server if enabled
    if (configManager_->get("api.enabled").get<bool>()) {
        if (apiServer_->start()) {
            std::cout << "REST API server started on port " << apiPort_ << std::endl;
            
            // Publish API started event
            auto event = std::make_shared<Event>(EventType::CUSTOM_EVENT, "api_server");
            event->setData("event_type", std::string("API_STARTED"));
            event->setData("port", apiPort_);
            eventBus_->publish(event);
        } else {
            std::cerr << "Failed to start REST API server on port " << apiPort_ << std::endl;
        }
    }
}

void ModernIntegration::setupIntegrationCallbacks() {
    // Subscribe to configuration change events and save automatically
    eventBus_->subscribe(EventType::CONFIG_CHANGED, [this](const Event::Pointer& event) {
        configManager_->save();
    });
    
    // Subscribe to Android Auto events and update state machine
    eventBus_->subscribe(EventType::ANDROID_AUTO_CONNECTED, [this](const Event::Pointer& event) {
        stateMachine_->transitionTo("connected");
    });
    
    eventBus_->subscribe(EventType::ANDROID_AUTO_DISCONNECTED, [this](const Event::Pointer& event) {
        stateMachine_->transitionTo("idle");
    });
    
    eventBus_->subscribe(EventType::ANDROID_AUTO_START, [this](const Event::Pointer& event) {
        stateMachine_->transitionTo("projection");
    });
    
    eventBus_->subscribe(EventType::ANDROID_AUTO_STOP, [this](const Event::Pointer& event) {
        if (stateMachine_->getCurrentStateName() == "projection") {
            stateMachine_->transitionTo("connected");
        }
    });
    
    // Subscribe to camera events
    eventBus_->subscribe(EventType::CAMERA_SHOW, [this](const Event::Pointer& event) {
        stateMachine_->transitionTo("camera");
    });
    
    eventBus_->subscribe(EventType::CAMERA_HIDE, [this](const Event::Pointer& event) {
        // Return to previous state before camera
        auto previousState = stateMachine_->getPreviousStateName();
        if (!previousState.empty() && previousState != "camera") {
            stateMachine_->transitionTo(previousState);
        } else {
            stateMachine_->transitionTo("idle");
        }
    });
    
    // Subscribe to system events
    eventBus_->subscribe(EventType::SYSTEM_SHUTDOWN, [this](const Event::Pointer& event) {
        stateMachine_->transitionTo("shutdown");
    });
    
    eventBus_->subscribe(EventType::SYSTEM_ERROR, [this](const Event::Pointer& event) {
        stateMachine_->transitionTo("error");
    });
}

} // namespace modern
} // namespace openauto

#endif // ENABLE_MODERN_API
