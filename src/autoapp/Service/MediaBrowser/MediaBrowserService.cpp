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
#include <f1x/openauto/autoapp/Service/MediaBrowser/MediaBrowserService.hpp>
#include <fstream>
#include <modern/Logger.hpp>

namespace f1x {
namespace openauto {
namespace autoapp {
namespace service {
namespace mediabrowser {

MediaBrowserService::MediaBrowserService(boost::asio::io_service &ioService,
                                         aasdk::messenger::IMessenger::Pointer messenger)
    : strand_(ioService),
      timer_(ioService),
      channel_(std::make_shared<aasdk::channel::mediabrowser::MediaBrowserService>(
          strand_, std::move(messenger))) {}

void MediaBrowserService::start() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
        LOG_INFO(ANDROID_AUTO, "[MediaBrowserService] start()");
    });
}

void MediaBrowserService::stop() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
        LOG_INFO(ANDROID_AUTO, "[MediaBrowserService] stop()");
    });
}

void MediaBrowserService::pause() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
        LOG_INFO(ANDROID_AUTO, "[MediaBrowserService] pause()");
    });
}

void MediaBrowserService::resume() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
        LOG_INFO(ANDROID_AUTO, "[MediaBrowserService] resume()");
    });
}

void MediaBrowserService::fillFeatures(
    aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    LOG_INFO(ANDROID_AUTO, "[MediaBrowserService] fillFeatures()");

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *mediaBrowser = service->mutable_media_browser_service();
    (void)mediaBrowser;  // Suppress unused variable warning
}

void MediaBrowserService::onChannelOpenRequest(
    const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    LOG_INFO(ANDROID_AUTO, "[MediaBrowserService] onChannelOpenRequest()");
    LOG_INFO(ANDROID_AUTO,
             ("[MediaBrowserService] Channel Id: " + std::to_string(request.service_id()) +
              ", Priority: " + std::to_string(request.priority()))
                 .c_str());

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status =
        aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&MediaBrowserService::onChannelError, this->shared_from_this(),
                                     std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
}

void MediaBrowserService::onChannelError(const aasdk::error::Error &e) {
    LOG_ERROR(ANDROID_AUTO,
              ("[MediaBrowserService] onChannelError(): " + std::string(e.what())).c_str());
}
}  // namespace mediabrowser
}  // namespace service
}  // namespace autoapp
}  // namespace openauto
}  // namespace f1x