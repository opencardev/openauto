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
#include <f1x/openauto/autoapp/Service/MediaPlaybackStatus/MediaPlaybackStatusService.hpp>
#include <fstream>
#include <QString>

namespace f1x {
  namespace openauto {
    namespace autoapp {
      namespace service {
        namespace mediaplaybackstatus {

          MediaPlaybackStatusService::MediaPlaybackStatusService(boost::asio::io_service &ioService,
                                                       aasdk::messenger::IMessenger::Pointer messenger)
              : strand_(ioService),
                timer_(ioService),
                channel_(std::make_shared<aasdk::channel::mediaplaybackstatus::MediaPlaybackStatusService>(strand_, std::move(messenger))) {

          }

          void MediaPlaybackStatusService::start() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[MediaPlaybackStatusService] start.";
            });
          }

          void MediaPlaybackStatusService::stop() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[MediaPlaybackStatusService] stop.";
            });
          }

          void MediaPlaybackStatusService::pause() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[MediaPlaybackStatusService] pause.";
            });
          }

          void MediaPlaybackStatusService::resume() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[MediaPlaybackStatusService] resume.";
            });
          }

          void MediaPlaybackStatusService::fillFeatures(
              aap_protobuf::channel::control::servicediscovery::notification::ServiceDiscoveryResponse &response) {
            OPENAUTO_LOG(info) << "[MediaPlaybackStatusService] fill features.";

            auto *channelDescriptor = response.add_channels();
            channelDescriptor->set_channel_id(static_cast<uint32_t>(channel_->getId()));

            auto *vendorExtension = channelDescriptor->mutable_wifi_projection_service();
          }

          void MediaPlaybackStatusService::onChannelError(const aasdk::error::Error &e) {
            OPENAUTO_LOG(error) << "[MediaPlaybackStatusService] channel error: " << e.what();
          }
        }
      }
    }
  }
}