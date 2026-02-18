#include "devicemanager.h"

#include "deviceexplorer.h"
#include "wasapiinputdevice.h"
#include "wasapioutputdevice.h"

namespace slk {

struct DeviceManager::impl_t
{
    DeviceExplorer explorer;
};

DeviceManager::DeviceManager()
{
    createImpl();
}

DeviceManager::~DeviceManager()
{
    
}

std::shared_ptr<Device> DeviceManager::defaultDevice(DeviceType type, Purpose purpose) const noexcept
{
#ifdef WIN32
    auto device = impl().explorer.defaultDevice(type, purpose);

    DeviceInfo info;
    info.device = std::move(device);
    info.friendlyName = impl().explorer.deviceFriendlyName(info.device.Get());
    info.type = type;

    switch (type) {
    case DeviceType::Playback:
        return std::shared_ptr<WASAPIOutputDevice>(new WASAPIOutputDevice(std::move(info)));
    case DeviceType::Record:
        return std::shared_ptr<WASAPIInputDevice>(new WASAPIInputDevice(std::move(info)));
    case DeviceType::All:
        break;
    }
#endif

    return nullptr;
}

std::shared_ptr<Device> DeviceManager::create(DeviceType type) const noexcept
{   
    ///! change to use device by name
    return defaultDevice(type, Purpose::Multimedia);
}

}
