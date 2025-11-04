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

#include <QApplication>
#include <mutex>
#include <f1x/openauto/autoapp/Projection/QtVideoOutput.hpp>
#include <f1x/openauto/Common/Log.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace projection
{

QtVideoOutput::QtVideoOutput(configuration::IConfiguration::Pointer configuration)
    : VideoOutput(std::move(configuration))
    , playerReady_(false)
    , initialBufferingDone_(false)
    , bytesWritten_(0)
{
    this->moveToThread(QApplication::instance()->thread());
    connect(this, &QtVideoOutput::startPlayback, this, &QtVideoOutput::onStartPlayback, Qt::BlockingQueuedConnection);
    connect(this, &QtVideoOutput::stopPlayback, this, &QtVideoOutput::onStopPlayback, Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "createVideoOutput", Qt::BlockingQueuedConnection);
}

void QtVideoOutput::createVideoOutput()
{
    OPENAUTO_LOG(info) << "[QtVideoOutput] createVideoOutput()";
    videoWidget_ = std::make_unique<QVideoWidget>();
    mediaPlayer_ = std::make_unique<QMediaPlayer>(nullptr, QMediaPlayer::StreamPlayback);
}


bool QtVideoOutput::open()
{
    return videoBuffer_.open(QIODevice::ReadWrite);
}

bool QtVideoOutput::init()
{
    emit startPlayback();
    return true;
}

void QtVideoOutput::stop()
{
    emit stopPlayback();
}

void QtVideoOutput::write(uint64_t, const aasdk::common::DataConstBuffer& buffer)
{
    std::lock_guard<std::mutex> lock(writeMutex_);
    
    // Write data to buffer - the SequentialBuffer will handle the buffering
    videoBuffer_.write(reinterpret_cast<const char*>(buffer.cdata), buffer.size);
    bytesWritten_ += buffer.size;
    
    // Log initial buffering milestone
    if (!initialBufferingDone_ && bytesWritten_ >= INITIAL_BUFFER_SIZE)
    {
        initialBufferingDone_ = true;
        OPENAUTO_LOG(info) << "[QtVideoOutput] Initial buffering complete (" << bytesWritten_ << " bytes written)";
    }
}

void QtVideoOutput::onStartPlayback()
{
    OPENAUTO_LOG(info) << "[QtVideoOutput] onStartPlayback()";
    
    videoWidget_->setAttribute(Qt::WA_OpaquePaintEvent, true);
    videoWidget_->setAttribute(Qt::WA_NoSystemBackground, true);
    videoWidget_->setAspectRatioMode(Qt::IgnoreAspectRatio);
    videoWidget_->setFocus();
    videoWidget_->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    videoWidget_->raise();
    videoWidget_->setFullScreen(true);
    videoWidget_->show();
    videoWidget_->activateWindow();

    // Connect state change signals to track when player is ready
    connect(mediaPlayer_.get(), &QMediaPlayer::mediaStatusChanged, this, &QtVideoOutput::onMediaStatusChanged);
    connect(mediaPlayer_.get(), &QMediaPlayer::stateChanged, this, &QtVideoOutput::onStateChanged);
    connect(mediaPlayer_.get(), static_cast<void(QMediaPlayer::*)(QMediaPlayer::Error)>(&QMediaPlayer::error), 
            this, &QtVideoOutput::onError);
    
    mediaPlayer_->setVideoOutput(videoWidget_.get());
    mediaPlayer_->setMedia(QMediaContent(), &videoBuffer_);
    mediaPlayer_->play();
    
    // Mark as ready immediately after calling play() since we're using blocking connection
    {
        std::lock_guard<std::mutex> lock(writeMutex_);
        playerReady_ = true;
    }

    OPENAUTO_LOG(info) << "[QtVideoOutput] Player started and marked ready";
    OPENAUTO_LOG(debug) << "[QtVideoOutput] Player error state -> " << mediaPlayer_->errorString().toStdString();
}

void QtVideoOutput::onStopPlayback()
{
    OPENAUTO_LOG(info) << "[QtVideoOutput] onStopPlayback()";
    
    std::lock_guard<std::mutex> lock(writeMutex_);
    playerReady_ = false;
    initialBufferingDone_ = false;
    bytesWritten_ = 0;
    
    videoWidget_->hide();
    videoWidget_->clearFocus();
    mediaPlayer_->stop();
    mediaPlayer_->setMedia(QMediaContent());
}

void QtVideoOutput::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    OPENAUTO_LOG(debug) << "[QtVideoOutput] Media status changed: " << status;
    
    // Mark player as ready when buffering or buffered
    if (status == QMediaPlayer::BufferingMedia || status == QMediaPlayer::BufferedMedia)
    {
        std::lock_guard<std::mutex> lock(writeMutex_);
        playerReady_ = true;
        OPENAUTO_LOG(info) << "[QtVideoOutput] Player is now ready to receive data";
    }
}

void QtVideoOutput::onStateChanged(QMediaPlayer::State state)
{
    OPENAUTO_LOG(debug) << "[QtVideoOutput] Player state changed: " << state;
    
    // Mark player as ready when playing state is reached
    if (state == QMediaPlayer::PlayingState)
    {
        std::lock_guard<std::mutex> lock(writeMutex_);
        playerReady_ = true;
        OPENAUTO_LOG(info) << "[QtVideoOutput] Player entered PLAYING state";
    }
    else if (state == QMediaPlayer::StoppedState)
    {
        std::lock_guard<std::mutex> lock(writeMutex_);
        playerReady_ = false;
        OPENAUTO_LOG(info) << "[QtVideoOutput] Player stopped";
    }
}

void QtVideoOutput::onError(QMediaPlayer::Error error)
{
    OPENAUTO_LOG(error) << "[QtVideoOutput] Media player error occurred!";
    OPENAUTO_LOG(error) << "[QtVideoOutput] Error code: " << error;
    OPENAUTO_LOG(error) << "[QtVideoOutput] Error string: " << mediaPlayer_->errorString().toStdString();
    
    // Provide helpful error messages for common issues
    if (error == QMediaPlayer::FormatError)
    {
        OPENAUTO_LOG(error) << "[QtVideoOutput] FORMAT ERROR - This usually means a required codec is missing";
        OPENAUTO_LOG(error) << "[QtVideoOutput] Video codec required: H.264";
        OPENAUTO_LOG(error) << "[QtVideoOutput] Please install: sudo apt-get install gstreamer1.0-libav gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly";
    }
    else if (error == QMediaPlayer::ResourceError)
    {
        OPENAUTO_LOG(error) << "[QtVideoOutput] RESOURCE ERROR - Failed to allocate resources for playback";
    }
    else if (error == QMediaPlayer::ServiceMissingError)
    {
        OPENAUTO_LOG(error) << "[QtVideoOutput] SERVICE MISSING - GStreamer backend may not be properly installed";
        OPENAUTO_LOG(error) << "[QtVideoOutput] Please install: sudo apt-get install gstreamer1.0-plugins-base gstreamer1.0-plugins-good";
    }
}

}
}
}
}
