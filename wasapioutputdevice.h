#pragma once

#include <functional>

#include "utils/utils.h"
#include "device.h"
#include "audiobuffer.h"
#include "ringbuffer.h"

namespace slk
{

class WASAPIOutputDevice : public Device
{
public:
    using ProcessCallback = std::function<void(AudioBuffer<float>&)>;

    WASAPIOutputDevice(DeviceInfo&& info);
    WASAPIOutputDevice() = delete;
    ~WASAPIOutputDevice();

    bool open() override;
    bool close() override;
    bool start() override;
    bool stop() override;

    void setSource(RingBuffer<float>& source);
    void setProcessCallback(ProcessCallback callback);
    const AudioFormat& format() const;

private:
    DECLARE_PIMPL_EX(WASAPIOutputDevice)
};

}
