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

#include <cstddef>

namespace slk::dsp::iterals
{

constexpr float operator""_Hz(unsigned long long freq)
{
    return static_cast<float>(freq);
}

constexpr float operator""_Hz(long double freq)
{
    return static_cast<float>(freq);
}

constexpr float operator""_kHz(unsigned long long freq)
{
    return static_cast<float>(freq * 1000.0);
}

constexpr float operator""_kHz(long double freq)
{
    return static_cast<float>(freq * 1000.0);
}

constexpr float operator""_MHz(unsigned long long freq)
{
    return static_cast<float>(freq * 1000000.0);
}

constexpr float operator""_MHz(long double freq)
{
    return static_cast<float>(freq * 1000000.0);
}

constexpr float operator""_sec(unsigned long long time)
{
    return static_cast<float>(time);
}

constexpr float operator""_sec(long double time)
{
    return static_cast<float>(time);
}

constexpr float operator""_ms(unsigned long long time)
{
    return static_cast<float>(time / 1000.0);
}

constexpr float operator""_ms(long double time)
{
    return static_cast<float>(time / 1000.0);
}
}
