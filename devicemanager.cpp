#include "devicemanager.h"

#include "deviceexplorer.h"

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

Device* DeviceManager::defaultDevice(DeviceType type, Purpose purpose) const noexcept
{
    IMMDevice* device = impl().explorer.defaultDevice(type, purpose);
    
    auto ret = new Device;
    DeviceInfo info;
    info.device = device;
    info.friendlyName = impl().explorer.deviceFriendlyName(device);
    info.type = type;
    ret->setInfo(info);
    
    ret->activate();

    return ret;
}

Device* DeviceManager::create(DeviceType type) const noexcept
{   
    ///! change to use device by name
    return defaultDevice(type, Purpose::Multimedia);
}

}
