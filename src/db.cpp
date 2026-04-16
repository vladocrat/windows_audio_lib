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

#include <slk/dsp/db.h>

#include <cmath>

namespace slk::dsp
{

Db::Db(float dB) noexcept : value { std::pow(10.0f, dB / 20.0f) }
{
}

float Db::count() const noexcept
{
    return value;
}

Db Db::operator*(Db rhs) const noexcept
{
    return fromLinear(value * rhs.value);
}

Db Db::operator*(float s) const noexcept
{
    return fromLinear(value * s);
}

Db Db::operator/(float s) const noexcept
{
    return fromLinear(value / s);
}

Db operator*(float s, Db d) noexcept
{
    return Db::fromLinear(s * d.value);
}

Db Db::fromLinear(float linear) noexcept
{
    Db d(0.0f);
    d.value = linear;
    return d;
}

} // namespace slk::dsp

namespace slk::dsp::literals
{

Db operator""_dB(unsigned long long dB)
{
    return Db { static_cast<float>(dB) };
}

Db operator""_dB(long double dB)
{
    return Db { static_cast<float>(dB) };
}

} // namespace slk::dsp::literals
