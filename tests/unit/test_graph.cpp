#include <gtest/gtest.h>

#include <slk/audiobuffer.h>
#include <slk/dsp/graph.h>
#include <slk/dsp/processor.h>

#include <vector>

using slk::dsp::AudioGraph;

namespace {

constexpr uint32_t kChannels = 1;
constexpr uint32_t kSamples  = 8;

// Multiplies every sample by `gain`.
struct Gain
{
    float gain;
    void operator()(slk::AudioBuffer<float>& buf)
    {
        for (auto& s : buf) s *= gain;
    }
};

// Identity (no-op processor).
struct Passthrough
{
    void operator()(slk::AudioBuffer<float>&) {}
};

// Input values stay well below the [-1, 1] saturation limit imposed by
// AudioBuffer::operator+= (used by the graph's mixing path). Tests that
// rely on the post-clamp value should pick magnitudes accordingly.
slk::AudioBuffer<float> makeBuffer(float value = 0.1f)
{
    slk::AudioBuffer<float> buf(kChannels, kSamples);
    for (auto& s : buf) s = value;
    return buf;
}

} // namespace

TEST(AudioGraph, SingleNodePassthrough)
{
    AudioGraph<float> g;
    auto a = g.addNode(Passthrough {});
    g.setOutput(a);

    ASSERT_TRUE(g.compile(kChannels, kSamples));

    auto buf = makeBuffer(0.5f);
    g.process(buf);

    for (auto& s : buf) EXPECT_FLOAT_EQ(s, 0.5f);
}

TEST(AudioGraph, GainProcessorScalesSamples)
{
    AudioGraph<float> g;
    auto a = g.addNode(Gain { 1.5f });
    g.setOutput(a);

    ASSERT_TRUE(g.compile(kChannels, kSamples));

    auto buf = makeBuffer(0.4f);
    g.process(buf);

    for (auto& s : buf) EXPECT_FLOAT_EQ(s, 0.6f);
}

TEST(AudioGraph, LinearChainComposesGains)
{
    // A (×0.5) -> B (×1.4) -> output  →  ×0.7
    AudioGraph<float> g;
    auto a = g.addNode(Gain { 0.5f });
    auto b = g.addNode(Gain { 1.4f });
    g.connect(a, b);
    g.setOutput(b);

    ASSERT_TRUE(g.compile(kChannels, kSamples));

    auto buf = makeBuffer(0.5f);
    g.process(buf);

    for (auto& s : buf) EXPECT_FLOAT_EQ(s, 0.35f);
}

TEST(AudioGraph, MixingTwoSourcesSums)
{
    // A (×0.4) ↘
    //            sum (passthrough) -> output  →  0.4 + 0.5 = 0.9 (no clamp)
    // B (×0.5) ↗
    //
    // operator+= on AudioBuffer clamps results to [-1, 1] (audio
    // saturation), so picked magnitudes that stay in range.
    AudioGraph<float> g;
    auto a   = g.addNode(Gain { 0.4f });
    auto b   = g.addNode(Gain { 0.5f });
    auto sum = g.addNode(Passthrough {});
    g.connect(a, sum);
    g.connect(b, sum);
    g.setOutput(sum);

    ASSERT_TRUE(g.compile(kChannels, kSamples));

    auto buf = makeBuffer(1.0f);
    g.process(buf);

    for (auto& s : buf) EXPECT_FLOAT_EQ(s, 0.9f);
}

TEST(AudioGraph, MixerSaturatesAtPlusOne)
{
    // Pin the AudioBuffer::operator+= clamp: 0.6 + 0.6 = 1.2 → clamps to 1.0.
    // If the clamp is ever lifted, this test forces the maintainer to
    // reconsider downstream consumers.
    AudioGraph<float> g;
    auto a   = g.addNode(Gain { 0.6f });
    auto b   = g.addNode(Gain { 0.6f });
    auto sum = g.addNode(Passthrough {});
    g.connect(a, sum);
    g.connect(b, sum);
    g.setOutput(sum);

    ASSERT_TRUE(g.compile(kChannels, kSamples));

    auto buf = makeBuffer(1.0f);
    g.process(buf);

    for (auto& s : buf) EXPECT_FLOAT_EQ(s, 1.0f);
}

TEST(AudioGraph, DiamondTopology)
{
    // A (×0.5) → B (×0.5) ↘
    //                       D (passthrough)  →  0.5*0.5 + 0.5*0.6 = 0.25 + 0.30 = 0.55
    // A (×0.5) → C (×0.6) ↗
    //
    // Magnitudes chosen to stay clear of the [-1, 1] saturation in the
    // final mix. Input is 1.0 → after A: 0.5 → fork to B and C.
    AudioGraph<float> g;
    auto a = g.addNode(Gain { 0.5f });
    auto b = g.addNode(Gain { 0.5f });
    auto c = g.addNode(Gain { 0.6f });
    auto d = g.addNode(Passthrough {});

    g.connect(a, b);
    g.connect(a, c);
    g.connect(b, d);
    g.connect(c, d);
    g.setOutput(d);

    ASSERT_TRUE(g.compile(kChannels, kSamples));

    auto buf = makeBuffer(1.0f);
    g.process(buf);

    for (auto& s : buf) EXPECT_FLOAT_EQ(s, 0.55f);
}

TEST(AudioGraph, CompileFailsOnCycle)
{
    AudioGraph<float> g;
    auto a = g.addNode(Passthrough {});
    auto b = g.addNode(Passthrough {});
    g.connect(a, b);
    g.connect(b, a); // cycle
    g.setOutput(b);

    EXPECT_FALSE(g.compile(kChannels, kSamples));
}

TEST(AudioGraph, AsCallbackProcessesBuffer)
{
    AudioGraph<float> g;
    auto a = g.addNode(Gain { 0.5f });
    g.setOutput(a);

    ASSERT_TRUE(g.compile(kChannels, kSamples));

    auto callback = g.asCallback();

    auto buf = makeBuffer(0.6f);
    callback(buf);

    for (auto& s : buf) EXPECT_FLOAT_EQ(s, 0.3f);
}

TEST(AudioGraph, MoveConstructPreservesNodes)
{
    AudioGraph<float> g1;
    auto a = g1.addNode(Gain { 0.7f });
    g1.setOutput(a);
    ASSERT_TRUE(g1.compile(kChannels, kSamples));

    AudioGraph<float> g2 = std::move(g1);

    auto buf = makeBuffer(0.5f);
    g2.process(buf);

    for (auto& s : buf) EXPECT_FLOAT_EQ(s, 0.35f);
}
