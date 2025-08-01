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

#include <QString>
#include <f1x/openauto/autoapp/Service/VendorExtension/VendorExtensionService.hpp>
#include <fstream>
#include <modern/Logger.hpp>

namespace f1x::openauto::autoapp::service::vendorextension {

VendorExtensionService::VendorExtensionService(boost::asio::io_service &ioService,
                                               aasdk::messenger::IMessenger::Pointer messenger)
    : strand_(ioService),
      timer_(ioService),
      channel_(std::make_shared<aasdk::channel::vendorextension::VendorExtensionService>(
          strand_, std::move(messenger))) {}

void VendorExtensionService::start() {
    strand_.dispatch([this, self = this->shared_from_this()]() { LOG_INFO(GENERAL, "start()"); });
}

void VendorExtensionService::stop() {
    strand_.dispatch([this, self = this->shared_from_this()]() { LOG_INFO(GENERAL, "stop()"); });
}

void VendorExtensionService::pause() {
    strand_.dispatch([this, self = this->shared_from_this()]() { LOG_INFO(GENERAL, "pause()"); });
}

void VendorExtensionService::resume() {
    strand_.dispatch([this, self = this->shared_from_this()]() { LOG_INFO(GENERAL, "resume()"); });
}

void VendorExtensionService::fillFeatures(
    aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    LOG_INFO(GENERAL, "fillFeatures();");

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *vendorExtension = service->mutable_vendor_extension_service();
    (void)vendorExtension;  // Suppress unused variable warning
}

void VendorExtensionService::onChannelError(const aasdk::error::Error &e) {
    LOG_ERROR_STREAM(GENERAL, "onChannelError(): " + std::string(e.what()));
}

void VendorExtensionService::onChannelOpenRequest(
    const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    LOG_INFO_STREAM(GENERAL, "onChannelOpenRequest()");
    LOG_INFO_STREAM(GENERAL, "Channel Id: " + std::to_string(request.service_id()) +
                                 ", Priority: " + std::to_string(request.priority()));

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status =
        aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&VendorExtensionService::onChannelError,
                                     this->shared_from_this(), std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
}
}  // namespace f1x::openauto::autoapp::service::vendorextension
