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

#include "modern/EventBus.hpp"
#include <algorithm>
#include <chrono>

namespace openauto {
namespace modern {

EventBus::EventBus() : running_(false) {
}

EventBus::~EventBus() {
    stop();
}

void EventBus::start() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!running_) {
        running_ = true;
        processingThread_ = std::thread(&EventBus::processEvents, this);
    }
}

void EventBus::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
    condition_.notify_all();
    
    if (processingThread_.joinable()) {
        processingThread_.join();
    }
}

void EventBus::publish(const Event::Pointer& event) {
    if (!event) return;
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        eventQueue_.push(event);
        
        // Store event in history
        eventHistory_.push_back(event);
        if (eventHistory_.size() > maxHistorySize_) {
            eventHistory_.pop_front();
        }
    }
    condition_.notify_one();
}

void EventBus::subscribe(EventType eventType, EventHandler handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto id = nextSubscriberId_++;
    subscribers_[eventType][id] = handler;
    subscriberIds_[id] = eventType;
}

void EventBus::subscribe(EventType eventType, EventHandler handler, SubscriptionId& subscriptionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    subscriptionId = nextSubscriberId_++;
    subscribers_[eventType][subscriptionId] = handler;
    subscriberIds_[subscriptionId] = eventType;
}

void EventBus::unsubscribe(SubscriptionId subscriptionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subscriberIds_.find(subscriptionId);
    if (it != subscriberIds_.end()) {
        EventType eventType = it->second;
        subscribers_[eventType].erase(subscriptionId);
        subscriberIds_.erase(it);
    }
}

void EventBus::unsubscribeAll(EventType eventType) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subscribers_.find(eventType);
    if (it != subscribers_.end()) {
        for (const auto& [id, handler] : it->second) {
            subscriberIds_.erase(id);
        }
        it->second.clear();
    }
}

std::vector<Event::Pointer> EventBus::getEventHistory(size_t maxEvents) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Event::Pointer> result;
    
    size_t startIndex = 0;
    if (eventHistory_.size() > maxEvents) {
        startIndex = eventHistory_.size() - maxEvents;
    }
    
    for (size_t i = startIndex; i < eventHistory_.size(); ++i) {
        result.push_back(eventHistory_[i]);
    }
    
    return result;
}

std::vector<Event::Pointer> EventBus::getEventHistory(
    EventType eventType, 
    const std::chrono::system_clock::time_point& since) const {
    
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Event::Pointer> result;
    
    for (const auto& event : eventHistory_) {
        if (event->getType() == eventType && event->getTimestamp() >= since) {
            result.push_back(event);
        }
    }
    
    return result;
}

void EventBus::clearHistory() {
    std::lock_guard<std::mutex> lock(mutex_);
    eventHistory_.clear();
}

size_t EventBus::getSubscriberCount(EventType eventType) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = subscribers_.find(eventType);
    return (it != subscribers_.end()) ? it->second.size() : 0;
}

bool EventBus::hasSubscribers(EventType eventType) const {
    return getSubscriberCount(eventType) > 0;
}

void EventBus::setMaxHistorySize(size_t maxSize) {
    std::lock_guard<std::mutex> lock(mutex_);
    maxHistorySize_ = maxSize;
    
    // Trim history if needed
    while (eventHistory_.size() > maxHistorySize_) {
        eventHistory_.pop_front();
    }
}

EventBus& EventBus::getInstance() {
    static EventBus instance;
    return instance;
}

void EventBus::processEvents() {
    while (true) {
        Event::Pointer event;
        
        {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this] { return !running_ || !eventQueue_.empty(); });
            
            if (!running_) {
                // Process remaining events before stopping
                while (!eventQueue_.empty()) {
                    event = eventQueue_.front();
                    eventQueue_.pop();
                    lock.unlock();
                    
                    notifySubscribers(event);
                    
                    lock.lock();
                }
                break;
            }
            
            if (!eventQueue_.empty()) {
                event = eventQueue_.front();
                eventQueue_.pop();
            }
        }
        
        if (event) {
            notifySubscribers(event);
        }
    }
}

void EventBus::notifySubscribers(const Event::Pointer& event) {
    std::vector<EventHandler> handlers;
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subscribers_.find(event->getType());
        if (it != subscribers_.end()) {
            for (const auto& [id, handler] : it->second) {
                handlers.push_back(handler);
            }
        }
    }
    
    // Call handlers outside of lock to prevent deadlocks
    for (const auto& handler : handlers) {
        try {
            handler(event);
        } catch (const std::exception& e) {
            // Log error but continue processing other handlers
            // In a real implementation, you might want to use a proper logging system
        }
    }
}

} // namespace modern
} // namespace openauto
