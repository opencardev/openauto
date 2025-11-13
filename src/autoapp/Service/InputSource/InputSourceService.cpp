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
              OPENAUTO_LOG(info) << "[InputSourceService] start()";
              channel_->receive(this->shared_from_this());
            });
          }

          void InputSourceService::stop() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[InputSourceService] stop()";
              inputDevice_->stop();
            });
          }

          void InputSourceService::pause() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[InputSourceService] pause()";
            });
          }

          void InputSourceService::resume() {
            strand_.dispatch([this, self = this->shared_from_this()]() {
              OPENAUTO_LOG(info) << "[InputSourceService] resume()";
            });
          }

          void InputSourceService::fillFeatures(
              aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
            OPENAUTO_LOG(info) << "[InputSourceService] fillFeatures()";

            auto *service = response.add_channels();
            service->set_id(static_cast<uint32_t>(channel_->getId()));

            auto *inputChannel = service->mutable_input_source_service();

            const auto &supportedButtonCodes = inputDevice_->getSupportedButtonCodes();

            for (const auto &buttonCode: supportedButtonCodes) {
              inputChannel->add_keycodes_supported(buttonCode);
            }

            if (inputDevice_->hasTouchscreen()) {
              const auto &touchscreenSurface = inputDevice_->getTouchscreenGeometry();
              auto touchscreenConfig = inputChannel->add_touchscreen();

              touchscreenConfig->set_width(touchscreenSurface.width());
              touchscreenConfig->set_height(touchscreenSurface.height());
            }
          }

          void InputSourceService::onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
            OPENAUTO_LOG(info) << "[InputSourceService] onChannelOpenRequest()";
            OPENAUTO_LOG(debug) << "[InputSourceService] Channel Id: " << request.service_id() << ", Priority: " << request.priority();


            aap_protobuf::service::control::message::ChannelOpenResponse response;
            const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
            response.set_status(status);

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then([]() {},
                          std::bind(&InputSourceService::onChannelError, this->shared_from_this(), std::placeholders::_1));
            channel_->sendChannelOpenResponse(response, std::move(promise));
            channel_->receive(this->shared_from_this());
          }

          void InputSourceService::onKeyBindingRequest(const aap_protobuf::service::media::sink::message::KeyBindingRequest &request) {
            OPENAUTO_LOG(debug) << "[InputSourceService] onKeyBindingRequest()";
            OPENAUTO_LOG(debug) << "[InputSourceService] KeyCodes Count: " << request.keycodes_size();

            aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
            const auto &supportedButtonCodes = inputDevice_->getSupportedButtonCodes();

            for (int i = 0; i < request.keycodes_size(); ++i) {
              if (std::find(supportedButtonCodes.begin(), supportedButtonCodes.end(), request.keycodes(i)) ==
                  supportedButtonCodes.end()) {
                OPENAUTO_LOG(error) << "[InputSourceService] onKeyBindingRequest is not supported for KeyCode: " << request.keycodes(i);
                status = aap_protobuf::shared::MessageStatus::STATUS_KEYCODE_NOT_BOUND;
                break;
              }
            }

            aap_protobuf::service::media::sink::message::KeyBindingResponse response;
            response.set_status(status);

            if (status == aap_protobuf::shared::MessageStatus::STATUS_SUCCESS) {
              inputDevice_->start(*this);
            }

            OPENAUTO_LOG(debug) << "[InputSourceService] Sending KeyBindingResponse with Status: " << status;

            auto promise = aasdk::channel::SendPromise::defer(strand_);
            promise->then([]() {},
                          std::bind(&InputSourceService::onChannelError, this->shared_from_this(), std::placeholders::_1));
            channel_->sendKeyBindingResponse(response, std::move(promise));
            channel_->receive(this->shared_from_this());
          }

          void InputSourceService::onChannelError(const aasdk::error::Error &e) {
            // OPERATION_ABORTED is expected during shutdown when messenger stops
            if (e.getCode() == aasdk::error::ErrorCode::OPERATION_ABORTED) {
              OPENAUTO_LOG(debug) << "[InputSourceService] onChannelError(): " << e.what() << " (expected during stop)";
            } else {
              OPENAUTO_LOG(error) << "[InputSourceService] onChannelError(): " << e.what();
            }
          }

          void InputSourceService::onButtonEvent(const projection::ButtonEvent &event) {
            OPENAUTO_LOG(error) << "[InputSourceService] onButtonEvent()";
            auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch());

            strand_.dispatch(
                [this, self = this->shared_from_this(), event = std::move(event), timestamp = std::move(timestamp)]() {
                  aap_protobuf::service::inputsource::message::InputReport inputReport;
                  inputReport.set_timestamp(timestamp.count());

                  if (event.code == aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_ROTARY_CONTROLLER) {
                    auto relativeEvent = inputReport.mutable_relative_event()->add_data();
                    relativeEvent->set_delta(event.wheelDirection == projection::WheelDirection::LEFT ? -1 : 1);
                    relativeEvent->set_keycode(event.code);
                  } else {
                    auto buttonEvent = inputReport.mutable_key_event()->add_keys();
                    buttonEvent->set_metastate(0);
                    buttonEvent->set_down(event.type == projection::ButtonEventType::PRESS);
                    buttonEvent->set_longpress(false);
                    buttonEvent->set_keycode(event.code);
                  }

                  auto promise = aasdk::channel::SendPromise::defer(strand_);
                  promise->then([]() {}, std::bind(&InputSourceService::onChannelError, this->shared_from_this(),
                                                   std::placeholders::_1));
                  channel_->sendInputReport(inputReport, std::move(promise));
                });
          }

          void InputSourceService::onTouchEvent(const projection::TouchEvent &event) {
            OPENAUTO_LOG(debug) << "[InputSourceService] onTouchEvent: action=" << event.type 
                                << " pointerCount=" << event.pointers.size() 
                                << " actionIndex=" << event.actionIndex;
            
            auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch());

            strand_.dispatch(
                [this, self = this->shared_from_this(), event = std::move(event), timestamp = std::move(timestamp)]() {
                  aap_protobuf::service::inputsource::message::InputReport inputReport;
                  inputReport.set_timestamp(timestamp.count());

                  auto touchEvent = inputReport.mutable_touch_event();
                  touchEvent->set_action(event.type);
                  touchEvent->set_action_index(event.actionIndex);
                  
                  // Add all touch points
                  for(const auto& pointer : event.pointers)
                  {
                      auto touchLocation = touchEvent->add_pointer_data();
                      touchLocation->set_x(pointer.x);
                      touchLocation->set_y(pointer.y);
                      touchLocation->set_pointer_id(pointer.pointerId);
                  }

                  auto promise = aasdk::channel::SendPromise::defer(strand_);
                  promise->then([]() {}, std::bind(&InputSourceService::onChannelError, this->shared_from_this(),
                                                   std::placeholders::_1));
                  channel_->sendInputReport(inputReport, std::move(promise));
                });
          }
        }
      }
    }
  }
}