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
