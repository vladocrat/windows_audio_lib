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

#include <cstdint>

#include "utils.h"

namespace slk
{

class AudioFormat
{
public:
    enum class Type : uint8_t
    {
        PCM,
        FLOAT
    };

    AudioFormat();
    AudioFormat(uint16_t channels, uint32_t sampleRate, uint16_t bitsPerSample, Type audioFormat);
    ~AudioFormat();

    AudioFormat(const AudioFormat&) = delete;
    AudioFormat& operator=(const AudioFormat&) = delete;
    AudioFormat(AudioFormat&&) noexcept;
    AudioFormat& operator=(AudioFormat&&) noexcept;

    [[nodiscard]] Type type() const;
    [[nodiscard]] uint32_t sampleRate() const;
    [[nodiscard]] uint32_t channels() const;
    [[nodiscard]] uint16_t bitsPerSample() const;

private:
    DECLARE_PIMPL
};

}
