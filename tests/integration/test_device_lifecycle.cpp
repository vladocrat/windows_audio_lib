// Regression tests for the WASAPI{Input,Output}Device::stop() handle-close
// race fix. Each test exercises a stop/start interleaving that, under the
// pre-fix code, could close `deviceEvent` while the worker thread was inside
// WaitForSingleObject on it (Win32 UB; manifested as crashes on some
// hardware and silent voice dropouts on others).
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

    // stop() must be safe to call even before start() — there is no event,
    // no worker, nothing to wake. Pre-fix the destructor's CloseHandle
    // guard handled this; the regression is to make sure stop() itself
    // remains a no-op.
    EXPECT_TRUE(input->stop());
    EXPECT_TRUE(input->close());
}

TEST_F(DeviceLifecycleTest, InputBasicStartStopCycle)
{
    slk::DeviceManager manager;
    auto input = manager.defaultInputDevice();
    if (!input) GTEST_SKIP() << "no input device";
    if (!input->open()) GTEST_SKIP() << "cannot open input";

    std::atomic<bool> startReturned { false };
    std::thread worker([&]() {
        input->start();
        startReturned.store(true, std::memory_order_release);
    });

    // Let the loop enter WaitForMultipleObjects at least once.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_TRUE(input->stop());
    worker.join();
    EXPECT_TRUE(startReturned.load(std::memory_order_acquire));
    EXPECT_TRUE(input->close());
}

TEST_F(DeviceLifecycleTest, InputStopRightAfterStart)
{
    // Stop with a window so small the worker may not have hit Wait yet —
    // the fix must handle this just as well as the well-rested case.
    slk::DeviceManager manager;
    auto input = manager.defaultInputDevice();
    if (!input) GTEST_SKIP() << "no input device";
    if (!input->open()) GTEST_SKIP() << "cannot open input";

    std::thread worker([&]() { input->start(); });
    // No sleep — race straight into stop().
    input->stop();
    worker.join();
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
        std::thread worker([&]() { input->start(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        EXPECT_TRUE(input->stop()) << "stop failed on iteration " << i;
        worker.join();
    }

    EXPECT_TRUE(input->close());
}

TEST_F(DeviceLifecycleTest, InputStopWhileCallbackBusy)
{
    // Set a callback that does nontrivial work each invocation so stop()
    // is very likely to fire while the loop is mid-callback rather than
    // sitting in Wait. The fix must still produce a clean shutdown.
    slk::DeviceManager manager;
    auto input = manager.defaultInputDevice();
    if (!input) GTEST_SKIP() << "no input device";
    if (!input->open()) GTEST_SKIP() << "cannot open input";

    std::atomic<int> callbackCount { 0 };
    input->setProcessCallback([&](slk::AudioBuffer<float>&) {
        ++callbackCount;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    });

    std::thread worker([&]() { input->start(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    EXPECT_TRUE(input->stop());
    worker.join();
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

    std::atomic<bool> startReturned { false };
    std::thread worker([&]() {
        output->start();
        startReturned.store(true, std::memory_order_release);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_TRUE(output->stop());
    worker.join();
    EXPECT_TRUE(startReturned.load(std::memory_order_acquire));
    EXPECT_TRUE(output->close());
}

TEST_F(DeviceLifecycleTest, OutputStopRightAfterStart)
{
    slk::DeviceManager manager;
    auto output = manager.defaultOutputDevice();
    if (!output) GTEST_SKIP() << "no output device";
    if (!output->open()) GTEST_SKIP() << "cannot open output";

    std::thread worker([&]() { output->start(); });
    output->stop();
    worker.join();
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
        std::thread worker([&]() { output->start(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        EXPECT_TRUE(output->stop()) << "stop failed on iteration " << i;
        worker.join();
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

    std::thread worker([&]() { output->start(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    EXPECT_TRUE(output->stop());
    worker.join();
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
        std::thread worker([&]() { input->start(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        input->stop();
        worker.join();
    }
    EXPECT_TRUE(input->close());

    // Re-open on the same shared_ptr: the impl's events were nulled in
    // start()'s epilogue, so a second open()/start()/stop() must not
    // double-close the (already-released) handles.
    ASSERT_TRUE(input->open());
    {
        std::thread worker([&]() { input->start(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        input->stop();
        worker.join();
    }
    EXPECT_TRUE(input->close());
}
