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

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include "Event.hpp"

namespace openauto {
namespace modern {

class EventSubscriber {
  public:
    virtual ~EventSubscriber() = default;
    virtual void onEvent(const Event::Pointer& event) = 0;
    virtual std::string getSubscriberId() const = 0;
};

using EventHandler = std::function<void(const Event::Pointer&)>;

class EventBus {
  public:
    static EventBus& getInstance();

    // Subscription management
    void subscribe(EventType type, std::shared_ptr<EventSubscriber> subscriber);
    void subscribe(EventType type, const std::string& subscriberId, EventHandler handler);
    void unsubscribe(EventType type, const std::string& subscriberId);
    void unsubscribeAll(const std::string& subscriberId);

    // Event publishing
    void publish(const Event::Pointer& event);
    void publish(EventType type, const std::string& source = "system");
    void publish(EventType type, const EventData& data, const std::string& source = "system");

    // Async event processing
    void start();
    void stop();
    void startEventProcessing();
    void stopEventProcessing();

    // Event history and filtering
    std::vector<Event::Pointer> getEventHistory(size_t maxEvents = 100) const;
    std::vector<Event::Pointer> getEventsOfType(EventType type, size_t maxEvents = 50) const;
    std::vector<Event::Pointer> getEventsFromSource(const std::string& source,
                                                    size_t maxEvents = 50) const;

    // External interface for REST API
    nlohmann::json getSubscribersInfo() const;
    nlohmann::json getEventQueueStatus() const;

  private:
    EventBus();
    ~EventBus();

    void processEvents();
    void deliverEvent(const Event::Pointer& event);

    struct SubscriberInfo {
        std::string id;
        std::weak_ptr<EventSubscriber> subscriber;
        EventHandler handler;
        bool isHandler = false;
    };

    mutable std::mutex subscribersMutex_;
    std::unordered_map<EventType, std::vector<SubscriberInfo>> subscribers_;

    mutable std::mutex eventQueueMutex_;
    std::queue<Event::Pointer> eventQueue_;
    std::condition_variable eventCondition_;

    mutable std::mutex historyMutex_;
    std::vector<Event::Pointer> eventHistory_;
    static constexpr size_t MAX_HISTORY_SIZE = 1000;

    std::atomic<bool> processing_{false};
    std::thread processingThread_;
};

// Convenience macros for event publishing
#define PUBLISH_EVENT(type) openauto::modern::EventBus::getInstance().publish(type, __FUNCTION__)
#define PUBLISH_EVENT_WITH_DATA(type, data) \
    openauto::modern::EventBus::getInstance().publish(type, data, __FUNCTION__)

}  // namespace modern
}  // namespace openauto
