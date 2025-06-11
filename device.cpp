#include "device.h"

#include <mmdeviceapi.h>
#include <windows.h>
#include <mmreg.h>
#include <mmsystem.h>
#include <QDebug>


#include "devicethread.h"
#include "general.h"

namespace slk {

struct Device::impl_t
{
    DeviceInfo info;
    IAudioClient* client { nullptr };
    BYTE* data { nullptr };
    uint32_t bufferFrameSize;
    HANDLE event;
    union {
        IAudioRenderClient* renderClient;
        IAudioCaptureClient* captureClient;
    };
    DeviceThread* recordThread;
};

QDataStream& operator<<(QDataStream& out, const Device::Data& data)
{
    out << data.bufferFrameSize;
    out << data.size;
    out << static_cast<uint64_t>(data.status);

    if (data.data && data.size > 0)
    {
        out.writeRawData(reinterpret_cast<const char*>(data.data), data.size);
    }

    return out;
}

QDataStream& operator>>(QDataStream& in, Device::Data& data)
{
    in >> data.bufferFrameSize;
    in >> data.size;
    uint64_t status;
    in >> status;
    data.status = static_cast<DWORD>(status);

    if (data.size > 0)
    {
        data.data = new BYTE[data.size];
        in.readRawData(reinterpret_cast<char*>(data.data), data.size);
    }
    else
    {
        data.data = nullptr;
    }

    return in;
}

Device::Device()
{
    createImpl();
}

Device::~Device()
{
    
}

const DeviceInfo& Device::info() const noexcept
{
    return impl().info;
}

void Device::playback(const Data& data)
{
    if (impl().info.type != DeviceType::Playback) return;
    if (data.size == 0) return;
    
    const auto format = impl().info.format;
    const auto bytesPerSample = format->wBitsPerSample / 8;
    const size_t frameSize = format->nChannels * bytesPerSample;

    for (size_t i = 0; i < data.size; i += frameSize) {
        if (format->wBitsPerSample >= 2) {
            std::memcpy(&data.data[i + bytesPerSample], &data.data[i], bytesPerSample);
        }
    }

    impl().renderClient->GetBuffer(data.bufferFrameSize, &impl().data);
    if (impl().data && data.data && data.size > 0) {
        CopyMemory(impl().data, data.data, data.size);
    }
    impl().renderClient->ReleaseBuffer(data.bufferFrameSize, NULL);
}

void Device::start()
{
    if (impl().recordThread && impl().recordThread->isRunning()) {
        return;
    }

    impl().event = CreateEvent(NULL, FALSE, FALSE, NULL);
    impl().client->SetEventHandle(impl().event);
    impl().recordThread = new DeviceThread(impl().captureClient, impl().event, impl().bufferFrameSize, impl().info.format);
    QObject::connect(impl().recordThread, &DeviceThread::audioDataReady, this, [this](BYTE* data, UINT32 frames, DWORD status) {
        emit readyRead({data, frames, frames * impl().info.format->nBlockAlign, status});
    });

    if (impl().info.type == DeviceType::Record) {
        impl().recordThread->start();
    }

    impl().client->Start();
}

void Device::stop()
{
    impl().client->Stop();

    if (impl().recordThread) {
        impl().recordThread->requestInterruption();
        impl().recordThread->wait();
    }

    CloseHandle(impl().event);
}

void Device::activate() noexcept
{
    impl().info.device->Activate(__uuidof(IAudioClient),
                                 CLSCTX_ALL,
                                 NULL,
                                 reinterpret_cast<void**>(&impl().client));
    impl().client->GetMixFormat(&impl().info.format);
    impl().client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, 0, 0, impl().info.format, NULL);
    
    if (impl().info.type == DeviceType::Record)
    {
        impl().client->GetService(__uuidof(IAudioCaptureClient), reinterpret_cast<void**>(&impl().captureClient));
    }
    else if (impl().info.type == DeviceType::Playback)
    {
        impl().client->GetService(__uuidof(IAudioRenderClient), reinterpret_cast<void**>(&impl().renderClient));
    }
    
    impl().client->GetBufferSize(&impl().bufferFrameSize);
    impl().data = new BYTE[impl().bufferFrameSize * impl().info.format->nBlockAlign];
}

void Device::setInfo(const DeviceInfo& info) noexcept
{
    impl().info = info;
}

}
