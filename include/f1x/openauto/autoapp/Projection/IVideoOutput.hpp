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

#include <memory>
#include <QRect>
#include <aasdk/Common/Data.hpp>
#include <aasdk/Messenger/Timestamp.hpp>
#include <aap_protobuf/service/media/sink/message/VideoFrameRateType.pb.h>
#include <aap_protobuf/service/media/sink/message/VideoCodecResolutionType.pb.h>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace projection
{

class IVideoOutput
{
public:
    typedef std::shared_ptr<IVideoOutput> Pointer;

    IVideoOutput() = default;
    virtual ~IVideoOutput() = default;

    virtual bool open() = 0;
    virtual bool init() = 0;
    virtual void write(aasdk::messenger::Timestamp::ValueType timestamp, const aasdk::common::DataConstBuffer& buffer) = 0;
    virtual void stop() = 0;

    virtual aap_protobuf::service::media::sink::message::VideoFrameRateType getVideoFPS() const = 0;
    virtual aap_protobuf::service::media::sink::message::VideoCodecResolutionType getVideoResolution() const = 0;
    virtual size_t getScreenDPI() const = 0;
    virtual QRect getVideoMargins() const = 0;

};

}
}
}
}
