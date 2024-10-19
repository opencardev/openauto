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

#include <aasdk_proto/DrivingStatusEnum.pb.h>
#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/autoapp/Service/SensorService.hpp>
#include <fstream>
#include <cmath>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace service
{

SensorService::SensorService(boost::asio::io_service& ioService, aasdk::messenger::IMessenger::Pointer messenger)
    : strand_(ioService),
      timer_(ioService),
      channel_(std::make_shared<aasdk::channel::sensor::SensorServiceChannel>(strand_, std::move(messenger)))
{

}

void SensorService::start()
{
    strand_.dispatch([this, self = this->shared_from_this()]() {
        if (gps_open("127.0.0.1", "2947", &this->gpsData_))
        {
            OPENAUTO_LOG(warning) << "[SensorService] can't connect to GPSD.";
        }
        else
        {
            OPENAUTO_LOG(info) << "[SensorService] Connected to GPSD.";
            gps_stream(&this->gpsData_, WATCH_ENABLE | WATCH_JSON, NULL);
            this->gpsEnabled_ = true;
        }

        if (is_file_exist("/tmp/night_mode_enabled")) {
            this->isNight = true;
        }
        this->sensorPolling();

        OPENAUTO_LOG(info) << "[SensorService] start.";
        channel_->receive(this->shared_from_this());
    });

}

void SensorService::stop()
{
    this->stopPolling = true;

    strand_.dispatch([this, self = this->shared_from_this()]() {
        if (this->gpsEnabled_)
        {
            gps_stream(&this->gpsData_, WATCH_DISABLE, NULL);
            gps_close(&this->gpsData_);
            this->gpsEnabled_ = false;
        }

        OPENAUTO_LOG(info) << "[SensorService] stop.";
    });
}

void SensorService::pause()
{
    strand_.dispatch([this, self = this->shared_from_this()]() {
        OPENAUTO_LOG(info) << "[SensorService] pause.";
    });
}

void SensorService::resume()
{
    strand_.dispatch([this, self = this->shared_from_this()]() {
        OPENAUTO_LOG(info) << "[SensorService] resume.";
    });
}

void SensorService::fillFeatures(aasdk::proto::messages::ServiceDiscoveryResponse& response)
{
    OPENAUTO_LOG(info) << "[SensorService] fill features.";

    auto* channelDescriptor = response.add_channels();
    channelDescriptor->set_channel_id(static_cast<uint32_t>(channel_->getId()));

    auto* sensorChannel = channelDescriptor->mutable_sensor_channel();
    sensorChannel->add_sensors()->set_type(aasdk::proto::enums::SensorType::DRIVING_STATUS);
    sensorChannel->add_sensors()->set_type(aasdk::proto::enums::SensorType::LOCATION);
    sensorChannel->add_sensors()->set_type(aasdk::proto::enums::SensorType::NIGHT_DATA);
}

void SensorService::onChannelOpenRequest(const aasdk::proto::messages::ChannelOpenRequest& request)
{
    OPENAUTO_LOG(info) << "[SensorService] open request, priority: " << request.priority();
    const aasdk::proto::enums::Status::Enum status = aasdk::proto::enums::Status::OK;
    OPENAUTO_LOG(info) << "[SensorService] open status: " << status;

    aasdk::proto::messages::ChannelOpenResponse response;
    response.set_status(status);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendChannelOpenResponse(response, std::move(promise));

    channel_->receive(this->shared_from_this());
}

void SensorService::onSensorStartRequest(const aasdk::proto::messages::SensorStartRequestMessage& request)
{
    OPENAUTO_LOG(info) << "[SensorService] sensor start request, type: " << request.sensor_type();

    aasdk::proto::messages::SensorStartResponseMessage response;
    response.set_status(aasdk::proto::enums::Status::OK);

    auto promise = aasdk::channel::SendPromise::defer(strand_);

    if(request.sensor_type() == aasdk::proto::enums::SensorType::DRIVING_STATUS)
    {
        promise->then(std::bind(&SensorService::sendDrivingStatusUnrestricted, this->shared_from_this()),
                      std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    }
    else if(request.sensor_type() == aasdk::proto::enums::SensorType::NIGHT_DATA)
    {
        promise->then(std::bind(&SensorService::sendNightData, this->shared_from_this()),
                      std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    }
    else
    {
        promise->then([]() {}, std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    }

    channel_->sendSensorStartResponse(response, std::move(promise));
    channel_->receive(this->shared_from_this());
}

void SensorService::sendDrivingStatusUnrestricted()
{
    aasdk::proto::messages::SensorEventIndication indication;
    indication.add_driving_status()->set_status(aasdk::proto::enums::DrivingStatus::UNRESTRICTED);

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendSensorEventIndication(indication, std::move(promise));
}

void SensorService::sendNightData()
{
    aasdk::proto::messages::SensorEventIndication indication;

    if (SensorService::isNight) {
        OPENAUTO_LOG(info) << "[SensorService] Mode night triggered";
        indication.add_night_mode()->set_is_night(true);
    } else {
        OPENAUTO_LOG(info) << "[SensorService] Mode day triggered";
        indication.add_night_mode()->set_is_night(false);
    }

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendSensorEventIndication(indication, std::move(promise));
    if (this->firstRun) {
        this->firstRun = false;
        this->previous = this->isNight;
    }
}

void SensorService::sendGPSLocationData()
{
    aasdk::proto::messages::SensorEventIndication indication;
    auto * locInd = indication.add_gps_location();

    // epoch seconds
    locInd->set_timestamp(this->gpsData_.fix.time * 1e3);
    // degrees
    locInd->set_latitude(this->gpsData_.fix.latitude * 1e7);
    locInd->set_longitude(this->gpsData_.fix.longitude * 1e7);
    // meters
    auto accuracy = sqrt(pow(this->gpsData_.fix.epx, 2) + pow(this->gpsData_.fix.epy, 2));
    locInd->set_accuracy(accuracy * 1e3);

    if (this->gpsData_.set & ALTITUDE_SET)
    {
        // meters above ellipsoid
        locInd->set_altitude(this->gpsData_.fix.altitude * 1e2);
    }
    if (this->gpsData_.set & SPEED_SET)
    {
        // meters per second to knots
        locInd->set_speed(this->gpsData_.fix.speed * 1.94384 * 1e3);
    }
    if (this->gpsData_.set & TRACK_SET)
    {
        // degrees
        locInd->set_bearing(this->gpsData_.fix.track * 1e6);
    }

    auto promise = aasdk::channel::SendPromise::defer(strand_);
    promise->then([]() {}, std::bind(&SensorService::onChannelError, this->shared_from_this(), std::placeholders::_1));
    channel_->sendSensorEventIndication(indication, std::move(promise));
}

void SensorService::sensorPolling()
{
    if (!this->stopPolling) {
        strand_.dispatch([this, self = this->shared_from_this()]() {
            this->isNight = is_file_exist("/tmp/night_mode_enabled");
            if (this->previous != this->isNight && !this->firstRun) {
                this->previous = this->isNight;
                this->sendNightData();
            }

            if ((this->gpsEnabled_) &&
               (gps_waiting(&this->gpsData_, 0)) &&
               (gps_read(&this->gpsData_) > 0) &&
               (this->gpsData_.status != STATUS_NO_FIX) &&
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

bool SensorService::is_file_exist(const char *fileName)
{
    std::ifstream ifile(fileName, std::ios::in);
    return ifile.good();
}

void SensorService::onChannelError(const aasdk::error::Error& e)
{
    OPENAUTO_LOG(error) << "[SensorService] channel error: " << e.what();
}

}
}
}
}
