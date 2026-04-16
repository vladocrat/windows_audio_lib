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

#include <compare>

namespace slk::dsp
{

struct Hertz
{
    float value;

    constexpr explicit Hertz(float v) noexcept : value { v }
    {
    }

    [[nodiscard]] constexpr float count() const noexcept
    {
        return value;
    }

    constexpr auto operator<=>(const Hertz&) const = default;

    constexpr Hertz operator+(Hertz rhs) const noexcept
    {
        return Hertz { value + rhs.value };
    }

    constexpr Hertz operator-(Hertz rhs) const noexcept
    {
        return Hertz { value - rhs.value };
    }

    constexpr Hertz operator*(float s) const noexcept
    {
        return Hertz { value * s };
    }

    constexpr Hertz operator/(float s) const noexcept
    {
        return Hertz { value / s };
    }

    constexpr float operator/(Hertz rhs) const noexcept
    {
        return value / rhs.value;
    }

    friend constexpr Hertz operator*(float s, Hertz h) noexcept
    {
        return Hertz { s * h.value };
    }
};

} // namespace slk::dsp

namespace slk::dsp::literals
{

constexpr Hertz operator""_Hz(unsigned long long freq)
{
    return Hertz { static_cast<float>(freq) };
}

constexpr Hertz operator""_Hz(long double freq)
{
    return Hertz { static_cast<float>(freq) };
}

constexpr Hertz operator""_kHz(unsigned long long freq)
{
    return Hertz { static_cast<float>(static_cast<double>(freq) * 1000.0) };
}

constexpr Hertz operator""_kHz(long double freq)
{
    return Hertz { static_cast<float>(freq * 1000.0) };
}

constexpr Hertz operator""_MHz(unsigned long long freq)
{
    return Hertz { static_cast<float>(static_cast<double>(freq) * 1000000.0) };
}

constexpr Hertz operator""_MHz(long double freq)
{
    return Hertz { static_cast<float>(freq * 1000000.0) };
}

} // namespace slk::dsp::literals
