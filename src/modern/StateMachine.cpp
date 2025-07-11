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

#include "modern/StateMachine.hpp"
#include <stdexcept>

namespace openauto {
namespace modern {

StateMachine::StateMachine() : currentState_(SystemState::INITIALIZING) {
    initializeTransitions();
}

StateMachine::~StateMachine() = default;

SystemState StateMachine::getCurrentState() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return currentState_;
}

bool StateMachine::transition(Trigger trigger) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Transition trans{currentState_, trigger, SystemState::INITIALIZING};
    auto it = transitions_.find(trans);
    
    if (it != transitions_.end()) {
        SystemState oldState = currentState_;
        SystemState newState = it->second;
        
        // Execute state exit callback
        executeStateExit(oldState);
        
        // Change state
        currentState_ = newState;
        
        // Execute state entry callback
        executeStateEntry(newState);
        
        // Notify state change
        if (stateChangeCallback_) {
            stateChangeCallback_(oldState, newState, trigger);
        }
        
        return true;
    }
    
    return false;
}

bool StateMachine::canTransition(Trigger trigger) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return isValidTransition(currentState_, trigger);
}

void StateMachine::setStateChangeCallback(StateChangeCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    stateChangeCallback_ = callback;
}

void StateMachine::setStateEntryCallback(SystemState state, StateEntryCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    entryCallbacks_[state] = callback;
}

void StateMachine::setStateExitCallback(SystemState state, StateExitCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    exitCallbacks_[state] = callback;
}

void StateMachine::handleEvent(const Event::Pointer& event) {
    if (!event) return;
    
    Trigger trigger = eventTypeToTrigger(event->getType());
    transition(trigger);
}

std::string StateMachine::stateToString(SystemState state) const {
    switch (state) {
        case SystemState::INITIALIZING: return "INITIALIZING";
        case SystemState::IDLE: return "IDLE";
        case SystemState::ANDROID_AUTO_ACTIVE: return "ANDROID_AUTO_ACTIVE";
        case SystemState::CAMERA_VIEW: return "CAMERA_VIEW";
        case SystemState::SETTINGS: return "SETTINGS";
        case SystemState::BLUETOOTH_PAIRING: return "BLUETOOTH_PAIRING";
        case SystemState::WIFI_SETUP: return "WIFI_SETUP";
        case SystemState::UPDATING: return "UPDATING";
        case SystemState::SHUTTING_DOWN: return "SHUTTING_DOWN";
        case SystemState::ERROR_STATE: return "ERROR_STATE";
        default: return "UNKNOWN";
    }
}

std::string StateMachine::triggerToString(Trigger trigger) const {
    switch (trigger) {
        case Trigger::SYSTEM_START: return "SYSTEM_START";
        case Trigger::ANDROID_AUTO_CONNECT: return "ANDROID_AUTO_CONNECT";
        case Trigger::ANDROID_AUTO_DISCONNECT: return "ANDROID_AUTO_DISCONNECT";
        case Trigger::CAMERA_BUTTON_PRESS: return "CAMERA_BUTTON_PRESS";
        case Trigger::SETTINGS_BUTTON_PRESS: return "SETTINGS_BUTTON_PRESS";
        case Trigger::BACK_BUTTON_PRESS: return "BACK_BUTTON_PRESS";
        case Trigger::BLUETOOTH_PAIR_REQUEST: return "BLUETOOTH_PAIR_REQUEST";
        case Trigger::WIFI_SETUP_REQUEST: return "WIFI_SETUP_REQUEST";
        case Trigger::UPDATE_START: return "UPDATE_START";
        case Trigger::SHUTDOWN_REQUEST: return "SHUTDOWN_REQUEST";
        case Trigger::ERROR_OCCURRED: return "ERROR_OCCURRED";
        case Trigger::OPERATION_COMPLETE: return "OPERATION_COMPLETE";
        default: return "UNKNOWN";
    }
}

std::vector<Trigger> StateMachine::getValidTransitions() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Trigger> validTriggers;
    
    for (const auto& [transition, toState] : transitions_) {
        if (transition.fromState == currentState_) {
            validTriggers.push_back(transition.trigger);
        }
    }
    
    return validTriggers;
}

