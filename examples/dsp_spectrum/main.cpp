// DSP spectrum example — captures from the default microphone and prints
// the top 5 frequency peaks every 1024 samples using a Hann-windowed DFT.

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <algorithm>

#include <slk/devicemanager.h>
#include <slk/inputdevice.h>
#include <slk/dsp/dsp.h>
#include <slk/dsp/window.h>
#include <slk/audioformat.h>

static constexpr size_t kWindowSize = 1024;

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    slk::DeviceManager manager;
    auto input = manager.defaultInputDevice(slk::Purpose::Multimedia);

    if (!input) {
        qCritical() << "No recording device found";
        return 1;
    }

    if (!input->open()) {
        qCritical() << "Failed to open input device";
        return 1;
    }

    const float sampleRate = static_cast<float>(input->format().sampleRate());
    qDebug() << "Sample rate:" << sampleRate;

    slk::Window<slk::WindowType::Hann, float, kWindowSize> window;

    QObject::connect(input.get(), &slk::Device::readyRead,
        [&](const slk::AudioBuffer<float>& buf) {
            // Mix down to mono and ensure we have enough samples
            auto mono = buf.mono();
            if (mono.numSamples() < kWindowSize)
                return;

            // Apply Hann window into a separate buffer
            slk::AudioBuffer<float> windowed(1, kWindowSize);
            window.apply(
                std::span<float>(mono.data().data(), kWindowSize),
                std::span<float>(windowed.data().data(), kWindowSize)
            );

            // Compute DFT and convert to frequency/magnitude pairs
            auto spectrum  = slk::dsp::dft<float>(windowed, sampleRate);
            auto freqMags  = slk::dsp::freqMag<float>(spectrum, sampleRate);

            // Sort descending by magnitude and print the top 5 peaks
            std::partial_sort(freqMags.begin(),
                              freqMags.begin() + std::min<size_t>(5, freqMags.size()),
                              freqMags.end(),
                              [](const auto& a, const auto& b) {
                                  return a.second > b.second;
                              });

            qDebug() << "--- top 5 frequencies ---";
            for (size_t i = 0; i < 5 && i < freqMags.size(); ++i)
                qDebug() << freqMags[i].first << "Hz  mag:" << freqMags[i].second;
        });

    input->start();
    qDebug() << "Analysing for 5 seconds (1024-point Hann-windowed DFT)...";

    QTimer::singleShot(5000, &app, [&]() {
        input->stop();
        input->close();
        app.quit();
    });

    return app.exec();
}
