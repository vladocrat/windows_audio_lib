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
#include <slk/types.h>

namespace slk::filter
{

template <class T>
struct LowPassFilter
{
    dsp::Hertz cutOff;
    dsp::Hertz sampleRate;

    LowPassFilter(const dsp::Hertz cutOff, const dsp::Hertz sampleRate) noexcept
        : cutOff { cutOff }, sampleRate { sampleRate }
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
            previous = (alpha * sample) + ((1.0f - alpha) * previous);
            sample = previous;
        }
    }
};

template <class T>
struct SimpleGainFilter
{
    dsp::Db gain;

    SimpleGainFilter(const dsp::Db gain) noexcept : gain { gain }
    {
    }

    void operator()(AudioBuffer<T>& buffer) const
    {
        const auto numChannels = buffer.channels();
        const auto numSamples = buffer.numSamples();
        const auto linear = gain.count();

        for (uint32_t i = 0; i < numSamples; ++i) {
            for (uint32_t ch = 0; ch < numChannels; ++ch) {
                buffer[(i * numChannels) + ch] = static_cast<T>(buffer[(i * numChannels) + ch] * linear);
            }
        }
    }
};

template <class T>
struct SimpleSoftLimiter
{
    dsp::Db threshold;

    SimpleSoftLimiter(const dsp::Db threshold) noexcept : threshold { threshold }
    {
    }

    void operator()(AudioBuffer<T>& buffer) const
    {
        const auto linear = threshold.count();

        for (auto& sample : buffer) {
            if (std::abs(sample) > linear) {
                const float sign = sample >= 0 ? 1.0f : -1.0f;
                sample = static_cast<T>(sign * (linear + std::tanh(std::abs(sample) - linear)));
            }
        }
    }
};

}
