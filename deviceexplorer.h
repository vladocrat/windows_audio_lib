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

	std::vector<IMMDevice*> devices(slk::DeviceType type = slk::DeviceType::All, slk::DeviceState state = slk::DeviceState::All) const noexcept;
    std::wstring deviceFriendlyName(IMMDevice* device) const noexcept;

    friend class slk::DeviceManager;
    
private:
    [[nodiscard]] Microsoft::WRL::ComPtr<IMMDevice> device(const std::wstring& friendlyName, slk::DeviceType type, slk::Purpose purpose = slk::Purpose::Multimedia);
    [[nodiscard]] Microsoft::WRL::ComPtr<IMMDevice> defaultDevice(slk::DeviceType type, slk::Purpose purpose = slk::Purpose::Multimedia) const noexcept;

private:
    DECLARE_PIMPL_EX(DeviceExplorer)
};

}
