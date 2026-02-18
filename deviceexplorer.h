#pragma once

#include <wrl/client.h>

#include <vector>
#include <string>

#include "general.h"
#include "utils/utils.h"

struct IMMDevice;

namespace slk {

class DeviceManager;
struct DeviceInfo;

class DeviceExplorer
{
public:
    DeviceExplorer();
    ~DeviceExplorer();

    std::vector<slk::DeviceInfo> devices(slk::DeviceType type = slk::DeviceType::All, slk::DeviceState state = slk::DeviceState::All) const noexcept;
    [[nodiscard]] IMMDevice* defaultDevice(slk::DeviceType type, slk::Purpose purpose = slk::Purpose::Console) const noexcept;
    std::wstring deviceFriendlyName(IMMDevice* device) const noexcept;

    friend class slk::DeviceManager;
    
private:
    std::wstring deviceFriendlyName(IMMDevice* device) const noexcept;
    [[nodiscard]] Microsoft::WRL::ComPtr<IMMDevice> defaultDevice(slk::DeviceType type, slk::Purpose purpose = slk::Purpose::Console) const noexcept;

private:
    DECLARE_PIMPL_EX(DeviceExplorer)
};

}
