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

#include "coreaudiodevice.h"

#include <vector>

#include <slk/audioformat.h>

namespace slk
{

struct CoreAudioDevice::impl_t // NOLINT(cppcoreguidelines-special-member-functions)
{
    DeviceInfo info;
    AudioFormat format;
    AudioStreamBasicDescription asbd {};
    uint32_t bufferFrameSize { 0 };

    impl_t(DeviceInfo&& deviceInfo) : info(std::move(deviceInfo))
    {
    }

    ~impl_t() = default;
};

CoreAudioDevice::CoreAudioDevice(DeviceInfo&& info)
{
    createImpl(std::move(info));
}

CoreAudioDevice::~CoreAudioDevice() = default;

bool CoreAudioDevice::open(AudioObjectPropertyScope scope)
{
    const AudioDeviceID devId = impl().info.audioDeviceId;

    const AudioObjectPropertyAddress fmtAddr {
        kAudioDevicePropertyStreamFormat,
        scope,
        kAudioObjectPropertyElementMain
    };

    UInt32 size { sizeof(AudioStreamBasicDescription) };
    OSStatus status = AudioObjectGetPropertyData(devId, &fmtAddr, 0, nullptr, &size, &impl().asbd);

    if (status != noErr) {
        return false;
    }

    const AudioObjectPropertyAddress bufAddr {
        kAudioDevicePropertyBufferFrameSize,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };

    UInt32 bufSize { sizeof(uint32_t) };
    status = AudioObjectGetPropertyData(devId, &bufAddr, 0, nullptr, &bufSize, &impl().bufferFrameSize);

    if (status != noErr) {
        return false;
    }

    // kAudioDevicePropertyStreamFormat gives mChannelsPerFrame per buffer (1 for non-interleaved).
    // Query kAudioDevicePropertyStreamConfiguration to count the actual total channel count.
    const AudioObjectPropertyAddress configAddr {
        kAudioDevicePropertyStreamConfiguration,
        scope,
        kAudioObjectPropertyElementMain
    };

    UInt32 configSize { 0 };
    status = AudioObjectGetPropertyDataSize(devId, &configAddr, 0, nullptr, &configSize);

    if (status != noErr || configSize == 0) {
        return false;
    }

    std::vector<uint8_t> configBuf(configSize);
    auto* bufList = reinterpret_cast<AudioBufferList*>(configBuf.data());
    status = AudioObjectGetPropertyData(devId, &configAddr, 0, nullptr, &configSize, bufList);

    if (status != noErr) {
        return false;
    }

    uint16_t channels { 0 };
    for (uint32_t i = 0; i < bufList->mNumberBuffers; ++i) {
        channels += static_cast<uint16_t>(bufList->mBuffers[i].mNumberChannels);
    }

    if (channels == 0) {
        return false;
    }

    const auto sampleRate = static_cast<uint32_t>(impl().asbd.mSampleRate);
    const auto bitsPerSample = static_cast<uint16_t>(impl().asbd.mBitsPerChannel);

    impl().format = AudioFormat(channels, sampleRate, bitsPerSample, AudioFormat::Type::FLOAT);

    return true;
}

const DeviceInfo& CoreAudioDevice::info() const
{
    return impl().info;
}

const AudioFormat& CoreAudioDevice::format() const
{
    return impl().format;
}

DeviceDescriptor CoreAudioDevice::descriptor() const
{
    DeviceDescriptor desc;
    desc.name = impl().info.friendlyName;
    desc.id = impl().info.deviceId;
    desc.type = impl().info.type;
    return desc;
}

const AudioStreamBasicDescription& CoreAudioDevice::asbd() const
{
    return impl().asbd;
}

uint32_t CoreAudioDevice::bufferFrameSize() const
{
    return impl().bufferFrameSize;
}

AudioDeviceID CoreAudioDevice::deviceId() const
{
    return impl().info.audioDeviceId;
}

}
