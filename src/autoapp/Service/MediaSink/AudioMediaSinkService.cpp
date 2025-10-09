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

#include <openauto/Common/ModernLogger.hpp>
#include <f1x/openauto/autoapp/Service/MediaSink/AudioMediaSinkService.hpp>

namespace f1x {
  namespace openauto {
    namespace autoapp {
      namespace service {
        namespace mediasink {

          AudioMediaSinkService::AudioMediaSinkService(boost::asio::io_service &ioService,
                                                       aasdk::channel::mediasink::audio::IAudioMediaSinkService::Pointer channel,
                                                       projection::IAudioOutput::Pointer audioOutput)
              : strand_(ioService), channel_(std::move(channel)), audioOutput_(std::move(audioOutput)), session_(-1) {

          }

          void AudioMediaSinkService::start() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] start()");
              OPENAUTO_LOG_INFO(ANDROID_AUTO, (std::stringstream() << "[AudioMediaSinkService] Channel " << aasdk::messenger::channelIdToString(channel_->getId())).str());
              channel_->receive(this->shared_from_this());
            });
          }

          void AudioMediaSinkService::stop() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] stop()");
              OPENAUTO_LOG_INFO(ANDROID_AUTO, (std::stringstream() << "[AudioMediaSinkService] Channel " << aasdk::messenger::channelIdToString(channel_->getId())).str());
              audioOutput_->stop();
            });
          }

          void AudioMediaSinkService::pause() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] pause()");
              OPENAUTO_LOG_INFO(ANDROID_AUTO, (std::stringstream() << "[AudioMediaSinkService] Channel " << aasdk::messenger::channelIdToString(channel_->getId())).str());

            });
          }

          void AudioMediaSinkService::resume() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] resume()");
              OPENAUTO_LOG_INFO(ANDROID_AUTO, (std::stringstream() << "[AudioMediaSinkService] Channel " << aasdk::messenger::channelIdToString(channel_->getId())).str());

            });
          }

          /*
           * Service Discovery
           */

          void AudioMediaSinkService::fillFeatures(
              aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
            OPENAUTO_LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] fillFeatures()");
            OPENAUTO_LOG_INFO(ANDROID_AUTO, (std::stringstream() << "[AudioMediaSinkService] Channel: " << aasdk::messenger::channelIdToString(channel_->getId())).str());

            auto *service = response.add_channels();
            service->set_id(static_cast<uint32_t>(channel_->getId()));

            auto audioChannel = service->mutable_media_sink_service();

            audioChannel->set_available_type(
                aap_protobuf::service::media::shared::message::MediaCodecType::MEDIA_CODEC_AUDIO_PCM);

            switch (channel_->getId()) {
              case aasdk::messenger::ChannelId::MEDIA_SINK_SYSTEM_AUDIO:
                OPENAUTO_LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] System Audio.");
                audioChannel->set_audio_type(
                    aap_protobuf::service::media::sink::message::AudioStreamType::AUDIO_STREAM_SYSTEM_AUDIO);
                break;

              case aasdk::messenger::ChannelId::MEDIA_SINK_MEDIA_AUDIO:
                OPENAUTO_LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] Music Audio.");
                audioChannel->set_audio_type(aap_protobuf::service::media::sink::message::AudioStreamType::AUDIO_STREAM_MEDIA);
                break;

              case aasdk::messenger::ChannelId::MEDIA_SINK_GUIDANCE_AUDIO:
                OPENAUTO_LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] Guidance Audio.");
                audioChannel->set_audio_type(
                    aap_protobuf::service::media::sink::message::AudioStreamType::AUDIO_STREAM_GUIDANCE);
                break;

              case aasdk::messenger::ChannelId::MEDIA_SINK_TELEPHONY_AUDIO:
                OPENAUTO_LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] Telephony Audio.");
                audioChannel->set_audio_type(
                    aap_protobuf::service::media::sink::message::AudioStreamType::AUDIO_STREAM_TELEPHONY);
                break;
              default:
                OPENAUTO_LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] Unknown Audio.");
                break;
            }

            audioChannel->set_available_while_in_call(true);

            auto *audioConfig = audioChannel->add_audio_configs();
            audioConfig->set_sampling_rate(audioOutput_->getSampleRate());
            audioConfig->set_number_of_bits(audioOutput_->getSampleSize());
            audioConfig->set_number_of_channels(audioOutput_->getChannelCount());

            OPENAUTO_LOG_INFO(ANDROID_AUTO, (std::stringstream() << "[AudioMediaSinkService] getSampleRate " << audioOutput_->getSampleRate()).str());
            OPENAUTO_LOG_INFO(ANDROID_AUTO, (std::stringstream() << "[AudioMediaSinkService] getSampleSize " << audioOutput_->getSampleSize()).str());
            OPENAUTO_LOG_INFO(ANDROID_AUTO, (std::stringstream() << "[AudioMediaSinkService] getChannelCount " << audioOutput_->getChannelCount()).str());
            //OPENAUTO_LOG_INFO(ANDROID_AUTO, (std::stringstream() << "[AudioMediaSinkService] SampleRate " << audioConfig->sampling_rate() << " / " << audioConfig->number_of_bits() << " / " << audioConfig->number_of_channels()).str());
          }

          /*
           * Base Channel Handling
           */

          void AudioMediaSinkService::onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
            OPENAUTO_LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] onChannelOpenRequest()");
            OPENAUTO_LOG_INFO(ANDROID_AUTO, (std::stringstream() << "[AudioMediaSinkService] Channel Id: " << request.service_id() << ", Priority: " << request.priority()).str());

            OPENAUTO_LOG_INFO(ANDROID_AUTO, (std::stringstream() << "[AudioMediaSinkService] Sample Rate: " << audioOutput_->getSampleRate() << ", Sample Size: " << audioOutput_->getSampleSize() << ", Audio Channels: " << audioOutput_->getChannelCount()).str());

            const aap_protobuf::shared::MessageStatus status = audioOutput_->open()
                                                               ? aap_protobuf::shared::MessageStatus::STATUS_SUCCESS
                                                               : aap_protobuf::shared::MessageStatus::STATUS_INVALID_CHANNEL;

            OPENAUTO_LOG_DEBUG(ANDROID_AUTO, (std::stringstream() << "[AudioMediaSinkService] Status determined: " << aap_protobuf::shared::MessageStatus_Name(status)).str());

            aap_protobuf::service::control::message::ChannelOpenResponse response;
            response.set_status(status);

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then([]() {}, std::bind(&AudioMediaSinkService::onChannelError, this->shared_from_this(),
                                             std::placeholders::_1));
            channel_->sendChannelOpenResponse(response, std::move(promise));
            channel_->receive(this->shared_from_this());
          }

          void AudioMediaSinkService::onChannelError(const aasdk::error::Error &e) {
            OPENAUTO_LOG_ERROR(ANDROID_AUTO, (std::stringstream() << "[AudioMediaSinkService] onChannelError(): " << e.what()
                                << ", channel: " << aasdk::messenger::channelIdToString(channel_->getId())).str());
          }

          /*
           * Media Channel Handling
           */

          void AudioMediaSinkService::onMediaChannelSetupRequest(const aap_protobuf::service::media::shared::message::Setup &request) {
            OPENAUTO_LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] onMediaChannelSetupRequest()");
            OPENAUTO_LOG_INFO(ANDROID_AUTO, (std::stringstream() << "[AudioMediaSinkService] Channel Id: " << aasdk::messenger::channelIdToString(channel_->getId()) << ", Codec: " << MediaCodecType_Name(request.type())).str());

            aap_protobuf::service::media::shared::message::Config response;
            auto status = aap_protobuf::service::media::shared::message::Config::STATUS_READY;
            response.set_status(status);
            response.set_max_unacked(1);
            response.add_configuration_indices(0);

            auto promise = aasdk::channel::SendPromise::defer(strand_);

            promise->then([]() {}, std::bind(&AudioMediaSinkService::onChannelError, this->shared_from_this(),
                                             std::placeholders::_1));
            channel_->sendChannelSetupResponse(response, std::move(promise));
            channel_->receive(this->shared_from_this());
          }


          void AudioMediaSinkService::onMediaChannelStartIndication(const aap_protobuf::service::media::shared::message::Start &indication) {
            OPENAUTO_LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] onMediaChannelStartIndication()");
            OPENAUTO_LOG_INFO(ANDROID_AUTO, (std::stringstream() << "[AudioMediaSinkService] Channel Id: " << aasdk::messenger::channelIdToString(channel_->getId()) << ", session: " << indication.session_id()).str());
            session_ = indication.session_id();
            audioOutput_->start();
            channel_->receive(this->shared_from_this());
          }

          void AudioMediaSinkService::onMediaChannelStopIndication(const aap_protobuf::service::media::shared::message::Stop &indication) {
            OPENAUTO_LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] onMediaChannelStopIndication()");
            OPENAUTO_LOG_INFO(ANDROID_AUTO, (std::stringstream() << "[AudioMediaSinkService] Channel Id: " << aasdk::messenger::channelIdToString(channel_->getId()) << ", session: " << session_).str());

            session_ = -1;
            audioOutput_->suspend();

            channel_->receive(this->shared_from_this());
          }

          void AudioMediaSinkService::onMediaWithTimestampIndication(aasdk::messenger::Timestamp::ValueType timestamp,
                                                                     const aasdk::common::DataConstBuffer &buffer) {
            OPENAUTO_LOG_DEBUG(ANDROID_AUTO, "[AudioMediaSinkService] onMediaWithTimestampIndication()");
            OPENAUTO_LOG_DEBUG(ANDROID_AUTO, (std::stringstream() << "[AudioMediaSinkService] Channel Id: " << aasdk::messenger::channelIdToString(channel_->getId()) << ", session: " << session_).str());

            audioOutput_->write(timestamp, buffer);

            aap_protobuf::service::media::source::message::Ack indication;
            indication.set_session_id(session_);
            indication.set_ack(1);

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then([]() {}, std::bind(&AudioMediaSinkService::onChannelError, this->shared_from_this(),
                                             std::placeholders::_1));
            channel_->sendMediaAckIndication(indication, std::move(promise));
            channel_->receive(this->shared_from_this());
          }

          void AudioMediaSinkService::onMediaIndication(const aasdk::common::DataConstBuffer &buffer) {
            OPENAUTO_LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] onMediaIndication()");

            this->onMediaWithTimestampIndication(0, buffer);
          }


        }
      }
    }
  }
}