#include <modern/EventBus.hpp>
#include <modern/Logger.hpp>
#include <algorithm>
#include <chrono>

namespace openauto {
namespace modern {

EventBus::EventBus() {
}

EventBus::~EventBus() {
    stop();
}

EventBus& EventBus::getInstance() {
    static EventBus instance;
    return instance;
}

void EventBus::start() {
    std::lock_guard<std::mutex> lock(eventQueueMutex_);
    if (!processing_.load()) {
        processing_.store(true);
        processingThread_ = std::thread(&EventBus::processEvents, this);
    }
}

void EventBus::stop() {
    {
        std::lock_guard<std::mutex> lock(eventQueueMutex_);
        processing_.store(false);
    }
    eventCondition_.notify_all();
    
    if (processingThread_.joinable()) {
        processingThread_.join();
    }
}

void EventBus::startEventProcessing() {
    start();
}

void EventBus::stopEventProcessing() {
    stop();
}

void EventBus::publish(const Event::Pointer& event) {
    if (!event) return;
    
    {
        std::lock_guard<std::mutex> lock(eventQueueMutex_);
        eventQueue_.push(event);
    }
    
    {
        std::lock_guard<std::mutex> lock(historyMutex_);
        eventHistory_.push_back(event);
        if (eventHistory_.size() > MAX_HISTORY_SIZE) {
            eventHistory_.erase(eventHistory_.begin());
        }
    }
    
    eventCondition_.notify_one();
}

void EventBus::publish(EventType type, const std::string& source) {
    auto event = std::make_shared<Event>(type, source);
    publish(event);
}

void EventBus::publish(EventType type, const EventData& data, const std::string& source) {
    auto event = std::make_shared<Event>(type, source);
    for (const auto& [key, value] : data) {
        event->setData(key, value);
    }
    publish(event);
}

void EventBus::subscribe(EventType type, std::shared_ptr<EventSubscriber> subscriber) {
    if (!subscriber) return;
    
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    SubscriberInfo info;
    info.id = subscriber->getSubscriberId();
    info.subscriber = subscriber;
    info.isHandler = false;
    
    subscribers_[type].push_back(info);
}

void EventBus::subscribe(EventType type, const std::string& subscriberId, EventHandler handler) {
    if (!handler) return;
    
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    SubscriberInfo info;
    info.id = subscriberId;
    info.handler = handler;
    info.isHandler = true;
    
    subscribers_[type].push_back(info);
}

void EventBus::unsubscribe(EventType type, const std::string& subscriberId) {
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    auto& typeSubscribers = subscribers_[type];
    typeSubscribers.erase(
        std::remove_if(typeSubscribers.begin(), typeSubscribers.end(),
                       [&subscriberId](const SubscriberInfo& info) {
                           return info.id == subscriberId;
                       }),
        typeSubscribers.end());
}

void EventBus::unsubscribeAll(const std::string& subscriberId) {
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    for (auto& [type, typeSubscribers] : subscribers_) {
        typeSubscribers.erase(
            std::remove_if(typeSubscribers.begin(), typeSubscribers.end(),
                           [&subscriberId](const SubscriberInfo& info) {
                               return info.id == subscriberId;
                           }),
            typeSubscribers.end());
    }
}

std::vector<Event::Pointer> EventBus::getEventHistory(size_t maxEvents) const {
    std::lock_guard<std::mutex> lock(historyMutex_);
    
    if (maxEvents >= eventHistory_.size()) {
        return eventHistory_;
    }
    
    auto start = eventHistory_.end() - maxEvents;
    return std::vector<Event::Pointer>(start, eventHistory_.end());
}

std::vector<Event::Pointer> EventBus::getEventsOfType(EventType type, size_t maxEvents) const {
    std::lock_guard<std::mutex> lock(historyMutex_);
    
    std::vector<Event::Pointer> result;
    for (auto it = eventHistory_.rbegin(); it != eventHistory_.rend() && result.size() < maxEvents; ++it) {
        if ((*it)->getType() == type) {
            result.push_back(*it);
        }
    }
    
    std::reverse(result.begin(), result.end());
    return result;
}

std::vector<Event::Pointer> EventBus::getEventsFromSource(const std::string& source, size_t maxEvents) const {
    std::lock_guard<std::mutex> lock(historyMutex_);
    
    std::vector<Event::Pointer> result;
    for (auto it = eventHistory_.rbegin(); it != eventHistory_.rend() && result.size() < maxEvents; ++it) {
        if ((*it)->getSource() == source) {
            result.push_back(*it);
        }
    }
    
    std::reverse(result.begin(), result.end());
    return result;
}

nlohmann::json EventBus::getSubscribersInfo() const {
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    
    nlohmann::json result;
    for (const auto& [type, typeSubscribers] : subscribers_) {
        nlohmann::json typeInfo;
        typeInfo["type"] = static_cast<int>(type);
        typeInfo["count"] = typeSubscribers.size();
        
        nlohmann::json subscribersList = nlohmann::json::array();
        for (const auto& info : typeSubscribers) {
            nlohmann::json subInfo;
            subInfo["id"] = info.id;
            subInfo["type"] = info.isHandler ? "handler" : "subscriber";
            subscribersList.push_back(subInfo);
        }
        typeInfo["subscribers"] = subscribersList;
        
        result[std::to_string(static_cast<int>(type))] = typeInfo;
    }
    
    return result;
}

nlohmann::json EventBus::getEventQueueStatus() const {
    std::lock_guard<std::mutex> lock(eventQueueMutex_);
    
    nlohmann::json result;
    result["queue_size"] = eventQueue_.size();
    result["processing"] = processing_.load();
    
    {
        std::lock_guard<std::mutex> historyLock(historyMutex_);
        result["history_size"] = eventHistory_.size();
        result["max_history_size"] = MAX_HISTORY_SIZE;
    }
    
    return result;
}

void EventBus::processEvents() {
    while (processing_.load()) {
        std::unique_lock<std::mutex> lock(eventQueueMutex_);
        eventCondition_.wait(lock, [this] { 
            return !processing_.load() || !eventQueue_.empty(); 
        });
        
        if (!processing_.load()) {
            break;
        }
        
        if (!eventQueue_.empty()) {
            auto event = eventQueue_.front();
            eventQueue_.pop();
            lock.unlock();
            
            deliverEvent(event);
        }
    }
}

void EventBus::deliverEvent(const Event::Pointer& event) {
    if (!event) return;
    
    std::lock_guard<std::mutex> lock(subscribersMutex_);
    auto it = subscribers_.find(event->getType());
    if (it == subscribers_.end()) {
        return;
    }
    
    for (const auto& info : it->second) {
        try {
            if (info.isHandler && info.handler) {
                info.handler(event);
            } else if (!info.isHandler) {
                auto subscriber = info.subscriber.lock();
                if (subscriber) {
                    subscriber->onEvent(event);
                }
            }
        } catch (const std::exception& e) {
            SLOG_ERROR(SYSTEM, "event_delivery", "Error delivering event to subscriber " + info.id + ": " + e.what());
        }
    }
}

} // namespace modern
} // namespace openauto
