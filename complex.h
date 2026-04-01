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
