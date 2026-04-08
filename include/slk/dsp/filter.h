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

#include <slk/audiobuffer.h>

namespace slk
{
namespace filter
{

template <class T>
struct LowPassFilter
{
    float _cutOff;
    float _sampleRate;

    LowPassFilter(const float cutOff, const float sampleRate) : _cutOff { cutOff }, _sampleRate { sampleRate }
    {
    }

    void operator()(AudioBuffer<T>& buffer) const
    {
        const auto alpha = [&]() {
            auto temp = _cutOff / _sampleRate;
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

}
}
