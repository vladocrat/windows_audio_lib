#pragma once

#include "device.h"
#include "general.h"
#include "utils/utils.h"

#include <memory>

namespace slk {

class DeviceManager
{
public:
    DeviceManager();
    ~DeviceManager();
    
    std::optional<std::shared_ptr<Device>> defaultDevice(slk::DeviceType type, slk::Purpose purpose = slk::Purpose::Multimedia) const noexcept;
    std::optional<std::shared_ptr<Device>> create(slk::DeviceType type, Purpose purpose = slk::Purpose::Multimedia, const QString& friendlyName = "") const noexcept;
    
private:
    DECLARE_PIMPL_EX(DeviceManager);
};

}
