// Loopback example — enumerates available input and output devices,
// lets you select one of each, then routes captured audio directly
// to the output so you can hear yourself in real time.

#ifdef _WIN32
#include <Windows.h>
#endif

#include <iostream>
#include <thread>
#include <string>

#include <slk/deviceexplorer.h>
#include <slk/devicemanager.h>
#include <slk/inputdevice.h>
#include <slk/outputdevice.h>
#include <slk/ringbuffer.h>
#include <slk/dsp/filter.h>

namespace
{

slk::DeviceDescriptor pickDevice(const std::vector<slk::DeviceDescriptor>& devices, const char* label)
{
    std::cout << "\nAvailable " << label << " devices:\n";

    for (size_t i = 0; i < devices.size(); ++i) {
        std::wcout << L" [" << i << L"] " << devices[i].name << L"\n";
    }

    std::cout << "Select " << label << " device index (Enter for default): ";

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

    slk::DeviceExplorer explorer;
    slk::DeviceManager manager;

    const auto inputDescs = explorer.devices(slk::DeviceType::Record, slk::DeviceState::Active);
    const auto outputDescs = explorer.devices(slk::DeviceType::Playback, slk::DeviceState::Active);

    const auto inputDesc = pickDevice(inputDescs, "input");
    const auto outputDesc = pickDevice(outputDescs, "output");

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

    std::wcout << L"Input  — " << input->descriptor().name << L" | rate: " << input->format().sampleRate()
               << L" channels: " << input->format().channels() << L"\n";

    std::wcout << L"Output — " << output->descriptor().name << L" | rate: " << output->format().sampleRate()
               << L" channels: " << output->format().channels() << L"\n";

    // Ring buffer sized for ~1 second at 48 kHz stereo (power of 2)
    slk::RingBuffer<float> ring(131072);

    output->setSource(ring);

    slk::filter::SimpleGainFilter<float> gain(5.f);
    slk::filter::SimpleSoftLimiter<float> limiter(0.9f);

    input->setProcessCallback([&](slk::AudioBuffer<float>& buf) {
        buf | gain | limiter;
        ring.write(buf.data());
    });

    std::thread captureThread([&]() { input->start(); });
    std::thread playbackThread([&]() { output->start(); });

    std::cout << "Loopback running — press Enter to stop...\n";
    std::string dummy;
    std::getline(std::cin, dummy);

    input->stop();
    output->stop();

    captureThread.join();
    playbackThread.join();

    input->close();
    output->close();

#ifdef _WIN32
    CoUninitialize();
#endif
    return 0;
}
