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

#include <slk/device.h>

namespace {
std::vector<IMMDevice*> collectionToList(IMMDeviceCollection* collection)
{
    if (!collection) return {};

    std::vector<IMMDevice*> ret;
    UINT collectionSize { 0 };

    if (FAILED(collection->GetCount(&collectionSize))) {
        collection->Release();
        return {};
    }

    ret.reserve(collectionSize);

    for (UINT i = 0; i < collectionSize; i++)
    {
        IMMDevice* device { nullptr };
        if (SUCCEEDED(collection->Item(i, &device)) && device)
            ret.push_back(device);
    }

    collection->Release();
    return ret;
}

const PROPERTYKEY getKey(const std::string& name)
{
    if (name.empty()) return {};
    
    static const std::unordered_map<std::string, GUID> guids = {
        {"PKEY_Device_FriendlyName", { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } }}
    };
    
    PROPERTYKEY key;
    
    if (name == "PKEY_Device_FriendlyName")
    {
        key.pid = 14;
        key.fmtid = guids.at(name);
    }

    PROPVARIANT varName;
    PropVariantInit(&varName);
    return key;
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
    
    if (it != deviceStateMap.end())
    {
        return it->second;
    }
    
    return { };
}

std::vector<IMMDevice*> DeviceExplorer::devices(slk::DeviceType type, slk::DeviceState state) const noexcept
{
    if (!impl().enumerator) return {};

    IMMDeviceCollection* devices { nullptr };
    if (FAILED(impl().enumerator->EnumAudioEndpoints(static_cast<EDataFlow>(type), getDeviceStateBitFlag(state), &devices))) {
        return {};
    }

    return collectionToList(devices);
}

std::wstring DeviceExplorer::deviceFriendlyName(IMMDevice* device) const noexcept
{
    if (!device) return {};

    IPropertyStore* props { nullptr };
    if (FAILED(device->OpenPropertyStore(STGM_READ, &props)) || !props) {
        return {};
    }

    PROPVARIANT propVariant;
    PropVariantInit(&propVariant);

    std::wstring deviceName;

    if (SUCCEEDED(props->GetValue(getKey("PKEY_Device_FriendlyName"), &propVariant)) && propVariant.pwszVal) {
        deviceName = propVariant.pwszVal;
    }

    PropVariantClear(&propVariant);
    props->Release();

    return deviceName;
}

Microsoft::WRL::ComPtr<IMMDevice> DeviceExplorer::device(const std::wstring& friendlyName, slk::DeviceType type, slk::Purpose purpose)
{
    using Microsoft::WRL::ComPtr;

    if (!impl().enumerator) {
        return {};
    }

    ComPtr<IMMDevice> device;
    impl().enumerator->GetDevice(L"", &device);

    return device;
}

Microsoft::WRL::ComPtr<IMMDevice> DeviceExplorer::defaultDevice(slk::DeviceType type, slk::Purpose purpose) const noexcept
{
    using Microsoft::WRL::ComPtr;

    if (!impl().enumerator) {
        return {};
    }

    ComPtr<IMMDevice> device;
    impl().enumerator->GetDefaultAudioEndpoint(static_cast<EDataFlow>(type), static_cast<ERole>(purpose), &device);

    return device;
}

}
