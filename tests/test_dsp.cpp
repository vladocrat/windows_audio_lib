#include <gtest/gtest.h>

#include <slk/audiobuffer.h>
#include <slk/dsp/noise.h>
#include <slk/dsp/filter.h>
#include <slk/dsp/complex.h>
#include <slk/dsp/dsp.h>
#include <slk/dsp/window.h>

#include <cmath>
#include <numbers>
#include <algorithm>

TEST(DSP, WhiteNoiseSize)
{
    auto buf = slk::dsp::whiteNoise<float>(1024, 0.5f);
    EXPECT_EQ(buf.channels(), 1u);
    EXPECT_EQ(buf.numSamples(), 1024u);
}

TEST(DSP, WhiteNoiseAmplitude)
{
    auto buf = slk::dsp::whiteNoise<float>(4096, 0.5f);

    for (size_t i = 0; i < buf.size(); ++i) {
        EXPECT_GE(buf[i], -0.5f);
        EXPECT_LE(buf[i], 0.5f);
    }
}

TEST(DSP, GainFilter)
{
    slk::AudioBuffer<float> buf(1, 32);
    for (auto& s : buf)
        s = 0.5f;

    slk::filter::SimpleGainFilter<float> gain(2.0f);
    buf | gain;

    for (const auto& s : buf)
        EXPECT_NEAR(s, 1.0f, 1e-5f);
}

TEST(DSP, GainFilterMultichannel)
{
    slk::AudioBuffer<float> buf(2, 16);
    for (auto& s : buf)
        s = 0.25f;

    slk::filter::SimpleGainFilter<float> gain(4.0f);
    buf | gain;

    for (const auto& s : buf)
        EXPECT_NEAR(s, 1.0f, 1e-5f);
}

TEST(DSP, LowPassFilter)
{
    slk::AudioBuffer<float> buf(1, 64);
    for (uint32_t i = 0; i < 64; ++i)
        buf[i] = (i % 2 == 0) ? 1.0f : -1.0f;

    slk::filter::LowPassFilter<float> lpf(1000.0f, 48000.0f);
    buf | lpf;

    float maxAbs = 0.0f;
    for (const auto& s : buf)
        maxAbs = std::max(maxAbs, std::abs(s));

    EXPECT_LT(maxAbs, 1.0f);
}

TEST(DSP, SoftLimiter)
{
    slk::AudioBuffer<float> buf(1, 16);
    for (auto& s : buf)
        s = 2.0f;

    slk::filter::SimpleSoftLimiter<float> limiter(0.9f);
    buf | limiter;

    for (const auto& s : buf) {
        EXPECT_LT(s, 2.0f);
        EXPECT_GT(s, 0.9f);
    }
}

TEST(DSP, SoftLimiterNegative)
{
    slk::AudioBuffer<float> buf(1, 16);
    for (auto& s : buf)
        s = -2.0f;

    slk::filter::SimpleSoftLimiter<float> limiter(0.9f);
    buf | limiter;

    for (const auto& s : buf) {
        EXPECT_GT(s, -2.0f);  // compressed from -2.0
        EXPECT_LT(s, -0.9f);  // still beyond threshold (symmetric with positive)
    }
}

TEST(DSP, SoftLimiterBelowThreshold)
{
    slk::AudioBuffer<float> buf(1, 16);
    for (auto& s : buf)
        s = 0.5f;

    slk::filter::SimpleSoftLimiter<float> limiter(0.9f);
    buf | limiter;

    for (const auto& s : buf)
        EXPECT_FLOAT_EQ(s, 0.5f);
}

TEST(DSP, FilterChain)
{
    slk::AudioBuffer<float> buf(1, 32);
    for (auto& s : buf)
        s = 0.3f;

    slk::filter::SimpleGainFilter<float> gain(3.0f);
    slk::filter::SimpleSoftLimiter<float> limiter(0.8f);

    buf | gain | limiter;

    for (const auto& s : buf) {
        EXPECT_GT(s, 0.0f);
        EXPECT_LT(s, 1.5f);
    }
}

