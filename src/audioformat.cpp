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

#include <slk/audioformat.h>

namespace slk
{

struct AudioFormat::impl_t
{
    uint16_t channels { 0 };
    uint32_t sampleRate { 0 };
    uint16_t bitsPerSample { 0 };
    Type type { Type::PCM };
};

AudioFormat::AudioFormat()
{
    createImpl();
}

AudioFormat::AudioFormat(uint16_t channels, uint32_t sampleRate, uint16_t bitsPerSample, Type audioFormat)
{
    createImpl();
    impl().channels = channels;
    impl().sampleRate = sampleRate;
    impl().bitsPerSample = bitsPerSample;
    impl().type = audioFormat;
}

AudioFormat::~AudioFormat()
{
}

AudioFormat::AudioFormat(AudioFormat&&) noexcept = default;
AudioFormat& AudioFormat::operator=(AudioFormat&&) noexcept = default;

AudioFormat::Type AudioFormat::type() const
{
    return impl().type;
}

uint32_t AudioFormat::sampleRate() const
{
    return impl().sampleRate;
}

uint32_t AudioFormat::channels() const
{
    return static_cast<uint32_t>(impl().channels);
}

uint16_t AudioFormat::bitsPerSample() const
{
    return impl().bitsPerSample;
}

}
