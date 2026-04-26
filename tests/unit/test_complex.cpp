#include <gtest/gtest.h>

#include <slk/dsp/complex.h>

using slk::dsp::Complex;
using slk::dsp::Spectrum;

TEST(Complex, ConstructAndAccess)
{
    Complex<float> c { 1.5f, -2.25f };
    EXPECT_FLOAT_EQ(c.real(), 1.5f);
    EXPECT_FLOAT_EQ(c.img(), -2.25f);
}

TEST(Complex, ConstructDouble)
{
    Complex<double> c { 3.0, 4.0 };
    EXPECT_DOUBLE_EQ(c.real(), 3.0);
    EXPECT_DOUBLE_EQ(c.img(), 4.0);
}

TEST(Complex, PlusEqualsAccumulates)
{
    Complex<float> a { 1.0f, 2.0f };
    Complex<float> b { 3.0f, 4.0f };

    a += b;

    EXPECT_FLOAT_EQ(a.real(), 4.0f);
    EXPECT_FLOAT_EQ(a.img(), 6.0f);

    // b is unchanged
    EXPECT_FLOAT_EQ(b.real(), 3.0f);
    EXPECT_FLOAT_EQ(b.img(), 4.0f);
}

TEST(Complex, ScalarMultiplyMutates)
{
    // Note: current operator* mutates the receiver and returns it. This
    // test pins down that contract; if operator* is later made non-mutating,
    // update accordingly.
    Complex<float> c { 2.0f, 3.0f };
    auto result = c * 2.5f;

    EXPECT_FLOAT_EQ(result.real(), 5.0f);
    EXPECT_FLOAT_EQ(result.img(), 7.5f);
}

TEST(Complex, ScalarMultiplyByZero)
{
    Complex<float> c { 5.0f, -5.0f };
    auto result = c * 0.0f;

    EXPECT_FLOAT_EQ(result.real(), 0.0f);
    EXPECT_FLOAT_EQ(result.img(), 0.0f);
}

TEST(Complex, SpectrumIsVectorOfComplex)
{
    Spectrum<float> spec;
    spec.emplace_back(1.0f, 0.0f);
    spec.emplace_back(0.0f, 1.0f);
    spec.emplace_back(-1.0f, -1.0f);

    EXPECT_EQ(spec.size(), 3u);
    EXPECT_FLOAT_EQ(spec[0].real(), 1.0f);
    EXPECT_FLOAT_EQ(spec[1].img(), 1.0f);
    EXPECT_FLOAT_EQ(spec[2].real(), -1.0f);
    EXPECT_FLOAT_EQ(spec[2].img(), -1.0f);
}
