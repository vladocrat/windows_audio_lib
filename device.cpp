#include "device.h"

#include <mmdeviceapi.h>
#include <windows.h>
#include <mmreg.h>
#include <mmsystem.h>

struct Device::impl_t
{
    WAVEFORMATEX* format { nullptr };
    IAudioClient* client { nullptr };
    IMMDevice* device { nullptr };
    BYTE* data { nullptr };
    uint32_t bufferFrameSize { 0 };
};

Device::Device()
{
    createImpl();
}

Device::~Device()
{
    assert(impl().format);
    assert(impl().client);
    assert(impl().device);
    assert(impl().data);

    impl().device->Release();
    impl().client->Release();

    delete impl().format;
    delete impl().client;
    delete impl().device;
    delete[] impl().data;
}

const uint32_t& Device::frameSize() const noexcept
{
    return impl().bufferFrameSize;
}

const BYTE* Device::data() const noexcept
{
    return impl().data;
}

const IMMDevice* Device::device() const noexcept
{
    return impl().device;
}

void Device::setDevice(IMMDevice* device) noexcept
{
    //! Comparing addresses if fine
    if (device == impl().device) return;
    impl().device = device;
}

const IAudioClient* Device::client() const noexcept
{
    return impl().client;
}

IAudioClient* Device::client() noexcept
{
    return impl().client;
}

const WAVEFORMATEX* Device::waveFormat() const noexcept
{
    return impl().format;
}

uint32_t* Device::refFrameSize() noexcept
{
    return &impl().bufferFrameSize;
}

BYTE** Device::refData() noexcept
{
    return &impl().data;
}

void Device::newBuffer(size_t size) noexcept
{
    impl().data = new BYTE[size];
}

bool Device::activate() noexcept
{
    auto err = impl().device->Activate(__uuidof(IAudioClient),
                                CLSCTX_ALL,
                                NULL,
                                reinterpret_cast<void**>(&impl().client));

    if (FAILED(err)) return false;

    if (!impl().format) {
        impl().client->GetMixFormat(&impl().format);
    }

    err = impl().client->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 0, 0, impl().format, NULL);

    return SUCCEEDED(err);
}
