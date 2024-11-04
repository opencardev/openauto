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

#include <time.h>
#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Service/MediaSource/MediaSourceService.hpp>
#include <aap_protobuf/service/media/sink/message/MediaSinkChannelSetupResponse.pb.h>

namespace f1x {
  namespace openauto {
    namespace autoapp {
      namespace service {
        namespace mediasource {

          MediaSourceService::MediaSourceService(boost::asio::io_service &ioService,
                                                 aasdk::channel::mediasource::IMediaSourceService::Pointer channel,
                                                 projection::IAudioInput::Pointer audioInput)
              : strand_(ioService), channel_(std::move(channel)), audioInput_(std::move(audioInput)), session_(-1) {

          }

          void MediaSourceService::start() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[AudioInputService] start.";
              channel_->receive(this->shared_from_this());
            });
          }

          void MediaSourceService::stop() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[AudioInputService] stop.";
              audioInput_->stop();
            });
          }

          void MediaSourceService::pause() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[AudioInputService] pause.";
            });
          }

          void MediaSourceService::resume() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[AudioInputService] resume.";
            });
          }

          /*
           * Service Discovery
           */

          /**
           * Fill Features of Service
           * @param response
           */
          void MediaSourceService::fillFeatures(
              aap_protobuf::channel::control::servicediscovery::notification::ServiceDiscoveryResponse &response) {
            OPENAUTO_LOG(info) << "[AudioInputService] fill features.";

            auto *channelDescriptor = response.add_channels();
            channelDescriptor->set_channel_id(static_cast<uint32_t>(channel_->getId()));

            auto *avInputChannel = channelDescriptor->mutable_media_source_service();
            avInputChannel->set_stream_type(
                aap_protobuf::service::media::shared::message::MediaCodecType::MEDIA_CODEC_AUDIO_PCM);

            auto audioConfig = avInputChannel->mutable_audio_config();
            audioConfig->set_sampling_rate(audioInput_->getSampleRate());
            audioConfig->set_number_of_bits(audioInput_->getSampleSize());
            audioConfig->set_number_of_channels(audioInput_->getChannelCount());
          }

          /*
           * Base Channel Handling
           */

          /**
           * Open Service Channel Request
           * @param request
           */
          void MediaSourceService::onChannelOpenRequest(const aap_protobuf::channel::ChannelOpenRequest &request) {
            OPENAUTO_LOG(info) << "[AudioInputService] open request, priority: " << request.priority();
            const aap_protobuf::shared::MessageStatus status = audioInput_->open()
                                                               ? aap_protobuf::shared::MessageStatus::STATUS_SUCCESS
                                                               : aap_protobuf::shared::MessageStatus::STATUS_UNSOLICITED_MESSAGE;
            OPENAUTO_LOG(info) << "[AudioInputService] open status: " << status;

            aap_protobuf::channel::ChannelOpenResponse response;
            response.set_status(status);

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then([]() {},
                          std::bind(&MediaSourceService::onChannelError, this->shared_from_this(),
                                    std::placeholders::_1));
            channel_->sendChannelOpenResponse(response, std::move(promise));

            channel_->receive(this->shared_from_this());
          }

          /**
           * Generic Channel Error
           * @param e
           */
          void MediaSourceService::onChannelError(const aasdk::error::Error &e) {
            OPENAUTO_LOG(error) << "[AudioInputService] channel error: " << e.what();
          }

          /*
           * Media Channel Handling
           */

          /**
           * Generic Media Channel Setup Request
           * @param request
           */
          void
          MediaSourceService::onMediaChannelSetupRequest(const aap_protobuf::channel::media::event::Setup &request) {

            OPENAUTO_LOG(info) << "[MediaSinkService] setup request"
                               << ", channel: " << aasdk::messenger::channelIdToString(channel_->getId())
                               << ", codec type: " << MediaCodecType_Name(request.type());

            auto status = aap_protobuf::service::media::sink::MediaSinkChannelSetupStatus::STATUS_READY;

            OPENAUTO_LOG(info) << "[MediaSinkService] setup status: " << status;

            aap_protobuf::service::media::sink::message::MediaSinkChannelSetupResponse response;
            response.set_media_status(status);
            response.set_max_unacked(1);
            response.add_configuration_indices(0);

            auto promise = aasdk::channel::SendPromise::defer(strand_);

            promise->then([]() {}, std::bind(&MediaSourceService::onChannelError, this->shared_from_this(),
                                             std::placeholders::_1));
            channel_->sendChannelSetupResponse(response, std::move(promise));
            channel_->receive(this->shared_from_this());
          }

          /**
           * Generic Media Ack
           */
          void MediaSourceService::onMediaChannelAckIndication(
              const aap_protobuf::service::media::source::message::MediaSourceMediaAckIndication &) {
            channel_->receive(this->shared_from_this());
          }

          /*
           * Source Media Channel Handling
           */

          // TODO: These are Source Channel Handlers - should be moved to their own handlers in case any more are implemented in the future.

          /**
           * Handle request to open Microphone Channel
           * @param request
           */
          void MediaSourceService::onMediaSourceOpenRequest(
              const aap_protobuf::service::media::source::message::MicrophoneRequest &request) {
            OPENAUTO_LOG(info) << "[AudioInputService] input open request, open: " << request.open()
                               << ", anc: " << request.anc_enabled()
                               << ", ec: " << request.ec_enabled()
                               << ", max unacked: " << request.max_unacked();

            if (request.open()) {
              auto startPromise = projection::IAudioInput::StartPromise::defer(strand_);
              startPromise->then(std::bind(&MediaSourceService::onMediaSourceOpenSuccess, this->shared_from_this()),
                                 [this, self = this->shared_from_this()]() {
                                   OPENAUTO_LOG(error) << "[AudioInputService] audio input open failed.";

                                   aap_protobuf::service::media::source::message::MicrophoneResponse response;
                                   response.set_session_id(session_);
                                   // TODO: Matches previous number, but doesn't seem like the right status.
                                   response.set_status(aap_protobuf::shared::MessageStatus::STATUS_INTERNAL_ERROR);

                                   auto sendPromise = aasdk::channel::SendPromise::defer(strand_);
                                   sendPromise->then([]() {},
                                                     std::bind(&MediaSourceService::onChannelError,
                                                               this->shared_from_this(),
                                                               std::placeholders::_1));
                                   channel_->sendMicrophoneOpenResponse(response, std::move(sendPromise));
                                 });

              audioInput_->start(std::move(startPromise));
            } else {
              audioInput_->stop();

              aap_protobuf::service::media::source::message::MicrophoneResponse response;
              response.set_session_id(session_);
              // TODO: Matches previous number, but doesn't seem like the right status.
              response.set_status(aap_protobuf::shared::MessageStatus::STATUS_SUCCESS);

              auto sendPromise = aasdk::channel::SendPromise::defer(strand_);
              sendPromise->then([]() {}, std::bind(&MediaSourceService::onChannelError, this->shared_from_this(),
                                                   std::placeholders::_1));
              channel_->sendMicrophoneOpenResponse(response, std::move(sendPromise));
            }

            channel_->receive(this->shared_from_this());
          }


          /**
           * Sends response to advise Microphone is Open
           */
          void MediaSourceService::onMediaSourceOpenSuccess() {
            OPENAUTO_LOG(info) << "[AudioInputService] audio input open succeed.";

            aap_protobuf::service::media::source::message::MicrophoneResponse response;
            response.set_session_id(session_);
            response.set_status(aap_protobuf::shared::MessageStatus::STATUS_SUCCESS);

            auto sendPromise = aasdk::channel::SendPromise::defer(strand_);
            sendPromise->then([]() {}, std::bind(&MediaSourceService::onChannelError, this->shared_from_this(),
                                                 std::placeholders::_1));

            channel_->sendMicrophoneOpenResponse(response, std::move(sendPromise));

            this->readMediaSource();
          }

          /**
           * Resolves promise from readMediaSource. Sends Media with Timestamp Indication to channel.
           * @param data
           */
          void MediaSourceService::onMediaSourceDataReady(aasdk::common::Data data) {
            auto sendPromise = aasdk::channel::SendPromise::defer(strand_);
            sendPromise->then(std::bind(&MediaSourceService::readMediaSource, this->shared_from_this()),
                              std::bind(&MediaSourceService::onChannelError, this->shared_from_this(),
                                        std::placeholders::_1));

            auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch());
            channel_->sendMediaSourceWithTimestampIndication(timestamp.count(), std::move(data), std::move(sendPromise));
          }

          /**
           * Reads audio from a MediaSource (eg Microphone). Promise resolves to onMediaSourceDataReady.
           */
          void MediaSourceService::readMediaSource() {
            if (audioInput_->isActive()) {
              auto readPromise = projection::IAudioInput::ReadPromise::defer(strand_);
              readPromise->then(
                  std::bind(&MediaSourceService::onMediaSourceDataReady, this->shared_from_this(),
                            std::placeholders::_1),
                  [this, self = this->shared_from_this()]() {
                    OPENAUTO_LOG(info) << "[AudioInputService] audio input read rejected.";
                  });

              audioInput_->read(std::move(readPromise));
            }
          }
        }
      }
    }
  }
}
