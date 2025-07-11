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

#include <modern/Logger.hpp>
#include <f1x/openauto/autoapp/Projection/IInputDeviceEventHandler.hpp>
#include <f1x/openauto/autoapp/Projection/InputDevice.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace projection
{

InputDevice::InputDevice(QObject& parent, configuration::IConfiguration::Pointer configuration, const QRect& touchscreenGeometry, const QRect& displayGeometry)
    : parent_(parent)
    , configuration_(std::move(configuration))
    , touchscreenGeometry_(touchscreenGeometry)
    , displayGeometry_(displayGeometry)
    , eventHandler_(nullptr)
{
    this->moveToThread(parent.thread());
}

void InputDevice::start(IInputDeviceEventHandler& eventHandler)
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    LOG_INFO(GENERAL, "[InputDevice] start()");
    eventHandler_ = &eventHandler;
    parent_.installEventFilter(this);
}

void InputDevice::stop()
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    LOG_INFO(GENERAL, "[InputDevice] stop()");
    parent_.removeEventFilter(this);
    eventHandler_ = nullptr;
}

bool InputDevice::eventFilter(QObject* obj, QEvent* event)
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    if(eventHandler_ != nullptr)
    {
        if(event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
        {
            QKeyEvent* key = static_cast<QKeyEvent*>(event);
            if(!key->isAutoRepeat())
            {
                return this->handleKeyEvent(event, key);
            }
        }
        else if(event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove)
        {
            return this->handleTouchEvent(event);
        }
    }

    return QObject::eventFilter(obj, event);
}

bool InputDevice::handleKeyEvent(QEvent* event, QKeyEvent* key)
{
    auto eventType = event->type() == QEvent::KeyPress ? ButtonEventType::PRESS : ButtonEventType::RELEASE;
    aap_protobuf::service::media::sink::message::KeyCode buttonCode;
    WheelDirection wheelDirection = WheelDirection::NONE;

    switch(key->key())
    {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_DPAD_CENTER;
        break;

    case Qt::Key_Left:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_DPAD_LEFT;
        break;

    case Qt::Key_Right:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_DPAD_RIGHT;
        break;

    case Qt::Key_Up:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_DPAD_UP;
        break;

    case Qt::Key_Down:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_DPAD_DOWN;
        break;

    case Qt::Key_Escape:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_BACK;
        break;

    case Qt::Key_H:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_HOME;
        break;

    case Qt::Key_P:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_CALL;
        break;

    case Qt::Key_O:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_ENDCALL;
        break;

    case Qt::Key_MediaPlay:
    case Qt::Key_X:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_MEDIA_PLAY;
        break;

    case Qt::Key_MediaPause:
    case Qt::Key_C:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_MEDIA_PAUSE;
        break;

    case Qt::Key_MediaPrevious:
    case Qt::Key_V:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_MEDIA_PREVIOUS;
        break;

    case Qt::Key_MediaTogglePlayPause:
    case Qt::Key_B:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_MEDIA_PLAY_PAUSE;
        break;

    case Qt::Key_MediaNext:
    case Qt::Key_N:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_MEDIA_NEXT;
        break;

    case Qt::Key_M:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_SEARCH;
        break;

    case Qt::Key_1:
        wheelDirection = WheelDirection::LEFT;
        eventType = ButtonEventType::NONE;
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_ROTARY_CONTROLLER;
        break;

    case Qt::Key_2:
        wheelDirection = WheelDirection::RIGHT;
        eventType = ButtonEventType::NONE;
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_ROTARY_CONTROLLER;
        break;

    case Qt::Key_F:
        buttonCode = aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_NAVIGATION;
        break;

    default:
        return true;
    }

    const auto& buttonCodes = this->getSupportedButtonCodes();
    if(std::find(buttonCodes.begin(), buttonCodes.end(), buttonCode) != buttonCodes.end())
    {
        if(buttonCode != aap_protobuf::service::media::sink::message::KeyCode::KEYCODE_ROTARY_CONTROLLER || event->type() == QEvent::KeyRelease)
        {
            eventHandler_->onButtonEvent({eventType, wheelDirection, buttonCode});
        }
    }

    return true;
}

bool InputDevice::handleTouchEvent(QEvent* event)
{
    if(!configuration_->getTouchscreenEnabled())
    {
        return true;
    }

    aap_protobuf::service::inputsource::message::PointerAction type;

    switch(event->type())
    {
    case QEvent::MouseButtonPress:
        type = aap_protobuf::service::inputsource::message::PointerAction::ACTION_DOWN;
        break;
    case QEvent::MouseButtonRelease:
        type = aap_protobuf::service::inputsource::message::PointerAction::ACTION_UP;
        break;
    case QEvent::MouseMove:
        type = aap_protobuf::service::inputsource::message::PointerAction::ACTION_MOVED;
        break;
    default:
        return true;
    };

    QMouseEvent* mouse = static_cast<QMouseEvent*>(event);
    if(event->type() == QEvent::MouseButtonRelease || mouse->buttons().testFlag(Qt::LeftButton))
    {
        const uint32_t x = (static_cast<float>(mouse->pos().x()) / touchscreenGeometry_.width()) * displayGeometry_.width();
        const uint32_t y = (static_cast<float>(mouse->pos().y()) / touchscreenGeometry_.height()) * displayGeometry_.height();
        eventHandler_->onTouchEvent({type, x, y, 0});
    }

    return true;
}

bool InputDevice::hasTouchscreen() const
{
    return configuration_->getTouchscreenEnabled();
}

QRect InputDevice::getTouchscreenGeometry() const
{
    return touchscreenGeometry_;
}

IInputDevice::ButtonCodes InputDevice::getSupportedButtonCodes() const
{
    return configuration_->getButtonCodes();
}

}
}
}
}
