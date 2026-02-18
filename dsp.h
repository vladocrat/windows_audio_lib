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
