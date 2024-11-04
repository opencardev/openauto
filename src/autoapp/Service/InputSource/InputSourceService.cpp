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

#include <aap_protobuf/service/input/message/InputEventIndication.pb.h>
#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Service/InputSource/InputSourceService.hpp>

namespace f1x {
  namespace openauto {
    namespace autoapp {
      namespace service {
        namespace inputsource {
          InputSourceService::InputSourceService(boost::asio::io_service &ioService,
                                                 aasdk::messenger::IMessenger::Pointer messenger,
                                                 projection::IInputDevice::Pointer inputDevice)
              : strand_(ioService),
                channel_(std::make_shared<aasdk::channel::inputsource::InputSourceService>(strand_, std::move(messenger))),
                inputDevice_(std::move(inputDevice)) {

          }

          void InputSourceService::start() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[InputService] start.";
              channel_->receive(this->shared_from_this());
            });
          }

          void InputSourceService::stop() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[InputService] stop.";
              inputDevice_->stop();
            });
          }

          void InputSourceService::pause() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[InputService] pause.";
            });
          }

          void InputSourceService::resume() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[InputService] resume.";
            });
          }

          void InputSourceService::fillFeatures(
              aap_protobuf::channel::control::servicediscovery::notification::ServiceDiscoveryResponse &response) {
            OPENAUTO_LOG(info) << "[InputService] fill features.";

            auto *channelDescriptor = response.add_channels();
            channelDescriptor->set_channel_id(static_cast<uint32_t>(channel_->getId()));

            auto *inputChannel = channelDescriptor->mutable_input_service();

            const auto &supportedButtonCodes = inputDevice_->getSupportedButtonCodes();

            for (const auto &buttonCode: supportedButtonCodes) {
              inputChannel->add_supported_keycodes(buttonCode);
            }

            if (inputDevice_->hasTouchscreen()) {
              const auto &touchscreenSurface = inputDevice_->getTouchscreenGeometry();
              auto touchscreenConfig = inputChannel->add_touch_screen_config();

              touchscreenConfig->set_width(touchscreenSurface.width());
              touchscreenConfig->set_height(touchscreenSurface.height());
            }
          }

          void InputSourceService::onChannelOpenRequest(const aap_protobuf::channel::ChannelOpenRequest &request) {
            OPENAUTO_LOG(info) << "[InputService] open request, priority: " << request.priority();
            const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
            OPENAUTO_LOG(info) << "[InputService] open status: " << status;

            aap_protobuf::channel::ChannelOpenResponse response;
            response.set_status(status);

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then([]() {},
                          std::bind(&InputSourceService::onChannelError, this->shared_from_this(), std::placeholders::_1));
            channel_->sendChannelOpenResponse(response, std::move(promise));

            channel_->receive(this->shared_from_this());
          }
          void InputSourceService::onBindingRequest(const aap_protobuf::channel::input::event::BindingRequest &request) {
            OPENAUTO_LOG(info) << "[InputService] binding request, scan codes count: " << request.keycodes_size();

            aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
            const auto &supportedButtonCodes = inputDevice_->getSupportedButtonCodes();

            for (int i = 0; i < request.keycodes_size(); ++i) {
              if (std::find(supportedButtonCodes.begin(), supportedButtonCodes.end(), request.keycodes(i)) ==
                  supportedButtonCodes.end()) {
                OPENAUTO_LOG(error) << "[InputService] binding request, scan code: " << request.keycodes(i)
                                    << " is not supported.";

                status = aap_protobuf::shared::MessageStatus::STATUS_UNSOLICITED_MESSAGE;
                break;
              }
            }

            aap_protobuf::service::media::sink::message::BindingResponse response;
            response.set_status(status);

            if (status == aap_protobuf::shared::MessageStatus::STATUS_SUCCESS) {
              inputDevice_->start(*this);
            }

            OPENAUTO_LOG(info) << "[InputService] binding request, status: " << status;

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then([]() {},
                          std::bind(&InputSourceService::onChannelError, this->shared_from_this(), std::placeholders::_1));
            channel_->sendBindingResponse(response, std::move(promise));
            channel_->receive(this->shared_from_this());
          }

          void InputSourceService::onChannelError(const aasdk::error::Error &e) {
            OPENAUTO_LOG(error) << "[InputSourceService] channel error: " << e.what();
          }

          void InputSourceService::onButtonEvent(const projection::ButtonEvent &event) {
            auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch());

            strand_.dispatch(
                [this, self = this->shared_from_this(), event = std::move(event), timestamp = std::move(timestamp)]() {
                  aap_protobuf::service::input::message::InputEventIndication inputEventIndication;
                  inputEventIndication.set_timestamp(timestamp.count());

                  if (event.code == aap_protobuf::service::media::sink::KeyCode::KEYCODE_ROTARY_CONTROLLER) {
                    auto relativeEvent = inputEventIndication.mutable_relative_input_event()->add_relative_input_events();
                    relativeEvent->set_delta(event.wheelDirection == projection::WheelDirection::LEFT ? -1 : 1);
                    relativeEvent->set_scan_code(event.code);
                  } else {
                    auto buttonEvent = inputEventIndication.mutable_button_event()->add_keys();
                    buttonEvent->set_metastate(0);
                    buttonEvent->set_down(event.type == projection::ButtonEventType::PRESS);
                    buttonEvent->set_longpress(false);
                    buttonEvent->set_keycode(event.code);
                  }

                  auto promise = aasdk::channel::SendPromise::defer(strand_);
                  promise->then([]() {}, std::bind(&InputSourceService::onChannelError, this->shared_from_this(),
                                                   std::placeholders::_1));
                  channel_->sendInputEventIndication(inputEventIndication, std::move(promise));
                });
          }

          void InputSourceService::onTouchEvent(const projection::TouchEvent &event) {
            auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch());

            strand_.dispatch(
                [this, self = this->shared_from_this(), event = std::move(event), timestamp = std::move(timestamp)]() {
                  aap_protobuf::service::input::message::InputEventIndication inputEventIndication;
                  inputEventIndication.set_timestamp(timestamp.count());

                  auto touchEvent = inputEventIndication.mutable_touch_event();
                  touchEvent->set_touch_action(event.type);
                  auto touchLocation = touchEvent->add_touch_location();
                  touchLocation->set_x(event.x);
                  touchLocation->set_y(event.y);
                  touchLocation->set_pointer_id(0);

                  auto promise = aasdk::channel::SendPromise::defer(strand_);
                  promise->then([]() {}, std::bind(&InputSourceService::onChannelError, this->shared_from_this(),
                                                   std::placeholders::_1));
                  channel_->sendInputEventIndication(inputEventIndication, std::move(promise));
                });
          }



        }
      }
    }
  }
}