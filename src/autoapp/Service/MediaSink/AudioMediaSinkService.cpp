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

#include <f1x/openauto/autoapp/Service/MediaSink/AudioMediaSinkService.hpp>
#include <modern/Logger.hpp>

namespace f1x {
namespace openauto {
namespace autoapp {
namespace service {
namespace mediasink {

AudioMediaSinkService::AudioMediaSinkService(
    boost::asio::io_service &ioService,
    aasdk::channel::mediasink::audio::IAudioMediaSinkService::Pointer channel,
    projection::IAudioOutput::Pointer audioOutput)
    : strand_(ioService),
      channel_(std::move(channel)),
      audioOutput_(std::move(audioOutput)),
      session_(-1) {}

void AudioMediaSinkService::start() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
        LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] start()");
        LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] Channel " +
                                   aasdk::messenger::channelIdToString(channel_->getId()));
        channel_->receive(this->shared_from_this());
    });
}

void AudioMediaSinkService::stop() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
        LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] stop()");
        LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] Channel " +
                                   aasdk::messenger::channelIdToString(channel_->getId()));
        audioOutput_->stop();
    });
}

void AudioMediaSinkService::pause() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
        LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] pause()");
        LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] Channel " +
                                   aasdk::messenger::channelIdToString(channel_->getId()));
    });
}

void AudioMediaSinkService::resume() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
        LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] resume()");
        LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] Channel " +
                                   aasdk::messenger::channelIdToString(channel_->getId()));
    });
}

/*
 * Service Discovery
 */

void AudioMediaSinkService::fillFeatures(
    aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] fillFeatures()");
    LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] Channel: " +
                               aasdk::messenger::channelIdToString(channel_->getId()));

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto audioChannel = service->mutable_media_sink_service();

    audioChannel->set_available_type(
        aap_protobuf::service::media::shared::message::MediaCodecType::MEDIA_CODEC_AUDIO_PCM);

    switch (channel_->getId()) {
        case aasdk::messenger::ChannelId::MEDIA_SINK_SYSTEM_AUDIO:
            LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] System Audio.");
            audioChannel->set_audio_type(aap_protobuf::service::media::sink::message::
                                             AudioStreamType::AUDIO_STREAM_SYSTEM_AUDIO);
            break;

        case aasdk::messenger::ChannelId::MEDIA_SINK_MEDIA_AUDIO:
            LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] Music Audio.");
            audioChannel->set_audio_type(
                aap_protobuf::service::media::sink::message::AudioStreamType::AUDIO_STREAM_MEDIA);
            break;

        case aasdk::messenger::ChannelId::MEDIA_SINK_GUIDANCE_AUDIO:
            LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] Guidance Audio.");
            audioChannel->set_audio_type(aap_protobuf::service::media::sink::message::
                                             AudioStreamType::AUDIO_STREAM_GUIDANCE);
            break;

        case aasdk::messenger::ChannelId::MEDIA_SINK_TELEPHONY_AUDIO:
            LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] Telephony Audio.");
            audioChannel->set_audio_type(aap_protobuf::service::media::sink::message::
                                             AudioStreamType::AUDIO_STREAM_TELEPHONY);
            break;
        default:
            LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] Unknown Audio.");
            break;
    }

    audioChannel->set_available_while_in_call(true);

    auto *audioConfig = audioChannel->add_audio_configs();
    audioConfig->set_sampling_rate(audioOutput_->getSampleRate());
    audioConfig->set_number_of_bits(audioOutput_->getSampleSize());
    audioConfig->set_number_of_channels(audioOutput_->getChannelCount());

    LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] getSampleRate " +
                               std::to_string(audioOutput_->getSampleRate()));
    LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] getSampleSize " +
                               std::to_string(audioOutput_->getSampleSize()));
    LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] getChannelCount " +
                               std::to_string(audioOutput_->getChannelCount()));
    // LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] SampleRate " << audioConfig->sampling_rate()
    // << " / " << audioConfig->number_of_bits() << " / " << audioConfig->number_of_channels()");
}

/*
 * Base Channel Handling
 */

