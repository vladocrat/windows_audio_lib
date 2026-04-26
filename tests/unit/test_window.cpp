#include <gtest/gtest.h>

#include <slk/dsp/window.h>

#include <array>
#include <cmath>
#include <numbers>
#include <vector>

using slk::Hann;
using slk::Window;
using slk::WindowType;

TEST(Window, HannGenerateZeroAtEdges)
{
    constexpr size_t N = 16;
    auto coeffs = Hann::generate<float, N>();

    EXPECT_EQ(coeffs.size(), N);
    EXPECT_NEAR(coeffs[0], 0.0f, 1e-6f);
    // Implementation walks 0..N-1 — last coefficient is therefore close to,
    // but not exactly, zero (cos(2π*(N-1)/N) ≠ 1). Pin it down at ~0.
    EXPECT_LT(coeffs[N - 1], 0.05f);
}

TEST(Window, HannPeaksAtCenter)
{
    constexpr size_t N = 32;
    auto coeffs = Hann::generate<float, N>();

    // For Hann, max coefficient should be 1.0 at the center (i = N/2).
    EXPECT_NEAR(coeffs[N / 2], 1.0f, 1e-5f);
}

TEST(Window, HannSymmetry)
{
    constexpr size_t N = 64;
    auto coeffs = Hann::generate<double, N>();

    for (size_t i = 1; i < N / 2; ++i) {
        // Hann is symmetric around N/2: w[i] == w[N-i].
        EXPECT_NEAR(coeffs[i], coeffs[N - i], 1e-9);
    }
}

TEST(Window, HannMatchesFormula)
{
    constexpr size_t N = 8;
    auto coeffs = Hann::generate<double, N>();

    for (size_t i = 0; i < N; ++i) {
        const double expected =
            0.5 * (1.0 - std::cos(2.0 * std::numbers::pi_v<double> * i / N));
        EXPECT_NEAR(coeffs[i], expected, 1e-12) << "at index " << i;
    }
}

TEST(Window, IndexingOperator)
{
    constexpr size_t N = 16;
    Window<WindowType::Hann, float, N> win;
    auto coeffs = Hann::generate<float, N>();

    for (size_t i = 0; i < N; ++i) {
        EXPECT_FLOAT_EQ(win[i], coeffs[i]);
    }
}

TEST(Window, ApplyMultipliesPointwise)
{
    constexpr size_t N = 8;
    Window<WindowType::Hann, float, N> win;

    std::array<float, N> input;
    std::array<float, N> output;
    input.fill(1.0f);

    win.apply(input, output);

    for (size_t i = 0; i < N; ++i) {
        EXPECT_FLOAT_EQ(output[i], win[i]);
    }
}

TEST(Window, ApplyWithNonUnitInput)
{
    constexpr size_t N = 8;
    Window<WindowType::Hann, float, N> win;

    std::array<float, N> input;
    std::array<float, N> output;
    for (size_t i = 0; i < N; ++i) input[i] = 2.0f;

    win.apply(input, output);

    for (size_t i = 0; i < N; ++i) {
        EXPECT_NEAR(output[i], 2.0f * win[i], 1e-6f);
    }
}

TEST(Window, FlatTopReturnsAllZeroes)
{
    // FlatTop is currently a stub returning a zero-initialised array.
    // This pins the current behaviour so that an actual implementation
    // forces a deliberate test update.
    constexpr size_t N = 8;
    Window<WindowType::FlatTop, float, N> win;

    for (size_t i = 0; i < N; ++i) {
        EXPECT_FLOAT_EQ(win[i], 0.0f);
    }
}
