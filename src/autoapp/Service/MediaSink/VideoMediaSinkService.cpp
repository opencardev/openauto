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
              LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] start()");
              LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] Channel "
                                 << aasdk::messenger::channelIdToString(channel_->getId())");
              channel_->receive(this->shared_from_this());
            });
          }

          void VideoMediaSinkService::stop() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] stop()");
              LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] Channel "
                                 << aasdk::messenger::channelIdToString(channel_->getId())");
              videoOutput_->stop();
            });
          }

          void VideoMediaSinkService::pause() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] pause()");
              LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] Channel "
                                 << aasdk::messenger::channelIdToString(channel_->getId())");
            });
          }

          void VideoMediaSinkService::resume() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] resume()");
              LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] Channel "
                                 << aasdk::messenger::channelIdToString(channel_->getId())");

            });
          }

          void VideoMediaSinkService::fillFeatures(
              aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
            LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] fillFeatures()");
            LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] Channel "
                               << aasdk::messenger::channelIdToString(channel_->getId())");

            auto *service = response.add_channels();
            service->set_id(static_cast<uint32_t>(channel_->getId()));

            auto *videoChannel = service->mutable_media_sink_service();

            videoChannel->set_available_type(
                aap_protobuf::service::media::shared::message::MediaCodecType::MEDIA_CODEC_VIDEO_H264_BP);
            videoChannel->set_available_while_in_call(true);


            auto *videoConfig1 = videoChannel->add_video_configs();
            videoConfig1->set_codec_resolution(videoOutput_->getVideoResolution());
            videoConfig1->set_frame_rate(videoOutput_->getVideoFPS());

            const auto &videoMargins = videoOutput_->getVideoMargins();
            videoConfig1->set_height_margin(videoMargins.height());
            videoConfig1->set_width_margin(videoMargins.width());
            videoConfig1->set_density(videoOutput_->getScreenDPI());

            LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] getVideoResolution " << VideoCodecResolutionType_Name(videoOutput_->getVideoResolution())");
            LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] getVideoFPS " << VideoFrameRateType_Name(videoOutput_->getVideoFPS())");
            LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] width " << videoMargins.width()");
            LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] height " << videoMargins.height()");
            LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] getScreenDPI " << videoOutput_->getScreenDPI()");
          }

          void
          VideoMediaSinkService::onMediaChannelSetupRequest(const aap_protobuf::service::media::shared::message::Setup &request) {
            LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] onMediaChannelSetupRequest()");
            LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] Channel Id: "
                               << aasdk::messenger::channelIdToString(channel_->getId()) << ", Codec: "
                               << MediaCodecType_Name(request.type())");


            auto status = videoOutput_->init()
                          ? aap_protobuf::service::media::shared::message::Config::STATUS_READY
                          : aap_protobuf::service::media::shared::message::Config::STATUS_WAIT;

            LOG_DEBUG(ANDROID_AUTO, "[VideoMediaSinkService] setup status: " << Config_Status_Name(status)");

            aap_protobuf::service::media::shared::message::Config response;
            response.set_status(status);
            response.set_max_unacked(1);
            response.add_configuration_indices(0);

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then(std::bind(&VideoMediaSinkService::sendVideoFocusIndication, this->shared_from_this()),
                          std::bind(&VideoMediaSinkService::onChannelError, this->shared_from_this(),
                                    std::placeholders::_1));

            channel_->sendChannelSetupResponse(response, std::move(promise));
            channel_->receive(this->shared_from_this());
          }

          void VideoMediaSinkService::onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
            LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] onChannelOpenRequest()");
            LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] Channel Id: " << request.service_id() << ", Priority: "
                               << request.priority()");

            const aap_protobuf::shared::MessageStatus status = videoOutput_->open()
                                                               ? aap_protobuf::shared::MessageStatus::STATUS_SUCCESS
                                                               : aap_protobuf::shared::MessageStatus::STATUS_INTERNAL_ERROR;

            LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] Status determined: "
                               << aap_protobuf::shared::MessageStatus_Name(status)");

            aap_protobuf::service::control::message::ChannelOpenResponse response;
            response.set_status(status);

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then([]() {}, std::bind(&VideoMediaSinkService::onChannelError, this->shared_from_this(),
                                             std::placeholders::_1));
            channel_->sendChannelOpenResponse(response, std::move(promise));
            channel_->receive(this->shared_from_this());
          }

          void VideoMediaSinkService::onMediaChannelStartIndication(
              const aap_protobuf::service::media::shared::message::Start &indication) {
            LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] onMediaChannelStartIndication()");
            LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] Channel Id: "
                               << aasdk::messenger::channelIdToString(channel_->getId()) << ", session: "
                               << indication.session_id()");

            session_ = indication.session_id();
            channel_->receive(this->shared_from_this());
          }

          void VideoMediaSinkService::onMediaChannelStopIndication(
              const aap_protobuf::service::media::shared::message::Stop &indication) {
            LOG_INFO(ANDROID_AUTO, "[onMediaChannelStopIndication] onMediaChannelStopIndication()");
            LOG_INFO(ANDROID_AUTO, "[onMediaChannelStopIndication] Channel Id: "
                               << aasdk::messenger::channelIdToString(channel_->getId()) << ", session: " << session_");

            channel_->receive(this->shared_from_this());
          }

          void VideoMediaSinkService::onMediaWithTimestampIndication(aasdk::messenger::Timestamp::ValueType timestamp,
                                                                     const aasdk::common::DataConstBuffer &buffer) {
            LOG_DEBUG(ANDROID_AUTO, "[VideoMediaSinkService] onMediaWithTimestampIndication()");
            LOG_DEBUG(ANDROID_AUTO, "[VideoMediaSinkService] Channel Id: "
                               << aasdk::messenger::channelIdToString(channel_->getId()) << ", session: " << session_");

            videoOutput_->write(timestamp, buffer);

            aap_protobuf::service::media::source::message::Ack indication;
            indication.set_session_id(session_);
            indication.set_ack(1);

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then([]() {}, std::bind(&VideoMediaSinkService::onChannelError, this->shared_from_this(),
                                             std::placeholders::_1));
            channel_->sendMediaAckIndication(indication, std::move(promise));
            channel_->receive(this->shared_from_this());
          }

          void VideoMediaSinkService::onMediaIndication(const aasdk::common::DataConstBuffer &buffer) {
            LOG_DEBUG(ANDROID_AUTO, "[VideoMediaSinkService] onMediaIndication()");
            this->onMediaWithTimestampIndication(0, buffer);
          }

          void VideoMediaSinkService::onChannelError(const aasdk::error::Error &e) {
            LOG_ERROR(ANDROID_AUTO, "[VideoMediaSinkService] onChannelError(): " << e.what()
                                << ", channel: " << aasdk::messenger::channelIdToString(channel_->getId())");
          }

          void VideoMediaSinkService::onVideoFocusRequest(
              const aap_protobuf::service::media::video::message::VideoFocusRequestNotification &request) {
            LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] onVideoFocusRequest()");
            // Note: disp_channel_id() is deprecated but still used for logging compatibility
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] Display index: " << request.disp_channel_id() << ", focus mode: " << VideoFocusMode_Name(request.mode()) << ", focus reason: " << VideoFocusReason_Name(request.reason())");
            #pragma GCC diagnostic pop

            if (request.mode() ==
                aap_protobuf::service::media::video::message::VideoFocusMode::VIDEO_FOCUS_NATIVE) {
              // Return to OS
              LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] Returning to OS.");
              try {
                if (!std::ifstream("/tmp/entityexit")) {
                  std::ofstream("/tmp/entityexit");
                }
              } catch (...) {
                LOG_ERROR(ANDROID_AUTO, "[VideoMediaSinkService] Error in creating /tmp/entityexit");
              }
            }

            this->sendVideoFocusIndication();
            channel_->receive(this->shared_from_this());
          }

          void VideoMediaSinkService::sendVideoFocusIndication() {
            LOG_INFO(ANDROID_AUTO, "[VideoMediaSinkService] sendVideoFocusIndication()");

            aap_protobuf::service::media::video::message::VideoFocusNotification videoFocusIndication;
            videoFocusIndication.set_focus(
                aap_protobuf::service::media::video::message::VideoFocusMode::VIDEO_FOCUS_PROJECTED);
            videoFocusIndication.set_unsolicited(false);

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then([]() { }, std::bind(&VideoMediaSinkService::onChannelError, this->shared_from_this(),
                                             std::placeholders::_1));
            channel_->sendVideoFocusIndication(videoFocusIndication, std::move(promise));
          }
        }
      }
    }
  }
}