void AudioMediaSinkService::onChannelOpenRequest(
    const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] onChannelOpenRequest()");
    LOG_INFO(ANDROID_AUTO,
             "[AudioMediaSinkService] Channel Id: " + std::to_string(request.service_id()) +
                 ", Priority: " + std::to_string(request.priority()));

    LOG_INFO(
        ANDROID_AUTO,
        "[AudioMediaSinkService] Sample Rate: " + std::to_string(audioOutput_->getSampleRate()) +
            ", Sample Size: " + std::to_string(audioOutput_->getSampleSize()) +
            ", Audio Channels: " + std::to_string(audioOutput_->getChannelCount()));

    const aap_protobuf::shared::MessageStatus status =
        audioOutput_->open() ? aap_protobuf::shared::MessageStatus::STATUS_SUCCESS
                             : aap_protobuf::shared::MessageStatus::STATUS_INVALID_CHANNEL;

    LOG_DEBUG(ANDROID_AUTO, "[AudioMediaSinkService] Status determined: " +
                                std::string(aap_protobuf::shared::MessageStatus_Name(status)));

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&AudioMediaSinkService::onChannelError,
                                     this->shared_from_this(), std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
}

void AudioMediaSinkService::onChannelError(const aasdk::error::Error &e) {
    LOG_ERROR(ANDROID_AUTO,
              "[AudioMediaSinkService] onChannelError(): " + std::string(e.what()) +
                  ", channel: " + aasdk::messenger::channelIdToString(channel_->getId()));
}

/*
 * Media Channel Handling
 */

void AudioMediaSinkService::onMediaChannelSetupRequest(
    const aap_protobuf::service::media::shared::message::Setup &request) {
    LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] onMediaChannelSetupRequest()");
    LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] Channel Id: " +
                               aasdk::messenger::channelIdToString(channel_->getId()) +
                               ", Codec: " + MediaCodecType_Name(request.type()));

    aap_protobuf::service::media::shared::message::Config response;
    auto status = aap_protobuf::service::media::shared::message::Config::STATUS_READY;
    response.set_status(status);
    response.set_max_unacked(1);
    response.add_configuration_indices(0);

    auto promise = aasdk::channel::SendPromise::defer(strand_);

    promise->then([]() {}, std::bind(&AudioMediaSinkService::onChannelError,
                                     this->shared_from_this(), std::placeholders::_1));
    channel_->sendChannelSetupResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
}

void AudioMediaSinkService::onMediaChannelStartIndication(
    const aap_protobuf::service::media::shared::message::Start &indication) {
    LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] onMediaChannelStartIndication()");
    LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] Channel Id: " +
                               aasdk::messenger::channelIdToString(channel_->getId()) +
                               ", session: " + std::to_string(indication.session_id()));
    session_ = indication.session_id();
    audioOutput_->start();
    channel_->receive(this->shared_from_this());
}

void AudioMediaSinkService::onMediaChannelStopIndication(
    const aap_protobuf::service::media::shared::message::Stop &indication) {
    LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] onMediaChannelStopIndication()");
    LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] Channel Id: " +
                               aasdk::messenger::channelIdToString(channel_->getId()) +
                               ", session: " + std::to_string(session_));

    session_ = -1;
    audioOutput_->suspend();

    channel_->receive(this->shared_from_this());
}

void AudioMediaSinkService::onMediaWithTimestampIndication(
    aasdk::messenger::Timestamp::ValueType timestamp,
    const aasdk::common::DataConstBuffer &buffer) {
    LOG_DEBUG(ANDROID_AUTO, "[AudioMediaSinkService] onMediaWithTimestampIndication()");
    LOG_DEBUG(ANDROID_AUTO, "[AudioMediaSinkService] Channel Id: " +
                                aasdk::messenger::channelIdToString(channel_->getId()) +
                                ", session: " + std::to_string(session_));

    audioOutput_->write(timestamp, buffer);

    aap_protobuf::service::media::source::message::Ack indication;
    indication.set_session_id(session_);
    indication.set_ack(1);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&AudioMediaSinkService::onChannelError,
                                     this->shared_from_this(), std::placeholders::_1));
    channel_->sendMediaAckIndication(indication, std::move(promise));
    channel_->receive(this->shared_from_this());
}

void AudioMediaSinkService::onMediaIndication(const aasdk::common::DataConstBuffer &buffer) {
    LOG_INFO(ANDROID_AUTO, "[AudioMediaSinkService] onMediaIndication()");

    this->onMediaWithTimestampIndication(0, buffer);
}

}  // namespace mediasink
}  // namespace service
}  // namespace autoapp
}  // namespace openauto
}  // namespace f1x