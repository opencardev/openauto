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
#include "modern/Logger.hpp"

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

        setupLogger();
        setupEventBus();
        setupStateMachine();
        setupConfigManager();
        setupApiServer();
        setupIntegrationCallbacks();

        initialized_ = true;
        std::cout << "Modern OpenAuto Architecture initialized successfully" << std::endl;

        // Publish initialization event
        publishLegacyEvent("SYSTEM_STARTUP", "modern_integration",
                           "Modern architecture initialized");

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

    publishLegacyEvent("SYSTEM_SHUTDOWN", "modern_integration",
                       "Modern architecture shutting down");

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

void ModernIntegration::setConfigPath(const std::string& path) { configPath_ = path; }

void ModernIntegration::publishLegacyEvent(const std::string& eventType, const std::string& source,
                                           const std::string& data) {
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
        // Map string state to appropriate trigger - this is a simplified mapping
        if (newState == "connected") {
            stateMachine_->transition(openauto::modern::Trigger::ANDROID_AUTO_CONNECT);
        } else if (newState == "idle") {
            stateMachine_->transition(openauto::modern::Trigger::ANDROID_AUTO_DISCONNECT);
        } else if (newState == "projection") {
            stateMachine_->transition(openauto::modern::Trigger::ANDROID_AUTO_CONNECT);
        } else if (newState == "camera") {
            stateMachine_->transition(openauto::modern::Trigger::CAMERA_BUTTON_PRESS);
        } else if (newState == "settings") {
            stateMachine_->transition(openauto::modern::Trigger::SETTINGS_BUTTON_PRESS);
        } else if (newState == "error") {
            stateMachine_->transition(openauto::modern::Trigger::ERROR_OCCURRED);
        } else if (newState == "shutdown") {
            stateMachine_->transition(openauto::modern::Trigger::SHUTDOWN_REQUEST);
        }
        // Add more state mappings as needed
    } catch (const std::exception& e) {
        std::cerr << "Failed to update legacy state: " << e.what() << std::endl;
    }
}

void ModernIntegration::setLegacyConfig(const std::string& key, const std::string& value) {
    if (!configManager_) {
        return;
    }

    try {
        configManager_->setValue(key, openauto::modern::ConfigValue(value));
    } catch (const std::exception& e) {
        std::cerr << "Failed to set legacy config: " << e.what() << std::endl;
    }
}

std::string ModernIntegration::getLegacyConfig(const std::string& key,
                                               const std::string& defaultValue) {
    if (!configManager_) {
        return defaultValue;
    }

    try {
        return configManager_->getValue<std::string>(key, defaultValue);
    } catch (const std::exception& e) {
        std::cerr << "Failed to get legacy config: " << e.what() << std::endl;
        return defaultValue;
    }
}

void ModernIntegration::setupEventBus() {
    // Use the singleton EventBus instance
    auto& eventBusInstance = openauto::modern::EventBus::getInstance();
    eventBus_ = std::shared_ptr<openauto::modern::EventBus>(&eventBusInstance,
                                                            [](openauto::modern::EventBus*) {
                                                                // Custom no-op deleter since it's a
                                                                // singleton
                                                            });

    // Note: subscribeToAll method doesn't exist in the current EventBus interface
    // This would need to be implemented if required
}

void ModernIntegration::setupStateMachine() {
    stateMachine_ = std::make_shared<openauto::modern::StateMachine>();

    // The StateMachine uses built-in states and triggers
    // Set up state change callback to publish events
    stateMachine_->setStateChangeCallback([this](openauto::modern::SystemState oldState,
                                                 openauto::modern::SystemState newState,
                                                 openauto::modern::Trigger trigger) {
        auto event = std::make_shared<openauto::modern::Event>(
            openauto::modern::EventType::CUSTOM_EVENT, "state_machine");
        event->setData("event_type", std::string("STATE_CHANGED"));
        event->setData("from_state", stateMachine_->stateToString(oldState));
        event->setData("to_state", stateMachine_->stateToString(newState));
        event->setData("trigger", stateMachine_->triggerToString(trigger));
        eventBus_->publish(event);
    });

    // Note: The actual StateMachine has predefined states and transitions
    // No need to manually add states or transitions
}

