#pragma once

#include "audiobuffer.h"

namespace slk
{

class AudioFormat;

template<class SampleData>
class AudioData
{
public:
    AudioData();

    uint32_t bufferSize() const;
    void setBufferSize(const uint32_t);

    AudioData<float> toFloat();

private:
    AudioBuffer<SampleData*> _data;
    uint32_t _bufferSize;
    uint64_t _status;
};

}
