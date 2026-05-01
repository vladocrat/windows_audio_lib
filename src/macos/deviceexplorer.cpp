#include "deviceexplorer.h"

#include <ranges>

#include <CoreAudio/CoreAudio.h>

#include "deviceinfo.h"

namespace
{

std::wstring cfStringToWString(CFStringRef str)
{
    if (!str) {
        return {};
    }

    const CFIndex length = CFStringGetLength(str);

    if (length == 0) {
        return {};
    }

    CFIndex bufSize = 0;
    CFStringGetBytes(str, CFRangeMake(0, length), kCFStringEncodingUTF32LE, 0, false, nullptr, 0, &bufSize);

    if (bufSize == 0) {
        return {};
    }

    std::wstring result(bufSize / sizeof(wchar_t), L'\0');
    CFStringGetBytes(str,
                     CFRangeMake(0, length),
                     kCFStringEncodingUTF32LE,
                     0,
                     false,
                     reinterpret_cast<UInt8*>(result.data()),
                     bufSize,
                     nullptr);
    return result;
}

std::wstring getDeviceName(AudioDeviceID deviceId)
{
    const AudioObjectPropertyAddress addr { kAudioObjectPropertyName,
                                      kAudioObjectPropertyScopeGlobal,
                                      kAudioObjectPropertyElementMain };

    CFStringRef name { nullptr };
    UInt32 size { sizeof(CFStringRef) };
    const auto status = AudioObjectGetPropertyData(deviceId, &addr, 0, nullptr, &size, static_cast<void*>(&name));

    if (status != noErr || !name) {
        return {};
    }

    std::wstring result = cfStringToWString(name);
    CFRelease(name);
    return result;
}

std::wstring getDeviceUID(AudioDeviceID deviceId)
{
    const AudioObjectPropertyAddress addr { kAudioDevicePropertyDeviceUID,
                                      kAudioObjectPropertyScopeGlobal,
                                      kAudioObjectPropertyElementMain };

    CFStringRef uid { nullptr };
    UInt32 size { sizeof(CFStringRef) };
    const auto status = AudioObjectGetPropertyData(deviceId, &addr, 0, nullptr, &size, static_cast<void*>(&uid));

    if (status != noErr || !uid) {
        return {};
    }

    std::wstring result = cfStringToWString(uid);
    CFRelease(uid);
    return result;
}

bool deviceIsAlive(AudioDeviceID deviceId)
{
    const AudioObjectPropertyAddress addr { kAudioDevicePropertyDeviceIsAlive,
                                      kAudioObjectPropertyScopeGlobal,
                                      kAudioObjectPropertyElementMain };

    UInt32 isAlive { 0 };
    UInt32 size { sizeof(isAlive) };
    const auto status = AudioObjectGetPropertyData(deviceId, &addr, 0, nullptr, &size, static_cast<void*>(&isAlive));
    return status == noErr && isAlive;
}

bool deviceHasStreams(AudioDeviceID deviceId, AudioObjectPropertyScope scope)
{
    const AudioObjectPropertyAddress addr { kAudioDevicePropertyStreams, scope, kAudioObjectPropertyElementMain };

    UInt32 dataSize { 0 };
    const auto status = AudioObjectGetPropertyDataSize(deviceId, &addr, 0, nullptr, &dataSize);
    return status == noErr && dataSize > 0;
}

bool deviceMatchesType(AudioDeviceID deviceId, slk::DeviceType type)
{
    if (type == slk::DeviceType::All) {
        return true;
    }
    const auto scope = (type == slk::DeviceType::Playback) ? kAudioObjectPropertyScopeOutput
                                                            : kAudioObjectPropertyScopeInput;
    return deviceHasStreams(deviceId, scope);
}

std::vector<AudioDeviceID> getAllDeviceIds()
{
    const AudioObjectPropertyAddress addr { kAudioHardwarePropertyDevices,
                                      kAudioObjectPropertyScopeGlobal,
                                      kAudioObjectPropertyElementMain };

    UInt32 dataSize { 0 };
    auto status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &addr, 0, nullptr, &dataSize);

    if (status != noErr || dataSize == 0) {
        return {};
    }

    std::vector<AudioDeviceID> deviceIds(dataSize / sizeof(AudioDeviceID));
    status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, nullptr, &dataSize, deviceIds.data());

    if (status != noErr) {
        return {};
    }

    return deviceIds;
}

AudioDeviceID findDeviceByUID(const std::wstring& uid)
{
    // kAudioHardwarePropertyTranslateUIDToDevice was deprecated in macOS 12
    // and is unreliable on modern systems. Enumerate devices and match the
    // UID directly instead.
    if (uid.empty()) {
        return kAudioDeviceUnknown;
    }

    for (const auto id : getAllDeviceIds()) {
        if (getDeviceUID(id) == uid) {
            return id;
        }
    }

    return kAudioDeviceUnknown;
}

AudioDeviceID getDefaultDeviceId(slk::DeviceType type)
{
    const auto selector = (type == slk::DeviceType::Record) ? kAudioHardwarePropertyDefaultInputDevice
                                                             : kAudioHardwarePropertyDefaultOutputDevice;

    const AudioObjectPropertyAddress addr { selector, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain };

    AudioDeviceID deviceId = kAudioDeviceUnknown;
    UInt32 size { sizeof(deviceId) };
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, nullptr, &size, &deviceId);
    return deviceId;
}

} // namespace

namespace slk::macos
{

DeviceExplorer::DeviceExplorer() = default;

std::vector<slk::DeviceDescriptor> DeviceExplorer::devices(slk::DeviceType type, slk::DeviceState state) const noexcept
{
    const auto deviceIds = getAllDeviceIds();

    auto res = deviceIds
               | std::views::filter([state](const auto& id) {
                    return state != slk::DeviceState::Active || deviceIsAlive(id);
               }) | std::views::filter([type](const auto& id) {
                   return deviceMatchesType(id, type);
               }) | std::views::transform([type, state](const auto& id) {
                   slk::DeviceDescriptor desc;
                   desc.name = getDeviceName(id);
                   desc.id = getDeviceUID(id);
                   desc.type = type;
                   desc.state = state;
                   return desc;
               });

    std::vector<slk::DeviceDescriptor> result(res.begin(), res.end());
    return result;
}

slk::DeviceInfo DeviceExplorer::resolveDevice(const slk::DeviceDescriptor& descriptor) const noexcept
{
    const AudioDeviceID deviceId = findDeviceByUID(descriptor.id);

    if (deviceId == kAudioDeviceUnknown) {
        return {};
    }

    slk::DeviceInfo info;
    info.audioDeviceId = deviceId;
    info.friendlyName = getDeviceName(deviceId);
    info.deviceId = descriptor.id;
    info.type = descriptor.type;

    return info;
}

slk::DeviceInfo DeviceExplorer::resolveDefaultDevice(slk::DeviceType type, [[maybe_unused]] slk::Purpose purpose) const noexcept
{
    const AudioDeviceID deviceId = getDefaultDeviceId(type);

    if (deviceId == kAudioDeviceUnknown) {
        return {};
    }

    slk::DeviceInfo info;
    info.audioDeviceId = deviceId;
    info.friendlyName = getDeviceName(deviceId);
    info.deviceId = getDeviceUID(deviceId);
    info.type = type;

    return info;
}

}
