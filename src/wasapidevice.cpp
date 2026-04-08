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

#include "wasapidevice.h"

#include <initguid.h>
#include <mmdeviceapi.h>

#include <chrono>

#include <slk/audioformat.h>

namespace
{

using namespace std::chrono_literals;

static constexpr const auto BUFFER_LATENCY = 10ms;

WAVEFORMATEX* negotiateFloatFormat(IAudioClient* client)
{
    WAVEFORMATEX* mixFormat { nullptr };
    client->GetMixFormat(&mixFormat);

    if (!mixFormat) {
        return nullptr;
    }

    WAVEFORMATEXTENSIBLE floatFormat;
    floatFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    floatFormat.Format.nChannels = mixFormat->nChannels;
    floatFormat.Format.nSamplesPerSec = mixFormat->nSamplesPerSec;
    floatFormat.Format.wBitsPerSample = 32;
    floatFormat.Format.nBlockAlign = (floatFormat.Format.nChannels * 32) / 8;
    floatFormat.Format.nAvgBytesPerSec = floatFormat.Format.nSamplesPerSec * floatFormat.Format.nBlockAlign;
    floatFormat.Format.cbSize = 22;
    floatFormat.Samples.wValidBitsPerSample = 32;
    floatFormat.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;

    if (mixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        floatFormat.dwChannelMask = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(mixFormat)->dwChannelMask;
    } else {
        floatFormat.dwChannelMask = (mixFormat->nChannels == 2) ?
                                        (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT) : SPEAKER_FRONT_CENTER;
    }

    WAVEFORMATEX* closestMatch { nullptr };
    HRESULT hr = client->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, reinterpret_cast<WAVEFORMATEX*>(&floatFormat), &closestMatch);

    WAVEFORMATEX* result { nullptr };

    if (hr == S_OK) {
        result = static_cast<WAVEFORMATEX*>(CoTaskMemAlloc(sizeof(WAVEFORMATEXTENSIBLE)));
        memcpy(result, &floatFormat, sizeof(WAVEFORMATEXTENSIBLE));
        CoTaskMemFree(mixFormat);
    } else {
        result = mixFormat;
    }

    if (closestMatch) {
        CoTaskMemFree(closestMatch);
    }

    return result;
}
}

namespace slk
{

struct WASAPIDevice::impl_t
{
    DeviceInfo info;
    IAudioClient* client { nullptr };
    AudioBuffer<BYTE> data;
    AudioFormat format;
    WAVEFORMATEX* rawFormat { nullptr };

    impl_t(DeviceInfo&& info)
        : info { std::move(info) }
    {

    }

    ~impl_t()
    {
        if (client) {
            client->Release();
        }

        if (rawFormat) {
            CoTaskMemFree(rawFormat);
        }
    }
};

WASAPIDevice::WASAPIDevice(DeviceInfo&& info)
{
    createImpl(std::move(info));
}

WASAPIDevice::~WASAPIDevice()
{

}

bool WASAPIDevice::open(const DWORD streamFlags)
{
    auto res = info().device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, reinterpret_cast<void**>(&impl().client));

    if (res != S_OK) {
        return false;
    }

    impl().rawFormat = negotiateFloatFormat(impl().client);

    if (!impl().rawFormat) {
        return false;
    }

    auto* fmt = impl().rawFormat;
    auto type = AudioFormat::Type::PCM;

    if (fmt->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        auto* ext = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(fmt);
        if (ext->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
            type = AudioFormat::Type::FLOAT;
        }
    }

    impl().format = AudioFormat(
        static_cast<uint16_t>(fmt->nChannels),
        fmt->nSamplesPerSec,
        static_cast<uint16_t>(fmt->wBitsPerSample),
        type
    );

    res = impl().client->Initialize(AUDCLNT_SHAREMODE_SHARED, streamFlags, BUFFER_LATENCY.count(), 0, impl().rawFormat, NULL);

    if (res != S_OK) {
        return false;
    }

    return true;
}

const DeviceInfo& WASAPIDevice::info() const
{
    return impl().info;
}

IAudioClient* const WASAPIDevice::audioClient() const
{
    return impl().client;
}

const AudioBuffer<BYTE>& WASAPIDevice::buffer() const
{
    return impl().data;
}

const AudioFormat& WASAPIDevice::format() const
{
    return impl().format;
}

DeviceDescriptor WASAPIDevice::descriptor() const
{
    DeviceDescriptor desc;
    desc.name = impl().info.friendlyName;
    desc.id = impl().info.deviceId;
    desc.type = impl().info.type;
    return desc;
}

}
