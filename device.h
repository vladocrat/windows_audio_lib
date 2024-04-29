#pragma once

#include "utils/utils.h"

#include <audioclient.h>

struct IMMDevice;
struct IAudioClient;

class Device
{
public:
    Device();
    ~Device();

    const IMMDevice* device() const noexcept;
    IMMDevice *getdevice();//! TODO: temporary

    const IAudioClient* client() const noexcept;
    IAudioClient* client() noexcept;

    const WAVEFORMATEX* waveFormat() const noexcept;

    [[nodiscard]] bool activate() noexcept;

private:
    DECLARE_PIMPL_EX(Device)
};

