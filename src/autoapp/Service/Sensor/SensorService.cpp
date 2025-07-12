/*
*  This file is part of openauto project.
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
*
*   void SensorService::onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    LOG_INFO(ANDROID_AUTO, "[SensorService] onChannelOpenRequest()");
    LOG_DEBUG(ANDROID_AUTO, "[SensorService] Channel Id: " + std::to_string(request.service_id()) + ", Priority: " + std::to_string(request.priority()));

    aap_protobuf::service::control::message::ChannelOpenResponse response;uto is free software: you can redistribute it and/or modify
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


#include <modern/Logger.hpp>
#include <f1x/openauto/autoapp/Service/Sensor/SensorService.hpp>
#include <fstream>
#include <cmath>
#include <gps.h>

namespace f1x::openauto::autoapp::service::sensor {
  SensorService::SensorService(boost::asio::io_service &ioService,
                               aasdk::messenger::IMessenger::Pointer messenger)
      : strand_(ioService),
        timer_(ioService),
        channel_(std::make_shared<aasdk::channel::sensorsource::SensorSourceService>(strand_, std::move(messenger))) {

  }

  void SensorService::start() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      if (gps_open("127.0.0.1", "2947", &this->gpsData_)) {
        LOG_WARN(ANDROID_AUTO, "[SensorService] can't connect to GPSD.");
      } else {
        LOG_INFO(ANDROID_AUTO, "[SensorService] Connected to GPSD.");
        gps_stream(&this->gpsData_, WATCH_ENABLE | WATCH_JSON, NULL);
        this->gpsEnabled_ = true;
      }

      if (is_file_exist("/tmp/night_mode_enabled")) {
        this->isNight = true;
      }
      this->sensorPolling();

      LOG_INFO(ANDROID_AUTO, "[SensorService] start()");
      channel_->receive(this->shared_from_this());
    });

  }

  void SensorService::stop() {
    this->stopPolling = true;

    strand_.dispatch([this, self = this->shared_from_this()]() {
      if (this->gpsEnabled_) {
        gps_stream(&this->gpsData_, WATCH_DISABLE, NULL);
        gps_close(&this->gpsData_);
        this->gpsEnabled_ = false;
      }

      LOG_INFO(ANDROID_AUTO, "[SensorService] stop()");
    });
  }

  void SensorService::pause() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      LOG_INFO(ANDROID_AUTO, "[SensorService] pause()");
    });
  }

  void SensorService::resume() {
    strand_.dispatch([this, self = this->shared_from_this()]() {
      LOG_INFO(ANDROID_AUTO, "[SensorService] resume()");
    });
  }

  void SensorService::fillFeatures(
      aap_protobuf::service::control::message::ServiceDiscoveryResponse &response) {
    LOG_INFO(ANDROID_AUTO, "[SensorService] fillFeatures()");

    auto *service = response.add_channels();
    service->set_id(static_cast<uint32_t>(channel_->getId()));

    auto *sensorChannel = service->mutable_sensor_source_service();
    sensorChannel->add_sensors()->set_sensor_type(
        aap_protobuf::service::sensorsource::message::SensorType::SENSOR_DRIVING_STATUS_DATA);
    sensorChannel->add_sensors()->set_sensor_type(
        aap_protobuf::service::sensorsource::message::SensorType::SENSOR_LOCATION);
    sensorChannel->add_sensors()->set_sensor_type(
        aap_protobuf::service::sensorsource::message::SensorType::SENSOR_NIGHT_MODE);
  }

  void SensorService::onChannelOpenRequest(const aap_protobuf::service::control::message::ChannelOpenRequest &request) {
    LOG_INFO(ANDROID_AUTO, "[SensorService] onChannelOpenRequest()");
    LOG_DEBUG(ANDROID_AUTO, "[SensorService] Channel Id: " + std::to_string(request.service_id()) + ", Priority: " + std::to_string(request.priority()));

    aap_protobuf::service::control::message::ChannelOpenResponse response;
    const aap_protobuf::shared::MessageStatus status = aap_protobuf::shared::MessageStatus::STATUS_SUCCESS;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {},
                  std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
  }

  void SensorService::onSensorStartRequest(
      const aap_protobuf::service::sensorsource::message::SensorRequest &request) {
    LOG_INFO(ANDROID_AUTO, "[SensorService] onSensorStartRequest()");
    LOG_DEBUG(ANDROID_AUTO, "[SensorService] Request Type: " + std::to_string(request.type()));

    aap_protobuf::service::sensorsource::message::SensorStartResponseMessage response;
    response.set_status(aap_protobuf::shared::MessageStatus::STATUS_SUCCESS);

    auto promise = aasdk::channel::SendPromise::defer(strand_);

    if (request.type() == aap_protobuf::service::sensorsource::message::SENSOR_DRIVING_STATUS_DATA) {
      promise->then(std::bind(&SensorService::sendDrivingStatusUnrestricted, this->shared_from_this()),
                    std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    } else if (request.type() == aap_protobuf::service::sensorsource::message::SensorType::SENSOR_NIGHT_MODE) {
      promise->then(std::bind(&SensorService::sendNightData, this->shared_from_this()),
                    std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    } else {
      promise->then([]() {},
                    std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    }

    channel_->sendSensorStartResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
  }

  void SensorService::sendDrivingStatusUnrestricted() {
    LOG_INFO(ANDROID_AUTO, "[SensorService] sendDrivingStatusUnrestricted()");
    aap_protobuf::service::sensorsource::message::SensorBatch indication;
    indication.add_driving_status_data()->set_status(
        aap_protobuf::service::sensorsource::message::DrivingStatus::DRIVE_STATUS_UNRESTRICTED);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {},
                  std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendSensorEventIndication(indication, std::move(promise));
  }

  void SensorService::sendNightData() {
    LOG_INFO(ANDROID_AUTO, "[SensorService] sendNightData()");
    aap_protobuf::service::sensorsource::message::SensorBatch indication;

    if (SensorService::isNight) {
      LOG_INFO(ANDROID_AUTO, "[SensorService] Night Mode Triggered");
      indication.add_night_mode_data()->set_night_mode(true);
    } else {
      LOG_INFO(ANDROID_AUTO, "[SensorService] Day Mode Triggered");
      indication.add_night_mode_data()->set_night_mode(false);
    }

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {},
                  std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendSensorEventIndication(indication, std::move(promise));
    if (this->firstRun) {
      this->firstRun = false;
      this->previous = this->isNight;
    }
  }

  void SensorService::sendGPSLocationData() {
    LOG_INFO(ANDROID_AUTO, "[SensorService] sendGPSLocationData()");
    aap_protobuf::service::sensorsource::message::SensorBatch indication;

    auto *locInd = indication.add_location_data();

    // epoch seconds
    // Note: set_timestamp() is deprecated but still needed for compatibility
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#if GPSD_API_MAJOR_VERSION >= 7
    locInd->set_timestamp(this->gpsData_.fix.time.tv_sec);
#else
    locInd->set_timestamp(this->gpsData_.fix.time);
#endif
    #pragma GCC diagnostic pop
    // degrees
    locInd->set_latitude_e7(this->gpsData_.fix.latitude * 1e7);
    locInd->set_longitude_e7(this->gpsData_.fix.longitude * 1e7);
    // meters
    auto accuracy = sqrt(pow(this->gpsData_.fix.epx, 2) + pow(this->gpsData_.fix.epy, 2));
    locInd->set_accuracy_e3(accuracy * 1e3);

    if (this->gpsData_.set & ALTITUDE_SET) {
      // meters above ellipsoid
      locInd->set_altitude_e2(this->gpsData_.fix.altitude * 1e2);
    }
    if (this->gpsData_.set & SPEED_SET) {
      // meters per second to knots
      locInd->set_speed_e3(this->gpsData_.fix.speed * 1.94384 * 1e3);
    }
    if (this->gpsData_.set & TRACK_SET) {
      // degrees
      locInd->set_bearing_e6(this->gpsData_.fix.track * 1e6);
    }

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {},
                  std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendSensorEventIndication(indication, std::move(promise));
  }

  void SensorService::sensorPolling() {
    LOG_INFO(ANDROID_AUTO, "[SensorService] sensorPolling()");
    if (!this->stopPolling) {
      strand_.dispatch([this, self = this->shared_from_this()]() {
        this->isNight = is_file_exist("/tmp/night_mode_enabled");
        if (this->previous != this->isNight && !this->firstRun) {
          this->previous = this->isNight;
          this->sendNightData();
        }
        bool gpsDataAvailable = false;
#if GPSD_API_MAJOR_VERSION >= 7
        if (gps_read (&this->gpsData_, NULL, 0) != -1) {
          gpsDataAvailable = true;
        }
#else
        if (gps_read (&this->gpsData_) != -1) {
                    gpsDataAvailable = true;
                }
#endif
        if ((this->gpsEnabled_) &&
            (gps_waiting(&this->gpsData_, 0)) &&
            (gpsDataAvailable == true) &&
            (this->gpsData_.fix.mode == MODE_2D || this->gpsData_.fix.mode == MODE_3D) &&
            (this->gpsData_.set & TIME_SET) &&
            (this->gpsData_.set & LATLON_SET))
        {
          this->sendGPSLocationData();
        }

        timer_.expires_from_now(boost::posix_time::milliseconds(250));
        timer_.async_wait(strand_.wrap(std::bind(&SensorService::sensorPolling, this->shared_from_this())));
      });
    }
  }

  bool SensorService::is_file_exist(const char *fileName) {
    LOG_INFO(ANDROID_AUTO, "[SensorService] is_file_exist()");
    std::ifstream ifile(fileName, std::ios::in);
    return ifile.good();
  }

  void SensorService::onChannelError(const aasdk::error::Error &e) {
    LOG_ERROR(ANDROID_AUTO, "[SensorService] onChannelError(): " + std::string(e.what()));
  }
}



