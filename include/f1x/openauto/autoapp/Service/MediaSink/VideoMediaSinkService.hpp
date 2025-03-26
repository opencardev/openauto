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

#include <aasdk/Messenger/IMessenger.hpp>
#include <aasdk/Channel/MediaSink/Video/IVideoMediaSinkService.hpp>
#include <aasdk/Channel/MediaSink/Video/IVideoMediaSinkServiceEventHandler.hpp>
#include <f1x/openauto/autoapp/Projection/IVideoOutput.hpp>
#include <f1x/openauto/autoapp/Service/IService.hpp>

namespace f1x {
  namespace openauto {
    namespace autoapp {
      namespace service {
        namespace mediasink {

          class VideoMediaSinkService :
              public aasdk::channel::mediasink::video::IVideoMediaSinkServiceEventHandler,
              public IService,
              public std::enable_shared_from_this<VideoMediaSinkService> {
          public:
              typedef std::shared_ptr<VideoMediaSinkService> Pointer;

            // General Constructor
            VideoMediaSinkService(boost::asio::io_service& ioService,
                                  aasdk::channel::mediasink::video::IVideoMediaSinkService::Pointer channel,
                                  projection::IVideoOutput::Pointer videoOutput);

            void start() override;
            void stop() override;
            void pause() override;
            void resume() override;
            void fillFeatures(aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) override;

            void onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) override;

            void onMediaChannelSetupRequest(
                const aap_protobuf::service::media::shared::message::Setup &request) override;

            void onMediaChannelStartIndication(
                const aap_protobuf::service::media::shared::message::Start &indication) override;

            void onMediaChannelStopIndication(
                const aap_protobuf::service::media::shared::message::Stop &indication) override;

            void onMediaWithTimestampIndication(aasdk::messenger::Timestamp::ValueType timestamp,
                                                const aasdk::common::DataConstBuffer &buffer) override;

            void onMediaIndication(const aasdk::common::DataConstBuffer &buffer) override;

            void onChannelError(const aasdk::error::Error &e) override;

            void onVideoFocusRequest(const aap_protobuf::service::media::video::message::VideoFocusRequestNotification &request) override;
            void sendVideoFocusIndication();
          protected:
            using std::enable_shared_from_this<VideoMediaSinkService>::shared_from_this;
            boost::asio::io_service::strand strand_;
            aasdk::channel::mediasink::video::IVideoMediaSinkService::Pointer channel_;
            projection::IVideoOutput::Pointer videoOutput_;
            int32_t session_;
          };
        }
      }
    }
  }
}