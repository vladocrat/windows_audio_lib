// Playback example — generates white noise, fills a ring buffer,
// and plays it through the default output device for 3 seconds.

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

#include <slk/devicemanager.h>
#include <slk/outputdevice.h>
#include <slk/ringbuffer.h>
#include <slk/dsp/noise.h>
#include <slk/audioformat.h>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    slk::DeviceManager manager;
    auto output = manager.defaultOutputDevice(slk::Purpose::Multimedia);

    if (!output) {
        qCritical() << "No playback device found";
        return 1;
    }

    if (!output->open()) {
        qCritical() << "Failed to open output device";
        return 1;
    }

    const float sampleRate = static_cast<float>(output->format().sampleRate());
    const uint32_t channels = output->format().channels();
    qDebug() << "Sample rate:" << sampleRate << "  Channels:" << channels;

    // Pre-fill the ring buffer with 3 seconds of soft white noise
    const size_t totalSamples = static_cast<size_t>(sampleRate) * channels * 3;
    // Ring buffer capacity must be a power of 2 and larger than totalSamples
    slk::RingBuffer<float> ring(524288); // 2^19 ~ 11 sec at 48 kHz stereo

    auto noise = slk::dsp::whiteNoise<float>(totalSamples, 0.05f);
    ring.write(std::span<const float>(noise.data().data(), noise.size()));

    output->setSource(ring);
    output->start();
    qDebug() << "Playing white noise for 3 seconds...";

    QTimer::singleShot(3000, &app, [&]() {
        output->stop();
        output->close();
        app.quit();
    });

    return app.exec();
}
