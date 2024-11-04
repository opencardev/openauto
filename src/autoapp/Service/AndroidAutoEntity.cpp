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

#include <aap_protobuf/connection/WirelessTcpConfiguration.pb.h>
#include <aap_protobuf/connection/PingConfiguration.pb.h>
#include <aap_protobuf/connection/ConnectionConfiguration.pb.h>
#include <aap_protobuf/channel/control/focus/audio/event/AudioFocusRequestType.pb.h>
#include <aap_protobuf/channel/control/focus/audio/notification/AudioFocusStateType.pb.h>
#include <aap_protobuf/channel/control/focus/navigation/shared/NavFocusType.pb.h>
#include <aasdk/Channel/Control/ControlServiceChannel.hpp>
#include <f1x/openauto/autoapp/Service/AndroidAutoEntity.hpp>
#include <f1x/openauto/Common/Log.hpp>

/*
 * HU > MD Version Request
 * HU < MD ServiceDiscoveryRequest
 * HU > MD Car MetaData (Make, Model, year etc)
 * HU < MD when Video Projection starts, it MUST be shown without User Ineraction
 * HU < MD Prompt Use to Enable and Pair with Car
 * HU < MD Request Video Focus for Projection (HU Grant)
 *
 * AAP neds Bluetooth HFP for Telephone
 *
 * HU > MD Bluetooth Announcement (HU MAC Address, Supported Pairing Methods)
 * HU < MD Bluetooth Pairing Request
 * HU > MD Bluetoth Pairing Response
 *
 * AfterPairing, HU can request the Bluetooth PhoneBookAccessProtocol. Sensible UI.
 *
 * HU < MD connect to Bluetooth HFP
 * HU Suppress BAP or MAP while AAP connected.
 * A2DP should be treated by OEM as another such such as a USB stick or radio. If the user plays music via AA, HU should grant request from AA to change focus to AA. HU manages connectivity.
 * MD connects to HU and routes call over Bluetooth (non Bluetooth call)
 * MD connects Blueooth call and display projection mode
 * MD on call to HFP device - MD continues call, disconnects from other HFP and connects to HFP on Vehicle.
 * AA only uses HFP, hhowever HU may use MAP, PBAP, PAN and RSAP
 * MD will reconnect when required.
 *
 * Video
 * HU < MD - During Service Discovery, MD requests Video Configs supported
 * HU > MD sends Config Message with Prioritised indices for Video Conffigurations
 * HU < MD MD selects config
 * HU < MD sends start message
 * HU < MD sends focus request
 * HU > MD sends focus granted (unless unsafe - ie reverse camera etc)
 * HU < MD Audio Focus Requests when MD wants to play.
 * HU > MD Audio Focus Navigations (can be  unsolicited or responses to requests)
 * HU < MD VoiceSessionRequestNotification with VOICE_SESSION_START, HU should stop all sounds. MD will request GAIN or GAIN_TRANS to play beeps/tones and ASR response.
 * Nav Focus for onboard navigation.
 * UI System Sounds does not require audio focus as sounds should be played ASAP. System Stream is optionals (not required to support).
 * HU should wait to receive two frames of audio before starting playback to minimise buffer underruns.
 * AA Latency types supported - Audio Setup - max 500ms, Audio Output max 50ms.
 * HU > MD Navigation Focus Notification specified with NF is Phone or Car.
 * HU < MD Navigation Focus Request
 * "For vehicles that support next turn information in the instrument cluster, the HU can subscribe to next turn updates from the MD navigation engine." (NExt Turn etc)
 * same for Media Playback Status
 * Radio - Allows Control of Radio from Within AA.
 * Vehicle IDs SHOULD have at least 64 bits
 */


namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace service
{

AndroidAutoEntity::AndroidAutoEntity(boost::asio::io_service& ioService,
                                     aasdk::messenger::ICryptor::Pointer cryptor,
                                     aasdk::transport::ITransport::Pointer transport,
                                     aasdk::messenger::IMessenger::Pointer messenger,
                                     configuration::IConfiguration::Pointer configuration,
                                     ServiceList serviceList,
                                     IPinger::Pointer pinger)
    : strand_(ioService)
    , cryptor_(std::move(cryptor))
    , transport_(std::move(transport))
    , messenger_(std::move(messenger))
    , controlServiceChannel_(std::make_shared<aasdk::channel::control::ControlServiceChannel>(strand_, messenger_))
    , configuration_(std::move(configuration))
    , serviceList_(std::move(serviceList))
    , pinger_(std::move(pinger))
    , eventHandler_(nullptr)
{
}

AndroidAutoEntity::~AndroidAutoEntity()
{
    OPENAUTO_LOG(debug) << "[AndroidAutoEntity] destroy.";
}

void AndroidAutoEntity::start(IAndroidAutoEntityEventHandler& eventHandler)
{
    strand_.dispatch([this, self = this->shared_from_this(), eventHandler = &eventHandler]() {
        OPENAUTO_LOG(info) << "[AndroidAutoEntity] start.";

        eventHandler_ = eventHandler;
        std::for_each(serviceList_.begin(), serviceList_.end(), std::bind(&IService::start, std::placeholders::_1));
        //this->schedulePing();

        auto versionRequestPromise = aasdk::channel::SendPromise::defer(strand_);
        versionRequestPromise->then([]() {}, std::bind(&AndroidAutoEntity::onChannelError, this->shared_from_this(), std::placeholders::_1));
        controlServiceChannel_->sendVersionRequest(std::move(versionRequestPromise));
        controlServiceChannel_->receive(this->shared_from_this());
    });
}

void AndroidAutoEntity::stop()
{
    strand_.dispatch([this, self = this->shared_from_this()]() {
        OPENAUTO_LOG(info) << "[AndroidAutoEntity] stop.";

        try {
            eventHandler_ = nullptr;
            std::for_each(serviceList_.begin(), serviceList_.end(), std::bind(&IService::stop, std::placeholders::_1));
            //pinger_->cancel();
            messenger_->stop();
            transport_->stop();
            cryptor_->deinit();
        } catch (...) {
            OPENAUTO_LOG(info) << "[AndroidAutoEntity] exception in stop.";
        }
    });
}

void AndroidAutoEntity::pause()
{
    strand_.dispatch([this, self = this->shared_from_this()]() {
        OPENAUTO_LOG(info) << "[AndroidAutoEntity] pause.";

        try {
            std::for_each(serviceList_.begin(), serviceList_.end(), std::bind(&IService::pause, std::placeholders::_1));
        } catch (...) {
            OPENAUTO_LOG(info) << "[AndroidAutoEntity] exception in pause.";
        }
    });
}

void AndroidAutoEntity::resume()
{
    strand_.dispatch([this, self = this->shared_from_this()]() {
        OPENAUTO_LOG(info) << "[AndroidAutoEntity] resume.";

        try {
            std::for_each(serviceList_.begin(), serviceList_.end(), std::bind(&IService::resume, std::placeholders::_1));
        } catch (...) {
            OPENAUTO_LOG(info) << "[AndroidAutoEntity] exception in resume.";
        }
    });
}

void AndroidAutoEntity::onVersionResponse(uint16_t majorCode, uint16_t minorCode, aap_protobuf::shared::MessageStatus status)
{
    OPENAUTO_LOG(info) << "[AndroidAutoEntity] version response, version: " << majorCode
                       << "." << minorCode
                       << ", status: " << status;

    if(status == aap_protobuf::shared::MessageStatus::STATUS_NO_COMPATIBLE_VERSION)
    {
        OPENAUTO_LOG(error) << "[AndroidAutoEntity] version mismatch.";
        this->triggerQuit();
    }
    else
    {
        OPENAUTO_LOG(info) << "[AndroidAutoEntity] Begin handshake.";

        try
        {
            cryptor_->doHandshake();

            auto handshakePromise = aasdk::channel::SendPromise::defer(strand_);
            handshakePromise->then([]() {}, std::bind(&AndroidAutoEntity::onChannelError, this->shared_from_this(), std::placeholders::_1));
            controlServiceChannel_->sendHandshake(cryptor_->readHandshakeBuffer(), std::move(handshakePromise));
            controlServiceChannel_->receive(this->shared_from_this());
        }
        catch(const aasdk::error::Error& e)
        {
            this->onChannelError(e);
        }
    }
}

void AndroidAutoEntity::onHandshake(const aasdk::common::DataConstBuffer& payload)
{
    OPENAUTO_LOG(info) << "[AndroidAutoEntity] Handshake, size: " << payload.size;

    try
    {
        cryptor_->writeHandshakeBuffer(payload);

        if(!cryptor_->doHandshake())
        {
            OPENAUTO_LOG(info) << "[AndroidAutoEntity] continue handshake.";

            auto handshakePromise = aasdk::channel::SendPromise::defer(strand_);
            handshakePromise->then([]() {}, std::bind(&AndroidAutoEntity::onChannelError, this->shared_from_this(), std::placeholders::_1));
            controlServiceChannel_->sendHandshake(cryptor_->readHandshakeBuffer(), std::move(handshakePromise));
        }
        else
        {
            OPENAUTO_LOG(info) << "[AndroidAutoEntity] Auth completed.";

            aap_protobuf::channel::control::auth::AuthResponse authCompleteIndication;
            authCompleteIndication.set_status(aap_protobuf::shared::MessageStatus::STATUS_SUCCESS);

            auto authCompletePromise = aasdk::channel::SendPromise::defer(strand_);
            authCompletePromise->then([]() {}, std::bind(&AndroidAutoEntity::onChannelError, this->shared_from_this(), std::placeholders::_1));
            controlServiceChannel_->sendAuthComplete(authCompleteIndication, std::move(authCompletePromise));
        }

        controlServiceChannel_->receive(this->shared_from_this());
    }
    catch(const aasdk::error::Error& e)
    {
        this->onChannelError(e);
    }
}

void AndroidAutoEntity::onServiceDiscoveryRequest(const aap_protobuf::channel::control::servicediscovery::event::ServiceDiscoveryRequest& request)
{
    OPENAUTO_LOG(info) << "[AndroidAutoEntity] Discovery request, device name: " << request.device_name()
                       << ", brand: " << request.label_text();

    /*
    aap_protobuf::connection::PingConfiguration pingConfiguration;
    pingConfiguration.set_high_latency_threshold_ms();
    pingConfiguration.set_interval_ms();
    pingConfiguration.set_timeout_ms();
    pingConfiguration.set_tracked_ping_count();

    aap_protobuf::connection::WirelessTcpConfiguration wirelessTcpConfiguration;
    wirelessTcpConfiguration.set_socket_read_timeout_ms();
    wirelessTcpConfiguration.set_socket_receive_buffer_size_kb();
    wirelessTcpConfiguration.set_socket_send_buffer_size_kb();

    aap_protobuf::connection::ConnectionConfiguration connectionConfiguration;
    connectionConfiguration.set_allocated_ping_configuration();
    connectionConfiguration.set_allocated_wireless_tcp_configuration();*/

    aap_protobuf::channel::control::servicediscovery::notification::HeadUnitInfo headUnitInfo;
    headUnitInfo.set_make("CubeOne");
    headUnitInfo.set_model("Journey");
    headUnitInfo.set_year("2024");
    headUnitInfo.set_vehicle_id("2009");
    headUnitInfo.set_head_unit_make("CubeOne");
    headUnitInfo.set_head_unit_model("Journey");
    headUnitInfo.set_head_unit_software_build("2024.10.15");
    headUnitInfo.set_head_unit_software_version("1");

    aap_protobuf::channel::control::servicediscovery::notification::ServiceDiscoveryResponse serviceDiscoveryResponse;
    serviceDiscoveryResponse.mutable_channels()->Reserve(256);

    //serviceDiscoveryResponse.set_headunit_sw_build("2024.10.15");
    //serviceDiscoveryResponse.set_headunit_sw_version("1");
    serviceDiscoveryResponse.set_display_name("JourneyOS");

    serviceDiscoveryResponse.set_allocated_headunit_info(&headUnitInfo);

    //serviceDiscoveryResponse.set_can_play_native_media_during_vr(false);

    std::for_each(serviceList_.begin(), serviceList_.end(), std::bind(&IService::fillFeatures, std::placeholders::_1, std::ref(serviceDiscoveryResponse)));

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&AndroidAutoEntity::onChannelError, this->shared_from_this(), std::placeholders::_1));
    controlServiceChannel_->sendServiceDiscoveryResponse(serviceDiscoveryResponse, std::move(promise));
    controlServiceChannel_->receive(this->shared_from_this());
}

