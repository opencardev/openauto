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
#include <f1x/openauto/autoapp/Service/NavigationStatus/NavigationStatusService.hpp>
#include <fstream>
#include <QString>

namespace f1x::openauto::autoapp::service::navigationstatus {

  NavigationStatusService::NavigationStatusService(boost::asio::io_service &ioService,
                                                   aasdk::messenger::IMessenger::Pointer messenger)
      : strand_(ioService),
        timer_(ioService),
        channel_(std::make_shared<aasdk::channel::navigationstatus::NavigationStatusService>(strand_,
                                                                                             std::move(messenger))) {

  }

  void NavigationStatusService::start() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[NavigationStatusService] start()";
    });
  }

  void NavigationStatusService::stop() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[NavigationStatusService] stop()";
    });
  }

  void NavigationStatusService::pause() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[NavigationStatusService] pause()";
    });
  }

  void NavigationStatusService::resume() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[NavigationStatusService] resume()";
    });
  }

  void NavigationStatusService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    OPENAUTO_LOG(info) << "[NavigationStatusService] fillFeatures()";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *navigationStatus = service->mutable_navigation_status_service();
    (void)navigationStatus; // Suppress unused variable warning
  }

  void NavigationStatusService::onChannelOpenRequest(
      const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    OPENAUTO_LOG(info) << "[NavigationStatusService] onChannelOpenRequest()";
    OPENAUTO_LOG(info) << "[NavigationStatusService] Channel Id: " << request.service_id() << ", Priority: "
                       << request.priority();

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&NavigationStatusService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
  }

  void NavigationStatusService::onStatusUpdate(
      const aap_protobuf::service::navigationstatus::message::NavigationStatus &navStatus) {
    channel_->receive(this->shared_from_this());
  }


  void NavigationStatusService::onTurnEvent(
      const aap_protobuf::service::navigationstatus::message::NavigationNextTurnEvent &turnEvent) {
    channel_->receive(this->shared_from_this());
  }

  void NavigationStatusService::onDistanceEvent(
      const aap_protobuf::service::navigationstatus::message::NavigationNextTurnDistanceEvent &distanceEvent) {
    channel_->receive(this->shared_from_this());
  }


  void NavigationStatusService::onChannelError(const aasdk::error::Error &e) {
    OPENAUTO_LOG(error) << "[NavigationStatusService] onChannelError(): " << e.what();
  }
}



