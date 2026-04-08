// DSP spectrum example — captures from the default microphone and prints
// the top 5 frequency peaks every 1024 samples using a Hann-windowed DFT.

#include <Windows.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <vector>

#include <slk/devicemanager.h>
#include <slk/inputdevice.h>
#include <slk/dsp/dsp.h>
#include <slk/dsp/window.h>
#include <slk/audioformat.h>

static constexpr size_t kWindowSize = 1024;

int main()
{
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    slk::DeviceManager manager;
    auto input = manager.defaultInputDevice(slk::Purpose::Multimedia);

    if (!input) {
        std::cerr << "No recording device found\n";
        return 1;
    }

    if (!input->open()) {
        std::cerr << "Failed to open input device\n";
        return 1;
    }

    const float sampleRate = static_cast<float>(input->format().sampleRate());
    std::cout << "Sample rate: " << sampleRate << "\n";

    slk::Window<slk::WindowType::Hann, float, kWindowSize> window;
    std::vector<float> accumulator;
    accumulator.reserve(kWindowSize * 2);

    input->setProcessCallback([&](slk::AudioBuffer<float>& buf) {
        auto mono = buf.mono();
        const auto* src = mono.data().data();
        accumulator.insert(accumulator.end(), src, src + mono.numSamples());

        while (accumulator.size() >= kWindowSize) {
            slk::AudioBuffer<float> windowed(1, kWindowSize);
            window.apply(
                std::span<float>(accumulator.data(), kWindowSize),
                std::span<float>(windowed.data().data(), kWindowSize)
            );

            auto spectrum  = slk::dsp::dft<float>(windowed, sampleRate);
            auto freqMags  = slk::dsp::freqMag<float>(spectrum, sampleRate);

            std::partial_sort(freqMags.begin(),
                              freqMags.begin() + std::min<size_t>(5, freqMags.size()),
                              freqMags.end(),
                              [](const auto& a, const auto& b) {
                                  return a.second > b.second;
                              });

            std::cout << "--- top 5 frequencies ---\n";
            for (size_t i = 0; i < 5 && i < freqMags.size(); ++i)
                std::cout << freqMags[i].first << " Hz  mag: " << freqMags[i].second << "\n";

            accumulator.erase(accumulator.begin(), accumulator.begin() + kWindowSize);
        }
    });

    std::thread captureThread([&input]() { input->start(); });

    std::cout << "Analysing for 5 seconds (1024-point Hann-windowed DFT)...\n";
    std::this_thread::sleep_for(std::chrono::seconds(5));

    input->stop();
    captureThread.join();
    input->close();

    CoUninitialize();
    return 0;
}
