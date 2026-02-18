#pragma once

#include <Audioclient.h>

#include "utils/utils.h"

namespace slk
{

class AudioFormat
{
public:
    enum class Type
    {
        PCM,
        FLOAT
    };

    AudioFormat();
    AudioFormat(uint16_t channels, uint32_t sampleRate, uint16_t bitsPerSample, Type audioFormat);
    ~AudioFormat();

    void setFormat(IAudioClient* const client);

    Type type() const;

    void toFloat();

    const WAVEFORMATEX* const format() const;
    uint32_t channels() const;

private:
    DECLARE_PIMPL
};

}
