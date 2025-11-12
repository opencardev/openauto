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

#include <atomic>
#include <boost/asio.hpp>
#include <aasdk/Transport/ITransport.hpp>
#include <aasdk/Channel/Control/IControlServiceChannel.hpp>
#include <aasdk/Channel/Control/IControlServiceChannelEventHandler.hpp>
#include <aasdk/Channel/MediaSink/Video/Channel/VideoChannel.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <f1x/openauto/autoapp/Service/IAndroidAutoEntity.hpp>
#include <f1x/openauto/autoapp/Service/IService.hpp>
#include <f1x/openauto/autoapp/Service/IPinger.hpp>
#include <Transport/ITransport.hpp>
#include <aap_protobuf/service/control/message/AudioFocusRequestType.pb.h>
#include <aap_protobuf/service/control/message/AudioFocusStateType.pb.h>
#include <aap_protobuf/service/control/message/NavFocusType.pb.h>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace service
{

class AndroidAutoEntity: public IAndroidAutoEntity, public aasdk::channel::control::IControlServiceChannelEventHandler, public std::enable_shared_from_this<AndroidAutoEntity>
{
public:
    AndroidAutoEntity(boost::asio::io_service& ioService,
                      aasdk::messenger::ICryptor::Pointer cryptor,
                      aasdk::transport::ITransport::Pointer transport,
                      aasdk::messenger::IMessenger::Pointer messenger,
                      configuration::IConfiguration::Pointer configuration,
                      ServiceList serviceList,
                      IPinger::Pointer pinger);
    ~AndroidAutoEntity() override;

    void start(IAndroidAutoEntityEventHandler& eventHandler) override;
    void stop() override;
    void pause() override;
    void resume() override;
    // TODO: on channel open request... on channel close...  on navigation focus, on voice session notification, on user switch, on call availability, on service disc update, on battery status, on car connected devices
    void onVersionResponse(uint16_t majorCode, uint16_t minorCode, aap_protobuf::shared::MessageStatus status) override;
    void onHandshake(const aasdk::common::DataConstBuffer& payload) override;
    void onServiceDiscoveryRequest(const aap_protobuf::service::control::message::ServiceDiscoveryRequest& request) override;
    void onAudioFocusRequest(const aap_protobuf::service::control::message::AudioFocusRequest& request) override;
    void onByeByeRequest(const aap_protobuf::service::control::message::ByeByeRequest& request) override;
    void onByeByeResponse(const aap_protobuf::service::control::message::ByeByeResponse& response) override;
    void onNavigationFocusRequest(const aap_protobuf::service::control::message::NavFocusRequestNotification& request) override;
    void onVoiceSessionRequest(const aap_protobuf::service::control::message::VoiceSessionNotification &request) override;
    void onBatteryStatusNotification(const aap_protobuf::service::control::message::BatteryStatusNotification &notification) override;
    void onPingResponse(const aap_protobuf::service::control::message::PingResponse& response) override;
    void onPingRequest(const aap_protobuf::service::control::message::PingRequest& request) override;
    void onChannelError(const aasdk::error::Error& e) override;

private:
    using std::enable_shared_from_this<AndroidAutoEntity>::shared_from_this;
    void triggerQuit();
    void schedulePing();
    void sendPing();

    boost::asio::io_service::strand strand_;
    aasdk::messenger::ICryptor::Pointer cryptor_;
    aasdk::transport::ITransport::Pointer transport_;
    aasdk::messenger::IMessenger::Pointer messenger_;
    aasdk::channel::control::IControlServiceChannel::Pointer controlServiceChannel_;
    configuration::IConfiguration::Pointer configuration_;
    ServiceList serviceList_;
    IPinger::Pointer pinger_;
    IAndroidAutoEntityEventHandler* eventHandler_;
    // Guard to avoid re-entrant quit handling and spurious error-triggered quits during shutdown
    std::atomic<bool> stopping_{false};
};

}
}
}
}
