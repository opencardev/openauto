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
              OPENAUTO_LOG(info) << "[MediaSinkService] start, channel: "
                                 << aasdk::messenger::channelIdToString(channel_->getId());
              channel_->receive(this->shared_from_this());
            });
          }

          void AudioMediaSinkService::stop() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[MediaSinkService] stop, channel: "
                                 << aasdk::messenger::channelIdToString(channel_->getId());
              audioOutput_->stop();
            });
          }

          void AudioMediaSinkService::pause() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[MediaSinkService] pause.";
            });
          }

          void AudioMediaSinkService::resume() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[MediaSinkService] resume.";
            });
          }

          /*
           * Service Discovery
           */

          void AudioMediaSinkService::fillFeatures(
              aap_protobuf::channel::control::servicediscovery::notification::ServiceDiscoveryResponse &response) {
            OPENAUTO_LOG(info) << "[MediaSinkService] fill features, channel: "
                               << aasdk::messenger::channelIdToString(channel_->getId());

            auto *channelDescriptor = response.add_channels();
            channelDescriptor->set_channel_id(static_cast<uint32_t>(channel_->getId()));

            auto audioChannel = channelDescriptor->mutable_media_sink_service();

            audioChannel->set_stream_type(
                aap_protobuf::service::media::shared::message::MediaCodecType::MEDIA_CODEC_AUDIO_PCM);

            switch (channel_->getId()) {
              case aasdk::messenger::ChannelId::MEDIA_SINK_SYSTEM_AUDIO:
                audioChannel->set_audio_type(
                    aap_protobuf::service::media::sink::AudioStreamType::AUDIO_STREAM_SYSTEM_AUDIO);
                break;

              case aasdk::messenger::ChannelId::MEDIA_SINK_MEDIA_AUDIO:
                audioChannel->set_audio_type(aap_protobuf::service::media::sink::AudioStreamType::AUDIO_STREAM_MEDIA);
                break;

              case aasdk::messenger::ChannelId::MEDIA_SINK_GUIDANCE_AUDIO:
                audioChannel->set_audio_type(
                    aap_protobuf::service::media::sink::AudioStreamType::AUDIO_STREAM_GUIDANCE);
                break;

              case aasdk::messenger::ChannelId::MEDIA_SINK_TELEPHONY_AUDIO:
                audioChannel->set_audio_type(
                    aap_protobuf::service::media::sink::AudioStreamType::AUDIO_STREAM_TELEPHONY);
                break;
              default:
                break;
            }

            audioChannel->set_available_while_in_call(true);

            auto *audioConfig = audioChannel->add_audio_configs();
            audioConfig->set_sampling_rate(audioOutput_->getSampleRate());
            audioConfig->set_number_of_bits(audioOutput_->getSampleSize());
            audioConfig->set_number_of_channels(audioOutput_->getChannelCount());
          }

          /*
           * Base Channel Handling
           */

          void AudioMediaSinkService::onChannelOpenRequest(const aap_protobuf::channel::ChannelOpenRequest &request) {
            OPENAUTO_LOG(info) << "[MediaSinkService] Channel Open Request with priority " << request.priority()
                               << " on channel " << aasdk::messenger::channelIdToString(channel_->getId());

            OPENAUTO_LOG(debug) << "[AudioMediaSinkService] Sample Rate: " << audioOutput_->getSampleRate()
                                << ", Sample Size: " << audioOutput_->getSampleSize()
                                << ", Channels: " << audioOutput_->getChannelCount();

            const aap_protobuf::shared::MessageStatus status = audioOutput_->open()
                                                               ? aap_protobuf::shared::MessageStatus::STATUS_SUCCESS
                                                               : aap_protobuf::shared::MessageStatus::STATUS_INTERNAL_ERROR;

            OPENAUTO_LOG(info) << "[MediaSinkService] open status: " << status;

            aap_protobuf::channel::ChannelOpenResponse response;
            response.set_status(status);

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then([]() {}, std::bind(&AudioMediaSinkService::onChannelError, this->shared_from_this(),
                                             std::placeholders::_1));
            channel_->sendChannelOpenResponse(response, std::move(promise));
            channel_->receive(this->shared_from_this());
          }

          void AudioMediaSinkService::onChannelError(const aasdk::error::Error &e) {
            OPENAUTO_LOG(error) << "[MediaSinkService] channel error: " << e.what()
                                << ", channel: " << aasdk::messenger::channelIdToString(channel_->getId());
          }

          /*
           * Media Channel Handling
           */

          void AudioMediaSinkService::onMediaChannelSetupRequest(const aap_protobuf::channel::media::event::Setup &request) {

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

            promise->then([]() {}, std::bind(&AudioMediaSinkService::onChannelError, this->shared_from_this(),
                                             std::placeholders::_1));
            channel_->sendChannelSetupResponse(response, std::move(promise));
            channel_->receive(this->shared_from_this());
          }


          void AudioMediaSinkService::onMediaChannelStartIndication(const aap_protobuf::channel::media::event::Start &indication) {
            OPENAUTO_LOG(info) << "[MediaSinkService] start indication"
                               << ", channel: " << aasdk::messenger::channelIdToString(channel_->getId())
                               << ", session: " << indication.session_id();
            session_ = indication.session_id();
            audioOutput_->start();
            channel_->receive(this->shared_from_this());
          }

          void AudioMediaSinkService::onMediaChannelStopIndication(const aap_protobuf::channel::media::event::Stop &indication) {
            OPENAUTO_LOG(info) << "[MediaSinkService] stop indication"
                               << ", channel: " << aasdk::messenger::channelIdToString(channel_->getId())
                               << ", session: " << session_;

            session_ = -1;
            audioOutput_->suspend();

            channel_->receive(this->shared_from_this());
          }

          void AudioMediaSinkService::onMediaWithTimestampIndication(aasdk::messenger::Timestamp::ValueType timestamp,
                                                                     const aasdk::common::DataConstBuffer &buffer) {
            audioOutput_->write(timestamp, buffer);

            // TODO: Move MediaSourceMediaAckIndication to Ack and move to Shared.
            aap_protobuf::service::media::source::message::MediaSourceMediaAckIndication indication;
            indication.set_session_id(session_);
            indication.set_ack(1);

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then([]() {}, std::bind(&AudioMediaSinkService::onChannelError, this->shared_from_this(),
                                             std::placeholders::_1));
            channel_->sendMediaAckIndication(indication, std::move(promise));
            channel_->receive(this->shared_from_this());
          }

          void AudioMediaSinkService::onMediaIndication(const aasdk::common::DataConstBuffer &buffer) {
            this->onMediaWithTimestampIndication(0, buffer);
          }


        }
      }
    }
  }
}