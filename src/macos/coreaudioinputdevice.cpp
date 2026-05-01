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

#include "coreaudioinputdevice.h"

#include <CoreAudio/CoreAudio.h>

#include "coreaudiodevice.h"

#include <slk/audioformat.h>
#include <slk/audiobuffer.h>

namespace slk
{

struct CoreAudioInputDevice::impl_t // NOLINT(cppcoreguidelines-special-member-functions)
{
    CoreAudioDevice device;
    AudioDeviceIOProcID ioProcId { nullptr };

    CoreAudioInputDevice::ProcessCallback processCallback {};

    impl_t(DeviceInfo&& info) : device(std::move(info))
    {
    }

    ~impl_t() = default;
};

OSStatus CoreAudioInputDevice::inputIOProc(AudioObjectID /*inDevice*/,
                                           const AudioTimeStamp* /*inNow*/,
                                           const AudioBufferList* inInputData,
                                           const AudioTimeStamp* /*inInputTime*/,
                                           AudioBufferList* /*outOutputData*/,
                                           const AudioTimeStamp* /*inOutputTime*/,
                                           void* inClientData)
{
    auto* self = static_cast<impl_t*>(inClientData);

    if (!inInputData || inInputData->mNumberBuffers == 0) {
        return noErr;
    }

    const uint32_t numBuffers = inInputData->mNumberBuffers;
    const uint32_t numFrames = inInputData->mBuffers[0].mDataByteSize / sizeof(float);
    if (numFrames == 0) {
        return noErr;
    }

    const uint32_t channels = numBuffers;

    AudioBuffer<float> captureBuffer(channels, numFrames);

    // Interleave from the non-interleaved CoreAudio input buffers into captureBuffer.
    float* dst = captureBuffer.data().data();
    for (uint32_t ch = 0; ch < numBuffers; ++ch) {
        const auto* src = static_cast<const float*>(inInputData->mBuffers[ch].mData);
        if (!src) {
            continue;
        }
        for (uint32_t frame = 0; frame < numFrames; ++frame) {
            dst[(frame * channels) + ch] = src[frame];
        }
    }

    if (self->processCallback) {
        self->processCallback(captureBuffer);
    }

    return noErr;
}

CoreAudioInputDevice::CoreAudioInputDevice(DeviceInfo&& info)
{
    createImpl(std::move(info));
}

CoreAudioInputDevice::~CoreAudioInputDevice()
{
    if (_impl && impl().ioProcId) {
        stop();
    }
}

bool CoreAudioInputDevice::open()
{
    return impl().device.open(kAudioObjectPropertyScopeInput);
}

bool CoreAudioInputDevice::close()
{
    return stop();
}

bool CoreAudioInputDevice::start()
{
    if (impl().ioProcId) {
        return false; // already running — reject double-start
    }

    const AudioDeviceID devId = impl().device.deviceId();

    AudioDeviceIOProcID procId { nullptr };
    OSStatus status = AudioDeviceCreateIOProcID(devId, inputIOProc, &impl(), &procId);

    if (status != noErr || !procId) {
        return false;
    }

    status = AudioDeviceStart(devId, procId);

    if (status != noErr) {
        AudioDeviceDestroyIOProcID(devId, procId);
        return false;
    }

    impl().ioProcId = procId;
    return true;
}

bool CoreAudioInputDevice::stop()
{
    if (!impl().ioProcId) {
        return true;
    }

    const AudioDeviceID devId = impl().device.deviceId();
    AudioDeviceStop(devId, impl().ioProcId);
    AudioDeviceDestroyIOProcID(devId, impl().ioProcId);
    impl().ioProcId = nullptr;

    return true;
}

void CoreAudioInputDevice::setProcessCallback(ProcessCallback callback)
{
    impl().processCallback = std::move(callback);
}

const AudioFormat& CoreAudioInputDevice::format() const
{
    return impl().device.format();
}

DeviceDescriptor CoreAudioInputDevice::descriptor() const
{
    return impl().device.descriptor();
}

}
