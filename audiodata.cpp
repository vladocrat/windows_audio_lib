#include "audiodata.h"

namespace slk
{

template<class SampleData>
AudioData<SampleData>::AudioData() {}

template<class SampleData>
uint32_t AudioData<SampleData>::bufferSize() const
{
    return _bufferSize;
}

template<class SampleData>
void AudioData<SampleData>::setBufferSize(const uint32_t size)
{
    _bufferSize = size;
}

template<class SampleData>
AudioData<float> AudioData<SampleData>::toFloat()
{
    _data = AudioBuffer<float>::fromWASAPI(_data);
}

}
