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

#include <algorithm>
#include <cmath>

#include <slk/audiobuffer.h>

namespace slk::filter
{

template <class T>
struct LowPassFilter
{
    float cutOff;
    float sampleRate;

    LowPassFilter(const float cutOff, const float sampleRate) noexcept : cutOff { cutOff }, sampleRate { sampleRate }
    {
    }

    void operator()(AudioBuffer<T>& buffer) const
    {
        const auto alpha = [&]() {
            auto temp = cutOff / sampleRate;
            temp = std::clamp(temp, 0.0f, 1.0f);
            return temp;
        }();

        T previous { 0 };

        for (auto& sample : buffer) {
            previous = alpha * sample + (1.0f - alpha) * previous;
            sample = previous;
        }
    }
};

template <class T>
struct SimpleGainFilter
{
    float gain;

    SimpleGainFilter(const float gain) noexcept : gain { gain }
    {
    }

    void operator()(AudioBuffer<T>& buffer) const
    {
        const auto numChannels = buffer.channels();
        const auto numSamples = buffer.numSamples();

        for (uint32_t i = 0; i < numSamples; ++i) {
            for (uint32_t ch = 0; ch < numChannels; ++ch) {
                buffer[i * numChannels + ch] = static_cast<T>(buffer[i * numChannels + ch] * gain);
            }
        }
    }
};

template <class T>
struct SimpleSoftLimiter
{
    float threshold;

    SimpleSoftLimiter(const float threshold) noexcept : threshold { threshold }
    {
    }

    void operator()(AudioBuffer<T>& buffer) const
    {
        for (auto& sample : buffer) {
            if (std::abs(sample) > threshold) {
                const float sign = sample >= 0 ? 1.0f : -1.0f;
                sample = static_cast<T>(sign * (threshold + std::tanh(sample - sign * threshold)));
            }
        }
    }
};

}
