#pragma once

namespace slk {

enum class DeviceType
{
    Playback = 0,
    Record,
    All
};

enum class DeviceState
{
    Active,
    Disable,
    NotPresent,
    Unplugged,
    All
};

enum class Purpose
{
    Console = 0,
    Multimedia,
    Communications,
};

}
