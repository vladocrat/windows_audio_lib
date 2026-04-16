#include <gtest/gtest.h>

#include <slk/audiobuffer.h>
#include <slk/dsp/noise.h>
#include <slk/dsp/filter.h>
#include <slk/dsp/complex.h>
#include <slk/dsp/dsp.h>
#include <slk/dsp/window.h>
#include <slk/dsp/processor.h>
#include <slk/dsp/chain.h>
#include <slk/dsp/graph.h>
#include <slk/types.h>

#include <cmath>
#include <numbers>
#include <algorithm>

using slk::dsp::Hertz;
using slk::dsp::Db;
using namespace slk::dsp::literals;

TEST(DSP, WhiteNoiseSize)
{
    auto buf = slk::dsp::whiteNoise<float>(1024, Db::fromLinear(0.5f));
    EXPECT_EQ(buf.channels(), 1u);
    EXPECT_EQ(buf.numSamples(), 1024u);
}

TEST(DSP, WhiteNoiseAmplitude)
{
    auto buf = slk::dsp::whiteNoise<float>(4096, Db::fromLinear(0.5f));

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

    slk::filter::SimpleGainFilter<float> gain(Db::fromLinear(2.0f));
    buf | gain;

    for (const auto& s : buf)
        EXPECT_NEAR(s, 1.0f, 1e-5f);
}

TEST(DSP, GainFilterMultichannel)
{
    slk::AudioBuffer<float> buf(2, 16);
    for (auto& s : buf)
        s = 0.25f;

    slk::filter::SimpleGainFilter<float> gain(Db::fromLinear(4.0f));
    buf | gain;

    for (const auto& s : buf)
        EXPECT_NEAR(s, 1.0f, 1e-5f);
}

TEST(DSP, LowPassFilter)
{
    slk::AudioBuffer<float> buf(1, 64);
    for (uint32_t i = 0; i < 64; ++i)
        buf[i] = (i % 2 == 0) ? 1.0f : -1.0f;

    slk::filter::LowPassFilter<float> lpf(1_kHz, 48_kHz);
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

    slk::filter::SimpleSoftLimiter<float> limiter(Db::fromLinear(0.9f));
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

    slk::filter::SimpleSoftLimiter<float> limiter(Db::fromLinear(0.9f));
    buf | limiter;

    for (const auto& s : buf) {
        EXPECT_GT(s, -2.0f); // compressed from -2.0
        EXPECT_LT(s, -0.9f); // still beyond threshold (symmetric with positive)
    }
}

TEST(DSP, SoftLimiterBelowThreshold)
{
    slk::AudioBuffer<float> buf(1, 16);
    for (auto& s : buf)
        s = 0.5f;

    slk::filter::SimpleSoftLimiter<float> limiter(Db::fromLinear(0.9f));
    buf | limiter;

    for (const auto& s : buf)
        EXPECT_FLOAT_EQ(s, 0.5f);
}

