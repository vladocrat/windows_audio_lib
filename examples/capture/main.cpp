// Capture example — records from the default microphone for 5 seconds,
// applying an 8 kHz low-pass filter to each buffer.

#ifdef _WIN32
#include <Windows.h>
#endif

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

#include <slk/devicemanager.h>
#include <slk/inputdevice.h>
#include <slk/dsp/filter.h>
#include <slk/audioformat.h>

int main()
{
#ifdef _WIN32
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#endif

    slk::DeviceManager manager;
    auto input = manager.defaultInputDevice();

    if (!input) {
        std::cerr << "No recording device found\n";
        return 1;
    }

    if (!input->open()) {
        std::cerr << "Failed to open input device\n";
        return 1;
    }

    const float sampleRate = static_cast<float>(input->format().sampleRate());
    const uint32_t channels = input->format().channels();
    std::cout << "Sample rate: " << sampleRate << "  Channels: " << channels << "\n";

    slk::filter::LowPassFilter<float> lpf(8000.0f, sampleRate);
    std::atomic<int> bufferCount { 0 };

    input->setProcessCallback([&](slk::AudioBuffer<float>& buf) {
        buf | lpf;
        const int n = ++bufferCount;
        if (n % 50 == 0)
            std::cout << "buffers: " << n << "  samples per buffer: " << buf.numSamples() << "\n";
    });

    std::thread captureThread([&input]() { input->start(); });

    std::cout << "Recording for 5 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(5));

    input->stop();
    captureThread.join();
    input->close();

    std::cout << "Done. Total buffers received: " << bufferCount.load() << "\n";

#ifdef _WIN32
    CoUninitialize();
#endif
    return 0;
}
