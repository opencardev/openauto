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
#include <f1x/openauto/autoapp/Service/PhoneStatus/PhoneStatusService.hpp>
#include <fstream>
#include <QString>

namespace f1x {
  namespace openauto {
    namespace autoapp {
      namespace service {
        namespace phonestatus {

          PhoneStatusService::PhoneStatusService(boost::asio::io_service &ioService,
                                                       aasdk::messenger::IMessenger::Pointer messenger)
              : strand_(ioService),
                timer_(ioService),
                channel_(std::make_shared<aasdk::channel::phonestatus::PhoneStatusService>(strand_, std::move(messenger))) {

          }

          void PhoneStatusService::start() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[PhoneStatusService] start()";
            });
          }

          void PhoneStatusService::stop() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[PhoneStatusService] stop()";
            });
          }

          void PhoneStatusService::pause() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[PhoneStatusService] pause()";
            });
          }

          void PhoneStatusService::resume() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[PhoneStatusService] resume()";
            });
          }

          void PhoneStatusService::fillFeatures(
              aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
            OPENAUTO_LOG(info) << "[PhoneStatusService] fillFeatures()";

            auto *service = response.add_channels();
            service->set_id(static_cast<uint32_t>(channel_->getId()));

            auto *phoneStatus = service->mutable_phone_status_service();
            (void)phoneStatus; // Suppress unused variable warning
          }

          void PhoneStatusService::onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
            OPENAUTO_LOG(info) << "[PhoneStatusService] onChannelOpenRequest()";
            OPENAUTO_LOG(debug) << "[PhoneStatusService] Channel Id: " << request.service_id() << ", Priority: " << request.priority();

            aap_protobuf::service::control::message::ChannelOpenResponse response;
            const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
            response.set_status(status);

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then([]() {}, std::bind(&PhoneStatusService::onChannelError, this->shared_from_this(),
                                             std::placeholders::_1));
            channel_->sendChannelOpenResponse(response, std::move(promise));

            channel_->receive(this->shared_from_this());
          }


          void PhoneStatusService::onChannelError(const aasdk::error::Error &e) {
            OPENAUTO_LOG(error) << "[PhoneStatusService] onChannelError(): " << e.what();
          }
        }
      }
    }
  }
}