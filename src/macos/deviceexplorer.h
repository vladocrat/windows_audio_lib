#pragma once

#include <vector>

#include <slk/general.h>

namespace slk
{
struct DeviceInfo;
struct DeviceDescriptor;
}

namespace slk::macos
{

class DeviceExplorer
{
public:
    DeviceExplorer();
    ~DeviceExplorer() = default;

    [[nodiscard]] std::vector<slk::DeviceDescriptor> devices(slk::DeviceType type,
                                                             slk::DeviceState state) const noexcept;

    [[nodiscard]] slk::DeviceInfo resolveDevice(const slk::DeviceDescriptor& desc) const noexcept;
    [[nodiscard]] slk::DeviceInfo resolveDefaultDevice(slk::DeviceType type, slk::Purpose purpose) const noexcept;
};

}
