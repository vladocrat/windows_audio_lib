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

#include <random>

#include "audiobuffer.h"

namespace slk
{
namespace dsp
{

template<class SampleType>
AudioBuffer<SampleType> whiteNoise(const size_t numSamples, const SampleType amplitude = 1.0f)
{
    AudioBuffer<SampleType> buffer(1, numSamples);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<SampleType> dist(-amplitude, amplitude);

    for (size_t i = 0; i < numSamples; i++) {
        buffer[i] = dist(gen);
    }

    return buffer;
}

}
}
