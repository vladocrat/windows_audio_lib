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

#include <slk/deviceexplorer.h>

#include "deviceinfo.h"

#ifdef WIN32
#include "windows/deviceexplorer.h"
#elif defined(__APPLE__)
#include "macos/deviceexplorer.h"
#endif

namespace slk
{

struct DeviceExplorer::impl_t
{
#ifdef WIN32
    windows::DeviceExplorer explorer;
#elif defined(__APPLE__)
    macos::DeviceExplorer explorer;
#endif
};

DeviceExplorer::DeviceExplorer()
{
    createImpl();
}

DeviceExplorer::~DeviceExplorer() = default;

std::vector<DeviceDescriptor> DeviceExplorer::devices([[maybe_unused]] slk::DeviceType type,
                                                      [[maybe_unused]] slk::DeviceState state) const noexcept
{
#if defined(WIN32) || defined(__APPLE__)
    return impl().explorer.devices(type, state);
#else
    return {};
#endif
}

DeviceInfo DeviceExplorer::resolveDevice([[maybe_unused]] const DeviceDescriptor& desc) const noexcept
{
#if defined(WIN32) || defined(__APPLE__)
    return impl().explorer.resolveDevice(desc);
#else
    return {};
#endif
}

DeviceInfo DeviceExplorer::resolveDefaultDevice([[maybe_unused]] DeviceType type,
                                                [[maybe_unused]] Purpose purpose) const noexcept
{
#if defined(WIN32) || defined(__APPLE__)
    return impl().explorer.resolveDefaultDevice(type, purpose);
#else
    return {};
#endif
}

}
