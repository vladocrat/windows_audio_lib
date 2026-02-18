#pragma once

#include "utils/utils.h"
#include "device.h"

namespace slk
{

class WASAPIDevice
{
public:
    WASAPIDevice(DeviceInfo&& info);
    virtual ~WASAPIDevice();

    bool open(const DWORD streamFlags);

    const DeviceInfo& info() const;
    IAudioClient* const audioClient() const;
    const AudioBuffer<BYTE>& buffer() const;
    const AudioFormat& format() const;

private:
    DECLARE_PIMPL_EX(WASAPIDevice)

};

}