void ModernIntegration::setupConfigManager() {
    configManager_ = std::make_shared<openauto::modern::ConfigurationManager>();
    configManager_->setConfigPath(configPath_);

    // Load default configuration values (use setValue since setDefault doesn't exist)
    configManager_->setValue("ui.brightness", openauto::modern::ConfigValue(75));
    configManager_->setValue("ui.volume", openauto::modern::ConfigValue(50));
    configManager_->setValue("ui.theme", openauto::modern::ConfigValue(std::string("dark")));
    configManager_->setValue("ui.language", openauto::modern::ConfigValue(std::string("en")));
    configManager_->setValue("ui.auto_launch", openauto::modern::ConfigValue(true));

    configManager_->setValue("audio.enabled", openauto::modern::ConfigValue(true));
    configManager_->setValue("audio.sample_rate", openauto::modern::ConfigValue(48000));
    configManager_->setValue("audio.channels", openauto::modern::ConfigValue(2));

    configManager_->setValue("video.enabled", openauto::modern::ConfigValue(true));
    configManager_->setValue("video.width", openauto::modern::ConfigValue(1920));
    configManager_->setValue("video.height", openauto::modern::ConfigValue(1080));
    configManager_->setValue("video.fps", openauto::modern::ConfigValue(60));

    configManager_->setValue("network.wifi.enabled", openauto::modern::ConfigValue(true));
    configManager_->setValue("network.bluetooth.enabled", openauto::modern::ConfigValue(true));
    configManager_->setValue("network.hotspot.enabled", openauto::modern::ConfigValue(false));

    configManager_->setValue("camera.enabled", openauto::modern::ConfigValue(true));
    configManager_->setValue("camera.rear.device",
                             openauto::modern::ConfigValue(std::string("/dev/video0")));
    configManager_->setValue("camera.front.device",
                             openauto::modern::ConfigValue(std::string("/dev/video1")));

    configManager_->setValue("api.enabled", openauto::modern::ConfigValue(true));
    configManager_->setValue("api.port", openauto::modern::ConfigValue(apiPort_));
    configManager_->setValue("api.cors.enabled", openauto::modern::ConfigValue(true));

    // Load existing configuration
    configManager_->load();

    // Update API port from configuration
    apiPort_ = configManager_->getValue<int>("api.port", apiPort_);
}

