#include <gtest/gtest.h>

#include <slk/audiobuffer.h>
#include <slk/dsp/filter.h>

TEST(AudioBuffer, Construct)
{
    slk::AudioBuffer<float> buf(2, 256);
    EXPECT_EQ(buf.channels(), 2u);
    EXPECT_EQ(buf.numSamples(), 256u);
    EXPECT_EQ(buf.size(), 512u);
}

TEST(AudioBuffer, DefaultConstruct)
{
    slk::AudioBuffer<float> buf;
    EXPECT_EQ(buf.channels(), 0u);
    EXPECT_EQ(buf.numSamples(), 0u);
    EXPECT_EQ(buf.size(), 0u);
}

TEST(AudioBuffer, Clear)
{
    slk::AudioBuffer<float> buf(1, 64);
    for (auto& s : buf)
        s = 1.0f;

    buf.clear();

    for (const auto& s : buf)
        EXPECT_FLOAT_EQ(s, 0.0f);
}

TEST(AudioBuffer, MonoDownmix)
{
    slk::AudioBuffer<float> stereo(2, 4);
    for (uint32_t i = 0; i < 4; ++i) {
        stereo[i * 2 + 0] = 0.8f; // L
        stereo[i * 2 + 1] = 0.2f; // R
    }

    auto mono = stereo.mono();
    EXPECT_EQ(mono.channels(), 1u);
    EXPECT_EQ(mono.numSamples(), 4u);

    for (uint32_t i = 0; i < 4; ++i)
        EXPECT_NEAR(mono[i], 0.5f, 1e-5f);
}

TEST(AudioBuffer, MonoPassthrough)
{
    slk::AudioBuffer<float> mono(1, 8);
    for (uint32_t i = 0; i < 8; ++i)
        mono[i] = static_cast<float>(i);

    auto result = mono.mono();
    EXPECT_EQ(result.channels(), 1u);
    for (uint32_t i = 0; i < 8; ++i)
        EXPECT_FLOAT_EQ(result[i], static_cast<float>(i));
}

TEST(AudioBuffer, StereoUpmix)
{
    slk::AudioBuffer<float> mono(1, 4);
    for (uint32_t i = 0; i < 4; ++i)
        mono[i] = 0.7f;

    auto stereo = mono.stereo();
    EXPECT_EQ(stereo.channels(), 2u);
    EXPECT_EQ(stereo.numSamples(), 4u);

    for (uint32_t i = 0; i < 4; ++i) {
        EXPECT_FLOAT_EQ(stereo[i * 2 + 0], 0.7f);
        EXPECT_FLOAT_EQ(stereo[i * 2 + 1], 0.7f);
    }
}

TEST(AudioBuffer, StereoPassthrough)
{
    slk::AudioBuffer<float> stereo(2, 4);
    for (size_t i = 0; i < stereo.size(); ++i)
        stereo[i] = static_cast<float>(i);

    auto result = stereo.stereo();
    EXPECT_EQ(result.channels(), 2u);
    for (size_t i = 0; i < result.size(); ++i)
        EXPECT_FLOAT_EQ(result[i], static_cast<float>(i));
}

TEST(AudioBuffer, Iterators)
{
    slk::AudioBuffer<float> buf(1, 16);
    float val = 0.0f;
    for (auto& s : buf)
        s = val++;

    val = 0.0f;
    for (const auto& s : buf) {
        EXPECT_FLOAT_EQ(s, val);
        val += 1.0f;
    }
}

TEST(AudioBuffer, SetSize)
{
    slk::AudioBuffer<float> buf(1, 8);
    EXPECT_EQ(buf.size(), 8u);

    buf.setSize(2, 32);
    EXPECT_EQ(buf.channels(), 2u);
    EXPECT_EQ(buf.numSamples(), 32u);
    EXPECT_EQ(buf.size(), 64u);

    for (const auto& s : buf)
        EXPECT_FLOAT_EQ(s, 0.0f);
}

TEST(AudioBuffer, PipeOperator)
{
    slk::AudioBuffer<float> buf(1, 16);
    for (auto& s : buf)
        s = 0.5f;

    slk::filter::SimpleGainFilter<float> gain(1.0f);
    buf | gain;

    for (const auto& s : buf)
        EXPECT_FLOAT_EQ(s, 0.5f);
}

TEST(AudioBuffer, MixOperator)
{
    slk::AudioBuffer<float> dst(1, 4);
    dst.clear();

    slk::AudioBuffer<float> src1(1, 4);
    slk::AudioBuffer<float> src2(1, 4);
    for (uint32_t i = 0; i < 4; ++i) {
        src1[i] = 0.7f;
        src2[i] = 0.6f;
    }

    std::vector<const slk::AudioBuffer<float>*> sources = {&src1, &src2};
    dst += sources;

    for (uint32_t i = 0; i < 4; ++i)
        EXPECT_FLOAT_EQ(dst[i], 1.0f); // clamped from 1.3
}

TEST(AudioBuffer, CopyConstruct)
{
    slk::AudioBuffer<float> original(2, 8);
    for (size_t i = 0; i < original.size(); ++i)
        original[i] = static_cast<float>(i);

    slk::AudioBuffer<float> copy(original);
    EXPECT_EQ(copy.channels(), 2u);
    EXPECT_EQ(copy.numSamples(), 8u);
    for (size_t i = 0; i < copy.size(); ++i)
        EXPECT_FLOAT_EQ(copy[i], static_cast<float>(i));
}

TEST(AudioBuffer, MoveConstruct)
{
    slk::AudioBuffer<float> original(2, 8);
    for (size_t i = 0; i < original.size(); ++i)
        original[i] = static_cast<float>(i);

    slk::AudioBuffer<float> moved(std::move(original));
    EXPECT_EQ(moved.channels(), 2u);
    EXPECT_EQ(moved.numSamples(), 8u);
    EXPECT_EQ(original.channels(), 0u);
    EXPECT_EQ(original.numSamples(), 0u);
}

TEST(AudioBuffer, CopyAssign)
{
    slk::AudioBuffer<float> original(1, 4);
    for (size_t i = 0; i < 4; ++i)
        original[i] = 0.5f;

    slk::AudioBuffer<float> copy;
    copy = original;
    EXPECT_EQ(copy.channels(), 1u);
    EXPECT_EQ(copy.numSamples(), 4u);
    for (size_t i = 0; i < 4; ++i)
        EXPECT_FLOAT_EQ(copy[i], 0.5f);
}

TEST(AudioBuffer, MoveAssign)
{
    slk::AudioBuffer<float> original(1, 4);
    for (size_t i = 0; i < 4; ++i)
        original[i] = 0.5f;

    slk::AudioBuffer<float> moved;
    moved = std::move(original);
    EXPECT_EQ(moved.channels(), 1u);
    EXPECT_EQ(moved.numSamples(), 4u);
    EXPECT_EQ(original.channels(), 0u);
}
