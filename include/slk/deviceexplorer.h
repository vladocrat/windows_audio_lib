// windows_audio_lib - Windows audio library
// Copyright (C) 2026  Vladislav Milovanov
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <vector>

#include <slk/general.h>

#include "utils.h"

namespace slk
{

struct DeviceInfo;

class DeviceExplorer final
{
public:
    DeviceExplorer();
    ~DeviceExplorer();

    [[nodiscard]] std::vector<DeviceDescriptor> devices(slk::DeviceType type = slk::DeviceType::All,
                                                        slk::DeviceState state = slk::DeviceState::All) const noexcept;

    [[nodiscard]] DeviceInfo resolveDevice(const DeviceDescriptor& desc) const noexcept;
    [[nodiscard]] DeviceInfo resolveDefaultDevice(slk::DeviceType type, slk::Purpose purpose) const noexcept;

private:
    DECLARE_PIMPL_EX(DeviceExplorer)
    DECLARE_DEFAULT_MOVE(DeviceExplorer)
};

}
