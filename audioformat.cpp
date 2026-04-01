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

#include "audioformat.h"

#include <initguid.h>
#include <mmdeviceapi.h>

namespace slk
{

struct AudioFormat::impl_t
{
    WAVEFORMATEX* format { nullptr };
    Type type;

    ~impl_t()
    {
        if (format) {
            CoTaskMemFree(format);
        }
    }
};

AudioFormat::AudioFormat()
{
    createImpl();
}

AudioFormat::AudioFormat(uint16_t channels, uint32_t sampleRate, uint16_t bitsPerSample, Type audioFormat)
{
    createImpl();

    const auto format = [audioFormat]() {
        switch (audioFormat) {
        case Type::PCM:
            return KSDATAFORMAT_SUBTYPE_PCM;
        case Type::FLOAT:
            return KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
        }
    };

    auto* fmt = static_cast<WAVEFORMATEXTENSIBLE*>(CoTaskMemAlloc(sizeof(WAVEFORMATEXTENSIBLE)));
    memset(fmt, 0, sizeof(WAVEFORMATEXTENSIBLE));

    fmt->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    fmt->Format.nChannels = channels;
    fmt->Format.nSamplesPerSec = sampleRate;
    fmt->Format.wBitsPerSample = bitsPerSample;
    fmt->Format.nBlockAlign = (channels * bitsPerSample) / 8;
    fmt->Format.nAvgBytesPerSec = sampleRate * fmt->Format.nBlockAlign;
    fmt->Format.cbSize = 22;

    fmt->Samples.wValidBitsPerSample = bitsPerSample;
    fmt->SubFormat = format();
    impl().type = audioFormat;

    if (channels == 1) {
        fmt->dwChannelMask = SPEAKER_FRONT_CENTER;
    } else if (channels == 2) {
        fmt->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
    } else {
        fmt->dwChannelMask = (1 << channels) - 1;
    }

    impl().format = reinterpret_cast<WAVEFORMATEX*>(fmt);
}


AudioFormat::~AudioFormat()
{

}

void AudioFormat::setFormat(IAudioClient* const client)
{
    WAVEFORMATEX* format;
    client->GetMixFormat(&format);

    WAVEFORMATEXTENSIBLE floatFormat;
    floatFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    floatFormat.Format.nChannels = format->nChannels;
    floatFormat.Format.nSamplesPerSec = format->nSamplesPerSec;
    floatFormat.Format.wBitsPerSample = 32;
    floatFormat.Format.nBlockAlign = (floatFormat.Format.nChannels * 32) / 8;
    floatFormat.Format.nAvgBytesPerSec = floatFormat.Format.nSamplesPerSec * floatFormat.Format.nBlockAlign;
    floatFormat.Format.cbSize = 22;
    floatFormat.Samples.wValidBitsPerSample = 32;
    floatFormat.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;

    if (format->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        floatFormat.dwChannelMask = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(format)->dwChannelMask;
    } else {
        floatFormat.dwChannelMask = (format->nChannels == 2) ?
                                        (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT) : SPEAKER_FRONT_CENTER;
    }

    WAVEFORMATEX* closestMatch { nullptr };
    HRESULT hr = client->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, reinterpret_cast<WAVEFORMATEX*>(&floatFormat), &closestMatch);

    if (hr == S_OK) {
        impl().format = static_cast<WAVEFORMATEX*>(CoTaskMemAlloc(sizeof(WAVEFORMATEXTENSIBLE)));
        memcpy(impl().format, &floatFormat, sizeof(WAVEFORMATEXTENSIBLE));
    } else {
        impl().format = format;
    }

    if (closestMatch) {
        CoTaskMemFree(closestMatch);
    }

    impl().type = Type::FLOAT;
}

AudioFormat::Type AudioFormat::type() const
{
    return impl().type;
}

void AudioFormat::toFloat()
{
    if (!impl().format) {
        return;
    }

    auto* fmt = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(impl().format);

    if (fmt->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
        fmt->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT &&
        fmt->Format.wBitsPerSample == 32) {
        return;
    }

    uint16_t channels = fmt->Format.nChannels;
    uint32_t sampleRate = fmt->Format.nSamplesPerSec;
    DWORD channelMask = fmt->dwChannelMask;

    fmt->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    fmt->Format.wBitsPerSample = 32;
    fmt->Format.nBlockAlign = (channels * 32) / 8;
    fmt->Format.nAvgBytesPerSec = sampleRate * fmt->Format.nBlockAlign;
    fmt->Format.cbSize = 22;
    fmt->Samples.wValidBitsPerSample = 32;
    fmt->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    fmt->dwChannelMask = channelMask;

    impl().type = Type::FLOAT;
}

const WAVEFORMATEX* const AudioFormat::format() const
{
    return impl().format;
}

uint32_t AudioFormat::channels() const
{
    if (!impl().format) {
        return 0;
    }

    return static_cast<uint32_t>(impl().format->nChannels);
}

}
