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

#include "coreaudiooutputdevice.h"

#include <CoreAudio/CoreAudio.h>

#include "coreaudiodevice.h"

#include <slk/audioformat.h>
#include <slk/audiobuffer.h>

namespace slk
{

struct CoreAudioOutputDevice::impl_t // NOLINT(cppcoreguidelines-special-member-functions)
{
    CoreAudioDevice device;
    AudioDeviceIOProcID ioProcId { nullptr };

    RingBuffer<float>* source { nullptr };
    CoreAudioOutputDevice::ProcessCallback processCallback;

    impl_t(DeviceInfo&& info) : device(std::move(info))
    {
    }

    ~impl_t() = default;
};

OSStatus CoreAudioOutputDevice::outputIOProc(AudioObjectID /*inDevice*/,
                                             const AudioTimeStamp* /*inNow*/,
                                             const AudioBufferList* /*inInputData*/,
                                             const AudioTimeStamp* /*inInputTime*/,
                                             AudioBufferList* outOutputData,
                                             const AudioTimeStamp* /*inOutputTime*/,
                                             void* inClientData)
{
    auto* self = static_cast<impl_t*>(inClientData);

    const uint32_t numBuffers = outOutputData->mNumberBuffers;
    if (numBuffers == 0) {
        return noErr;
    }

    const uint32_t numFrames = outOutputData->mBuffers[0].mDataByteSize / sizeof(float);
    if (numFrames == 0) {
        return noErr;
    }

    const uint32_t channels = numBuffers;

    AudioBuffer<float> tempBuffer(channels, numFrames);

    if (self->source) {
        const size_t maxSamples = static_cast<size_t>(numFrames) * channels;
        self->source->read(std::span<float>(tempBuffer.data().data(), maxSamples), maxSamples);
    }

    if (self->processCallback) {
        self->processCallback(tempBuffer);
    }

    // Deinterleave from tempBuffer into the non-interleaved CoreAudio output buffers.
    const float* src = tempBuffer.data().data();
    for (uint32_t ch = 0; ch < numBuffers; ++ch) {
        auto* dst = static_cast<float*>(outOutputData->mBuffers[ch].mData);
        if (!dst) {
            continue;
        }
        for (uint32_t frame = 0; frame < numFrames; ++frame) {
            dst[frame] = src[(frame * channels) + ch];
        }
    }

    return noErr;
}

CoreAudioOutputDevice::CoreAudioOutputDevice(DeviceInfo&& info)
{
    createImpl(std::move(info));
}

CoreAudioOutputDevice::~CoreAudioOutputDevice()
{
    if (_impl && impl().ioProcId) {
        stop();
    }
}

bool CoreAudioOutputDevice::open()
{
    return impl().device.open(kAudioObjectPropertyScopeOutput);
}

bool CoreAudioOutputDevice::close()
{
    return stop();
}

bool CoreAudioOutputDevice::start()
{
    if (impl().ioProcId) {
        return false; // already running — reject double-start
    }

    const AudioDeviceID devId = impl().device.deviceId();

    AudioDeviceIOProcID procId { nullptr };
    OSStatus status = AudioDeviceCreateIOProcID(devId, outputIOProc, &impl(), &procId);

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

bool CoreAudioOutputDevice::stop()
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

void CoreAudioOutputDevice::setSource(RingBuffer<float>& source)
{
    impl().source = &source;
}

void CoreAudioOutputDevice::setProcessCallback(ProcessCallback callback)
{
    impl().processCallback = std::move(callback);
}

const AudioFormat& CoreAudioOutputDevice::format() const
{
    return impl().device.format();
}

DeviceDescriptor CoreAudioOutputDevice::descriptor() const
{
    return impl().device.descriptor();
}

}
