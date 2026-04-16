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

struct Db
{
    float value; // stores LINEAR gain

    explicit Db(float dB) noexcept;

    [[nodiscard]] float count() const noexcept;

    auto operator<=>(const Db&) const = default;

    Db operator-() const noexcept;
    Db operator*(Db rhs) const noexcept;
    Db operator*(float s) const noexcept;
    Db operator/(float s) const noexcept;

    friend Db operator*(float s, Db d) noexcept;

    static Db fromLinear(float linear) noexcept;
};

} // namespace slk::dsp

namespace slk::dsp::literals
{

Db operator""_dB(unsigned long long dB);

Db operator""_dB(long double dB);

} // namespace slk::dsp::literals
