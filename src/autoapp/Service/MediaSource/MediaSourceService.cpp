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
#include <f1x/openauto/autoapp/Service/MediaSource/MediaSourceService.hpp>


namespace f1x::openauto::autoapp::service::mediasource {

  MediaSourceService::MediaSourceService(boost::asio::io_service &ioService,
                                         aasdk::channel::mediasource::IMediaSourceService::Pointer channel,
                                         projection::IAudioInput::Pointer audioInput)
      : strand_(ioService), channel_(std::move(channel)), audioInput_(std::move(audioInput)), session_(-1) {

  }

  void MediaSourceService::start() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[MediaSourceService] start()";
      channel_->receive(this->shared_from_this());
    });
  }

  void MediaSourceService::stop() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[MediaSourceService] stop()";
      audioInput_->stop();
    });
  }

  void MediaSourceService::pause() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[MediaSourceService] pause()";
    });
  }

  void MediaSourceService::resume() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      OPENAUTO_LOG(info) << "[MediaSourceService] resume()";
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
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    OPENAUTO_LOG(info) << "[MediaSourceService] fillFeatures()";

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *avInputChannel = service->mutable_media_source_service();
    avInputChannel->set_available_type(
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
  void
  MediaSourceService::onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    OPENAUTO_LOG(info) << "[MediaSourceService] onChannelOpenRequest()";
    OPENAUTO_LOG(info) << "[MediaSourceService] Channel Id: " << request.service_id() << ", Priority: "
                       << request.priority();

    const aap_protobuf::shared::MessageStatus status = audioInput_->open()
                                                       ? aap_protobuf::shared::MessageStatus::STATUS_SUCCESS
                                                       : aap_protobuf::shared::MessageStatus::STATUS_INTERNAL_ERROR;

    OPENAUTO_LOG(info) << "[MediaSourceService] Status determined: "
                       << aap_protobuf::shared::MessageStatus_Name(status);

    aap_protobuf::service::control::message::ChannelOpenResponse response;
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
    // OPERATION_ABORTED is expected during shutdown when messenger stops
    if (e.getCode() == aasdk::error::ErrorCode::OPERATION_ABORTED) {
      OPENAUTO_LOG(debug) << "[MediaSourceService] onChannelError(): " << e.what() << " (expected during stop)";
    } else {
      OPENAUTO_LOG(error) << "[MediaSourceService] onChannelError(): " << e.what();
    }
  }

  /*
   * Media Channel Handling
   */

  /**
   * Generic Media Channel Setup Request
   * @param request
   */
  void
  MediaSourceService::onMediaChannelSetupRequest(const aap_protobuf::service::media::shared::message::Setup &request) {

    OPENAUTO_LOG(info) << "[MediaSourceService] onMediaChannelSetupRequest()";
    OPENAUTO_LOG(info) << "[MediaSourceService] Channel Id: " << aasdk::messenger::channelIdToString(channel_->getId())
                       << ", Codec: " << MediaCodecType_Name(request.type());

    aap_protobuf::service::media::shared::message::Config response;
    auto status = aap_protobuf::service::media::shared::message::Config::STATUS_READY;
    response.set_status(status);
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
      const aap_protobuf::service::media::source::message::Ack &) {
    OPENAUTO_LOG(debug) << "[MediaSourceService] onMediaChannelAckIndication()";
    channel_->receive(this->shared_from_this());
  }

  /*
   * Source Media Channel Handling
   */

  /**
   * Handle request to Open or Close Microphone Source Channel
   * @param request
   */
  void MediaSourceService::onMediaSourceOpenRequest(
      const aap_protobuf::service::media::source::message::MicrophoneRequest &request) {
    OPENAUTO_LOG(info) << "[MediaSourceService] onMediaSourceOpenRequest()";
    OPENAUTO_LOG(info) << "[MediaSourceService] Request to Open?: " << request.open() << ", anc: "
                       << request.anc_enabled() << ", ec: " << request.ec_enabled() << ", max unacked: "
                       << request.max_unacked();

    if (request.open()) {
      // Request for Channel Open
      auto startPromise = projection::IAudioInput::StartPromise::defer(strand_);
      startPromise->then(std::bind(&MediaSourceService::onMediaSourceOpenSuccess, this->shared_from_this()),
                         [this, self = this->shared_from_this()]() {
                           OPENAUTO_LOG(error) << "[MediaSourceService] Media Source Open Failed";

                           aap_protobuf::service::media::source::message::MicrophoneResponse response;
                           response.set_session_id(session_);

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
      // Request for Channel Close
      audioInput_->stop();

      aap_protobuf::service::media::source::message::MicrophoneResponse response;
      response.set_session_id(session_);

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
    OPENAUTO_LOG(error) << "[MediaSourceService] onMediaSourceOpenSuccess()";

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
    OPENAUTO_LOG(error) << "[MediaSourceService] onMediaSourceDataReady()";
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
    OPENAUTO_LOG(debug) << "[MediaSourceService] readMediaSource()";
    if (audioInput_->isActive()) {
      auto readPromise = projection::IAudioInput::ReadPromise::defer(strand_);
      readPromise->then(
          std::bind(&MediaSourceService::onMediaSourceDataReady, this->shared_from_this(),
                    std::placeholders::_1),
          [this, self = this->shared_from_this()]() {
            OPENAUTO_LOG(debug) << "[MediaSourceService] audio input read rejected.";
          });

      audioInput_->read(std::move(readPromise));
    }
  }
}




