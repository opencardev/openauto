/*
*  This file is part of openauto project.
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
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
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <boost/asio.hpp>
#include <f1x/openauto/autoapp/EventBus/Event.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace eventbus
{

using EventHandler = std::function<void(Event::Pointer)>;

class IEventBus
{
public:
    virtual ~IEventBus() = default;
    
    virtual void publish(Event::Pointer event) = 0;
    virtual void publishSync(Event::Pointer event) = 0;
    virtual void subscribe(EventType eventType, EventHandler handler) = 0;
    virtual void unsubscribe(EventType eventType, EventHandler handler) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;
    virtual size_t getQueueSize() const = 0;
    virtual void clearQueue() = 0;
};

class EventBus : public IEventBus
{
public:
    using Pointer = std::shared_ptr<EventBus>;
    
    EventBus(boost::asio::io_service& ioService);
    virtual ~EventBus();
    
    void publish(Event::Pointer event) override;
    void publishSync(Event::Pointer event) override;
    void subscribe(EventType eventType, EventHandler handler) override;
    void unsubscribe(EventType eventType, EventHandler handler) override;
    void start() override;
    void stop() override;
    bool isRunning() const override;
    size_t getQueueSize() const override;
    void clearQueue() override;
    
    // External process communication
    void enableExternalCommunication(int port = 5001);
    void disableExternalCommunication();
    
private:
    void processEvents();
    void handleIncomingConnection(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void handleClientMessage(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    
    boost::asio::io_service& ioService_;
    boost::asio::io_service::strand strand_;
    std::unordered_map<EventType, std::vector<EventHandler>> handlers_;
    std::queue<Event::Pointer> eventQueue_;
    mutable std::mutex queueMutex_;
    mutable std::mutex handlersMutex_;
    std::condition_variable queueCondition_;
    std::atomic<bool> running_;
    std::thread processingThread_;
    
    // External communication
    std::unique_ptr<boost::asio::ip::tcp::acceptor> externalAcceptor_;
    std::atomic<bool> externalCommEnabled_;
    std::vector<std::shared_ptr<boost::asio::ip::tcp::socket>> connectedClients_;
    mutable std::mutex clientsMutex_;
};

}
}
}
}
