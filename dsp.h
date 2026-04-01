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

#include <numbers>
#include <cmath>

#include "audiobuffer.h"
#include "complex.h"

namespace slk
{

namespace dsp
{

template<class Type>
using FrequencyMagnitude = std::pair<Type, Type>;

template<class Type>
using FreqMags = std::vector<FrequencyMagnitude<Type>>;

template<class T>
T magnitude(const Complex<T>& sample)
{
    return std::sqrt(std::pow(sample.real(), 2) + std::pow(sample.img(), 2));
}

template<class SampleType>
Spectrum<SampleType> dft(const AudioBuffer<float>& buffer, const float sampleRate)
{
    const auto N = buffer.size();

    Spectrum<SampleType> ret;
    ret.reserve(N / 2 + 1);

    for (size_t k = 0; k < N / 2; k++) {
        Complex<SampleType> value(0, 0);

        for (size_t n = 0; n < N; n++) {
            const auto angle =  -2.0f * std::numbers::pi * k * n / N;

            const auto real = std::cos(angle);
            const auto img = std::sin(angle);
            value += Complex<SampleType>(real, img) * buffer[n];
        }

        ret.push_back(value);
    }

    return ret;
}

template<class Type>
FreqMags<Type> freqMag(const Spectrum<Type>& spectrum, const float sampleRate)
{
    FreqMags<Type> ret;
    ret.reserve(spectrum.size());

    for (size_t k = 0; k < spectrum.size(); k++) {
        const auto freq = k * sampleRate / spectrum.size();
        const auto mag = magnitude(spectrum[k]);
        ret.emplace_back(std::make_pair(freq, mag));
    }

    return ret;
}

}
}
