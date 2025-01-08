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
#include <f1x/openauto/autoapp/Service/WifiProjection/WifiProjectionService.hpp>
#include <fstream>
#include <QString>
#include <QNetworkInterface>


namespace f1x::openauto::autoapp::service::wifiprojection {
  WifiProjectionService::WifiProjectionService(boost::asio::io_service &ioService,
                                               aasdk::messenger::IMessenger::Pointer messenger,
                                               configuration::IConfiguration::Pointer configuration)
      : configuration_(std::move(configuration)),
        strand_(ioService),
        timer_(ioService),
        channel_(
            std::make_shared<aasdk::channel::wifiprojection::WifiProjectionService>(strand_, std::move(messenger))) {

  }

  void WifiProjectionService::start() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[WifiProjectionService] start()";
    });
  }

  void WifiProjectionService::stop() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[WifiProjectionService] stop()";
    });
  }

  void WifiProjectionService::pause() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[WifiProjectionService] pause()";
    });
  }

  void WifiProjectionService::resume() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[WifiProjectionService] resume()";
    });
  }

  void WifiProjectionService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    OPENAUTO_LOG(info) << "[WifiProjectionService] fillFeatures()";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *wifiChannel = service->mutable_wifi_projection_service();
    wifiChannel->set_car_wifi_bssid(QNetworkInterface::interfaceFromName("wlan0").hardwareAddress().toStdString());
  }

  void WifiProjectionService::onWifiCredentialsRequest(
      const aap_protobuf::service::wifiprojection::message::WifiCredentialsRequest &request) {

    OPENAUTO_LOG(info) << "[WifiProjectionService] onWifiCredentialsRequest()";

    aap_protobuf::service::wifiprojection::message::WifiCredentialsResponse response;

    response.set_access_point_type(aap_protobuf::service::wifiprojection::message::AccessPointType::STATIC);
    response.set_car_wifi_ssid(configuration_->getParamFromFile("/etc/hostapd/hostapd.conf","ssid").toStdString());
    response.set_car_wifi_password(configuration_->getParamFromFile("/etc/hostapd/hostapd.conf","wpa_passphrase").toStdString());

    // Might need to set WPA2_ENTERPRISE
    response.set_car_wifi_security_mode(
        aap_protobuf::service::wifiprojection::message::WifiSecurityMode::WPA2_PERSONAL);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&WifiProjectionService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendWifiCredentialsResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());

  }

  void WifiProjectionService::onChannelOpenRequest(
      const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    OPENAUTO_LOG(info) << "[WifiProjectionService] onChannelOpenRequest()";
    OPENAUTO_LOG(debug) << "[WifiProjectionService] Channel Id: " << request.service_id() << ", Priority: "
                        << request.priority();

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&WifiProjectionService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
  }

  void WifiProjectionService::onChannelError(const aasdk::error::Error &e) {
    OPENAUTO_LOG(error) << "[WifiProjectionService] onChannelError(): " << e.what();
  }


}



