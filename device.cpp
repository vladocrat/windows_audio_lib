#include "device.h"

#include <mmdeviceapi.h>
#include <windows.h>
#include <mmreg.h>
#include <mmsystem.h>

struct Device::impl_t
{
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

const IAudioClient *Device::client() const noexcept
{
    return impl().client;
}

IAudioClient* Device::client() noexcept
{
    return impl().client;
}

bool Device::activate(WAVEFORMATEX* waveFormat) noexcept
{
    auto err = impl().device->Activate(__uuidof(IAudioClient),
                                CLSCTX_ALL,
                                NULL,
                                reinterpret_cast<void**>(&impl().client));

    if (FAILED(err)) return false;

    if (!waveFormat) {
        impl().client->GetMixFormat(&waveFormat);
    }

    err = impl().client->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 0, 0, waveFormat, NULL);

    return SUCCEEDED(err);
}
