#include <gtest/gtest.h>

#include <slk/audioformat.h>

TEST(AudioFormat, DefaultConstruct)
{
    EXPECT_NO_THROW(slk::AudioFormat());
}

TEST(AudioFormat, Construct)
{
    slk::AudioFormat fmt(2, 48000, 32, slk::AudioFormat::Type::FLOAT);

    EXPECT_EQ(fmt.type(), slk::AudioFormat::Type::FLOAT);
    EXPECT_EQ(fmt.sampleRate(), 48000u);
    EXPECT_EQ(fmt.channels(), 2u);
    EXPECT_EQ(fmt.bitsPerSample(), 32u);
}

TEST(AudioFormat, ConstructPCM)
{
    slk::AudioFormat fmt(1, 44100, 16, slk::AudioFormat::Type::PCM);

    EXPECT_EQ(fmt.type(), slk::AudioFormat::Type::PCM);
    EXPECT_EQ(fmt.sampleRate(), 44100u);
    EXPECT_EQ(fmt.channels(), 1u);
    EXPECT_EQ(fmt.bitsPerSample(), 16u);
}

TEST(AudioFormat, MoveConstruct)
{
    slk::AudioFormat original(2, 48000, 32, slk::AudioFormat::Type::FLOAT);
    slk::AudioFormat moved(std::move(original));

    EXPECT_EQ(moved.sampleRate(), 48000u);
    EXPECT_EQ(moved.channels(), 2u);
}

TEST(AudioFormat, MoveAssign)
{
    slk::AudioFormat original(2, 48000, 32, slk::AudioFormat::Type::FLOAT);
    slk::AudioFormat other;
    other = std::move(original);

    EXPECT_EQ(other.sampleRate(), 48000u);
    EXPECT_EQ(other.channels(), 2u);
}
