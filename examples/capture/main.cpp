// Capture example — records from the default microphone for 5 seconds,
// applying an 8 kHz low-pass filter to each buffer.

#include "Windows.h"

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <thread>

#include <slk/devicemanager.h>
#include <slk/inputdevice.h>
#include <slk/dsp/filter.h>
#include <slk/audioformat.h>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    slk::DeviceManager manager;
    auto input = manager.defaultInputDevice();

    if (!input) {
        qCritical() << "No recording device found";
        return 1;
    }

    if (!input->open()) {
        qCritical() << "Failed to open input device";
        return 1;
    }

    const float sampleRate = static_cast<float>(input->format().sampleRate());
    const uint32_t channels = input->format().channels();
    qDebug() << "Sample rate:" << sampleRate << "  Channels:" << channels;

    slk::filter::LowPassFilter<float> lpf(8000.0f, sampleRate);
    std::atomic<int> bufferCount { 0 };

    input->setProcessCallback([&](slk::AudioBuffer<float>& buf) {
        buf | lpf;
        const int n = ++bufferCount;
        if (n % 50 == 0)
            qDebug() << "buffers:" << n << " samples per buffer:" << buf.numSamples();
    });

    // start() blocks in its capture loop — run it on a background thread
    std::thread captureThread([&input]() { input->start(); });

    qDebug() << "Recording for 5 seconds...";

    QTimer::singleShot(5000, &app, [&]() {
        input->stop();
        captureThread.join();
        input->close();
        qDebug() << "Done. Total buffers received:" << bufferCount.load();
        CoUninitialize();
        app.quit();
    });

    return app.exec();
}
