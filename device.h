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

    [[nodiscard]] virtual bool initialize() noexcept = 0;
    [[nodiscard]] virtual bool record() noexcept = 0;
    [[nodiscard]] virtual bool play() noexcept = 0;

    const uint32_t& frameSize() const noexcept;
    const BYTE* data() const noexcept;

    const IMMDevice* device() const noexcept;
    void setDevice(IMMDevice* device) noexcept;

    const IAudioClient* client() const noexcept;
    IAudioClient* client() noexcept;

    const WAVEFORMATEX* waveFormat() const noexcept;

protected:
    uint32_t* refFrameSize() noexcept;
    BYTE** refData() noexcept;
    void newBuffer(size_t size) noexcept;
    [[nodiscard]] bool activate() noexcept;

private:
    DECLARE_PIMPL_EX(Device)
};

