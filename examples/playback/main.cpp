// Playback example — generates white noise, fills a ring buffer,
// and plays it through the default output device for 3 seconds.

#ifdef _WIN32
#include <Windows.h>
#endif

#include <iostream>
#include <thread>
#include <chrono>

#include <slk/devicemanager.h>
#include <slk/outputdevice.h>
#include <slk/ringbuffer.h>
#include <slk/dsp/noise.h>
#include <slk/audioformat.h>

int main()
{
#ifdef _WIN32
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#endif

    slk::DeviceManager manager;
    auto output = manager.defaultOutputDevice(slk::Purpose::Multimedia);

    if (!output) {
        std::cerr << "No playback device found\n";
        return 1;
    }

    if (!output->open()) {
        std::cerr << "Failed to open output device\n";
        return 1;
    }

    const float sampleRate = static_cast<float>(output->format().sampleRate());
    const uint32_t channels = output->format().channels();
    std::cout << "Sample rate: " << sampleRate << "  Channels: " << channels << "\n";

    // Pre-fill the ring buffer with 3 seconds of soft white noise
    const size_t totalSamples = static_cast<size_t>(sampleRate) * channels * 3;
    // Ring buffer capacity must be a power of 2 and larger than totalSamples
    slk::RingBuffer<float> ring(524288); // 2^19 ~ 11 sec at 48 kHz stereo

    auto noise = slk::dsp::whiteNoise<float>(totalSamples, 0.05f);
    ring.write(std::span<const float>(noise.data().data(), noise.size()));

    output->setSource(ring);

    std::thread playbackThread([&output]() { output->start(); });

    std::cout << "Playing white noise for 3 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));

    output->stop();
    playbackThread.join();
    output->close();

#ifdef _WIN32
    CoUninitialize();
#endif
    return 0;
}
