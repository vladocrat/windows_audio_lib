#include "device.h"

#include <mmdeviceapi.h>
#include <windows.h>
#include <mmreg.h>
#include <mmsystem.h>

#include <QTimer>

#include "general.h"

namespace slk {

struct Device::impl_t
{
    DeviceInfo info;
    IAudioClient* client { nullptr };
    BYTE* data { nullptr };
    uint32_t bufferFrameSize;
    union {
        IAudioRenderClient* renderClient;
        IAudioCaptureClient* captureClient;
    };
    QTimer readyReadTimer;
};

Device::Device()
{
    createImpl();
}

Device::~Device()
{
    
}

const DeviceInfo* Device::info() const noexcept
{
    return &impl().info;
}

void Device::playback(const Data& data)
{
    if (impl().info.type != DeviceType::Playback) return;
    if (data.size == 0) return;
    
    impl().renderClient->GetBuffer(data.bufferFrameSize, &impl().data);
    CopyMemory(impl().data, data.data, data.size);    
    impl().renderClient->ReleaseBuffer(data.bufferFrameSize, NULL);
}

void Device::start()
{
    impl().readyReadTimer.callOnTimeout([this]() {
        if (impl().info.type != DeviceType::Record) return;
        impl().captureClient->ReleaseBuffer(impl().bufferFrameSize);
        
        DWORD statusData;
        
        impl().captureClient->GetBuffer(&impl().data, &impl().bufferFrameSize, &statusData, NULL, NULL);
        
        emit readyRead({impl().data, impl().bufferFrameSize, impl().bufferFrameSize * impl().info.format->nBlockAlign, statusData});
    });
    
    impl().client->Start();
    impl().readyReadTimer.start();
}

void Device::stop()
{
    impl().client->Stop();
    impl().readyReadTimer.stop();
}

void Device::activate() noexcept
{
    impl().info.device->Activate(__uuidof(IAudioClient),
                                    CLSCTX_ALL,
                                    NULL,
                                    reinterpret_cast<void**>(&impl().client));
    impl().client->GetMixFormat(&impl().info.format);
    impl().client->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 0, 0, impl().info.format, NULL);
    
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
