#pragma once

#include "utils/utils.h"

class IMMDevice;

class DeviceExplorer
{
public:
    enum class DeviceType
    {
        Playback = 0,
        Record
    };

    enum class Purpose
    {
        Console = 0,
        Multimedia,
        Communications,
    };

    DeviceExplorer();
    ~DeviceExplorer();

    [[nodiscard]] IMMDevice* defaultDevice(DeviceType type, Purpose purpose = DeviceExplorer::Purpose::Console) const noexcept;

private:
    DECLARE_PIMPL_EX(DeviceExplorer)
};

