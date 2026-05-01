// Loopback example — enumerates available input and output devices,
// lets you select one of each, then routes captured audio directly
// to the output so you can hear yourself in real time.

#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>
#include <thread>

#include <slk/deviceexplorer.h>
#include <slk/devicemanager.h>
#include <slk/inputdevice.h>
#include <slk/outputdevice.h>
#include <slk/ringbuffer.h>
#include <slk/dsp/filter.h>
#include <slk/types.h>

using namespace slk::dsp::literals;

namespace
{

bool stdinIsInteractive()
{
    return isatty(fileno(stdin)) != 0;
}

slk::DeviceDescriptor pickDevice(const std::vector<slk::DeviceDescriptor>& devices, const char* label)
{
    std::cout << "\nAvailable " << label << " devices:\n";

    for (size_t i = 0; i < devices.size(); ++i) {
        std::wcout << L" [" << i << L"] " << devices[i].name << L"\n";
    }

    std::cout << "Select " << label << " device index (Enter for default): " << std::flush;

    std::string line;
    std::getline(std::cin, line);

    if (line.empty() || devices.empty()) {
        return {};
    }

    try {
        const size_t idx = static_cast<size_t>(std::stoi(line));
        if (idx < devices.size()) {
            return devices[idx];
        }
    } catch (...) {
    }

    return {};
}

}

int main()
{
#ifdef _WIN32
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#endif

    const bool interactive = stdinIsInteractive();

    slk::DeviceExplorer explorer;
    slk::DeviceManager manager;

    slk::DeviceDescriptor inputDesc;
    slk::DeviceDescriptor outputDesc;

    if (interactive) {
        const auto inputDescs = explorer.devices(slk::DeviceType::Record, slk::DeviceState::Active);
        const auto outputDescs = explorer.devices(slk::DeviceType::Playback, slk::DeviceState::Active);
        inputDesc = pickDevice(inputDescs, "input");
        outputDesc = pickDevice(outputDescs, "output");
    } else {
        std::cout << "stdin is not a terminal — using default devices.\n";
    }

    auto input = inputDesc.id.empty() ? manager.defaultInputDevice() : manager.createInputDevice(inputDesc);
    auto output = outputDesc.id.empty() ? manager.defaultOutputDevice() : manager.createOutputDevice(outputDesc);

    if (!input) {
        std::cerr << "Failed to create input device\n";
        return 1;
    }

    if (!output) {
        std::cerr << "Failed to create output device\n";
        return 1;
    }

    if (!input->open()) {
        std::cerr << "Failed to open input device\n";
        return 1;
    }

    if (!output->open()) {
        std::cerr << "Failed to open output device\n";
        return 1;
    }

    std::wcout << L"Input  - " << input->descriptor().name << L" | rate: " << input->format().sampleRate()
               << L" channels: " << input->format().channels() << L"\n";

    std::wcout << L"Output - " << output->descriptor().name << L" | rate: " << output->format().sampleRate()
               << L" channels: " << output->format().channels() << L"\n";

    // Ring buffer sized for ~1 second at 48 kHz stereo (power of 2)
    slk::RingBuffer<float> ring(131072);

    output->setSource(ring);

    slk::filter::SimpleGainFilter<float> gain(14_dB);
    slk::filter::SimpleSoftLimiter<float> limiter(-1_dB);

    input->setProcessCallback([&](slk::AudioBuffer<float>& buf) {
        buf | gain | limiter;
        ring.write(buf.data());
    });

    if (!input->start() || !output->start()) {
        std::cerr << "Failed to start devices\n";
        return 1;
    }

    if (interactive) {
        std::cout << "Loopback running - press Enter to stop...\n";
        std::string dummy;
        std::getline(std::cin, dummy);
    } else {
        constexpr int kDurationSeconds = 15;
        std::cout << "Loopback running for " << kDurationSeconds << " seconds...\n";
        std::this_thread::sleep_for(std::chrono::seconds(kDurationSeconds));
    }

    input->stop();
    output->stop();

    input->close();
    output->close();

#ifdef _WIN32
    CoUninitialize();
#endif
    return 0;
}
