#include <gtest/gtest.h>

#include <slk/types.h>

using namespace slk::dsp::literals;

// Compile-time checks — Hertz
static_assert((440_Hz).count() == 440.0f);
static_assert((48_kHz).count() == 48000.0f);
static_assert((1_MHz).count() == 1000000.0f);

// Compile-time checks — Hertz arithmetic
static_assert((440_Hz + 60_Hz).count() == 500.0f);
static_assert((1_kHz - 200_Hz).count() == 800.0f);
static_assert((440_Hz * 2.0f).count() == 880.0f);
static_assert((440_Hz / 2.0f).count() == 220.0f);
static_assert(1_kHz / 500_Hz == 2.0f);

// Runtime tests — Hertz
TEST(Literals, Hz)
{
    auto freq = 440_Hz;
    EXPECT_FLOAT_EQ(freq.count(), 440.0f);

    auto freq2 = 8000_Hz;
    EXPECT_FLOAT_EQ(freq2.count(), 8000.0f);
}

TEST(Literals, HzFloat)
{
    auto freq = 440.5_Hz;
    EXPECT_FLOAT_EQ(freq.count(), 440.5f);
}

TEST(Literals, kHz)
{
    auto freq = 48_kHz;
    EXPECT_FLOAT_EQ(freq.count(), 48000.0f);

    auto freq2 = 5.5_kHz;
    EXPECT_FLOAT_EQ(freq2.count(), 5500.0f);
}

TEST(Literals, MHz)
{
    auto freq = 1_MHz;
    EXPECT_FLOAT_EQ(freq.count(), 1000000.0f);
}

// Runtime tests — Db
TEST(Literals, Db)
{
    // 0 dB = unity gain (1.0)
    auto unity = 0_dB;
    EXPECT_FLOAT_EQ(unity.count(), 1.0f);

    // -20 dB = 0.1 linear
    EXPECT_NEAR(slk::dsp::Db(-20.0f).count(), 0.1f, 1e-5f);

    // +20 dB = 10.0 linear
    EXPECT_NEAR(slk::dsp::Db(20.0f).count(), 10.0f, 1e-5f);

    // -6 dB ≈ 0.501
    EXPECT_NEAR(slk::dsp::Db(-6.0f).count(), 0.501187f, 1e-4f);
}

TEST(Literals, DbFromLinear)
{
    auto gain = slk::dsp::Db::fromLinear(0.5f);
    EXPECT_FLOAT_EQ(gain.count(), 0.5f);
}

TEST(Literals, DbArithmetic)
{
    // Chaining two gains: 0.5 * 0.5 = 0.25
    auto a = slk::dsp::Db::fromLinear(0.5f);
    auto b = slk::dsp::Db::fromLinear(0.5f);
    EXPECT_FLOAT_EQ((a * b).count(), 0.25f);

    // Scalar multiply
    EXPECT_FLOAT_EQ((a * 2.0f).count(), 1.0f);
}

// Type safety — Hertz comparisons
TEST(Literals, HertzComparison)
{
    EXPECT_TRUE(440_Hz < 1_kHz);
    EXPECT_TRUE(48_kHz > 44100_Hz);
    EXPECT_TRUE(440_Hz == 440_Hz);
    EXPECT_TRUE(1_kHz != 999_Hz);
}
