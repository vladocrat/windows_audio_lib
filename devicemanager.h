#pragma once

#include "device.h"
#include "general.h"
#include "utils/utils.h"

namespace slk {

class DeviceManager
{
public:
    DeviceManager();
    ~DeviceManager();
    
    Device* defaultDevice(slk::DeviceType type, slk::Purpose purpose = slk::Purpose::Multimedia) const noexcept;
    Device* create(slk::DeviceType type) const noexcept;
    
private:
    DECLARE_PIMPL_EX(DeviceManager);
};

}