void StateMachine::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    SystemState oldState = currentState_;
    
    executeStateExit(oldState);
    currentState_ = SystemState::INITIALIZING;
    executeStateEntry(currentState_);
    
    if (stateChangeCallback_) {
        stateChangeCallback_(oldState, currentState_, Trigger::SYSTEM_START);
    }
}

void StateMachine::initializeTransitions() {
    // From INITIALIZING
    transitions_[{SystemState::INITIALIZING, Trigger::SYSTEM_START, SystemState::IDLE}] = SystemState::IDLE;
    transitions_[{SystemState::INITIALIZING, Trigger::ERROR_OCCURRED, SystemState::ERROR_STATE}] = SystemState::ERROR_STATE;
    
    // From IDLE
    transitions_[{SystemState::IDLE, Trigger::ANDROID_AUTO_CONNECT, SystemState::ANDROID_AUTO_ACTIVE}] = SystemState::ANDROID_AUTO_ACTIVE;
    transitions_[{SystemState::IDLE, Trigger::CAMERA_BUTTON_PRESS, SystemState::CAMERA_VIEW}] = SystemState::CAMERA_VIEW;
    transitions_[{SystemState::IDLE, Trigger::SETTINGS_BUTTON_PRESS, SystemState::SETTINGS}] = SystemState::SETTINGS;
    transitions_[{SystemState::IDLE, Trigger::BLUETOOTH_PAIR_REQUEST, SystemState::BLUETOOTH_PAIRING}] = SystemState::BLUETOOTH_PAIRING;
    transitions_[{SystemState::IDLE, Trigger::WIFI_SETUP_REQUEST, SystemState::WIFI_SETUP}] = SystemState::WIFI_SETUP;
    transitions_[{SystemState::IDLE, Trigger::UPDATE_START, SystemState::UPDATING}] = SystemState::UPDATING;
    transitions_[{SystemState::IDLE, Trigger::SHUTDOWN_REQUEST, SystemState::SHUTTING_DOWN}] = SystemState::SHUTTING_DOWN;
    transitions_[{SystemState::IDLE, Trigger::ERROR_OCCURRED, SystemState::ERROR_STATE}] = SystemState::ERROR_STATE;
    
    // From ANDROID_AUTO_ACTIVE
    transitions_[{SystemState::ANDROID_AUTO_ACTIVE, Trigger::ANDROID_AUTO_DISCONNECT, SystemState::IDLE}] = SystemState::IDLE;
    transitions_[{SystemState::ANDROID_AUTO_ACTIVE, Trigger::CAMERA_BUTTON_PRESS, SystemState::CAMERA_VIEW}] = SystemState::CAMERA_VIEW;
    transitions_[{SystemState::ANDROID_AUTO_ACTIVE, Trigger::ERROR_OCCURRED, SystemState::ERROR_STATE}] = SystemState::ERROR_STATE;
    
    // From CAMERA_VIEW
    transitions_[{SystemState::CAMERA_VIEW, Trigger::BACK_BUTTON_PRESS, SystemState::IDLE}] = SystemState::IDLE;
    transitions_[{SystemState::CAMERA_VIEW, Trigger::ANDROID_AUTO_CONNECT, SystemState::ANDROID_AUTO_ACTIVE}] = SystemState::ANDROID_AUTO_ACTIVE;
    transitions_[{SystemState::CAMERA_VIEW, Trigger::ERROR_OCCURRED, SystemState::ERROR_STATE}] = SystemState::ERROR_STATE;
    
    // From SETTINGS
    transitions_[{SystemState::SETTINGS, Trigger::BACK_BUTTON_PRESS, SystemState::IDLE}] = SystemState::IDLE;
    transitions_[{SystemState::SETTINGS, Trigger::BLUETOOTH_PAIR_REQUEST, SystemState::BLUETOOTH_PAIRING}] = SystemState::BLUETOOTH_PAIRING;
    transitions_[{SystemState::SETTINGS, Trigger::WIFI_SETUP_REQUEST, SystemState::WIFI_SETUP}] = SystemState::WIFI_SETUP;
    transitions_[{SystemState::SETTINGS, Trigger::UPDATE_START, SystemState::UPDATING}] = SystemState::UPDATING;
    transitions_[{SystemState::SETTINGS, Trigger::ERROR_OCCURRED, SystemState::ERROR_STATE}] = SystemState::ERROR_STATE;
    
    // From BLUETOOTH_PAIRING
    transitions_[{SystemState::BLUETOOTH_PAIRING, Trigger::OPERATION_COMPLETE, SystemState::SETTINGS}] = SystemState::SETTINGS;
    transitions_[{SystemState::BLUETOOTH_PAIRING, Trigger::BACK_BUTTON_PRESS, SystemState::SETTINGS}] = SystemState::SETTINGS;
    transitions_[{SystemState::BLUETOOTH_PAIRING, Trigger::ERROR_OCCURRED, SystemState::ERROR_STATE}] = SystemState::ERROR_STATE;
    
    // From WIFI_SETUP
    transitions_[{SystemState::WIFI_SETUP, Trigger::OPERATION_COMPLETE, SystemState::SETTINGS}] = SystemState::SETTINGS;
    transitions_[{SystemState::WIFI_SETUP, Trigger::BACK_BUTTON_PRESS, SystemState::SETTINGS}] = SystemState::SETTINGS;
    transitions_[{SystemState::WIFI_SETUP, Trigger::ERROR_OCCURRED, SystemState::ERROR_STATE}] = SystemState::ERROR_STATE;
    
    // From UPDATING
    transitions_[{SystemState::UPDATING, Trigger::OPERATION_COMPLETE, SystemState::IDLE}] = SystemState::IDLE;
    transitions_[{SystemState::UPDATING, Trigger::ERROR_OCCURRED, SystemState::ERROR_STATE}] = SystemState::ERROR_STATE;
    
    // From ERROR_STATE
    transitions_[{SystemState::ERROR_STATE, Trigger::OPERATION_COMPLETE, SystemState::IDLE}] = SystemState::IDLE;
    transitions_[{SystemState::ERROR_STATE, Trigger::SYSTEM_START, SystemState::INITIALIZING}] = SystemState::INITIALIZING;
    
    // From SHUTTING_DOWN (no transitions out, terminal state)
}

