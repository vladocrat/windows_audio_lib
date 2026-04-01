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

#include "devicemanager.h"

#include "deviceexplorer.h"
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

std::shared_ptr<Device> DeviceManager::defaultDevice(DeviceType type, Purpose purpose) const noexcept
{

    auto device = impl().explorer.defaultDevice(type, purpose);

    DeviceInfo info;
    info.device = std::move(device);
    info.friendlyName = impl().explorer.deviceFriendlyName(info.device.Get());
    info.type = type;

#ifdef WIN32
    switch (type) {
    case DeviceType::Playback:
        return std::shared_ptr<WASAPIOutputDevice>(new WASAPIOutputDevice(std::move(info)));
    case DeviceType::Record:
        return std::shared_ptr<WASAPIInputDevice>(new WASAPIInputDevice(std::move(info)));
    case DeviceType::All:
        break;
    }
#endif

    return nullptr;
}

std::shared_ptr<Device> DeviceManager::create(DeviceType type, const std::string& deviceName) const noexcept
{
    return {};
}

}
