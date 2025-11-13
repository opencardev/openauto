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

#include <aasdk/Channel/Control/ControlServiceChannel.hpp>
#include <f1x/openauto/autoapp/Service/AndroidAutoEntity.hpp>
#include <f1x/openauto/Common/Log.hpp>

namespace f1x {
  namespace openauto {
    namespace autoapp {
      namespace service {

        AndroidAutoEntity::AndroidAutoEntity(boost::asio::io_service &ioService,
                                             aasdk::messenger::ICryptor::Pointer cryptor,
                                             aasdk::transport::ITransport::Pointer transport,
                                             aasdk::messenger::IMessenger::Pointer messenger,
                                             configuration::IConfiguration::Pointer configuration,
                                             ServiceList serviceList,
                                             IPinger::Pointer pinger)
            : strand_(ioService), cryptor_(std::move(cryptor)), transport_(std::move(transport)),
              messenger_(std::move(messenger)), controlServiceChannel_(
                std::make_shared<aasdk::channel::control::ControlServiceChannel>(strand_, messenger_)),
              configuration_(std::move(configuration)), serviceList_(std::move(serviceList)),
              pinger_(std::move(pinger)), eventHandler_(nullptr) {
        }

        AndroidAutoEntity::~AndroidAutoEntity() {
          OPENAUTO_LOG(debug) << "[AndroidAutoEntity] destroy.";
        }

        void AndroidAutoEntity::start(IAndroidAutoEntityEventHandler &eventHandler) {
          strand_.dispatch([this, self = this->shared_from_this(), eventHandler = &eventHandler]() {
            OPENAUTO_LOG(info) << "[AndroidAutoEntity] start()";

            eventHandler_ = eventHandler;
            std::for_each(serviceList_.begin(), serviceList_.end(), std::bind(&IService::start, std::placeholders::_1));

            auto versionRequestPromise = aasdk::channel::SendPromise::defer(strand_);
            versionRequestPromise->then([]() {  }, std::bind(&AndroidAutoEntity::onChannelError, this->shared_from_this(),
                                                           std::placeholders::_1));

            OPENAUTO_LOG(debug) << "[AndroidAutoEntity] Send Version Request.";
            controlServiceChannel_->sendVersionRequest(std::move(versionRequestPromise));
            controlServiceChannel_->receive(this->shared_from_this());
          });
        }

        void AndroidAutoEntity::stop() {
          // Mark stopping early to suppress error-triggered quits during teardown
          stopping_.store(true, std::memory_order_relaxed);
          strand_.dispatch([this, self = this->shared_from_this()]() {
            OPENAUTO_LOG(info) << "[AndroidAutoEntity] stop()";

            try {
              eventHandler_ = nullptr;
              std::for_each(serviceList_.begin(), serviceList_.end(),
                            std::bind(&IService::stop, std::placeholders::_1));

              messenger_->stop();
              transport_->stop();
              cryptor_->deinit();
            } catch (...) {
              OPENAUTO_LOG(error) << "[AndroidAutoEntity] stop() - exception when stopping.";
            }
          });
        }

        void AndroidAutoEntity::pause() {
          strand_.dispatch([this, self = this->shared_from_this()]() {
            OPENAUTO_LOG(info) << "[AndroidAutoEntity] pause()";

            try {
              std::for_each(serviceList_.begin(), serviceList_.end(),
                            std::bind(&IService::pause, std::placeholders::_1));
            } catch (...) {
              OPENAUTO_LOG(error) << "[AndroidAutoEntity] pause() - exception when pausing.";
            }
          });
        }

        void AndroidAutoEntity::resume() {
          strand_.dispatch([this, self = this->shared_from_this()]() {
            OPENAUTO_LOG(info) << "[AndroidAutoEntity] resume()";

            try {
              std::for_each(serviceList_.begin(), serviceList_.end(),
                            std::bind(&IService::resume, std::placeholders::_1));
            } catch (...) {
              OPENAUTO_LOG(error) << "[AndroidAutoEntity] resume() exception when resuming.";
            }
          });
        }