void AndroidAutoEntity::onAudioFocusRequest(const aap_protobuf::channel::control::focus::audio::event::AudioFocusRequest& request)
{
    OPENAUTO_LOG(info) << "[AndroidAutoEntity] requested audio focus, type: " << request.audio_focus_type();

    aap_protobuf::channel::control::focus::audio::notification::AudioFocusStateType audioFocusState =
            request.audio_focus_type() == aap_protobuf::channel::control::focus::audio::event::AudioFocusRequestType::AUDIO_FOCUS_RELEASE
            ? aap_protobuf::channel::control::focus::audio::notification::AudioFocusStateType::AUDIO_FOCUS_STATE_LOSS
            : aap_protobuf::channel::control::focus::audio::notification::AudioFocusStateType::AUDIO_FOCUS_STATE_GAIN;

    OPENAUTO_LOG(info) << "[AndroidAutoEntity] audio focus state: " << audioFocusState;

    aap_protobuf::channel::control::focus::audio::notification::AudioFocusNotification response;
    response.set_audio_focus_state(audioFocusState);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&AndroidAutoEntity::onChannelError, this->shared_from_this(), std::placeholders::_1));
    controlServiceChannel_->sendAudioFocusResponse(response, std::move(promise));
    controlServiceChannel_->receive(this->shared_from_this());
}