TEST(DSP, FilterChain)
{
    slk::AudioBuffer<float> buf(1, 32);
    for (auto& s : buf)
        s = 0.3f;

    slk::filter::SimpleGainFilter<float> gain(Db::fromLinear(3.0f));
    slk::filter::SimpleSoftLimiter<float> limiter(Db::fromLinear(0.8f));

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
    const auto sampleRate = 6400_Hz;
    constexpr size_t targetBin = 5;

    slk::AudioBuffer<float> buf(1, N);
    for (size_t i = 0; i < N; ++i) {
        float freq = static_cast<float>(targetBin) * sampleRate.count() / static_cast<float>(N);
        buf[i] = std::sin(2.0f * std::numbers::pi_v<float> * freq * static_cast<float>(i) / sampleRate.count());
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
    const auto sampleRate = 6400_Hz;

    slk::AudioBuffer<float> buf(1, N);
    for (size_t i = 0; i < N; ++i)
        buf[i] = std::sin(2.0f * std::numbers::pi_v<float> * 1000.0f * static_cast<float>(i) / sampleRate.count());

    auto spectrum = slk::dsp::dft<float>(buf, sampleRate);
    auto freqMags = slk::dsp::freqMag<float>(spectrum, sampleRate);

    EXPECT_EQ(freqMags.size(), spectrum.size());

    // Verify frequency values: bin k should map to k * sampleRate / N
    // where N = 2 * spectrum.size()
    for (size_t k = 0; k < freqMags.size(); ++k) {
        float expectedFreq = static_cast<float>(k) * sampleRate.count() / (2.0f * static_cast<float>(spectrum.size()));
        EXPECT_NEAR(freqMags[k].first, expectedFreq, 1e-2f);
    }
}

// ── FilterChain ───────────────────────────────────────────────────────────────

TEST(FilterChain, AppliesFiltersInOrder)
{
    slk::AudioBuffer<float> buf(1, 32);

    for (auto& s : buf)
        s = 0.1f;

    slk::filter::SimpleGainFilter<float> gain(Db::fromLinear(10.0f));
    slk::filter::SimpleSoftLimiter<float> limiter(Db::fromLinear(0.9f));
    auto chain = slk::dsp::makeChain<float>(gain, limiter);
    chain(buf);

    for (const auto& s : buf) {
        EXPECT_GT(s, 0.0f);
        EXPECT_LT(s, 2.0f);
    }
}

TEST(FilterChain, SatisfiesPipeOperator)
{
    slk::AudioBuffer<float> buf(1, 32);

    for (auto& s : buf)
        s = 0.5f;

    slk::filter::SimpleGainFilter<float> gain(Db::fromLinear(2.0f));
    auto chain = slk::dsp::makeChain<float>(gain);

    buf | chain;

    for (const auto& s : buf)
        EXPECT_NEAR(s, 1.0f, 1e-4f);
}

TEST(FilterChain, MultipleFilters)
{
    slk::AudioBuffer<float> buf(1, 32);

    for (auto& s : buf)
        s = 0.5f;

    slk::filter::SimpleGainFilter<float> gain1(Db::fromLinear(2.0f));
    slk::filter::SimpleGainFilter<float> gain2(Db::fromLinear(2.0f));
    auto chain = slk::dsp::makeChain<float>(gain1, gain2);
    chain(buf);

    for (const auto& s : buf)
        EXPECT_NEAR(s, 2.0f, 1e-4f);
}

// ── AudioGraph ────────────────────────────────────────────────────────────────

TEST(AudioGraph, LinearChain)
{
    slk::filter::SimpleGainFilter<float> gain(Db::fromLinear(2.0f));

    slk::dsp::AudioGraph<float> graph;
    auto h = graph.addNode(gain);
    graph.setOutput(h);
    graph.compile(1, 16);

    slk::AudioBuffer<float> buf(1, 16);

    for (auto& s : buf)
        s = 0.3f;

    graph.process(buf);

    for (const auto& s : buf)
        EXPECT_NEAR(s, 0.6f, 1e-4f);
}

TEST(AudioGraph, SourceSeesOriginalBuffer)
{
    slk::filter::SimpleGainFilter<float> gain(Db::fromLinear(2.0f));

    slk::dsp::AudioGraph<float> graph;
    auto h = graph.addNode(gain);
    graph.setOutput(h);
    graph.compile(1, 16);

    slk::AudioBuffer<float> buf(1, 16);

    for (auto& s : buf)
        s = 0.5f;

    graph.process(buf);

    for (const auto& s : buf)
        EXPECT_NEAR(s, 1.0f, 1e-4f);
}

TEST(AudioGraph, FanOutPreservesIndependentPaths)
{
    // Source fans out to gain1 and gain2.
    // Only the gain2 branch leads to the output.
    // gain1 must not corrupt gain2's buffer.
    slk::filter::SimpleGainFilter<float> gain1(Db::fromLinear(2.0f));
    slk::filter::SimpleGainFilter<float> gain2(Db::fromLinear(0.5f));

    slk::dsp::AudioGraph<float> graphA;
    auto hG1a = graphA.addNode(gain1);
    auto hG2a = graphA.addNode(gain2);

    // both read from the same source node (implicit fan-out)
    graphA.setOutput(hG2a);
    graphA.compile(1, 16);

    slk::dsp::AudioGraph<float> graphB;
    auto hG1b = graphB.addNode(gain1);
    auto hG2b = graphB.addNode(gain2);

    graphB.connect(hG1b, hG2b);
    graphB.setOutput(hG2b);
    graphB.compile(1, 16);

    slk::AudioBuffer<float> bufA(1, 16);
    slk::AudioBuffer<float> bufB(1, 16);

    for (auto& s : bufA)
        s = 0.4f;
    for (auto& s : bufB)
        s = 0.4f;

    graphA.process(bufA);
    graphB.process(bufB);

    // graphA: source(0.4) → gain2(0.5) = 0.2
    for (const auto& s : bufA)
        EXPECT_NEAR(s, 0.2f, 1e-4f);

    // graphB: source(0.4) → gain1(2.0) → gain2(0.5) = 0.4
    for (const auto& s : bufB)
        EXPECT_NEAR(s, 0.4f, 1e-4f);
}

TEST(AudioGraph, ImplicitMixSumsTwoPaths)
{
    slk::filter::SimpleGainFilter<float> gain1(Db::fromLinear(0.3f));
    slk::filter::SimpleGainFilter<float> gain2(Db::fromLinear(0.3f));
    slk::filter::SimpleGainFilter<float> output(0_dB);

    slk::dsp::AudioGraph<float> graph;
    auto hG1 = graph.addNode(gain1);
    auto hG2 = graph.addNode(gain2);
    auto hOut = graph.addNode(output);

    graph.connect(hG1, hOut);
    graph.connect(hG2, hOut);
    graph.setOutput(hOut);
    graph.compile(1, 16);

    slk::AudioBuffer<float> buf(1, 16);

    for (auto& s : buf)
        s = 1.0f;

    graph.process(buf);

    // 1.0 * 0.3 + 1.0 * 0.3 = 0.6
    for (const auto& s : buf)
        EXPECT_NEAR(s, 0.6f, 1e-4f);
}

TEST(AudioGraph, AsCallback)
{
    slk::filter::SimpleGainFilter<float> gain(Db::fromLinear(3.0f));

    slk::dsp::AudioGraph<float> graph;
    auto h = graph.addNode(gain);
    graph.setOutput(h);
    graph.compile(1, 8);

    auto cb = graph.asCallback();

    slk::AudioBuffer<float> buf(1, 8);

    for (auto& s : buf)
        s = 0.1f;

    cb(buf);

    for (const auto& s : buf)
        EXPECT_NEAR(s, 0.3f, 1e-4f);
}
