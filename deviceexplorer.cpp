#include "deviceexplorer.h"

#include <mmdeviceapi.h>
#include <unordered_map>

namespace {
std::vector<IMMDevice*> collectionToList(IMMDeviceEnumerator* enumerator, IMMDeviceCollection* collection)
{
    if (!collection) return {};
    
    std::vector<IMMDevice*> ret;
    UINT collectionSize;
    collection->GetCount(&collectionSize);
    ret.reserve(collectionSize);
    
    for (UINT i = 0; i < collectionSize; i++)
    {
        IMMDevice* device { nullptr };
        collection->Item(i, &device);        
        ret.push_back(device);
    }
    
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
	IMMDeviceCollection* devices { nullptr };
    impl().enumerator->EnumAudioEndpoints(static_cast<EDataFlow>(type), getDeviceStateBitFlag(state), &devices);
    
    return collectionToList(impl().enumerator, devices);
}

std::wstring DeviceExplorer::deviceFriendlyName(IMMDevice* device) const noexcept
{
    if (!device) return {};
    
    IPropertyStore* props { nullptr };
    device->OpenPropertyStore(STGM_READ, &props);
    
    PROPVARIANT propVariant;
    PropVariantInit(&propVariant);
    props->GetValue(getKey("PKEY_Device_FriendlyName"), &propVariant);
    
    std::wstring deviceName(propVariant.pwszVal);
    PropVariantClear(&propVariant);
    
    props->Release();
    
    return deviceName;
}

IMMDevice* DeviceExplorer::defaultDevice(slk::DeviceType type, slk::Purpose purpose) const noexcept
{
    IMMDevice* device { nullptr };
    impl().enumerator->GetDefaultAudioEndpoint(static_cast<EDataFlow>(type),
                                               static_cast<ERole>(purpose),
                                               &device);

    return device;
}