void ModernIntegration::setupApiServer() {
    apiServer_ = std::make_shared<openauto::modern::RestApiServer>(apiPort_, eventBus_,
                                                                   stateMachine_, configManager_);

    // Start the API server if enabled
    if (configManager_->getValue<bool>("api.enabled", true)) {
        if (apiServer_->start()) {
            std::cout << "REST API server started on port " << apiPort_ << std::endl;

            // Publish API started event
            auto event = std::make_shared<openauto::modern::Event>(
                openauto::modern::EventType::CUSTOM_EVENT, "api_server");
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
    eventBus_->subscribe(
        openauto::modern::EventType::CONFIG_CHANGED, "ModernIntegration_ConfigChanged",
        [this](const openauto::modern::Event::Pointer& event) { configManager_->save(); });

    // Subscribe to Android Auto events and update state machine
    eventBus_->subscribe(
        openauto::modern::EventType::ANDROID_AUTO_CONNECTED, "ModernIntegration_AAConnected",
        [this](const openauto::modern::Event::Pointer& event) {
            stateMachine_->transition(openauto::modern::Trigger::ANDROID_AUTO_CONNECT);
        });

    eventBus_->subscribe(
        openauto::modern::EventType::ANDROID_AUTO_DISCONNECTED, "ModernIntegration_AADisconnected",
        [this](const openauto::modern::Event::Pointer& event) {
            stateMachine_->transition(openauto::modern::Trigger::ANDROID_AUTO_DISCONNECT);
        });

    eventBus_->subscribe(
        openauto::modern::EventType::ANDROID_AUTO_START, "ModernIntegration_AAStart",
        [this](const openauto::modern::Event::Pointer& event) {
            stateMachine_->transition(openauto::modern::Trigger::ANDROID_AUTO_CONNECT);
        });

    eventBus_->subscribe(
        openauto::modern::EventType::ANDROID_AUTO_STOP, "ModernIntegration_AAStop",
        [this](const openauto::modern::Event::Pointer& event) {
            // Transition back if currently in Android Auto state
            auto currentState = stateMachine_->getCurrentState();
            if (currentState == openauto::modern::SystemState::ANDROID_AUTO_ACTIVE) {
                stateMachine_->transition(openauto::modern::Trigger::ANDROID_AUTO_DISCONNECT);
            }
        });

    // Subscribe to camera events
    eventBus_->subscribe(
        openauto::modern::EventType::CAMERA_SHOW, "ModernIntegration_CameraShow",
        [this](const openauto::modern::Event::Pointer& event) {
            stateMachine_->transition(openauto::modern::Trigger::CAMERA_BUTTON_PRESS);
        });

    eventBus_->subscribe(
        openauto::modern::EventType::CAMERA_HIDE, "ModernIntegration_CameraHide",
        [this](const openauto::modern::Event::Pointer& event) {
            // Return to previous appropriate state (simplified)
            stateMachine_->transition(openauto::modern::Trigger::BACK_BUTTON_PRESS);
        });

    // Subscribe to system events
    eventBus_->subscribe(openauto::modern::EventType::SYSTEM_SHUTDOWN, "ModernIntegration_Shutdown",
                         [this](const openauto::modern::Event::Pointer& event) {
                             stateMachine_->transition(openauto::modern::Trigger::SHUTDOWN_REQUEST);
                         });

    eventBus_->subscribe(openauto::modern::EventType::SYSTEM_ERROR, "ModernIntegration_Error",
                         [this](const openauto::modern::Event::Pointer& event) {
                             stateMachine_->transition(openauto::modern::Trigger::ERROR_OCCURRED);
                         });
}

void ModernIntegration::setupLogger() {
    auto& logger = openauto::modern::Logger::getInstance();

    // Configure logger based on configuration
    logger.setLevel(openauto::modern::LogLevel::INFO);
    logger.setAsync(true);
    logger.setMaxQueueSize(5000);

    // Add file sink for persistent logging
    auto fileSink =
        std::make_shared<openauto::modern::FileSink>("openauto.log", 10 * 1024 * 1024, 5);
    logger.addSink(fileSink);

    // Use JSON formatter for file logging
    auto jsonFormatter = std::make_shared<openauto::modern::JsonFormatter>(false);
    logger.setFormatter(jsonFormatter);

    // Set category-specific log levels
    logger.setCategoryLevel(openauto::modern::LogCategory::SYSTEM,
                            openauto::modern::LogLevel::DEBUG);
    logger.setCategoryLevel(openauto::modern::LogCategory::ANDROID_AUTO,
                            openauto::modern::LogLevel::INFO);
    logger.setCategoryLevel(openauto::modern::LogCategory::UI, openauto::modern::LogLevel::INFO);
    logger.setCategoryLevel(openauto::modern::LogCategory::API, openauto::modern::LogLevel::DEBUG);
    logger.setCategoryLevel(openauto::modern::LogCategory::EVENT,
                            openauto::modern::LogLevel::DEBUG);
    logger.setCategoryLevel(openauto::modern::LogCategory::STATE,
                            openauto::modern::LogLevel::DEBUG);

    SLOG_INFO(SYSTEM, "ModernIntegration", "Modern logger initialized successfully");
}

}  // namespace modern
}  // namespace openauto

#endif  // ENABLE_MODERN_API
