#pragma once

#include <algorithm>

#include "audiobuffer.h"

namespace slk
{
namespace filter
{

template<class T>
struct LowPassFilter
{
    float _cutOff;
    float _sampleRate;

    LowPassFilter(const float cutOff, const float sampleRate)
        : _cutOff { cutOff }
        , _sampleRate { sampleRate }
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
