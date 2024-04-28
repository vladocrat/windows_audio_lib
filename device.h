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
    const IAudioClient* client() const noexcept;
    IAudioClient* client() noexcept;
    [[nodiscard]] bool activate(WAVEFORMATEX* waveFormat = nullptr) noexcept;

private:
    DECLARE_PIMPL_EX(Device)
};