void AndroidAutoEntity::onByeByeRequest(const aap_protobuf::channel::control::byebye::event::ByeByeRequest& request)
{
    OPENAUTO_LOG(info) << "[AndroidAutoEntity] Shutdown request, reason: " << request.reason();

    aap_protobuf::channel::control::byebye::notification::ByeByeResponse response;
    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then(std::bind(&AndroidAutoEntity::triggerQuit, this->shared_from_this()),
                  std::bind(&AndroidAutoEntity::onChannelError, this->shared_from_this(), std::placeholders::_1));

    controlServiceChannel_->sendShutdownResponse(response, std::move(promise));
}

void AndroidAutoEntity::onByeByeResponse(const aap_protobuf::channel::control::byebye::notification::ByeByeResponse& response)
{
    OPENAUTO_LOG(info) << "[AndroidAutoEntity] Shutdown response ";
    this->triggerQuit();
}

void AndroidAutoEntity::onNavigationFocusRequest(const aap_protobuf::channel::control::focus::navigation::event::NavFocusRequestNotification& request)
{
    OPENAUTO_LOG(info) << "[AndroidAutoEntity] navigation focus request, type: " << request.focus_type();

    aap_protobuf::channel::control::focus::navigation::notification::NavFocusNotification response;
    response.set_focus_type(aap_protobuf::channel::control::focus::navigation::shared::NavFocusType::NAV_FOCUS_PROJECTED);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&AndroidAutoEntity::onChannelError, this->shared_from_this(), std::placeholders::_1));
    controlServiceChannel_->sendNavigationFocusResponse(response, std::move(promise));
    controlServiceChannel_->receive(this->shared_from_this());
}

  void onVoiceSessionRequest(const aap_protobuf::channel::control::voice::VoiceSessionNotification &request) {
    // TODO: FIXME
  }
  void AndroidAutoEntity::onPingRequest(const aap_protobuf::channel::control::ping::PingRequest& request)
  {
    OPENAUTO_LOG(info) << "[AndroidAutoEntity] Ping request, timestamp: "  << request.timestamp();
    //pinger_->ping();
    //controlServiceChannel_->receive(this->shared_from_this());
  }

void AndroidAutoEntity::onPingResponse(const aap_protobuf::channel::control::ping::PingResponse& response)
{
    OPENAUTO_LOG(info) << "[AndroidAutoEntity] Ping response, timestamp: "  << response.timestamp();
    pinger_->pong();
    controlServiceChannel_->receive(this->shared_from_this());
}

void AndroidAutoEntity::onChannelError(const aasdk::error::Error& e)
{
    OPENAUTO_LOG(error) << "[AndroidAutoEntity] channel error: " << e.what();
    this->triggerQuit();
}

void AndroidAutoEntity::triggerQuit()
{
    if(eventHandler_ != nullptr)
    {
        eventHandler_->onAndroidAutoQuit();
    }
}

void AndroidAutoEntity::schedulePing()
{
    auto promise = IPinger::Promise::defer(strand_);
    promise->then([this, self = this->shared_from_this()]() {
        this->sendPing();
        this->schedulePing();
    },
    [this, self = this->shared_from_this()](auto error) {
        if(error != aasdk::error::ErrorCode::OPERATION_ABORTED &&
           error != aasdk::error::ErrorCode::OPERATION_IN_PROGRESS)
        {
            OPENAUTO_LOG(error) << "[AndroidAutoEntity] ping timer exceeded.";
            this->triggerQuit();
        }
    });

    pinger_->ping(std::move(promise));
}

void AndroidAutoEntity::sendPing()
{
    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&AndroidAutoEntity::onChannelError, this->shared_from_this(), std::placeholders::_1));

    aap_protobuf::channel::control::ping::PingRequest request;
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
    request.set_timestamp(timestamp.count());
    controlServiceChannel_->sendPingRequest(request, std::move(promise));
}

}
}
}
}
