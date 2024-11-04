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

#include <f1x/openauto/autoapp/Projection/RtAudioOutput.hpp>
#include <f1x/openauto/Common/Log.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace projection
{

RtAudioOutput::RtAudioOutput(uint32_t channelCount, uint32_t sampleSize, uint32_t sampleRate)
    : channelCount_(channelCount)
    , sampleSize_(sampleSize)
    , sampleRate_(sampleRate)
{

  std::vector<RtAudio::Api> apis;
  RtAudio::getCompiledApi(apis);
  dac_ = std::find(apis.begin(), apis.end(), RtAudio::LINUX_PULSE) == apis.end() ? std::make_unique<RtAudio>() : std::make_unique<RtAudio>(RtAudio::LINUX_PULSE);

  // Set the error callback
  dac_->setErrorCallback([](RtAudioErrorType type, const std::string& errorText) {
    OPENAUTO_LOG(error) << "[RtAudioOutput] " << errorText;
  });
}

bool RtAudioOutput::open()
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    if(dac_->getDeviceCount() > 0)
    {
        RtAudio::StreamParameters parameters;
        parameters.deviceId = dac_->getDefaultOutputDevice();
        parameters.nChannels = channelCount_;
        parameters.firstChannel = 0;

        try
        {
            RtAudio::StreamOptions streamOptions;
            streamOptions.flags = RTAUDIO_MINIMIZE_LATENCY | RTAUDIO_SCHEDULE_REALTIME;
            uint32_t bufferFrames = sampleRate_ == 16000 ? 1024 : 2048; //according to the observation of audio packets
            auto result = dac_->openStream(&parameters, nullptr, RTAUDIO_SINT16, sampleRate_, &bufferFrames, &RtAudioOutput::audioBufferReadHandler, static_cast<void*>(this), &streamOptions);
            dac_->openStream(&parameters, nullptr, RTAUDIO_SINT16, sampleRate_, &bufferFrames, &RtAudioOutput::audioBufferReadHandler, static_cast<void*>(this), &streamOptions);
            if (result == RtAudioErrorType::RTAUDIO_NO_ERROR) {
              OPENAUTO_LOG(info) << "[RtAudioOutput] Sample Rate: " << sampleRate_;
              return audioBuffer_.open(QIODevice::ReadWrite);
            } else {
              // Log the error using the callback or handle it here directly
              OPENAUTO_LOG(error) << "[RtAudioOutput] Error opening stream: ";
            }
        }
        catch(const std::exception& e)
        {
          OPENAUTO_LOG(error) << "[RtAudioOutput] Exception: " << e.what();
        }
    }
    else
    {
        OPENAUTO_LOG(error) << "[RtAudioOutput] No output devices found.";
    }

    return false;
}

void RtAudioOutput::write(aasdk::messenger::Timestamp::ValueType timestamp, const aasdk::common::DataConstBuffer& buffer)
{
    audioBuffer_.write(reinterpret_cast<const char*>(buffer.cdata), buffer.size);
}

void RtAudioOutput::start()
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    if(dac_->isStreamOpen() && !dac_->isStreamRunning())
    {
        try
        {
            dac_->startStream();
        }
        catch(const RtAudioErrorType& e)
        {
          // TODO: What's the new RtAudio Error?
            //OPENAUTO_LOG(error) << "[RtAudioOutput] Failed to start audio output, what: " << e.what();
        }
    }
}

void RtAudioOutput::stop()
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);

    this->doSuspend();

    if(dac_->isStreamOpen())
    {
        dac_->closeStream();
    }
}

void RtAudioOutput::suspend()
{
    //not needed
}

uint32_t RtAudioOutput::getSampleSize() const
{
    return sampleSize_;
}

uint32_t RtAudioOutput::getChannelCount() const
{
    return channelCount_;
}

uint32_t RtAudioOutput::getSampleRate() const
{
    return sampleRate_;
}

void RtAudioOutput::doSuspend()
{
    if(dac_->isStreamOpen() && dac_->isStreamRunning())
    {
        try
        {
            dac_->stopStream();
        }
        catch(const RtAudioErrorType& e)
        {
          // TODO: What's the new RtAudio Error?
          //OPENAUTO_LOG(error) << "[RtAudioOutput] Failed to suspend audio output, what: " << e.what();
        }
    }
}

int RtAudioOutput::audioBufferReadHandler(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
                                          double streamTime, RtAudioStreamStatus status, void* userData)
{
    RtAudioOutput* self = static_cast<RtAudioOutput*>(userData);
    std::lock_guard<decltype(self->mutex_)> lock(self->mutex_);

    const auto bufferSize = nBufferFrames * (self->sampleSize_ / 8) * self->channelCount_;
    self->audioBuffer_.read(reinterpret_cast<char*>(outputBuffer), bufferSize);
    return 0;
}

}
}
}
}