TEST(DSP, Magnitude)
{
    slk::dsp::Complex<float> c(3.0f, 4.0f);
    EXPECT_NEAR(slk::dsp::magnitude(c), 5.0f, 1e-5f);
}

TEST(DSP, MagnitudeZero)
{
    slk::dsp::Complex<float> c(0.0f, 0.0f);
    EXPECT_NEAR(slk::dsp::magnitude(c), 0.0f, 1e-5f);
}

TEST(DSP, HannWindow)
{
    slk::Window<slk::WindowType::Hann, float, 64> window;

    // Endpoints should be near 0
    EXPECT_NEAR(window[0], 0.0f, 1e-3f);
    EXPECT_NEAR(window[63], 0.0f, 0.1f);

    // Midpoint should be near 1
    EXPECT_NEAR(window[32], 1.0f, 1e-3f);
}

TEST(DSP, HannWindowApply)
{
    constexpr size_t N = 64;
    slk::Window<slk::WindowType::Hann, float, N> window;

    std::vector<float> source(N, 1.0f);
    std::vector<float> dest(N, 0.0f);

    window.apply(source, dest);

    // Endpoints near 0 (1.0 * ~0 = ~0)
    EXPECT_NEAR(dest[0], 0.0f, 1e-3f);
    // Midpoint near 1 (1.0 * ~1 = ~1)
    EXPECT_NEAR(dest[32], 1.0f, 1e-3f);
}

TEST(DSP, DFTPeakBin)
{
    // Generate a pure sine at bin-aligned frequency.
    // DFT angle = -2*pi*k*n/N, so bin k corresponds to freq = k * sampleRate / N.
    // N=64 samples, sampleRate=6400 Hz, bin k=5 -> freq = 5 * 6400 / 64 = 500 Hz
    constexpr size_t N = 64;
    constexpr float sampleRate = 6400.0f;
    constexpr size_t targetBin = 5;

    slk::AudioBuffer<float> buf(1, N);
    for (size_t i = 0; i < N; ++i) {
        float freq = static_cast<float>(targetBin) * sampleRate / static_cast<float>(N);
        buf[i] = std::sin(2.0f * std::numbers::pi_v<float> * freq * static_cast<float>(i) / sampleRate);
    }

    auto spectrum = slk::dsp::dft<float>(buf, sampleRate);
    EXPECT_EQ(spectrum.size(), N / 2);

    // Find peak bin
    size_t peakBin = 0;
    float peakMag = 0.0f;
    for (size_t k = 0; k < spectrum.size(); ++k) {
        float mag = slk::dsp::magnitude(spectrum[k]);
        if (mag > peakMag) {
            peakMag = mag;
            peakBin = k;
        }
    }

    EXPECT_EQ(peakBin, targetBin);
}

TEST(DSP, FreqMag)
{
    constexpr size_t N = 64;
    constexpr float sampleRate = 6400.0f;

    slk::AudioBuffer<float> buf(1, N);
    for (size_t i = 0; i < N; ++i)
        buf[i] = std::sin(2.0f * std::numbers::pi_v<float> * 1000.0f * static_cast<float>(i) / sampleRate);

    auto spectrum = slk::dsp::dft<float>(buf, sampleRate);
    auto freqMags = slk::dsp::freqMag<float>(spectrum, sampleRate);

    EXPECT_EQ(freqMags.size(), spectrum.size());

    // Verify frequency values: bin k should map to k * sampleRate / N
    // where N = 2 * spectrum.size()
    for (size_t k = 0; k < freqMags.size(); ++k) {
        float expectedFreq = static_cast<float>(k) * sampleRate / (2.0f * static_cast<float>(spectrum.size()));
        EXPECT_NEAR(freqMags[k].first, expectedFreq, 1e-2f);
    }
}
