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
#include "deviceinfo.h"

#include <slk/device.h>

namespace slk
{

class CoreAudioDevice
{
public:
    CoreAudioDevice(DeviceInfo&& info);
    virtual ~CoreAudioDevice();

    bool open(AudioObjectPropertyScope scope);

    [[nodiscard]] const DeviceInfo& info() const;
    [[nodiscard]] const AudioFormat& format() const;
    [[nodiscard]] DeviceDescriptor descriptor() const;
    [[nodiscard]] const AudioStreamBasicDescription& asbd() const;
    [[nodiscard]] uint32_t bufferFrameSize() const;
    [[nodiscard]] AudioDeviceID deviceId() const;

private:
    DECLARE_PIMPL_EX(CoreAudioDevice)
    DECLARE_DEFAULT_MOVE(CoreAudioDevice)
};

}
