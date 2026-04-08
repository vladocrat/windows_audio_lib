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

#include <slk/devicemanager.h>

#include <slk/deviceexplorer.h>
#include "deviceinfo.h"
#include "wasapiinputdevice.h"
#include "wasapioutputdevice.h"

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

std::shared_ptr<InputDevice> DeviceManager::defaultInputDevice(Purpose purpose) const noexcept
{
    auto info = impl().explorer.resolveDefaultDevice(DeviceType::Record, purpose);

    if (!info.device) {
        return nullptr;
    }

    return std::shared_ptr<WASAPIInputDevice>(new WASAPIInputDevice(std::move(info)));
}

std::shared_ptr<OutputDevice> DeviceManager::defaultOutputDevice(Purpose purpose) const noexcept
{
    auto info = impl().explorer.resolveDefaultDevice(DeviceType::Playback, purpose);

    if (!info.device) {
        return nullptr;
    }

    return std::shared_ptr<WASAPIOutputDevice>(new WASAPIOutputDevice(std::move(info)));
}

std::shared_ptr<InputDevice> DeviceManager::createInputDevice(const DeviceDescriptor& desc) const noexcept
{
    auto info = impl().explorer.resolveDevice(desc);

    if (!info.device) {
        return nullptr;
    }

    return std::shared_ptr<WASAPIInputDevice>(new WASAPIInputDevice(std::move(info)));
}

std::shared_ptr<OutputDevice> DeviceManager::createOutputDevice(const DeviceDescriptor& desc) const noexcept
{
    auto info = impl().explorer.resolveDevice(desc);

    if (!info.device) {
        return nullptr;
    }

    return std::shared_ptr<WASAPIOutputDevice>(new WASAPIOutputDevice(std::move(info)));
}

}
