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

namespace slk
{
namespace dsp
{

template<class Type>
class Complex
{
public:
    Complex(Type real, Type img)
        : _real { real }
        , _img { img }
    {

    }

    Type real() const { return _real; }
    Type img() const { return _img; }

    Complex<Type> operator+(const Complex<Type>& other)
    {
        _real += other._real;
        _img += other._img;
        return *this;
    }

    Complex<Type>& operator+=(const Complex<Type>& other)
    {
        _real += other._real;
        _img += other._img;
        return *this;
    }

    Complex<Type> operator*(const Type other)
    {
        _real *= other;
        _img *= other;
        return *this;
    }

private:
    Type _real;
    Type _img;
};


template<class Type>
using Spectrum = std::vector<Complex<Type>>;

}
}
