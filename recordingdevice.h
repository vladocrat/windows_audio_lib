#pragma once

#include <audioclient.h>

#include "utils/utils.h"
#include "device.h"

class RecordingDevice : public Device
{
public:
    RecordingDevice();
    virtual ~RecordingDevice();

    [[nodiscard]] bool initialize() noexcept override;
    [[nodiscard]] bool record() noexcept override;
    [[nodiscard]] bool play() noexcept override;

private:
    DECLARE_PIMPL_EX(RecordingDevice)
};

