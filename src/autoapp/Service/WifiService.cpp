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

#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Service/WifiService.hpp>
#include <fstream>
#include <QNetworkInterface>
#include <QString>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace service
{

WifiService::WifiService(boost::asio::io_service& ioService, aasdk::messenger::IMessenger::Pointer messenger, configuration::IConfiguration::Pointer configuration)
    : configuration_(std::move(configuration))
    , strand_(ioService)
    , channel_(std::make_shared<aasdk::channel::wifi::WIFIServiceChannel>(strand_, std::move(messenger)))
{

}

void WifiService::start()
{
    strand_.dispatch([this, self = this->shared_from_this()]() {
        OPENAUTO_LOG(info) << "[WifiService] start.";
        channel_->receive(this->shared_from_this());
    });
}

void WifiService::stop()
{
    strand_.dispatch([this, self = this->shared_from_this()]() {
        OPENAUTO_LOG(info) << "[WifiService] stop.";
    });
}

void WifiService::pause()
{
    strand_.dispatch([this, self = this->shared_from_this()]() {
        OPENAUTO_LOG(info) << "[WifiService] pause.";
    });
}

void WifiService::resume()
{
    strand_.dispatch([this, self = this->shared_from_this()]() {
        OPENAUTO_LOG(info) << "[WifiService] resume.";
    });
}

void WifiService::fillFeatures(aasdk::proto::messages::ServiceDiscoveryResponse& response)
{
    OPENAUTO_LOG(info) << "[WifiService] fill features.";

    auto* channelDescriptor = response.add_channels();
    channelDescriptor->set_channel_id(static_cast<uint32_t>(channel_->getId()));

    auto* channel = channelDescriptor->mutable_wifi_channel();
    channel->set_ssid(configuration_->getParamFromFile("/etc/hostapd/hostapd.conf","ssid").toStdString());
}

void WifiService::onChannelOpenRequest(const aasdk::proto::messages::ChannelOpenRequest &request) {
    OPENAUTO_LOG(info) << "[WifiService] open request, priority: " << request.priority();
    const aasdk::proto::enums::Status::Enum status = aasdk::proto::enums::Status::OK;
    OPENAUTO_LOG(info) << "[WifiService] open status: " << status;

    aasdk::proto::messages::ChannelOpenResponse response;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&WifiService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
}

void WifiService::onChannelError(const aasdk::error::Error &e) {
    OPENAUTO_LOG(error) << "[WifiService] channel error: " << e.what();
}

void WifiService::onWifiSecurityRequest() {
    OPENAUTO_LOG(info) << "[WifiService] handle Wifi Security Request ";
    aasdk::proto::messages::WifiSecurityResponse response;

    response.set_access_point_type(aasdk::proto::messages::WifiSecurityResponse_AccessPointType_STATIC);
    response.set_ssid(configuration_->getParamFromFile("/etc/hostapd/hostapd.conf","ssid").toStdString());
    response.set_key(configuration_->getParamFromFile("/etc/hostapd/hostapd.conf","wpa_passphrase").toStdString());
    // response.set_bssid(QNetworkInterface::interfaceFromName("wlan0").hardwareAddress().toStdString());
    response.set_security_mode(aasdk::proto::messages::WifiSecurityResponse_SecurityMode_WPA2_PERSONAL);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&WifiService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendWifiSecurityResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
}
}
}
}
}