void StateMachine::executeStateEntry(SystemState state) {
    auto it = entryCallbacks_.find(state);
    if (it != entryCallbacks_.end() && it->second) {
        it->second(state);
    }
}

void StateMachine::executeStateExit(SystemState state) {
    auto it = exitCallbacks_.find(state);
    if (it != exitCallbacks_.end() && it->second) {
        it->second(state);
    }
}

bool StateMachine::isValidTransition(SystemState fromState, Trigger trigger) const {
    Transition trans{fromState, trigger, SystemState::INITIALIZING};
    return transitions_.find(trans) != transitions_.end();
}

Trigger StateMachine::eventTypeToTrigger(EventType eventType) const {
    switch (eventType) {
        case EventType::SYSTEM_STARTUP: return Trigger::SYSTEM_START;
        case EventType::ANDROID_AUTO_CONNECTED: return Trigger::ANDROID_AUTO_CONNECT;
        case EventType::ANDROID_AUTO_DISCONNECTED: return Trigger::ANDROID_AUTO_DISCONNECT;
        case EventType::CAMERA_SHOW: return Trigger::CAMERA_BUTTON_PRESS;
        case EventType::UI_BUTTON_PRESSED: return Trigger::SETTINGS_BUTTON_PRESS; // Context dependent
        case EventType::BLUETOOTH_PAIRING_REQUEST: return Trigger::BLUETOOTH_PAIR_REQUEST;
        case EventType::WIFI_CONNECTED: return Trigger::WIFI_SETUP_REQUEST;
        case EventType::UPDATE_STARTED: return Trigger::UPDATE_START;
        case EventType::SYSTEM_SHUTDOWN: return Trigger::SHUTDOWN_REQUEST;
        case EventType::SYSTEM_ERROR: return Trigger::ERROR_OCCURRED;
        case EventType::CONFIG_SAVED: return Trigger::OPERATION_COMPLETE;
        case EventType::UPDATE_COMPLETED: return Trigger::OPERATION_COMPLETE;
        case EventType::UPDATE_FAILED: return Trigger::ERROR_OCCURRED;
        default: return Trigger::OPERATION_COMPLETE; // Default trigger
    }
}

} // namespace modern
} // namespace openauto
