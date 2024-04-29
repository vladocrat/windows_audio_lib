#include "deviceexplorer.h"

#include <mmdeviceapi.h>

struct DeviceExplorer::impl_t
{
    IMMDeviceEnumerator* enumerator { nullptr };
};

DeviceExplorer::DeviceExplorer()
{
    createImpl();

    CoCreateInstance(__uuidof(MMDeviceEnumerator),
                     NULL,
                     CLSCTX_ALL,
                     __uuidof(IMMDeviceEnumerator),
                     reinterpret_cast<void**>(&impl().enumerator));
}

DeviceExplorer::~DeviceExplorer()
{
    impl().enumerator->Release();

    if (!impl().enumerator) return;

    delete impl().enumerator;
}

IMMDevice* DeviceExplorer::defaultDevice(DeviceType type, Purpose purpose) const noexcept
{
    IMMDevice* device { nullptr };
    impl().enumerator->GetDefaultAudioEndpoint(static_cast<EDataFlow>(type),
                                               static_cast<ERole>(purpose),
                                               &device);

    return device;
}
