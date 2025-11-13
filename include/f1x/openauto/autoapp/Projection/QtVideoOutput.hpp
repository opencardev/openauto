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

#include <QMediaPlayer>
#include <QVideoWidget>
#include <mutex>
#include <boost/noncopyable.hpp>
#include <f1x/openauto/autoapp/Projection/VideoOutput.hpp>
#include <f1x/openauto/autoapp/Projection/SequentialBuffer.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace projection
{

class QtVideoOutput: public QObject, public VideoOutput, boost::noncopyable
{
    Q_OBJECT

public:
    QtVideoOutput(configuration::IConfiguration::Pointer configuration);
    ~QtVideoOutput() override;
    bool open() override;
    bool init() override;
    void write(uint64_t timestamp, const aasdk::common::DataConstBuffer& buffer) override;
    void stop() override;

signals:
    void startPlayback();
    void stopPlayback();

protected slots:
    void createVideoOutput();
    void onStartPlayback();
    void onStopPlayback();
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onStateChanged(QMediaPlayer::State state);
    void onError(QMediaPlayer::Error error);

private:
    void cleanupPlayer();
    SequentialBuffer videoBuffer_;
    std::unique_ptr<QVideoWidget> videoWidget_;
    std::unique_ptr<QMediaPlayer> mediaPlayer_;
    bool playerReady_;
    bool initialBufferingDone_;
    size_t bytesWritten_;
    std::mutex writeMutex_;
    static constexpr size_t INITIAL_BUFFER_SIZE = 65536; // 64KB initial buffer before checking state
};

}
}
}
}