        void AndroidAutoEntity::onVersionResponse(uint16_t majorCode, uint16_t minorCode,
                                                  aap_protobuf::shared::MessageStatus status) {
          OPENAUTO_LOG(info) << "[AndroidAutoEntity] onVersionResponse()";
          OPENAUTO_LOG(info) << "[AndroidAutoEntity] Version Received: " << majorCode << "." << minorCode
                             << ", with status: " << status;

          if (status == aap_protobuf::shared::MessageStatus::STATUS_NO_COMPATIBLE_VERSION) {
            OPENAUTO_LOG(error) << "[AndroidAutoEntity] Version mismatch.";
            this->triggerQuit();
          } else {
            OPENAUTO_LOG(debug) << "[AndroidAutoEntity] Version matches.";

            try {
              OPENAUTO_LOG(info) << "[AndroidAutoEntity] Beginning SSL handshake.";
              cryptor_->doHandshake();

              auto handshakePromise = aasdk::channel::SendPromise::defer(strand_);
              handshakePromise->then([]() {  }, std::bind(&AndroidAutoEntity::onChannelError, this->shared_from_this(),
                                                        std::placeholders::_1));
              controlServiceChannel_->sendHandshake(cryptor_->readHandshakeBuffer(), std::move(handshakePromise));
              controlServiceChannel_->receive(this->shared_from_this());
            }
            catch (const aasdk::error::Error &e) {
              OPENAUTO_LOG(info) << "[AndroidAutoEntity] Handshake Error.";
              this->onChannelError(e);
            }
          }
        }

        void AndroidAutoEntity::onHandshake(const aasdk::common::DataConstBuffer &payload) {
          OPENAUTO_LOG(info) << "[AndroidAutoEntity] onHandshake()";
          OPENAUTO_LOG(debug) << "[AndroidAutoEntity] Payload size: " << payload.size;

          try {
            cryptor_->writeHandshakeBuffer(payload);

            if (!cryptor_->doHandshake()) {
              OPENAUTO_LOG(info) << "[AndroidAutoEntity] Re-attempting handshake.";

              auto handshakePromise = aasdk::channel::SendPromise::defer(strand_);
              handshakePromise->then([]() {}, std::bind(&AndroidAutoEntity::onChannelError, this->shared_from_this(),
                                                        std::placeholders::_1));
              controlServiceChannel_->sendHandshake(cryptor_->readHandshakeBuffer(), std::move(handshakePromise));
            } else {
              OPENAUTO_LOG(info) << "[AndroidAutoEntity] Handshake completed.";

              aap_protobuf::service::control::message::AuthResponse authCompleteIndication;
              authCompleteIndication.set_status(aap_protobuf::shared::MessageStatus::STATUS_SUCCESS);

              auto authCompletePromise = aasdk::channel::SendPromise::defer(strand_);
              authCompletePromise->then([]() {}, std::bind(&AndroidAutoEntity::onChannelError, this->shared_from_this(),
                                                           std::placeholders::_1));
              controlServiceChannel_->sendAuthComplete(authCompleteIndication, std::move(authCompletePromise));
            }

            controlServiceChannel_->receive(this->shared_from_this());
          }
          catch (const aasdk::error::Error &e) {
            OPENAUTO_LOG(error) << "[AndroidAutoEntity] Error during handshake";
            this->onChannelError(e);
          }
        }

        void AndroidAutoEntity::onServiceDiscoveryRequest(
            const aap_protobuf::service::control::message::ServiceDiscoveryRequest &request) {
          OPENAUTO_LOG(info) << "[AndroidAutoEntity] onServiceDiscoveryRequest()";
          OPENAUTO_LOG(debug) << "[AndroidAutoEntity] Type: " << request.label_text() << ", Model: "
                             << request.device_name();

          aap_protobuf::service::control::message::ServiceDiscoveryResponse serviceDiscoveryResponse;
          serviceDiscoveryResponse.mutable_channels()->Reserve(256);
          serviceDiscoveryResponse.set_driver_position(aap_protobuf::service::control::message::DriverPosition::DRIVER_POSITION_RIGHT);
          // serviceDiscoveryResponse.set_can_play_native_media_during_vr(false); // Deprecated function removed
          serviceDiscoveryResponse.set_display_name("Crankshaft-NG");
          serviceDiscoveryResponse.set_probe_for_support(false);

          auto *connectionConfiguration = serviceDiscoveryResponse.mutable_connection_configuration();

          auto *pingConfiguration = connectionConfiguration->mutable_ping_configuration();
          pingConfiguration->set_tracked_ping_count(5);
          pingConfiguration->set_timeout_ms(3000);
          pingConfiguration->set_interval_ms(1000);
          pingConfiguration->set_high_latency_threshold_ms(200);


          auto *headUnitInfo = serviceDiscoveryResponse.mutable_headunit_info();

          serviceDiscoveryResponse.set_display_name("Crankshaft-NG");
          headUnitInfo->set_make("Crankshaft");
          headUnitInfo->set_model("Universal");
          headUnitInfo->set_year("2018");
          headUnitInfo->set_vehicle_id("2024110822150988");
          headUnitInfo->set_head_unit_make("f1x");
          headUnitInfo->set_head_unit_model("Crankshaft-NG Autoapp");
          headUnitInfo->set_head_unit_software_build("1");
          headUnitInfo->set_head_unit_software_version("1.0");

          std::for_each(serviceList_.begin(), serviceList_.end(),
                        std::bind(&IService::fillFeatures, std::placeholders::_1, std::ref(serviceDiscoveryResponse)));

          auto promise = aasdk::channel::SendPromise::defer(strand_);
          promise->then([]() {  },
                        std::bind(&AndroidAutoEntity::onChannelError, this->shared_from_this(), std::placeholders::_1));
          controlServiceChannel_->sendServiceDiscoveryResponse(serviceDiscoveryResponse, std::move(promise));
          controlServiceChannel_->receive(this->shared_from_this());
        }

