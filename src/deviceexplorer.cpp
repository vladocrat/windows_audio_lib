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

#include <mmdeviceapi.h>
#include <unordered_map>

#include "deviceinfo.h"

namespace {

std::wstring getDeviceFriendlyName(IMMDevice* device)
{
    if (!device) return {};

    IPropertyStore* props { nullptr };
    if (FAILED(device->OpenPropertyStore(STGM_READ, &props)) || !props) {
        return {};
    }

    PROPERTYKEY key;
    key.fmtid = { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } };
    key.pid = 14;

    PROPVARIANT propVariant;
    PropVariantInit(&propVariant);

    std::wstring deviceName;

    if (SUCCEEDED(props->GetValue(key, &propVariant)) && propVariant.vt == VT_LPWSTR && propVariant.pwszVal) {
        deviceName = propVariant.pwszVal;
    }

    PropVariantClear(&propVariant);
    props->Release();

    return deviceName;
}

std::wstring getDeviceId(IMMDevice* device)
{
    if (!device) return {};

    LPWSTR id { nullptr };
    if (FAILED(device->GetId(&id)) || !id) {
        return {};
    }

    std::wstring deviceId(id);
    CoTaskMemFree(id);
    return deviceId;
}

UINT getDeviceStateBitFlag(slk::DeviceState state)
{
    static const std::unordered_map<slk::DeviceState, unsigned int> deviceStateMap = {
        {slk::DeviceState::Active, DEVICE_STATE_ACTIVE},
        {slk::DeviceState::Disable, DEVICE_STATE_DISABLED},
        {slk::DeviceState::NotPresent, DEVICE_STATE_NOTPRESENT},
        {slk::DeviceState::Unplugged, DEVICE_STATE_UNPLUGGED},
        {slk::DeviceState::All, DEVICE_STATEMASK_ALL}
    };

    auto it = deviceStateMap.find(state);

    if (it != deviceStateMap.end()) {
        return it->second;
    }

    return { };
}

}

namespace slk
{

struct DeviceExplorer::impl_t
{
    IMMDeviceEnumerator* enumerator { nullptr };
};

DeviceExplorer::DeviceExplorer()
{
    createImpl();

    CoCreateInstance(__uuidof(MMDeviceEnumerator),
                     NULL,
                     CLSCTX_ALL,
                     __uuidof(IMMDeviceEnumerator),
                     reinterpret_cast<void**>(&impl().enumerator));
}

DeviceExplorer::~DeviceExplorer()
{
    if (!impl().enumerator) return;

    impl().enumerator->Release();
}

std::vector<DeviceDescriptor> DeviceExplorer::devices(slk::DeviceType type, slk::DeviceState state) const noexcept
{
    if (!impl().enumerator) return {};

    IMMDeviceCollection* collection { nullptr };
    if (FAILED(impl().enumerator->EnumAudioEndpoints(static_cast<EDataFlow>(type), getDeviceStateBitFlag(state), &collection))) {
        return {};
    }

    if (!collection) return {};

    UINT count { 0 };
    if (FAILED(collection->GetCount(&count))) {
        collection->Release();
        return {};
    }

    std::vector<DeviceDescriptor> result;
    result.reserve(count);

    for (UINT i = 0; i < count; i++) {
        IMMDevice* device { nullptr };
        if (FAILED(collection->Item(i, &device)) || !device) {
            continue;
        }

        DeviceDescriptor desc;
        desc.name = getDeviceFriendlyName(device);
        desc.id = getDeviceId(device);
        desc.type = type;

        device->Release();
        result.push_back(std::move(desc));
    }

    collection->Release();
    return result;
}

DeviceInfo DeviceExplorer::resolveDevice(const DeviceDescriptor& desc) const noexcept
{
    if (!impl().enumerator) return {};

    DeviceInfo info;
    info.friendlyName = desc.name;
    info.deviceId = desc.id;
    info.type = desc.type;

    impl().enumerator->GetDevice(desc.id.c_str(), &info.device);

    return info;
}

DeviceInfo DeviceExplorer::resolveDefaultDevice(DeviceType type, Purpose purpose) const noexcept
{
    if (!impl().enumerator) return {};

    DeviceInfo info;
    info.type = type;

    impl().enumerator->GetDefaultAudioEndpoint(static_cast<EDataFlow>(type), static_cast<ERole>(purpose), &info.device);

    if (info.device) {
        IMMDevice* raw = info.device.Get();
        info.friendlyName = getDeviceFriendlyName(raw);
        info.deviceId = getDeviceId(raw);
    }

    return info;
}

}
