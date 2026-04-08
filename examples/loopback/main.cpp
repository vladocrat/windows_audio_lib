// Loopback example — enumerates available input and output devices,
// lets you select one of each, then routes captured audio directly
// to the output so you can hear yourself in real time.

#include "Windows.h"

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <thread>
#include <iostream>

#include <slk/deviceexplorer.h>
#include <slk/devicemanager.h>
#include <slk/inputdevice.h>
#include <slk/outputdevice.h>
#include <slk/ringbuffer.h>

namespace
{

slk::DeviceDescriptor pickDevice(const std::vector<slk::DeviceDescriptor>& devices, const char* label)
{
    qDebug() << "\nAvailable" << label << "devices:";

    for (size_t i = 0; i < devices.size(); ++i) {
        qDebug() << " [" << i << "]" << QString::fromStdWString(devices[i].name);
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
    } catch (...) {}

    return {};
}

}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    slk::DeviceExplorer explorer;
    slk::DeviceManager manager;

    const auto inputDescs  = explorer.devices(slk::DeviceType::Record,   slk::DeviceState::Active);
    const auto outputDescs = explorer.devices(slk::DeviceType::Playback, slk::DeviceState::Active);

    const auto inputDesc  = pickDevice(inputDescs,  "input");
    const auto outputDesc = pickDevice(outputDescs, "output");

    auto input  = inputDesc.id.empty()  ? manager.defaultInputDevice() : manager.createInputDevice(inputDesc);
    auto output = outputDesc.id.empty() ? manager.defaultOutputDevice() : manager.createOutputDevice(outputDesc);

    if (!input) {
        qCritical() << "Failed to create input device";
        return 1;
    }

    if (!output) {
        qCritical() << "Failed to create output device";
        return 1;
    }

    if (!input->open()) {
        qCritical() << "Failed to open input device";
        return 1;
    }

    if (!output->open()) {
        qCritical() << "Failed to open output device";
        return 1;
    }

    qDebug() << "Input  —" << QString::fromStdWString(input->descriptor().name)
             << "| rate:" << input->format().sampleRate()
             << "channels:" << input->format().channels();

    qDebug() << "Output —" << QString::fromStdWString(output->descriptor().name)
             << "| rate:" << output->format().sampleRate()
             << "channels:" << output->format().channels();

    // Ring buffer sized for ~1 second at 48 kHz stereo (power of 2)
    slk::RingBuffer<float> ring(131072);

    output->setSource(ring);

    input->setProcessCallback([&](slk::AudioBuffer<float>& buf) {
        ring.write(buf.data());
    });

    std::thread captureThread([&]() { input->start(); });
    std::thread playbackThread([&]() { output->start(); });

    qDebug() << "Loopback running — press Enter to stop...";
    std::string dummy;
    std::getline(std::cin, dummy);

    input->stop();
    output->stop();

    captureThread.join();
    playbackThread.join();

    input->close();
    output->close();

    CoUninitialize();
    return 0;
}
