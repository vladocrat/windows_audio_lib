// windows_audio_lib - Windows audio library
// Copyright (C) 2026  Vladislav Milovanov
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <CoreAudio/CoreAudio.h>

#include "utils.h"

#include <slk/outputdevice.h>
#include <slk/ringbuffer.h>

namespace slk
{

struct DeviceInfo;
struct DeviceDescriptor;
class AudioFormat;

class CoreAudioOutputDevice : public OutputDevice
{
public:
    CoreAudioOutputDevice(DeviceInfo&& info);
    CoreAudioOutputDevice() = delete;
    CoreAudioOutputDevice(CoreAudioOutputDevice&&) = default;
    CoreAudioOutputDevice& operator=(CoreAudioOutputDevice&&) = default;
    ~CoreAudioOutputDevice() override;

    bool open() override;
    bool close() override;
    bool start() override;
    bool stop() override;

    void setSource(RingBuffer<float>& source) override;
    void setProcessCallback(ProcessCallback callback) override;
    [[nodiscard]] const AudioFormat& format() const override;
    [[nodiscard]] DeviceDescriptor descriptor() const override;

private:
    static OSStatus outputIOProc(AudioObjectID inDevice,
                                 const AudioTimeStamp* inNow,
                                 const AudioBufferList* inInputData,
                                 const AudioTimeStamp* inInputTime,
                                 AudioBufferList* outOutputData,
                                 const AudioTimeStamp* inOutputTime,
                                 void* inClientData);

    DECLARE_PIMPL_EX(CoreAudioOutputDevice)
};

}
