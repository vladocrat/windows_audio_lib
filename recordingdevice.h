#pragma once

#include <audioclient.h>

#include "utils/utils.h"
#include "device.h"

class RecordingDevice : public Device
{
public:
    RecordingDevice();
    ~RecordingDevice();

    [[nodiscard]] bool initialize() noexcept;
    [[nodiscard]] bool record() noexcept;
    [[nodiscard]] bool play() noexcept;

    uint32_t frameSize() const noexcept;
    const BYTE* data() const noexcept;

private:
    DECLARE_PIMPL_EX(RecordingDevice)
};

