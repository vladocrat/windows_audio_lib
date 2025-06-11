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

std::optional<std::shared_ptr<Device>> DeviceManager::defaultDevice(DeviceType type, Purpose purpose) const noexcept
{
    IMMDevice* device = impl().explorer.defaultDevice(type, purpose);
    
    auto ret = std::make_shared<Device>();
    DeviceInfo info;
    info.device = device;
    info.friendlyName = impl().explorer.deviceFriendlyName(device);
    info.type = type;
    ret->setInfo(info);
    
    ret->activate();

    return ret;
}

std::optional<std::shared_ptr<Device>> DeviceManager::create(DeviceType type, Purpose purpose, const QString& friendlyName) const noexcept
{
    if (friendlyName.isEmpty()) {
        return defaultDevice(type, purpose);
    }

    const auto devices = impl().explorer.devices(type, DeviceState::Active);
    const auto it = std::find_if(devices.begin(), devices.end(), [&friendlyName](const auto& devInfo) {
        return devInfo.friendlyName == friendlyName.toStdWString();
    });

    if (it == devices.end()) {
        return std::nullopt;
    }

    const auto info = *it;
    auto ret = std::make_shared<Device>();
    ret->setInfo(info);
    ret->activate();

    return ret;
}

}
