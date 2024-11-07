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

#include <fstream>
#include <f1x/openauto/autoapp/Service/MediaSink/VideoMediaSinkService.hpp>

namespace f1x {
  namespace openauto {
    namespace autoapp {
      namespace service {
        namespace mediasink {
          VideoMediaSinkService::VideoMediaSinkService(boost::asio::io_service &ioService,
                                                       aasdk::channel::mediasink::video::IVideoMediaSinkService::Pointer channel,
                                                       projection::IVideoOutput::Pointer videoOutput)
              : strand_(ioService), channel_(std::move(channel)), videoOutput_(std::move(videoOutput)), session_(-1) {

          }

          void VideoMediaSinkService::start() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[VideoMediaSinkService] start()";
              OPENAUTO_LOG(info) << "[VideoMediaSinkService] Channel "
                                 << aasdk::messenger::channelIdToString(channel_->getId());
              channel_->receive(this->shared_from_this());
            });
          }

          void VideoMediaSinkService::stop() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[VideoMediaSinkService] stop()";
              OPENAUTO_LOG(info) << "[VideoMediaSinkService] Channel "
                                 << aasdk::messenger::channelIdToString(channel_->getId());
            });
          }

          void VideoMediaSinkService::pause() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[VideoMediaSinkService] pause()";
              OPENAUTO_LOG(info) << "[VideoMediaSinkService] Channel "
                                 << aasdk::messenger::channelIdToString(channel_->getId());
            });
          }

          void VideoMediaSinkService::resume() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[VideoMediaSinkService] resume()";
              OPENAUTO_LOG(info) << "[VideoMediaSinkService] Channel "
                                 << aasdk::messenger::channelIdToString(channel_->getId());

            });
          }

          void VideoMediaSinkService::fillFeatures(
              aap_protobuf::channel::control::servicediscovery::notification::ServiceDiscoveryResponse &response) {
            OPENAUTO_LOG(info) << "[VideoMediaSinkService] fillFeatures()";
            OPENAUTO_LOG(info) << "[VideoMediaSinkService] Channel "
                               << aasdk::messenger::channelIdToString(channel_->getId());

            auto *service = response.add_channels();
            service->set_id(static_cast<uint32_t>(channel_->getId()));

            auto *videoChannel = service->mutable_media_sink_service();
            videoChannel->set_stream_type(
                aap_protobuf::service::media::shared::message::MediaCodecType::MEDIA_CODEC_VIDEO_H264_BP);
            videoChannel->set_available_while_in_call(true);

            auto *videoConfig1 = videoChannel->add_video_configs();
            videoConfig1->set_codec_resolution(videoOutput_->getVideoResolution());
            videoConfig1->set_frame_rate(videoOutput_->getVideoFPS());

            const auto &videoMargins = videoOutput_->getVideoMargins();
            videoConfig1->set_height_margin(videoMargins.height());
            videoConfig1->set_width_margin(videoMargins.width());
            videoConfig1->set_density(videoOutput_->getScreenDPI());

          }

          void
          VideoMediaSinkService::onMediaChannelSetupRequest(const aap_protobuf::channel::media::event::Setup &request) {
            OPENAUTO_LOG(info) << "[VideoMediaSinkService] onMediaChannelSetupRequest()";
            OPENAUTO_LOG(info) << "[VideoMediaSinkService] Channel Id: "
                               << aasdk::messenger::channelIdToString(channel_->getId()) << ", Codec: "
                               << MediaCodecType_Name(request.type());


            auto status = videoOutput_->init()
                          ? aap_protobuf::service::media::sink::MediaSinkChannelSetupStatus::STATUS_READY
                          : aap_protobuf::service::media::sink::MediaSinkChannelSetupStatus::STATUS_WAIT;

            OPENAUTO_LOG(info) << "[VideoMediaSinkService] setup status: " << status;

            aap_protobuf::service::media::sink::message::MediaSinkChannelSetupResponse response;
            response.set_media_status(status);
            response.set_max_unacked(1);
            response.add_configuration_indices(0);

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then(std::bind(&VideoMediaSinkService::sendVideoFocusIndication, this->shared_from_this()),
                          std::bind(&VideoMediaSinkService::onChannelError, this->shared_from_this(),
                                    std::placeholders::_1));

            channel_->sendChannelSetupResponse(response, std::move(promise));
            channel_->receive(this->shared_from_this());
          }

          void VideoMediaSinkService::onChannelOpenRequest(const aap_protobuf::channel::ChannelOpenRequest &request) {
            OPENAUTO_LOG(info) << "[VideoMediaSinkService] onChannelOpenRequest()";
            OPENAUTO_LOG(info) << "[VideoMediaSinkService] Channel Id: " << request.service_id() << ", Priority: "
                               << request.priority();

            const aap_protobuf::shared::MessageStatus status = videoOutput_->open()
                                                               ? aap_protobuf::shared::MessageStatus::STATUS_SUCCESS
                                                               : aap_protobuf::shared::MessageStatus::STATUS_INTERNAL_ERROR;

            OPENAUTO_LOG(info) << "[VideoMediaSinkService] Status determined: "
                               << aap_protobuf::shared::MessageStatus_Name(status);

            aap_protobuf::channel::ChannelOpenResponse response;
            response.set_status(status);

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then([]() {}, std::bind(&VideoMediaSinkService::onChannelError, this->shared_from_this(),
                                             std::placeholders::_1));
            channel_->sendChannelOpenResponse(response, std::move(promise));
            channel_->receive(this->shared_from_this());
          }

          void VideoMediaSinkService::onMediaChannelStartIndication(
              const aap_protobuf::channel::media::event::Start &indication) {
            OPENAUTO_LOG(info) << "[VideoMediaSinkService] onMediaChannelStartIndication()";
            OPENAUTO_LOG(info) << "[VideoMediaSinkService] Channel Id: "
                               << aasdk::messenger::channelIdToString(channel_->getId()) << ", session: "
                               << indication.session_id();

            session_ = indication.session_id();
            channel_->receive(this->shared_from_this());
          }

          void VideoMediaSinkService::onMediaChannelStopIndication(
              const aap_protobuf::channel::media::event::Stop &indication) {
            OPENAUTO_LOG(info) << "[onMediaChannelStopIndication] onMediaChannelStopIndication()";
            OPENAUTO_LOG(info) << "[onMediaChannelStopIndication] Channel Id: "
                               << aasdk::messenger::channelIdToString(channel_->getId()) << ", session: " << session_;

            channel_->receive(this->shared_from_this());
          }

          void VideoMediaSinkService::onMediaWithTimestampIndication(aasdk::messenger::Timestamp::ValueType timestamp,
                                                                     const aasdk::common::DataConstBuffer &buffer) {
            OPENAUTO_LOG(info) << "[VideoMediaSinkService] onMediaWithTimestampIndication()";
            OPENAUTO_LOG(info) << "[VideoMediaSinkService] Channel Id: "
                               << aasdk::messenger::channelIdToString(channel_->getId()) << ", session: " << session_;

            videoOutput_->write(timestamp, buffer);

            aap_protobuf::service::media::source::message::MediaSourceMediaAckIndication indication;
            indication.set_session_id(session_);
            indication.set_ack(1);

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then([]() {}, std::bind(&VideoMediaSinkService::onChannelError, this->shared_from_this(),
                                             std::placeholders::_1));
            channel_->sendMediaAckIndication(indication, std::move(promise));
            channel_->receive(this->shared_from_this());
          }

          void VideoMediaSinkService::onMediaIndication(const aasdk::common::DataConstBuffer &buffer) {
            OPENAUTO_LOG(info) << "[VideoMediaSinkService] onMediaIndication()";
            this->onMediaWithTimestampIndication(0, buffer);
          }

          void VideoMediaSinkService::onChannelError(const aasdk::error::Error &e) {
            OPENAUTO_LOG(error) << "[VideoMediaSinkService] onChannelError(): " << e.what()
                                << ", channel: " << aasdk::messenger::channelIdToString(channel_->getId());
          }

          void VideoMediaSinkService::onVideoFocusRequest(
              const aap_protobuf::channel::control::focus::video::event::VideoFocusRequestNotification &request) {
            OPENAUTO_LOG(info) << "[VideoMediaSinkService] onMediaIndication()";
            OPENAUTO_LOG(info) << "[VideoMediaSinkService] Display index: " << request.disp_channel_id() << ", focus mode: " << VideoFocusMode_Name(request.mode()) << ", focus reason: " << VideoFocusReason_Name(request.reason());

            if (request.mode() ==
                aap_protobuf::channel::control::focus::video::shared::VideoFocusMode::VIDEO_FOCUS_NATIVE) {
              // Return to OS
              OPENAUTO_LOG(info) << "[VideoMediaSinkService] Returning to OS.";
              try {
                if (!std::ifstream("/tmp/entityexit")) {
                  std::ofstream("/tmp/entityexit");
                }
              } catch (...) {
                OPENAUTO_LOG(error) << "[VideoMediaSinkService] Error in creating /tmp/entityexit";
              }
            }

            this->sendVideoFocusIndication();
            channel_->receive(this->shared_from_this());
          }

          void VideoMediaSinkService::sendVideoFocusIndication() {
            OPENAUTO_LOG(info) << "[VideoMediaSinkService] sendVideoFocusIndication()";

            aap_protobuf::channel::control::focus::video::notification::VideoFocusNotification videoFocusIndication;
            videoFocusIndication.set_focus(
                aap_protobuf::channel::control::focus::video::shared::VideoFocusMode::VIDEO_FOCUS_PROJECTED);
            videoFocusIndication.set_unsolicited(false);

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then([]() {}, std::bind(&VideoMediaSinkService::onChannelError, this->shared_from_this(),
                                             std::placeholders::_1));
            channel_->sendVideoFocusIndication(videoFocusIndication, std::move(promise));
          }
        }
      }
    }
  }
}