// Regression tests for the device start()/stop() handle-close race fix.
//
// Each test exercises a stop/start interleaving that, under a buggy
// implementation, could close `deviceEvent` while the worker thread was
// inside WaitForSingleObject on it (Win32 UB), or call AudioDeviceStop
// while the IOProc was still executing (CoreAudio).
//
// With the managed-thread API:
//   - start() is non-blocking; the device owns its own worker.
//   - stop() is synchronous: it signals the worker, joins it, then closes
//     handles. After stop() returns, no other thread is touching the
//     event handles or IOProc.
//
// These are integration tests — they build everywhere but only execute on
// machines with a real audio backend. CI should *build* this target and
// skip running it. GTEST_SKIP is used when a default device cannot be
// opened so manual runs on bare hardware-less hosts also work.

#include <gtest/gtest.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <slk/devicemanager.h>
#include <slk/inputdevice.h>
#include <slk/outputdevice.h>

#include <atomic>
#include <chrono>
#include <thread>

class DeviceLifecycleTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
#ifdef _WIN32
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#endif
    }

    void TearDown() override
    {
#ifdef _WIN32
        CoUninitialize();
#endif
    }
};

// ── Input device ─────────────────────────────────────────────────────────────

TEST_F(DeviceLifecycleTest, InputStopWithoutStartIsNoOp)
{
    slk::DeviceManager manager;
    auto input = manager.defaultInputDevice();
    if (!input) GTEST_SKIP() << "no input device";
    if (!input->open()) GTEST_SKIP() << "cannot open input";

    // stop() must be safe to call before start() — there is no worker to
    // join, no event to signal, no IOProc to destroy.
    EXPECT_TRUE(input->stop());
    EXPECT_TRUE(input->close());
}

TEST_F(DeviceLifecycleTest, InputBasicStartStopCycle)
{
    slk::DeviceManager manager;
    auto input = manager.defaultInputDevice();
    if (!input) GTEST_SKIP() << "no input device";
    if (!input->open()) GTEST_SKIP() << "cannot open input";

    EXPECT_TRUE(input->start());
    // Let the worker enter its wait loop at least once.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(input->stop());
    EXPECT_TRUE(input->close());
}

TEST_F(DeviceLifecycleTest, InputStopRightAfterStart)
{
    // Stop with a window so small the worker may not have entered its wait
    // yet — the implementation must handle this just as well as the
    // well-rested case.
    slk::DeviceManager manager;
    auto input = manager.defaultInputDevice();
    if (!input) GTEST_SKIP() << "no input device";
    if (!input->open()) GTEST_SKIP() << "cannot open input";

    EXPECT_TRUE(input->start());
    // No sleep — race straight into stop().
    EXPECT_TRUE(input->stop());
    EXPECT_TRUE(input->close());
}

TEST_F(DeviceLifecycleTest, InputRapidRestartCycles)
{
    slk::DeviceManager manager;
    auto input = manager.defaultInputDevice();
    if (!input) GTEST_SKIP() << "no input device";
    if (!input->open()) GTEST_SKIP() << "cannot open input";

    constexpr int kCycles = 10;

    for (int i = 0; i < kCycles; ++i) {
        ASSERT_TRUE(input->start()) << "start failed on iteration " << i;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        EXPECT_TRUE(input->stop()) << "stop failed on iteration " << i;
    }

    EXPECT_TRUE(input->close());
}

