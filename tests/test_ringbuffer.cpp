#include <gtest/gtest.h>

#include <slk/ringbuffer.h>
#include <numeric>

TEST(RingBuffer, WriteRead)
{
    slk::RingBuffer<float> ring(128);

    std::vector<float> input(100);
    std::iota(input.begin(), input.end(), 0.0f);

    auto written = ring.write(input);
    EXPECT_EQ(written, 100u);
    EXPECT_EQ(ring.canRead(), 100u);

    std::vector<float> output(100);
    auto read = ring.read(output, 100);
    EXPECT_EQ(read, 100u);

    for (size_t i = 0; i < 100; ++i)
        EXPECT_FLOAT_EQ(output[i], static_cast<float>(i));
}

TEST(RingBuffer, Peek)
{
    slk::RingBuffer<float> ring(64);

    std::vector<float> input = {1.0f, 2.0f, 3.0f};
    ring.write(input);

    EXPECT_EQ(ring.canRead(), 3u);

    std::vector<float> peeked(3);
    auto count = ring.peek(peeked, 3);
    EXPECT_EQ(count, 3u);
    EXPECT_EQ(ring.canRead(), 3u); // unchanged

    EXPECT_FLOAT_EQ(peeked[0], 1.0f);
    EXPECT_FLOAT_EQ(peeked[1], 2.0f);
    EXPECT_FLOAT_EQ(peeked[2], 3.0f);
}

TEST(RingBuffer, PowerOfTwoRoundup)
{
    slk::RingBuffer<float> ring(100); // rounds to 128
    EXPECT_EQ(ring.canWrite(), 127u); // capacity - 1
}

TEST(RingBuffer, PowerOfTwoExact)
{
    slk::RingBuffer<float> ring(64);
    EXPECT_EQ(ring.canWrite(), 63u);
}

TEST(RingBuffer, Overflow)
{
    slk::RingBuffer<float> ring(16); // capacity 16, writable 15

    std::vector<float> input(20, 1.0f);
    auto written = ring.write(input);
    EXPECT_EQ(written, 15u); // only 15 fit
    EXPECT_EQ(ring.canRead(), 15u);
}

TEST(RingBuffer, Empty)
{
    slk::RingBuffer<float> ring(32);
    EXPECT_EQ(ring.canRead(), 0u);

    std::vector<float> output(10);
    auto read = ring.read(output, 10);
    EXPECT_EQ(read, 0u);
}

TEST(RingBuffer, WrapAround)
{
    slk::RingBuffer<float> ring(8); // capacity 8, writable 7

    // Fill and drain to advance indices
    std::vector<float> fill(5, 0.0f);
    ring.write(fill);
    std::vector<float> drain(5);
    ring.read(drain, 5);

    // Now write data that wraps around
    std::vector<float> input = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f};
    auto written = ring.write(input);
    EXPECT_EQ(written, 5u);

    std::vector<float> output(5);
    auto read = ring.read(output, 5);
    EXPECT_EQ(read, 5u);
    EXPECT_FLOAT_EQ(output[0], 10.0f);
    EXPECT_FLOAT_EQ(output[4], 50.0f);
}
