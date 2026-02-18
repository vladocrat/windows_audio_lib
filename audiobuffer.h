#pragma once

#include <vector>
#include <cassert>
#include <limits>
#include <algorithm>
#include <type_traits>

#include "audioformat.h"

namespace slk
{

class AudioFormat;

template<typename SampleType>
class AudioBuffer
{
public:
    AudioBuffer() = default;

    AudioBuffer(const uint32_t numChannels, const uint32_t numSamples)
    {
        setSize(numChannels, numSamples);
    }

    AudioBuffer(AudioBuffer&& other) noexcept
        : _data(std::move(other._data))
        , _numChannels(other._numChannels)
        , _numSamples(other._numSamples)
    {
        other._numChannels = 0;
        other._numSamples = 0;
    }

    AudioBuffer& operator=(AudioBuffer&& other) noexcept
    {
        if (this != &other) {
            _data = std::move(other._data);
            _numChannels = other._numChannels;
            _numSamples = other._numSamples;
            other._numChannels = 0;
            other._numSamples = 0;
        }
        return *this;
    }

    AudioBuffer(const AudioBuffer&) = delete;
    AudioBuffer& operator=(const AudioBuffer&) = delete;

    template<typename Filter>
    AudioBuffer& operator|(Filter&& filter) {
        filter(*this);
        return *this;
    }

    void setSize(const uint32_t numChannels, const uint32_t numSamples)
    {
        assert(numChannels > 0 && numSamples > 0);

        _numChannels = numChannels;
        _numSamples = numSamples;

        _data.resize(_numChannels * _numSamples);
        clear();
    }

    void clear()
    {
        std::fill(_data.begin(), _data.end(), SampleType{});
    }

    template<class TargetType>
    AudioBuffer<TargetType> to(const AudioFormat& sourceFormat) const
    {
        const size_t bytesPerSample = sourceFormat.format()->wBitsPerSample / 8;
        const size_t totalSamples = _data.size() / bytesPerSample;
        const size_t numFrames = totalSamples / _numChannels;

        auto readSample = [this, &sourceFormat](size_t i) -> double {
            if (sourceFormat.type() == AudioFormat::Type::FLOAT) {
                return reinterpret_cast<const float*>(_data.data())[i];
            }

            switch (sourceFormat.format()->wBitsPerSample) {
            case 16: {
                const auto* first = reinterpret_cast<const int16_t*>(_data.data());
                return first[i] / 32767.0;
            }
            case 24: {
                const auto* src = reinterpret_cast<const uint8_t*>(_data.data());
                int32_t sample = src[i * 3] | (src[i * 3 + 1] << 8) | (src[i * 3 + 2] << 16);
                if (sample & 0x800000) sample |= static_cast<int32_t>(0xFF000000);
                return sample / 8388608.0;
            }
            case 32: {
                const auto* first = reinterpret_cast<const int32_t*>(_data.data());
                return first[i] / 2147483647.0;
            }
            default:
                return 0.0;
            }
        };

        auto toTarget = [](double sample) -> TargetType {
            sample = (std::clamp)(sample, -1.0, 1.0);

            if constexpr (std::is_same_v<TargetType, float>) {
                return static_cast<float>(sample);
            }
            else if constexpr (std::is_same_v<TargetType, double>) {
                return sample;
            }
            else if constexpr (std::is_same_v<TargetType, int16_t>) {
                return static_cast<int16_t>(sample * 32767.0);
            }
            else if constexpr (std::is_same_v<TargetType, int32_t>) {
                return static_cast<int32_t>(sample * 2147483647.0);
            }
            else if constexpr (std::is_same_v<TargetType, uint8_t>) {
                return static_cast<uint8_t>((sample + 1.0) * 127.5);
            }
        };

        AudioBuffer<TargetType> result(_numChannels, numFrames);

        size_t i { 0 };
        std::generate(result.begin(), result.end(), [&]() {
            return toTarget(readSample(i++));
        });

        return result;
    }

    std::vector<SampleType>& data()
    {
        return _data;
    }

    const std::vector<SampleType>& data() const
    {
        return _data;
    }

    auto begin() { return _data.begin(); }
    auto end() { return _data.end(); }
    auto begin() const { return _data.begin(); }
    auto end() const { return _data.end(); }
    auto size() const { return _data.size(); }
    auto channels() const { return _numChannels; }
    auto numSamples() const { return _numSamples; }

    SampleType operator[](const size_t ix) const { return _data[ix]; }

    SampleType& operator[](const size_t ix) { return _data[ix]; }

private:
    template<typename U>
    friend class AudioBuffer;

    std::vector<SampleType> _data;
    uint32_t _numChannels { 0 };
    uint32_t _numSamples { 0 };
};

}