TEST_F(DeviceLifecycleTest, InputStopWhileCallbackBusy)
{
    // A callback that does nontrivial work each invocation so stop() is
    // very likely to fire while the loop is mid-callback rather than
    // sitting in its wait. The implementation must still produce a clean
    // shutdown.
    slk::DeviceManager manager;
    auto input = manager.defaultInputDevice();
    if (!input) GTEST_SKIP() << "no input device";
    if (!input->open()) GTEST_SKIP() << "cannot open input";

    std::atomic<int> callbackCount { 0 };
    input->setProcessCallback([&](slk::AudioBuffer<float>&) {
        ++callbackCount;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    });

    EXPECT_TRUE(input->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    EXPECT_TRUE(input->stop());
    EXPECT_TRUE(input->close());
}

// ── Output device ────────────────────────────────────────────────────────────

TEST_F(DeviceLifecycleTest, OutputStopWithoutStartIsNoOp)
{
    slk::DeviceManager manager;
    auto output = manager.defaultOutputDevice();
    if (!output) GTEST_SKIP() << "no output device";
    if (!output->open()) GTEST_SKIP() << "cannot open output";

    EXPECT_TRUE(output->stop());
    EXPECT_TRUE(output->close());
}

TEST_F(DeviceLifecycleTest, OutputBasicStartStopCycle)
{
    slk::DeviceManager manager;
    auto output = manager.defaultOutputDevice();
    if (!output) GTEST_SKIP() << "no output device";
    if (!output->open()) GTEST_SKIP() << "cannot open output";

    EXPECT_TRUE(output->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(output->stop());
    EXPECT_TRUE(output->close());
}

TEST_F(DeviceLifecycleTest, OutputStopRightAfterStart)
{
    slk::DeviceManager manager;
    auto output = manager.defaultOutputDevice();
    if (!output) GTEST_SKIP() << "no output device";
    if (!output->open()) GTEST_SKIP() << "cannot open output";

    EXPECT_TRUE(output->start());
    EXPECT_TRUE(output->stop());
    EXPECT_TRUE(output->close());
}

TEST_F(DeviceLifecycleTest, OutputRapidRestartCycles)
{
    slk::DeviceManager manager;
    auto output = manager.defaultOutputDevice();
    if (!output) GTEST_SKIP() << "no output device";
    if (!output->open()) GTEST_SKIP() << "cannot open output";

    constexpr int kCycles = 10;

    for (int i = 0; i < kCycles; ++i) {
        ASSERT_TRUE(output->start()) << "start failed on iteration " << i;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        EXPECT_TRUE(output->stop()) << "stop failed on iteration " << i;
    }

    EXPECT_TRUE(output->close());
}

TEST_F(DeviceLifecycleTest, OutputStopWhileCallbackBusy)
{
    slk::DeviceManager manager;
    auto output = manager.defaultOutputDevice();
    if (!output) GTEST_SKIP() << "no output device";
    if (!output->open()) GTEST_SKIP() << "cannot open output";

    std::atomic<int> callbackCount { 0 };
    output->setProcessCallback([&](slk::AudioBuffer<float>&) {
        ++callbackCount;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    });

    EXPECT_TRUE(output->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    EXPECT_TRUE(output->stop());
    EXPECT_TRUE(output->close());
}

// ── Mixed: open → start/stop → reopen on the same device handle ─────────────

TEST_F(DeviceLifecycleTest, InputCloseReopenStartStop)
{
    slk::DeviceManager manager;
    auto input = manager.defaultInputDevice();
    if (!input) GTEST_SKIP() << "no input device";

    if (!input->open()) GTEST_SKIP() << "cannot open input";
    {
        ASSERT_TRUE(input->start());
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        EXPECT_TRUE(input->stop());
    }
    EXPECT_TRUE(input->close());

    // Re-open on the same shared_ptr: the impl's events / IOProc handles
    // were nulled in stop(), so a second open()/start()/stop() must not
    // double-close any of them.
    ASSERT_TRUE(input->open());
    {
        ASSERT_TRUE(input->start());
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        EXPECT_TRUE(input->stop());
    }
    EXPECT_TRUE(input->close());
}

// ── Managed-thread contract ──────────────────────────────────────────────────

TEST_F(DeviceLifecycleTest, InputDoubleStartIsRejected)
{
    // start() must not silently spawn a second worker. Once running, a
    // second start() returns false until stop() has been called.
    slk::DeviceManager manager;
    auto input = manager.defaultInputDevice();
    if (!input) GTEST_SKIP() << "no input device";
    if (!input->open()) GTEST_SKIP() << "cannot open input";

    ASSERT_TRUE(input->start());
    EXPECT_FALSE(input->start());
    EXPECT_TRUE(input->stop());
    EXPECT_TRUE(input->close());
}

TEST_F(DeviceLifecycleTest, OutputDoubleStartIsRejected)
{
    slk::DeviceManager manager;
    auto output = manager.defaultOutputDevice();
    if (!output) GTEST_SKIP() << "no output device";
    if (!output->open()) GTEST_SKIP() << "cannot open output";

    ASSERT_TRUE(output->start());
    EXPECT_FALSE(output->start());
    EXPECT_TRUE(output->stop());
    EXPECT_TRUE(output->close());
}

TEST_F(DeviceLifecycleTest, InputCloseFromRunningStopsWorker)
{
    // close() must call stop() if the worker is running, so the caller
    // doesn't have to remember the order.
    slk::DeviceManager manager;
    auto input = manager.defaultInputDevice();
    if (!input) GTEST_SKIP() << "no input device";
    if (!input->open()) GTEST_SKIP() << "cannot open input";

    ASSERT_TRUE(input->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    EXPECT_TRUE(input->close()); // stops + closes; no separate stop() needed
}

TEST_F(DeviceLifecycleTest, OutputCloseFromRunningStopsWorker)
{
    slk::DeviceManager manager;
    auto output = manager.defaultOutputDevice();
    if (!output) GTEST_SKIP() << "no output device";
    if (!output->open()) GTEST_SKIP() << "cannot open output";

    ASSERT_TRUE(output->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    EXPECT_TRUE(output->close());
}
