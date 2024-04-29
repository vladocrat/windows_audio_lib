#pragma once

#include "device.h"

class PlaybackDevice : public Device
{
public:
    PlaybackDevice();
    virtual ~PlaybackDevice();

    bool initialize() noexcept override;
    bool record() noexcept override;
    bool play() noexcept override;

private:
    DECLARE_PIMPL_EX(PlaybackDevice)
};

