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
    , nextTouchPointId_(0)
{
    this->moveToThread(parent.thread());
    // Note: Touch events are accepted automatically when we install the event filter
    // No need to set WA_AcceptTouchEvents on QApplication parent
}

void InputDevice::start(IInputDeviceEventHandler& eventHandler)
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    OPENAUTO_LOG(info) << "[InputDevice] start()";
    eventHandler_ = &eventHandler;
    parent_.installEventFilter(this);
}

void InputDevice::stop()
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    OPENAUTO_LOG(info) << "[InputDevice] stop()";
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
        else if(event->type() == QEvent::TouchBegin || 
                event->type() == QEvent::TouchUpdate || 
                event->type() == QEvent::TouchEnd ||
                event->type() == QEvent::TouchCancel)
        {
            return this->handleMultiTouchEvent(static_cast<QTouchEvent*>(event));
        }
        else if(event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove)
        {
            // Fallback to mouse events if touch events are not available
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
        
        // Create single-touch event for mouse fallback
        TouchEvent event;
        event.type = type;
        event.actionIndex = 0;
        event.pointers.push_back({x, y, 0});
        
        eventHandler_->onTouchEvent(event);
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

bool InputDevice::handleMultiTouchEvent(QTouchEvent* touchEvent)
{
    if(!configuration_->getTouchscreenEnabled())
    {
        return true;
    }

    OPENAUTO_LOG(debug) << "[InputDevice] handleMultiTouchEvent: type=" << touchEvent->type() 
                        << " touchPointCount=" << touchEvent->touchPoints().size();

    TouchEvent event;
    event.actionIndex = 0; // Will be updated based on which pointer changed state
    
    // Determine the action type and which pointer triggered it
    const auto& touchPoints = touchEvent->touchPoints();
    
    if(touchPoints.isEmpty())
    {
        return true;
    }

    // Find the pointer that changed state (for ACTION_DOWN, ACTION_UP, ACTION_POINTER_DOWN, ACTION_POINTER_UP)
    int changedPointIndex = -1;
    Qt::TouchPointState changedState = Qt::TouchPointStationary;
    
    for(int i = 0; i < touchPoints.size(); ++i)
    {
        const auto& point = touchPoints[i];
        if(point.state() == Qt::TouchPointPressed || point.state() == Qt::TouchPointReleased)
        {
            changedPointIndex = i;
            changedState = point.state();
            break;
        }
    }

    // Determine the action based on event type and pointer states
    if(touchEvent->type() == QEvent::TouchBegin)
    {
        event.type = aap_protobuf::service::inputsource::message::PointerAction::ACTION_DOWN;
        event.actionIndex = 0;
    }
    else if(touchEvent->type() == QEvent::TouchEnd)
    {
        event.type = aap_protobuf::service::inputsource::message::PointerAction::ACTION_UP;
        // Find the released pointer index
        for(int i = 0; i < touchPoints.size(); ++i)
        {
            if(touchPoints[i].state() == Qt::TouchPointReleased)
            {
                event.actionIndex = i;
                break;
            }
        }
    }
    else if(touchEvent->type() == QEvent::TouchUpdate)
    {
        if(changedState == Qt::TouchPointPressed)
        {
            // Additional finger went down
            event.type = aap_protobuf::service::inputsource::message::PointerAction::ACTION_POINTER_DOWN;
            event.actionIndex = changedPointIndex;
        }
        else if(changedState == Qt::TouchPointReleased)
        {
            // One finger lifted while others remain
            event.type = aap_protobuf::service::inputsource::message::PointerAction::ACTION_POINTER_UP;
            event.actionIndex = changedPointIndex;
        }
        else
        {
            // Movement only
            event.type = aap_protobuf::service::inputsource::message::PointerAction::ACTION_MOVED;
            event.actionIndex = 0;
        }
    }
    else if(touchEvent->type() == QEvent::TouchCancel)
    {
        // Android Auto protocol doesn't support ACTION_CANCEL, treat as ACTION_UP (all fingers lifted)
        event.type = aap_protobuf::service::inputsource::message::PointerAction::ACTION_UP;
        event.actionIndex = 0;
        touchPointIdMap_.clear();
        nextTouchPointId_ = 0;
    }
    else
    {
        return true;
    }

    // Translate all touch points
    for(const auto& qtPoint : touchPoints)
    {
        // Skip released points except for UP actions
        if(qtPoint.state() == Qt::TouchPointReleased && 
           event.type != aap_protobuf::service::inputsource::message::PointerAction::ACTION_UP &&
           event.type != aap_protobuf::service::inputsource::message::PointerAction::ACTION_POINTER_UP)
        {
            continue;
        }

        TouchPoint ourPoint;
        translateTouchPoint(qtPoint, ourPoint);
        event.pointers.push_back(ourPoint);

        // Clean up released touch points from map
        if(qtPoint.state() == Qt::TouchPointReleased)
        {
            touchPointIdMap_.erase(qtPoint.id());
        }
    }

    if(!event.pointers.empty())
    {
        OPENAUTO_LOG(debug) << "[InputDevice] Sending touch event: action=" << event.type 
                            << " actionIndex=" << event.actionIndex 
                            << " pointerCount=" << event.pointers.size();
        eventHandler_->onTouchEvent(event);
    }

    return true;
}

void InputDevice::translateTouchPoint(const QTouchEvent::TouchPoint& qtPoint, TouchPoint& ourPoint)
{
    // Map Qt touch point ID to our sequential ID
    int qtId = qtPoint.id();
    if(touchPointIdMap_.find(qtId) == touchPointIdMap_.end())
    {
        touchPointIdMap_[qtId] = nextTouchPointId_++;
    }
    ourPoint.pointerId = touchPointIdMap_[qtId];

    // Scale coordinates from touchscreen geometry to display geometry
    QPointF pos = qtPoint.pos();
    ourPoint.x = static_cast<uint32_t>((pos.x() / touchscreenGeometry_.width()) * displayGeometry_.width());
    ourPoint.y = static_cast<uint32_t>((pos.y() / touchscreenGeometry_.height()) * displayGeometry_.height());

    OPENAUTO_LOG(debug) << "[InputDevice] Touch point: qtId=" << qtId 
                        << " ourId=" << ourPoint.pointerId 
                        << " pos=(" << ourPoint.x << "," << ourPoint.y << ")"
                        << " state=" << qtPoint.state();
}

}
}
}
}
