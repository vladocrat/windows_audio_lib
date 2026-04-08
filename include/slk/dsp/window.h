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

#include <array>
#include <span>
#include <numbers>
#include <cmath>
#include <algorithm>

namespace slk
{

enum class WindowType : uint8_t
{
    Hann = 0,
    FlatTop = 1
};

struct Hann
{
    //! Can't make constexpr due to std::cos not being constexpr till c++ 26
    template<class T, size_t N>
    static std::array<T, N> generate()
    {
        std::array<T, N> array;
        size_t i { 0 };

        std::generate(array.begin(), array.end(), [&i]() {
            return static_cast<T>(0.5 * (1.0 - std::cos(2.0 * std::numbers::pi_v<T> * (i++) / N)));
        });

        return array;
    }
};

template<WindowType Type, class SampleType, size_t windowSize>
struct Window
{
    constexpr Window() = default;

    void apply(std::span<SampleType> source, std::span<SampleType> dest) const
    {
        size_t i { 0 };
        std::transform(source.begin(), source.end(), dest.begin(), [&](const auto value) {
            return value * coefficients[i++];
        });
    }

    constexpr SampleType operator[](const size_t ix) const
    {
        return coefficients[ix];
    }

private:
    static std::array<SampleType, windowSize> generateCoefficients()
    {
        if constexpr (Type == WindowType::Hann) {
            return Hann::generate<SampleType, windowSize>();
        } else if constexpr (Type == WindowType::FlatTop) {
            return {};
        }
    }

    inline static std::array<SampleType, windowSize> coefficients = generateCoefficients();
};




}