        void AndroidAutoEntity::onAudioFocusRequest(
            const aap_protobuf::service::control::message::AudioFocusRequest &request) {
          OPENAUTO_LOG(info) << "[AndroidAutoEntity] onAudioFocusRequest()";
          OPENAUTO_LOG(debug) << "[AndroidAutoEntity] AudioFocusRequestType received: "
                             << AudioFocusRequestType_Name(request.audio_focus_type());

          /*
           * When the MD starts playing music for example, it sends a gain request. The HU replies:
           * STATE_GAIN - no restrictions
           * STATE_GAIN_MEDIA_ONLY when using a guidance channel
           * STATE_LOSS when vehicle is playing high priority sound after stopping native media (ie USB, RADIO)
           *
           * When HU starts playing music, we should send a STATE LOSS to stop MD music and guidance.
           */

          // If release, we should stop all playback
          // MD wants to play a sound, get a notifiation regarding GAIN
          // HU grants focus - to enable MD to send audio over both MEDIA and GUIDANCE channels.
          // MD can then play guidance over the MEDIA or GUIDANCE streams
          // HU should send STATE_LOSS to stop MD playing (ie if user starts radio player)
          aap_protobuf::service::control::message::AudioFocusStateType audioFocusStateType =
              request.audio_focus_type() ==
              aap_protobuf::service::control::message::AudioFocusRequestType::AUDIO_FOCUS_RELEASE
              ? aap_protobuf::service::control::message::AudioFocusStateType::AUDIO_FOCUS_STATE_LOSS
              : aap_protobuf::service::control::message::AudioFocusStateType::AUDIO_FOCUS_STATE_GAIN;

          OPENAUTO_LOG(debug) << "[AndroidAutoEntity] AudioFocusStateType determined: "
                             << AudioFocusStateType_Name(audioFocusStateType);

          aap_protobuf::service::control::message::AudioFocusNotification response;
          response.set_focus_state(audioFocusStateType);

          auto promise = aasdk::channel::SendPromise::defer(strand_);
          promise->then([]() {  },
                        [capture0 = this->shared_from_this()](auto && PH1) { capture0->onChannelError(std::forward<decltype(PH1)>(PH1)); });
          controlServiceChannel_->sendAudioFocusResponse(response, std::move(promise));
          controlServiceChannel_->receive(this->shared_from_this());
        }

        void AndroidAutoEntity::onByeByeRequest(
            const aap_protobuf::service::control::message::ByeByeRequest &request) {
          OPENAUTO_LOG(info) << "[AndroidAutoEntity] onByeByeRequest()";
          OPENAUTO_LOG(debug) << "[AndroidAutoEntity] Reason received: " << request.reason();

          aap_protobuf::service::control::message::ByeByeResponse response;
          auto promise = aasdk::channel::SendPromise::defer(strand_);
          promise->then(std::bind(&AndroidAutoEntity::triggerQuit, this->shared_from_this()),
                        std::bind(&AndroidAutoEntity::onChannelError, this->shared_from_this(), std::placeholders::_1));

          controlServiceChannel_->sendShutdownResponse(response, std::move(promise));
        }

        void AndroidAutoEntity::onByeByeResponse(
            const aap_protobuf::service::control::message::ByeByeResponse &response) {
          OPENAUTO_LOG(info) << "[AndroidAutoEntity] onByeByeResponse()";
          this->triggerQuit();
        }

