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

#include <vector>
#include <aap_protobuf/service/media/sink/message/KeyCode.pb.h>
#include <aap_protobuf/service/inputsource/message/PointerAction.pb.h>
#include <aasdk/IO/Promise.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace projection
{

enum class ButtonEventType
{
    NONE,
    PRESS,
    RELEASE
};

enum class WheelDirection
{
    NONE,
    LEFT,
    RIGHT
};

struct ButtonEvent
{
    ButtonEventType type;
    WheelDirection wheelDirection;
    aap_protobuf::service::media::sink::message::KeyCode code;
};

struct TouchPoint
{
    uint32_t x;
    uint32_t y;
    uint32_t pointerId;
};

struct TouchEvent
{
    aap_protobuf::service::inputsource::message::PointerAction type;
    std::vector<TouchPoint> pointers;
    uint32_t actionIndex; // Index of the pointer that changed state
};

}
}
}
}
