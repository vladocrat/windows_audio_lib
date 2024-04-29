#include "playbackdevice.h"

struct PlaybackDevice::impl_t
{

};

PlaybackDevice::PlaybackDevice()
{
    createImpl();
}

PlaybackDevice::~PlaybackDevice()
{

}

bool PlaybackDevice::initialize() noexcept
{
    return 0;
}

bool PlaybackDevice::record() noexcept
{
    return 0;
}

bool PlaybackDevice::play() noexcept
{
    return 0;
}