        void AndroidAutoEntity::onNavigationFocusRequest(
            const aap_protobuf::service::control::message::NavFocusRequestNotification &request) {
          OPENAUTO_LOG(info) << "[AndroidAutoEntity] onNavigationFocusRequest()";
          OPENAUTO_LOG(debug) << "[AndroidAutoEntity] NavFocusRequestNotification type received: " << NavFocusType_Name(request.focus_type());

          /*
           * If the MD sends NAV_FOCUS_PROJECTED in the request, we should stop any local navigation on the HU and grant NAV_FOCUS_NATIVE in the response.
           * If the HU starts its own Nav, we should send NAV_FOCUS_NATIVE.
           *
           * For now, this is fine to be hardcoded as OpenAuto does not provide any local navigation, only that provided through Android Auto.
           */
          aap_protobuf::service::control::message::NavFocusNotification response;
          response.set_focus_type(
              aap_protobuf::service::control::message::NavFocusType::NAV_FOCUS_PROJECTED);

          auto promise = aasdk::channel::SendPromise::defer(strand_);
          promise->then([]() {},
                        std::bind(&AndroidAutoEntity::onChannelError, this->shared_from_this(), std::placeholders::_1));
          controlServiceChannel_->sendNavigationFocusResponse(response, std::move(promise));
          controlServiceChannel_->receive(this->shared_from_this());
        }

        void AndroidAutoEntity::onBatteryStatusNotification(const aap_protobuf::service::control::message::BatteryStatusNotification &notification) {
          OPENAUTO_LOG(info) << "[AndroidAutoEntity] onBatteryStatusNotification()";
          controlServiceChannel_->receive(this->shared_from_this());
        }

        void AndroidAutoEntity::onPingRequest(const aap_protobuf::service::control::message::PingRequest& request) {
          OPENAUTO_LOG(info) << "[AndroidAutoEntity] onPingRequest()";
          controlServiceChannel_->receive(this->shared_from_this());
        }

        void AndroidAutoEntity::onVoiceSessionRequest(
            const aap_protobuf::service::control::message::VoiceSessionNotification &request) {
          OPENAUTO_LOG(info) << "[AndroidAutoEntity] onVoiceSessionRequest()";
          controlServiceChannel_->receive(this->shared_from_this());
        }

        void AndroidAutoEntity::onPingResponse(const aap_protobuf::service::control::message::PingResponse &response) {
          OPENAUTO_LOG(info) << "[AndroidAutoEntity] onPingResponse()";
          OPENAUTO_LOG(debug) << "[AndroidAutoEntity] Timestamp: " << response.timestamp();
          pinger_->pong();
          controlServiceChannel_->receive(this->shared_from_this());
        }

        void AndroidAutoEntity::onChannelError(const aasdk::error::Error &e) {
          // OPERATION_ABORTED is expected during shutdown when messenger stops
          if (e.getCode() == aasdk::error::ErrorCode::OPERATION_ABORTED) {
            OPENAUTO_LOG(debug) << "[AndroidAutoEntity] onChannelError(): " << e.what() << " (expected during stop)";
            return;
          }
          
          // Ignore other errors if we're already stopping to prevent re-entrant quit
          if (stopping_.load(std::memory_order_relaxed)) {
            OPENAUTO_LOG(info) << "[AndroidAutoEntity] onChannelError() during stopping, ignoring: " << e.what();
            return;
          }
          
          OPENAUTO_LOG(fatal) << "[AndroidAutoEntity] onChannelError(): " << e.what();
          this->triggerQuit();
        }

        void AndroidAutoEntity::triggerQuit() {
          OPENAUTO_LOG(info) << "[AndroidAutoEntity] triggerQuit()";
          if (eventHandler_ != nullptr) {
            eventHandler_->onAndroidAutoQuit();
          }
        }

        void AndroidAutoEntity::schedulePing() {
          OPENAUTO_LOG(info) << "[AndroidAutoEntity] schedulePing()";
          auto promise = IPinger::Promise::defer(strand_);
          promise->then([this, self = this->shared_from_this()]() {
                          this->sendPing();
                          this->schedulePing();
                        },
                        [this, self = this->shared_from_this()](auto error) {
                          if (error != aasdk::error::ErrorCode::OPERATION_ABORTED &&
                              error != aasdk::error::ErrorCode::OPERATION_IN_PROGRESS) {
                            OPENAUTO_LOG(error) << "[AndroidAutoEntity] Ping timer exceeded.";
                            this->triggerQuit();
                          }
                        });

          pinger_->ping(std::move(promise));
        }

        void AndroidAutoEntity::sendPing() {
          OPENAUTO_LOG(debug) << "[AndroidAutoEntity] sendPing()";
          auto promise = aasdk::channel::SendPromise::defer(strand_);
          promise->then([]() {},
                        std::bind(&AndroidAutoEntity::onChannelError, this->shared_from_this(), std::placeholders::_1));

          aap_protobuf::service::control::message::PingRequest request;
          auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::high_resolution_clock::now().time_since_epoch());
          request.set_timestamp(timestamp.count());
          controlServiceChannel_->sendPingRequest(request, std::move(promise));
        }
      }
    }
  }
}
