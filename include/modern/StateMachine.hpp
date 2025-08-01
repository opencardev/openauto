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

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "Event.hpp"

namespace openauto {
namespace modern {

enum class SystemState {
    INITIALIZING,
    IDLE,
    ANDROID_AUTO_ACTIVE,
    CAMERA_VIEW,
    SETTINGS,
    BLUETOOTH_PAIRING,
    WIFI_SETUP,
    UPDATING,
    SHUTTING_DOWN,
    ERROR_STATE
};

enum class Trigger {
    SYSTEM_START,
    ANDROID_AUTO_CONNECT,
    ANDROID_AUTO_DISCONNECT,
    CAMERA_BUTTON_PRESS,
    SETTINGS_BUTTON_PRESS,
    BACK_BUTTON_PRESS,
    BLUETOOTH_PAIR_REQUEST,
    WIFI_SETUP_REQUEST,
    UPDATE_START,
    SHUTDOWN_REQUEST,
    ERROR_OCCURRED,
    OPERATION_COMPLETE
};

class StateMachine {
  public:
    using StateChangeCallback =
        std::function<void(SystemState oldState, SystemState newState, Trigger trigger)>;
    using StateEntryCallback = std::function<void(SystemState state)>;
    using StateExitCallback = std::function<void(SystemState state)>;

    StateMachine();
    ~StateMachine();

    // State management
    SystemState getCurrentState() const;
    bool transition(Trigger trigger);
    bool canTransition(Trigger trigger) const;

    // State callbacks
    void setStateChangeCallback(StateChangeCallback callback);
    void setStateEntryCallback(SystemState state, StateEntryCallback callback);
    void setStateExitCallback(SystemState state, StateExitCallback callback);

    // Event handling
    void handleEvent(const Event::Pointer& event);

    // Utility
    std::string stateToString(SystemState state) const;
    std::string triggerToString(Trigger trigger) const;
    std::vector<Trigger> getValidTransitions() const;

    // Reset to initial state
    void reset();

  private:
    struct Transition {
        SystemState fromState;
        Trigger trigger;
        SystemState toState;

        bool operator==(const Transition& other) const {
            return fromState == other.fromState && trigger == other.trigger;
        }
    };

    struct TransitionHash {
        std::size_t operator()(const Transition& t) const {
            return std::hash<int>()(static_cast<int>(t.fromState)) ^
                   (std::hash<int>()(static_cast<int>(t.trigger)) << 1);
        }
    };

    mutable std::mutex mutex_;
    SystemState currentState_;
    std::unordered_map<Transition, SystemState, TransitionHash> transitions_;

    StateChangeCallback stateChangeCallback_;
    std::unordered_map<SystemState, StateEntryCallback> entryCallbacks_;
    std::unordered_map<SystemState, StateExitCallback> exitCallbacks_;

    void initializeTransitions();
    void executeStateEntry(SystemState state);
    void executeStateExit(SystemState state);
    bool isValidTransition(SystemState fromState, Trigger trigger) const;
    Trigger eventTypeToTrigger(EventType eventType) const;
};

}  // namespace modern
}  // namespace openauto
