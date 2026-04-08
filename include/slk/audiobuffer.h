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

#include <vector>
#include <cassert>
#include <algorithm>

namespace slk
{

class AudioFormat;

template <typename SampleType>
class AudioBuffer
{
public:
    AudioBuffer() = default;

    AudioBuffer(const uint32_t numChannels, const uint32_t numSamples)
    {
        setSize(numChannels, numSamples);
    }

    AudioBuffer(AudioBuffer&& other) noexcept
        : _data(std::move(other._data)), _numChannels(other._numChannels), _numSamples(other._numSamples)
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

    AudioBuffer(const AudioBuffer& other)
        : _data(other._data), _numChannels(other._numChannels), _numSamples(other._numSamples)
    {
    }

    AudioBuffer& operator=(const AudioBuffer& other)
    {
        if (this != &other) {
            _data = other._data;
            _numChannels = other._numChannels;
            _numSamples = other._numSamples;
        }
        return *this;
    }

    template <typename Filter>
    AudioBuffer& operator|(Filter&& filter)
    {
        filter(*this);
        return *this;
    }

    AudioBuffer& operator+=(const std::vector<const AudioBuffer<SampleType>*>& sources)
    {
        std::vector<SampleType> acc(_data.size(), 0.0);

        for (const auto* src : sources) {
            assert(src->channels() == _numChannels && src->numSamples() == _numSamples);
            std::transform(acc.begin(), acc.end(), src->data().begin(), acc.begin(), std::plus<SampleType> {});
        }

        std::transform(acc.begin(), acc.end(), _data.begin(), [](SampleType s) {
            return static_cast<SampleType>(std::clamp(s, -1.0, 1.0));
        });

        return *this;
    }

    AudioBuffer operator+(const std::vector<const AudioBuffer<SampleType>*>& sources) const
    {
        AudioBuffer out(*this);
        out += sources;
        return out;
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
        std::fill(_data.begin(), _data.end(), SampleType {});
    }

    AudioBuffer<SampleType> mono() const
    {
        if (_numChannels == 1) {
            return *this;
        }

        AudioBuffer<SampleType> result(1, _numSamples);

        for (uint32_t i = 0; i < _numSamples; ++i) {
            double mixed { 0.0 };

            for (uint32_t ch = 0; ch < _numChannels; ++ch) {
                mixed += static_cast<double>(_data[i * _numChannels + ch]);
            }

            result[i] = static_cast<SampleType>(mixed / _numChannels);
        }

        return result;
    }

    AudioBuffer<SampleType> stereo() const
    {
        if (_numChannels == 2) {
            return *this;
        }

        AudioBuffer<SampleType> result(2, _numSamples);

        if (_numChannels == 1)
            [[likely]]
            {
                for (uint32_t i = 0; i < _numSamples; ++i) {
                    result[i * 2] = _data[i];
                    result[i * 2 + 1] = _data[i];
                }

                return result;
            }

        for (uint32_t i = 0; i < _numSamples; ++i) {
            double left { 0.0 }, right { 0.0 };
            uint32_t leftCount { 0 }, rightCount { 0 };

            for (uint32_t ch = 0; ch < _numChannels; ++ch) {
                if (ch % 2 == 0)
                    [[likely]]
                    {
                        left += static_cast<double>(_data[i * _numChannels + ch]);
                        ++leftCount;
                    }
                else {
                    right += static_cast<double>(_data[i * _numChannels + ch]);
                    ++rightCount;
                }
            }

            result[i * 2] = static_cast<SampleType>(left / leftCount);
            result[i * 2 + 1] = static_cast<SampleType>(right / rightCount);
        }

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

    auto begin()
    {
        return _data.begin();
    }
    auto end()
    {
        return _data.end();
    }
    auto begin() const
    {
        return _data.begin();
    }
    auto end() const
    {
        return _data.end();
    }
    auto size() const
    {
        return _data.size();
    }
    auto channels() const
    {
        return _numChannels;
    }
    auto numSamples() const
    {
        return _numSamples;
    }

    SampleType operator[](const size_t ix) const
    {
        return _data[ix];
    }

    SampleType& operator[](const size_t ix)
    {
        return _data[ix];
    }

private:
    template <typename U>
    friend class AudioBuffer;

    std::vector<SampleType> _data;
    uint32_t _numChannels { 0 };
    uint32_t _numSamples { 0 };
};

}
