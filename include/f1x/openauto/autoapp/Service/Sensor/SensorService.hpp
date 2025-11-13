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

#pragma once

#include <gps.h>
#include <atomic>
#include <aap_protobuf/service/sensorsource/message/DrivingStatus.pb.h>
#include <aap_protobuf/service/sensorsource/message/SensorType.pb.h>
#include <aasdk/Channel/SensorSource/SensorSourceService.hpp>
#include <f1x/openauto/autoapp/Service/IService.hpp>
#include <boost/asio/io_service.hpp>
#include <aasdk/Messenger/IMessenger.hpp>


namespace f1x::openauto::autoapp::service::sensor {
  class SensorService :
      public aasdk::channel::sensorsource::ISensorSourceServiceEventHandler,
      public IService,
      public std::enable_shared_from_this<SensorService> {
  public:
    SensorService(boost::asio::io_service &ioService,
                  aasdk::messenger::IMessenger::Pointer messenger);

    bool isNight = false;
    bool previous = false;
    std::atomic<bool> stopPolling{false};

    void start() override;

    void stop() override;

    void pause() override;

    void resume() override;

    void fillFeatures(aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) override;

    void onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) override;

    void onSensorStartRequest(
        const aap_protobuf::service::sensorsource::message::SensorRequest &request) override;

    void onChannelError(const aasdk::error::Error &e) override;

  private:
    using std::enable_shared_from_this<SensorService>::shared_from_this;

    void sendDrivingStatusUnrestricted();

    void sendNightData();

    void sendGPSLocationData();

    bool is_file_exist(const char *filename);

    void sensorPolling();

    bool firstRun = true;

    boost::asio::io_service::strand strand_;
    boost::asio::deadline_timer timer_;
    aasdk::channel::sensorsource::SensorSourceService::Pointer channel_;
    struct gps_data_t gpsData_;
    bool gpsEnabled_ = false;
  };

}



