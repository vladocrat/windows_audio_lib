#include <gtest/gtest.h>

#include <Windows.h>

#include <slk/deviceexplorer.h>
#include <slk/devicemanager.h>
#include <slk/inputdevice.h>
#include <slk/outputdevice.h>
#include <slk/ringbuffer.h>
#include <slk/dsp/noise.h>

#include <thread>
#include <chrono>
#include <atomic>

class DeviceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    }

    void TearDown() override
    {
        CoUninitialize();
    }
};

TEST_F(DeviceTest, ExplorerEnumerate)
{
    slk::DeviceExplorer explorer;
    auto devices = explorer.devices(slk::DeviceType::All, slk::DeviceState::Active);
    // Just verify it doesn't throw — count may be 0
    SUCCEED() << "Found " << devices.size() << " active device(s)";
}

TEST_F(DeviceTest, ExplorerPlaybackDevices)
{
    slk::DeviceExplorer explorer;
    auto devices = explorer.devices(slk::DeviceType::Playback, slk::DeviceState::Active);
    // Informational — no assertion on count
    SUCCEED() << "Found " << devices.size() << " playback device(s)";
}

TEST_F(DeviceTest, ExplorerRecordDevices)
{
    slk::DeviceExplorer explorer;
    auto devices = explorer.devices(slk::DeviceType::Record, slk::DeviceState::Active);
    SUCCEED() << "Found " << devices.size() << " record device(s)";
}

TEST_F(DeviceTest, DefaultOutput)
{
    slk::DeviceManager manager;
    auto output = manager.defaultOutputDevice();

    if (!output)
        GTEST_SKIP() << "No output device available";

    EXPECT_TRUE(output->open());

    const auto& fmt = output->format();
    EXPECT_GT(fmt.sampleRate(), 0u);
    EXPECT_GT(fmt.channels(), 0u);
    EXPECT_GT(fmt.bitsPerSample(), 0u);

    const auto desc = output->descriptor();
    EXPECT_FALSE(desc.name.empty());

    EXPECT_TRUE(output->close());
}

TEST_F(DeviceTest, DefaultInput)
{
    slk::DeviceManager manager;
    auto input = manager.defaultInputDevice();

    if (!input)
        GTEST_SKIP() << "No input device available";

    EXPECT_TRUE(input->open());

    const auto& fmt = input->format();
    EXPECT_GT(fmt.sampleRate(), 0u);
    EXPECT_GT(fmt.channels(), 0u);
    EXPECT_GT(fmt.bitsPerSample(), 0u);

    EXPECT_TRUE(input->close());
}

TEST_F(DeviceTest, CaptureBrief)
{
    slk::DeviceManager manager;
    auto input = manager.defaultInputDevice();

    if (!input)
        GTEST_SKIP() << "No input device available";

    if (!input->open())
        GTEST_SKIP() << "Failed to open input device";

    std::atomic<int> bufferCount{0};
    input->setProcessCallback([&](slk::AudioBuffer<float>&) {
        ++bufferCount;
    });

    std::thread captureThread([&]() { input->start(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    input->stop();
    captureThread.join();
    input->close();

    EXPECT_GT(bufferCount.load(), 0) << "Expected at least one buffer callback in 500ms";
}

TEST_F(DeviceTest, PlaybackBrief)
{
    slk::DeviceManager manager;
    auto output = manager.defaultOutputDevice();

    if (!output)
        GTEST_SKIP() << "No output device available";

    if (!output->open())
        GTEST_SKIP() << "Failed to open output device";

    const auto sampleRate = output->format().sampleRate();
    const auto channels = output->format().channels();
    const auto totalSamples = static_cast<size_t>(sampleRate * channels); // ~1 sec worth

    slk::RingBuffer<float> ring(totalSamples);
    auto noise = slk::dsp::whiteNoise<float>(totalSamples, 0.05f);
    ring.write(noise.data());

    output->setSource(ring);

    std::thread playThread([&]() { output->start(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(600));

    output->stop();
    playThread.join();
    output->close();

    SUCCEED() << "Playback completed without crash";
}

TEST_F(DeviceTest, CreateFromDescriptor)
{
    slk::DeviceExplorer explorer;
    auto devices = explorer.devices(slk::DeviceType::Playback, slk::DeviceState::Active);

    if (devices.empty())
        GTEST_SKIP() << "No playback device available";

    slk::DeviceManager manager;
    auto output = manager.createOutputDevice(devices[0]);

    ASSERT_NE(output, nullptr);
    EXPECT_TRUE(output->open());
    EXPECT_TRUE(output->close());
}
