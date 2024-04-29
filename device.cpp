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
};

Device::Device()
{
    createImpl();
}

Device::~Device()
{
    impl().device->Release();
    impl().client->Release();
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
