#include <gtest/gtest.h>

#include <slk/types.h>

using namespace slk::dsp::literals;

// Compile-time checks
static_assert(440_Hz == 440.0f);
static_assert(48_kHz == 48000.0f);
static_assert(1_sec == 1.0f);
static_assert(500_ms == 0.5f);
static_assert(1_MHz == 1000000.0f);

// Runtime tests for GTest reporting
TEST(Literals, Hz)
{
    EXPECT_FLOAT_EQ(440_Hz, 440.0f);
    EXPECT_FLOAT_EQ(8000_Hz, 8000.0f);
}

TEST(Literals, HzFloat)
{
    EXPECT_FLOAT_EQ(440.5_Hz, 440.5f);
}

TEST(Literals, kHz)
{
    EXPECT_FLOAT_EQ(48_kHz, 48000.0f);
    EXPECT_FLOAT_EQ(5.5_kHz, 5500.0f);
}

TEST(Literals, MHz)
{
    EXPECT_FLOAT_EQ(1_MHz, 1000000.0f);
}

TEST(Literals, Sec)
{
    EXPECT_FLOAT_EQ(1_sec, 1.0f);
    EXPECT_FLOAT_EQ(5_sec, 5.0f);
    EXPECT_FLOAT_EQ(2.5_sec, 2.5f);
}

TEST(Literals, Ms)
{
    EXPECT_FLOAT_EQ(500_ms, 0.5f);
    EXPECT_FLOAT_EQ(1000_ms, 1.0f);
    EXPECT_FLOAT_EQ(100.0_ms, 0.1f);
}
