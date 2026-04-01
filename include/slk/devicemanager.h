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

#include <slk/device.h>
#include <slk/general.h>
#include "utils.h"

#include <memory>

namespace slk {

class DeviceManager
{
public:
    DeviceManager();
    ~DeviceManager();
    
    std::shared_ptr<Device> defaultDevice(slk::DeviceType type, slk::Purpose purpose = slk::Purpose::Multimedia) const noexcept;
    std::shared_ptr<Device> create(slk::DeviceType type, const std::string&) const noexcept;
    
private:
    DECLARE_PIMPL_EX(DeviceManager);
};

}
