#pragma once

#include <functional>

#include "utils/utils.h"
#include "device.h"
#include "audiobuffer.h"

namespace slk
{

class WASAPIInputDevice : public Device
{
public:
    using ProcessCallback = std::function<void(AudioBuffer<float>&)>;

    WASAPIInputDevice(DeviceInfo&& info);
    WASAPIInputDevice() = delete;
    ~WASAPIInputDevice();

    bool open() override;
    bool close() override;
    bool start() override;
    bool stop() override;

    void setProcessCallback(ProcessCallback callback);

private:
    DECLARE_PIMPL_EX(WASAPIInputDevice)
};

}
